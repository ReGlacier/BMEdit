#pragma once

#include <GameLib/MAT/MATTilingMode.h>

#include <cstdint>
#include <string>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	enum PresentedTextureSource : uint8_t
	{
		PTS_NOTHING,
		PTS_TEXTURE_ID,
		PTS_TEXTURE_PATH,
		PTS_TEXTURE_ID_AND_PATH
	};

	class MATTexture
	{
	public:
		MATTexture(std::string name, bool bEnabled, uint32_t textureId, std::string path, MATTilingMode tilingU, MATTilingMode tilingV, MATTilingMode tilingW);

		static MATTexture makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bool isEnabled() const;
		[[nodiscard]] uint32_t getTextureId() const;
		[[nodiscard]] const std::string& getTexturePath() const;
		[[nodiscard]] MATTilingMode getTilingU() const;
		[[nodiscard]] MATTilingMode getTilingV() const;
		[[nodiscard]] MATTilingMode getTilingW() const;
		[[nodiscard]] PresentedTextureSource getPresentedTextureSources() const;

	private:
		std::string m_name {};
		std::string m_texturePath {};
		std::uint32_t m_iTextureId { 0u };
		bool m_bEnabled { false };
		MATTilingMode m_tileU { MATTilingMode::TM_NONE };
		MATTilingMode m_tileV { MATTilingMode::TM_NONE };
		MATTilingMode m_tileW { MATTilingMode::TM_NONE };
	};
}