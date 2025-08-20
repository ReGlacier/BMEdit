#include <Editor/TextureProcessor.h>
#include <QImage>       // for PNG/JPG exporters
#include <QImageWriter> // for BMP exporter
#include <QBuffer>      // for in-memory exporters
#include <squish.h>     // for DXT1/DXT3 compression/decompression/import/export
#include <cmath>        // std::pow
#include <map>


namespace editor
{
	std::unique_ptr<std::uint8_t[]> TextureProcessor::decompressRGBA(const gamelib::tex::TEXEntry& textureEntry, uint16_t& realWidth, uint16_t& realHeight, std::size_t mipLevel)
	{
		using EntryType = gamelib::tex::TEXEntryType;

		realWidth = static_cast<uint16_t>(textureEntry.m_width / std::pow(2, mipLevel));
		realHeight = static_cast<uint16_t>(textureEntry.m_height / std::pow(2, mipLevel));

		if (mipLevel > textureEntry.m_numOfMipMaps)
		{
			assert(false && "Bad mip");
			return nullptr;
		}

		if (auto format = textureEntry.m_type1; format == EntryType::ET_BITMAP_32)
		{
			// RGBA: just copy into dest memory and return new buffer
			auto result = std::make_unique<std::uint8_t[]>(realWidth * realHeight * 4);
			std::memcpy(result.get(), textureEntry.m_mipLevels.at(mipLevel).m_buffer.get(), realWidth * realHeight * 4);
			return result;
		}
		else if (format == EntryType::ET_BITMAP_DXT1 || format == EntryType::ET_BITMAP_DXT3)
		{
			// DXT1, DXT3: Process via squish
			int flags = 0;

			flags |= (format == EntryType::ET_BITMAP_DXT1 ? squish::kDxt1 : 0);
			flags |= (format == EntryType::ET_BITMAP_DXT3 ? squish::kDxt3 : 0);
			auto result = std::make_unique<uint8_t[]>(static_cast<int>(realWidth) * static_cast<int>(realHeight) * 4);
			squish::DecompressImage(result.get(), realWidth, realHeight, textureEntry.m_mipLevels.at(mipLevel).m_buffer.get(), flags);

			return result;
		}
		else if (format == EntryType::ET_BITMAP_PAL)
		{
			// PAL: This format based on palette. Generally we need to iterate over each "src pixel" and find color in palette (just jmp)
			if (!textureEntry.m_palPalette.has_value())
			{
				assert(false && "Bad PAL palette!");
				return nullptr;
			}

			const auto& palette = textureEntry.m_palPalette.value();
			const int totalSrcPixels = static_cast<int>(realWidth) * static_cast<int>(realHeight);
			const int totalDstPixels = totalSrcPixels * 4;
			auto result = std::make_unique<uint8_t[]>(totalDstPixels);

			for (int i = 0; i < totalSrcPixels; ++i)
			{
				*reinterpret_cast<uint32_t*>(&result.get()[i * 4]) = *reinterpret_cast<uint32_t*>(&palette.m_data.get()[4 * textureEntry.m_mipLevels.at(mipLevel).m_buffer.get()[i]]);
			}

			return result;
		}
		else if (format == EntryType::ET_BITMAP_U8V8)
		{
			// Reversed by DronCode (ZBitmapU8V8::sub_43E8E0)
			const int totalSrcPixels = static_cast<int>(realWidth) * static_cast<int>(realHeight);
			const int totalPixels = totalSrcPixels * 4;
			auto result = std::make_unique<uint8_t[]>(totalPixels);

			for (int i = 0; i < totalSrcPixels; i++)
			{
				const uint16_t RG = *reinterpret_cast<uint16_t*>(&textureEntry.m_mipLevels.at(mipLevel).m_buffer.get()[i * 2 + 0]);
				const uint32_t color = (RG << 8) | 0xFF0000FF;
				*reinterpret_cast<uint32_t*>(&result.get()[i * 4 + 0]) = color;
			}

			return result;
		}
		else if (format == EntryType::ET_BITMAP_I8)
		{
			// Reversed by DronCode (ZBitmapI8::sub_43EA90)
			const int totalSrcPixels = static_cast<int>(realWidth) * static_cast<int>(realHeight);
			const int totalPixels = totalSrcPixels * 4;
			auto result = std::make_unique<uint8_t[]>(totalPixels);

			for (int i = 0; i < totalSrcPixels; i++)
			{
				*reinterpret_cast<uint32_t*>(&result.get()[i * 4]) = 0x1010101u * static_cast<uint32_t>(textureEntry.m_mipLevels.at(mipLevel).m_buffer.get()[i]);
			}

			return result;
		}

		//PALO: Obsolete thing. See ZBitmapPalOpac::sub_43E720 for details. To reverse when we will find at least 1 usage.

		assert(false && "Unsupported routine!");
		return nullptr;
	}

