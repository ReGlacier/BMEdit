#include <GameLib/MAT/MATTexture.h>
#include <GameLib/MAT/MATEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATTexture::MATTexture(std::string name, bool bEnabled, uint32_t textureId, std::string path, MATTilingMode tilingU, MATTilingMode tilingV, MATTilingMode tilingW)
		: m_name(std::move(name)), m_texturePath(std::move(path)), m_bEnabled(bEnabled), m_iTextureId(textureId), m_tileU(tilingU), m_tileV(tilingV), m_tileW(tilingW)
	{
	}

	MATTexture MATTexture::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name{};
		std::string path{};
		bool bEnabled{false};
		uint32_t textureId {0};
		MATTilingMode tileU { MATTilingMode::TM_NONE }, tileV { MATTilingMode::TM_NONE }, tileW { MATTilingMode::TM_NONE };

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
			else if (kind == MATPropertyKind::PK_TILINIG_U || kind == MATPropertyKind::PK_TILINIG_V || kind == MATPropertyKind::PK_TILINIG_W)
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
				else if (temp == "MIRRORED")
				{
					tempMode = MATTilingMode::TM_MIRRORED;
				}
				else
				{
					assert(false && "Unsupported mode!");
					continue;
				}

				if (kind == MATPropertyKind::PK_TILINIG_U) tileU = tempMode;
				if (kind == MATPropertyKind::PK_TILINIG_V) tileV = tempMode;
				if (kind == MATPropertyKind::PK_TILINIG_W) tileW = tempMode;
			}
			else if (kind == MATPropertyKind::PK_PATH)
			{
				// Path to the texture
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				path = binaryReader->readCString();
			}
			else
			{
				assert(false && "Unsupported entry! Need to support!");
			}
		}

		return MATTexture(std::move(name), bEnabled, textureId, path, tileU, tileV, tileW);
	}

	const std::string& MATTexture::getName() const
	{
		return m_name;
	}

	bool MATTexture::isEnabled() const
	{
		return m_bEnabled;
	}

	uint32_t MATTexture::getTextureId() const
	{
		return m_iTextureId;
	}

	const std::string& MATTexture::getTexturePath() const
	{
		return m_texturePath;
	}

	MATTilingMode MATTexture::getTilingU() const
	{
		return m_tileU;
	}

	MATTilingMode MATTexture::getTilingV() const
	{
		return m_tileV;
	}

	MATTilingMode MATTexture::getTilingW() const
	{
		return m_tileW;
	}

	PresentedTextureSource MATTexture::getPresentedTextureSources() const
	{
		if (m_texturePath.empty() && m_iTextureId == 0)
			return PresentedTextureSource::PTS_NOTHING;

		if (m_texturePath.empty() && m_iTextureId > 0)
			return PresentedTextureSource::PTS_TEXTURE_ID;

		if (!m_texturePath.empty() && m_iTextureId == 0)
			return PresentedTextureSource::PTS_TEXTURE_PATH;

		return PresentedTextureSource::PTS_TEXTURE_ID_AND_PATH;
	}
}
