#pragma once

#include <GameLib/TEX/TEXEntry.h>
#include <GameLib/Span.h>
#include <filesystem>
#include <QString>
#include <memory>


namespace editor
{
	class TextureProcessor
	{
	public:
		// Compress/decompress

		/**
		 * @fn decompressRGBA
		 * @brief decompress TEXEntry into RGBA buffer from various formats (RGBA,U8V8,I8,DXT1,DXT3)
		 * @param texEntry - TEX file entry to decompress
		 * @param realWidth - [out] calculated width of texture (by mip level)
		 * @param realHeight - [out] calculated height of texture (by mip level)
		 * @param mipLevel - mip level, pass 0 when no mip levels presented
		 * @return valid pointer to memory of size W*H*4 with pixels, or nullptr on error occurred
		 */
		static std::unique_ptr<std::uint8_t[]> decompressRGBA(const gamelib::tex::TEXEntry& texEntry, uint16_t& realWidth, uint16_t& realHeight, std::size_t mipLevel = 0);

		// Import/Export
		static bool exportTEXEntryAsPNG(const gamelib::tex::TEXEntry& texEntry, const std::filesystem::path& filePath, std::size_t mipLevel = 0);
		static bool importTextureToEntry(gamelib::tex::TEXEntry& texEntry, const QString& texturePath, const QString& textureName, gamelib::tex::TEXEntryType targetFormat, uint8_t mipLevelsCount);
	};
}