	bool TextureProcessor::exportTEXEntryAsPNG(const gamelib::tex::TEXEntry& texEntry, const std::filesystem::path& filePath, std::size_t mipLevel)
	{
		uint16_t w, h;
		auto buffer = TextureProcessor::decompressRGBA(texEntry, w, h, mipLevel);

		if (!buffer)
		{
			return false;
		}

		QImage image(buffer.get(), w, h, QImage::Format::Format_RGBA8888);
		image.save(QString::fromStdString(filePath.string()), "PNG", 100);

		return true;
	}

	bool TextureProcessor::importTextureToEntry(gamelib::tex::TEXEntry& targetTexture, const QString& texturePath, const QString& textureName, gamelib::tex::TEXEntryType targetFormat, uint8_t mipLevelsNr)
	{
		// Read texture as RGBA, convert to final format, generate MIP levels and override TEXEntry, PALPalette and other stuff
		QImage sourceImage { texturePath };
		if (sourceImage.isNull())
			return false;

		// Convert to RGBA8888
		sourceImage.convertTo(QImage::Format::Format_RGBA8888);

		auto isPow2 = [](uint32_t v) -> bool
		{
			return (v & (v - 1)) == 0;
		};

		// Check that source image is a pow of 2
		if (!isPow2(sourceImage.width()) || !isPow2(sourceImage.height()))
		{
			auto getPreviousPowOf2 = [](uint32_t current) -> uint32_t
			{
#ifdef Q_CC_MSVC
				return 1u << ((sizeof(current) * 8 - 1) - _lzcnt_u32(current));
#else
				return 1u << ((sizeof(current) * 8 - 1) - __builtin_clz(current));
#endif
			};

			uint32_t w = sourceImage.width();
			uint32_t h = sourceImage.height();

			if (!isPow2(sourceImage.width()))
			{
				w = getPreviousPowOf2(w);
			}

			if (!isPow2(sourceImage.height()))
			{
				h = getPreviousPowOf2(h);
			}

			auto newImage = sourceImage.scaled(static_cast<int>(w), static_cast<int>(h), Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::FastTransformation);
			sourceImage = std::move(newImage);
		}

		if (targetFormat == gamelib::tex::TEXEntryType::ET_BITMAP_PAL || targetFormat == gamelib::tex::TEXEntryType::ET_BITMAP_PAL_OPAC)
		{
			// Because in PAL and PAL_OPAC we can't have more than 1 palette
			mipLevelsNr = 1;
		}

		std::vector<QImage> mipLevels;
		mipLevels.resize(mipLevelsNr);

		mipLevels[0] = std::move(sourceImage);
		for (int mipLevel = 1; mipLevel < mipLevelsNr; mipLevel++)
		{
			auto currentW = static_cast<uint16_t>(mipLevels[0].width() / std::pow(2, mipLevel));
			auto currentH = static_cast<uint16_t>(mipLevels[0].height() / std::pow(2, mipLevel));

			mipLevels[mipLevel] = mipLevels[0].scaled(currentW, currentH, Qt::AspectRatioMode::IgnoreAspectRatio, Qt::TransformationMode::FastTransformation);
		}

		targetTexture.m_mipLevels.resize(mipLevelsNr);

		for (int i = 0; i < mipLevelsNr; i++)
		{
			const auto& sourceMIP = mipLevels[i];
			auto& textureMIP = targetTexture.m_mipLevels[i];

			switch (targetFormat)
			{
				case gamelib::tex::TEXEntryType::ET_BITMAP_I8:
					// Use simple grayscale and divide result by 0x10
					{
					    std::unique_ptr<uint8_t[]> pixelBuffer = std::make_unique<uint8_t[]>(sourceMIP.width() * sourceMIP.height());

					    for (int pixelIndex = 0; pixelIndex < sourceMIP.width() * sourceMIP.height(); pixelIndex++)
					    {
						    const uint8_t r = *reinterpret_cast<const uint8_t*>(&sourceMIP.bits()[pixelIndex * 4 + 0]);
						    const uint8_t g = *reinterpret_cast<const uint8_t*>(&sourceMIP.bits()[pixelIndex * 4 + 1]);
						    const uint8_t b = *reinterpret_cast<const uint8_t*>(&sourceMIP.bits()[pixelIndex * 4 + 2]);
						    // Do we need to use alpha too? Idk
						    const uint32_t mid = ((r + g + b) / 3) / 0x10;
						    *reinterpret_cast<uint8_t*>(&pixelBuffer[pixelIndex]) = static_cast<uint8_t>(mid);
					    }

					    textureMIP.m_mipLevelSize = sourceMIP.width() * sourceMIP.height();
					    textureMIP.m_buffer = std::move(pixelBuffer);
					}
					break;
			    case gamelib::tex::TEXEntryType::ET_BITMAP_U8V8:
				    // Just use only red and green pixels
					{
					    std::unique_ptr<uint8_t[]> pixelBuffer = std::make_unique<uint8_t[]>(sourceMIP.width() * sourceMIP.height() * 2);

					    for (int pixelIndex = 0; pixelIndex < sourceMIP.width() * sourceMIP.height(); pixelIndex++)
					    {
						    *reinterpret_cast<uint16_t*>(&pixelBuffer[pixelIndex * 2]) =
						        (*reinterpret_cast<const uint8_t*>(&sourceMIP.bits()[pixelIndex * 4 + 0])) << 8 |
						         *reinterpret_cast<const uint8_t*>(&sourceMIP.bits()[pixelIndex * 4 + 1]);
					    }

					    textureMIP.m_mipLevelSize = sourceMIP.width() * sourceMIP.height();
					    textureMIP.m_buffer = std::move(pixelBuffer);
					}
				    break;
				case gamelib::tex::TEXEntryType::ET_BITMAP_PAL:
					// "EZ"
				    {
					    /**
					     * @note: In this format we using dithering and this need to have only 1 MIP level.
					     *       We can't have more than 1 MIP level because we can't guarantee that for other MIP levels our palette will be OK.
					     *       So, we will break loop after first entry done
					     *
					     * @note: Ok, it really stupid solution, but in Format_Indexed8 format itself has max 256 palette so we can convert image to this pixel format,
					     *        write into memory and collect palette itself.
					     *
					     * @note: It's really weird results, quality bad, need to investigate it later. Workaround: use RGBA!
					     */
					    QImage bitmap = sourceMIP.convertToFormat(
					        QImage::Format::Format_Indexed8,
					        Qt::ImageConversionFlag::ThresholdDither | Qt::ImageConversionFlag::ThresholdAlphaDither | Qt::ImageConversionFlag::AvoidDither
						);
					    std::unique_ptr<std::uint8_t[]> pixelBuffer = std::make_unique<std::uint8_t[]>(bitmap.width() * bitmap.height());

					    // Ok, now it loaded, process pixels
					    std::uint8_t currentIndex = 0;
					    std::map<std::uint32_t, std::uint8_t> colorToIndex;

					    std::uint32_t pixelIndex = 0;
					    for (int y = 0; y < bitmap.height(); y++)
					    {
						    for (int x = 0; x < bitmap.width(); x++)
						    {
							    QRgb pixel = bitmap.pixel(x, y);

							    if (colorToIndex.contains(pixel))
							    {
								    pixelBuffer[pixelIndex] = colorToIndex[pixel];
							    }
							    else
							    {
								    currentIndex++;
								    colorToIndex[pixel] = currentIndex;
								    pixelBuffer[pixelIndex] = currentIndex;
							    }

							    pixelIndex++;
						    }
					    }

					    if (colorToIndex.size() > 256)
					    {
						    // Too big palette!
						    return false;
					    }

					    // Ok, we have a palette in colorToIndex and now we need to save only values into our buffer
					    gamelib::tex::PALPalette palette;
					    palette.m_size = colorToIndex.size();
					    palette.m_data = std::make_unique<std::uint8_t[]>(palette.m_size * 4);

					    std::uint32_t colorIndex = 0;
					    for (const auto& [color, _gIndex] : colorToIndex)
					    {
						    *reinterpret_cast<std::uint32_t*>(&palette.m_data.get()[colorIndex * 4]) = color;
						    ++colorIndex;
					    }

					    // Save palette
					    targetTexture.m_palPalette.emplace(std::move(palette));

					    // Now save pixel buffer
					    textureMIP.m_buffer = std::move(pixelBuffer);
						textureMIP.m_mipLevelSize = bitmap.width() * bitmap.height();
					}
					break;
				case gamelib::tex::TEXEntryType::ET_BITMAP_32:
					// Copy data
					{
						uint32_t w = sourceMIP.width();
						uint32_t h = sourceMIP.height();
						textureMIP.m_mipLevelSize = w * h * 4;
						textureMIP.m_buffer = std::make_unique<uint8_t[]>(textureMIP.m_mipLevelSize);
						std::memcpy(textureMIP.m_buffer.get(), sourceMIP.bits(), textureMIP.m_mipLevelSize);
					}
					break;
				case gamelib::tex::TEXEntryType::ET_BITMAP_DXT1:
				case gamelib::tex::TEXEntryType::ET_BITMAP_DXT3:
					// Use libsquish to create DXT1/DXT3 buffers
				    {
					    uint32_t w = sourceMIP.width();
					    uint32_t h = sourceMIP.height();
					    int flags = 0;

					    flags |= (targetFormat == gamelib::tex::TEXEntryType::ET_BITMAP_DXT1 ? squish::kDxt1 : 0);
					    flags |= (targetFormat == gamelib::tex::TEXEntryType::ET_BITMAP_DXT3 ? squish::kDxt3 : 0);

					    // Calculate & allocate space
					    textureMIP.m_mipLevelSize = squish::GetStorageRequirements(static_cast<int>(w), static_cast<int>(h), flags);
					    textureMIP.m_buffer = std::make_unique<uint8_t[]>(textureMIP.m_mipLevelSize);

					    // Compress image directly into pre-allocated space
					    squish::CompressImage(
					        reinterpret_cast<const squish::u8*>(sourceMIP.bits()),
					        static_cast<int>(w), static_cast<int>(h),
					        (void*)textureMIP.m_buffer.get(),
					        flags,
					        nullptr);
					}
					break;
				default:
					return false;
			}
		}

		// Update internals
		targetTexture.m_width = static_cast<uint16_t>(mipLevels[0].width());
		targetTexture.m_height = static_cast<uint16_t>(mipLevels[0].height());
		targetTexture.m_fileName = textureName.isEmpty() ? std::nullopt : std::make_optional<std::string>(textureName.toStdString());
		targetTexture.m_type1 = targetTexture.m_type2 = targetFormat;
		targetTexture.m_numOfMipMaps = static_cast<uint32_t>(targetTexture.m_mipLevels.size());
		targetTexture.m_fileSize = targetTexture.calculateSize(); // Update file size after all manipulations

		return true;
	}
}