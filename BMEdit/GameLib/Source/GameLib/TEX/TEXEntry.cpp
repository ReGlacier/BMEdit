#include <GameLib/TEX/TEXEntry.h>
#include <ZBinaryWriter.hpp>
#include <ZBinaryReader.hpp>
#include <cassert>


namespace gamelib::tex
{
	template <size_t Align, uint8_t Pad = 0x0>
	static void alignStream(ZBio::ZBinaryWriter::BinaryWriter *writerStream)
	{
		const size_t remainder = writerStream->tell() % Align;
		const size_t paddingSize = remainder > 0 ? Align - remainder : 0;

		for (int i = 0; i < paddingSize; i++)
		{
			writerStream->write<uint8_t, ZBio::Endianness::LE>(Pad);
		}
	}

	void PALPalette::deserialize(PALPalette &palette, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		palette.m_size = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		palette.m_data = std::make_unique<uint8_t[]>(palette.m_size * 4);
		binaryReader->read<uint8_t, ZBio::Endianness::LE>(palette.m_data.get(), static_cast<int64_t>(palette.m_size) * 4);
	}

	void PALPalette::serialize(const PALPalette &palette, ZBio::ZBinaryWriter::BinaryWriter *writerStream)
	{
		writerStream->write<uint32_t, ZBio::Endianness::LE>(palette.m_size);
		writerStream->write<uint8_t, ZBio::Endianness::LE>(palette.m_data.get(), static_cast<int64_t>(palette.m_size) * 4);
	}

	void TEXMipLevel::deserialize(TEXMipLevel &mipLevel, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		mipLevel.m_mipLevelSize = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		mipLevel.m_buffer = std::make_unique<std::uint8_t[]>(mipLevel.m_mipLevelSize);
		binaryReader->read<uint8_t, ZBio::Endianness::LE>(mipLevel.m_buffer.get(), static_cast<int64_t>(mipLevel.m_mipLevelSize));
	}

	void TEXMipLevel::serialize(const TEXMipLevel &mipLevel, ZBio::ZBinaryWriter::BinaryWriter *writerStream)
	{
		writerStream->write<uint32_t, ZBio::Endianness::LE>(mipLevel.m_mipLevelSize);
		writerStream->write<uint8_t, ZBio::Endianness::LE>(mipLevel.m_buffer.get(), static_cast<int64_t>(mipLevel.m_mipLevelSize));
	}

	void TEXCubeMaps::deserialize(TEXCubeMaps &cubeMaps, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		cubeMaps.m_count = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		cubeMaps.m_textureIndices.resize(cubeMaps.m_count);
		binaryReader->read<uint32_t, ZBio::Endianness::LE>(cubeMaps.m_textureIndices.data(), cubeMaps.m_count);
	}

	void TEXCubeMaps::serialize(const TEXCubeMaps &cubeMaps, ZBio::ZBinaryWriter::BinaryWriter *writerStream)
	{
		writerStream->write<uint32_t, ZBio::Endianness::LE>(static_cast<uint32_t>(cubeMaps.m_textureIndices.size()));
		writerStream->write<uint32_t, ZBio::Endianness::LE>(cubeMaps.m_textureIndices.data(), static_cast<int64_t>(cubeMaps.m_textureIndices.size()));
	}

	void TEXEntry::deserialize(TEXEntry &entry, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		entry.m_fileSize = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		auto checkFormat = [](uint32_t format) -> bool {
			if (auto type = static_cast<TEXEntryType>(format);
			    type != TEXEntryType::ET_BITMAP_I8 && type != TEXEntryType::ET_BITMAP_EMBM &&
			    type != TEXEntryType::ET_BITMAP_DOT3 && type != TEXEntryType::ET_BITMAP_CUBE && type != TEXEntryType::ET_BITMAP_DMAP &&
			    type != TEXEntryType::ET_BITMAP_PAL && type != TEXEntryType::ET_BITMAP_PAL_OPAC &&
			    type != TEXEntryType::ET_BITMAP_32 && type != TEXEntryType::ET_BITMAP_U8V8 &&
			    type != TEXEntryType::ET_BITMAP_DXT1 && type != TEXEntryType::ET_BITMAP_DXT3
			) {
				return false;
			}

			return true;
		};

		{
			if (auto type = binaryReader->read<uint32_t, ZBio::Endianness::LE>(); !checkFormat(type))
			{
				assert(false && "Bad format!");
				return;
			}
			else
			{
				entry.m_type1 = static_cast<TEXEntryType>(type);
			}
		}

		{
			if (auto type = binaryReader->read<uint32_t, ZBio::Endianness::LE>(); !checkFormat(type))
			{
				assert(false && "Bad format [2]!");
				return;
			}
			else
			{
				entry.m_type2 = static_cast<TEXEntryType>(type);
			}
		}

		entry.m_index = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.m_height = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		entry.m_width = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		entry.m_numOfMipMaps = binaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// https://github.com/pavledev/GlacierTEXEditor/blob/master/GlacierTEXEditor/Form1.cs#L296

		entry.m_flags = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.m_unk2  = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entry.m_unk3  = binaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// Allocate mip levels
		entry.m_mipLevels.resize(entry.m_numOfMipMaps);

		// Read filename
		if (auto fileName = binaryReader->readCString(); !fileName.empty())
		{
			entry.m_fileName = std::move(fileName);
		}
		else
		{
			entry.m_fileName = std::nullopt;
		}

		// Read mip levels
		for (auto& mipEntry : entry.m_mipLevels)
		{
			TEXMipLevel::deserialize(mipEntry, binaryReader);
		}

		// If PALN - need to save palette. TODO: Maybe for PALO need save this too?
		if (entry.m_type1 == TEXEntryType::ET_BITMAP_PAL)
		{
			PALPalette& palette = entry.m_palPalette.emplace();
			PALPalette::deserialize(palette, binaryReader);
		}
	}

