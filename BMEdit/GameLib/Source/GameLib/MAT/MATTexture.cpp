#include <GameLib/MAT/MATTexture.h>
#include <GameLib/MAT/MATEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATTexture::MATTexture(std::string name, bool bEnabled, uint32_t textureId, MATTilingMode tilingU, MATTilingMode tilingV)
		: m_name(std::move(name)), m_bEnabled(bEnabled), m_iTextureId(textureId), m_tileU(tilingU), m_tileV(tilingV)
	{
	}

	MATTexture MATTexture::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name{};
		bool bEnabled{false};
		uint32_t textureId {0};
		MATTilingMode tileU { MATTilingMode::TM_NONE }, tileV { MATTilingMode::TM_NONE };

		for (int i = 0; i < propertiesCount; i++)
		{
			MATPropertyEntry entry;
			MATPropertyEntry::deserialize(entry, binaryReader);

			if (auto kind = entry.kind; kind == MATPropertyKind::PK_NAME)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				name = binaryReader->readCString();
			}
			else if (kind == MATPropertyKind::PK_ENABLE)
			{
				bEnabled = static_cast<bool>(entry.reference);
			}
			else if (kind == MATPropertyKind::PK_TEXTURE_ID)
			{
				textureId = entry.reference;
			}
			else if (kind == MATPropertyKind::PK_TILINIG_U || kind == MATPropertyKind::PK_TILINIG_V)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				std::string temp = binaryReader->readCString();
				MATTilingMode tempMode = MATTilingMode::TM_NONE;

				if (temp == "NONE" || temp.empty())
				{
					tempMode = MATTilingMode::TM_NONE;
				}
				else if (temp == "TILED")
				{
					tempMode = MATTilingMode::TM_TILED;
				}
				else
				{
					assert(false && "Unsupported mode!");
					continue;
				}

				if (kind == MATPropertyKind::PK_TILINIG_U) tileU = tempMode;
				if (kind == MATPropertyKind::PK_TILINIG_V) tileV = tempMode;
			}
		}

		return MATTexture(std::move(name), bEnabled, textureId, tileU, tileV);
	}

	const std::string& MATTexture::getName() const
	{
		return m_name;
	}

	uint32_t MATTexture::getTextureId() const
	{
		return m_iTextureId;
	}

	MATTilingMode MATTexture::getTilingU() const
	{
		return m_tileU;
	}

	MATTilingMode MATTexture::getTilingV() const
	{
		return m_tileV;
	}
}
