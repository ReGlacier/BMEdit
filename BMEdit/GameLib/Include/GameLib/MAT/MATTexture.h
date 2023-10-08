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
	class MATTexture
	{
	public:
		MATTexture(std::string name, bool bEnabled, uint32_t textureId, MATTilingMode tilingU, MATTilingMode tilingV);

		static MATTexture makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bool isEnabled() const;
		[[nodiscard]] uint32_t getTextureId() const;
		[[nodiscard]] MATTilingMode getTilingU() const;
		[[nodiscard]] MATTilingMode getTilingV() const;

	private:
		std::string m_name {};
		std::uint32_t m_iTextureId { 0u };
		bool m_bEnabled { false };
		MATTilingMode m_tileU { MATTilingMode::TM_NONE };
		MATTilingMode m_tileV { MATTilingMode::TM_NONE };
	};
}