	void TEXEntry::serialize(const TEXEntry &entry, ZBio::ZBinaryWriter::BinaryWriter *writerStream, uint32_t& entryOffset, uint32_t& cubeMapsDescOffset)
	{
		// Save stream pos
		entryOffset = writerStream->tell();

		// Write data
		writerStream->write<uint32_t, ZBio::Endianness::LE>(entry.m_fileSize);
		writerStream->write<uint32_t, ZBio::Endianness::LE>(static_cast<uint32_t>(entry.m_type1));
		writerStream->write<uint32_t, ZBio::Endianness::LE>(static_cast<uint32_t>(entry.m_type2));
		writerStream->write<uint32_t, ZBio::Endianness::LE>(entry.m_index);
		writerStream->write<uint16_t, ZBio::Endianness::LE>(entry.m_height);
		writerStream->write<uint16_t, ZBio::Endianness::LE>(entry.m_width);
		writerStream->write<uint32_t, ZBio::Endianness::LE>(static_cast<uint32_t>(entry.m_mipLevels.size()));
		writerStream->write<uint32_t, ZBio::Endianness::LE>(entry.m_flags);
		writerStream->write<uint32_t, ZBio::Endianness::LE>(entry.m_unk2);
		writerStream->write<uint32_t, ZBio::Endianness::LE>(entry.m_unk3);
		if (entry.m_fileName.has_value() && !entry.m_fileName.value().empty())
		{
			writerStream->writeCString<ZBio::Endianness::BE>(entry.m_fileName.value());
		}
		else
		{
			writerStream->write<std::uint8_t, ZBio::Endianness::LE>(0u);
		}

		// Write MIP levels
		for (const auto& mipEntry : entry.m_mipLevels)
		{
			TEXMipLevel::serialize(mipEntry, writerStream);
		}

		if (entry.m_type1 == TEXEntryType::ET_BITMAP_PAL) //NOTE: Check for PALO (Opac) here, or remove support of that texture type
		{
			// Write palette
			assert(entry.m_palPalette.has_value());
			PALPalette::serialize(entry.m_palPalette.value(), writerStream);
		}

		alignStream<0x10, 0x00>(writerStream);

		if (!entry.m_cubeMaps.m_textureIndices.empty())
		{
			cubeMapsDescOffset = writerStream->tell();

			TEXCubeMaps::serialize(entry.m_cubeMaps, writerStream);
			alignStream<0x10, 0x00>(writerStream);
		}
		else
		{
			cubeMapsDescOffset = 0;
		}
	}

	uint32_t TEXEntry::calculateSize() const
	{
		/**
		 * @source https://github.com/pavledev/GlacierTEXEditor/blob/master/GlacierTEXEditor/Form1.cs#L852
		 */
		const std::string kEmptyStr {};
		uint32_t fileSize = sizeof(m_type1) + sizeof(m_type1) +
		    sizeof(uint32_t) +
		    sizeof(uint16_t) + sizeof(uint16_t) +
		    sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) +
		    (m_fileName.value_or(kEmptyStr).length()) + 1;

		for (const auto& mipLevel : m_mipLevels)
		{
			fileSize += mipLevel.m_mipLevelSize;
		}

		if (m_type1 == TEXEntryType::ET_BITMAP_PAL)
		{
			fileSize += m_palPalette.value().m_size * 4;
		}

		if (!m_cubeMaps.m_textureIndices.empty())
		{
			fileSize += sizeof(uint32_t) + m_cubeMaps.m_textureIndices.size();
		}

		return fileSize;
	}
}
