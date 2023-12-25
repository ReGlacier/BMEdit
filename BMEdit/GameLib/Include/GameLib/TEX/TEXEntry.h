#pragma once

#include <GameLib/TEX/TEXTypes.h>
#include <GameLib/TEX/TEXEntryType.h>

#include <optional>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>


namespace ZBio::ZBinaryWriter
{
	class BinaryWriter;
}

namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::tex
{
	struct PALPalette
	{
		uint32_t m_size { 0 };
		std::unique_ptr<std::uint8_t[]> m_data { nullptr };

	public:
		static void deserialize(PALPalette &palette, ZBio::ZBinaryReader::BinaryReader *binaryReader);
		static void serialize(const PALPalette &palette, ZBio::ZBinaryWriter::BinaryWriter *writerStream);
	};

	struct TEXMipLevel
	{
		uint32_t m_mipLevelSize { 0u };
		std::unique_ptr<std::uint8_t[]> m_buffer { nullptr };

	public:
		static void deserialize(TEXMipLevel &mipLevel, ZBio::ZBinaryReader::BinaryReader *binaryReader);
		static void serialize(const TEXMipLevel &mipLevel, ZBio::ZBinaryWriter::BinaryWriter *writerStream);
	};

	struct TEXCubeMaps
	{
		uint32_t m_count { 0u };
		std::vector<uint32_t> m_textureIndices {};

	public:
		static void deserialize(TEXCubeMaps &cubeMaps, ZBio::ZBinaryReader::BinaryReader *binaryReader);
		static void serialize(const TEXCubeMaps &cubeMaps, ZBio::ZBinaryWriter::BinaryWriter *writerStream);
	};

	using TEXEntryFlag = uint32_t;

	enum TEXEntryFlags : TEXEntryFlag
	{
		  TEX_EF_IsGUI     = 0x2
		, TEX_EF_Unk40     = 0x40
		, TEX_EF_IsCubeMap = 0x400
		, TEX_EF_Unk1000   = 0x1000
	};

	struct TEXEntry
	{
		TEXEntry() = default;

		static void deserialize(TEXEntry &header, ZBio::ZBinaryReader::BinaryReader *binaryReader);
		static void serialize(const TEXEntry &header, ZBio::ZBinaryWriter::BinaryWriter *writerStream, uint32_t& entryOffset, uint32_t& cubeMapsDescOffset);

		[[nodiscard]] uint32_t calculateSize() const;

	public:
		uint32_t m_offset { 0u };
		uint32_t m_fileSize { 0u };
		TEXEntryType m_type1 {};
		TEXEntryType m_type2 {};
		uint32_t m_index { 0u };
		uint16_t m_width { 0u };
		uint16_t m_height { 0u };
		uint32_t m_numOfMipMaps { 0u };
		TEXEntryFlag m_flags { 0u };
		uint32_t m_unk2 { 0u };
		uint32_t m_unk3 { 0u };
		std::optional<std::string> m_fileName {};
		std::vector<TEXMipLevel> m_mipLevels {};
		std::optional<PALPalette> m_palPalette;
		TEXCubeMaps m_cubeMaps;
	};
}