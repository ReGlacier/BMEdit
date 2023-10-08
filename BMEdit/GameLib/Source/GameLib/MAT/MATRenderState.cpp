#include <GameLib/MAT/MATRenderState.h>
#include <GameLib/MAT/MATEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>


namespace gamelib::mat
{
	MATRenderState MATRenderState::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name {};
		bool bEnabled { false }, bBlendEnabled { false }, bAlphaTest { false }, bFogEnabled { false }, bZBias { false };
		float fOpacity { 1.0f }, fZOffset { .0f };
		uint32_t iAlphaReference { 0u };
		MATCullMode cullMode { MATCullMode::CM_DontCare };
		MATBlendMode blendMode { MATBlendMode::BM_ADD };

		for (int i = 0; i < propertiesCount; i++)
		{
			MATPropertyEntry entry;
			MATPropertyEntry::deserialize(entry, binaryReader);

			switch (entry.kind)
			{
				case MATPropertyKind::PK_NAME:
				{
				    ZBioSeekGuard guard { binaryReader };
				    binaryReader->seek(entry.reference);

				    name = binaryReader->readCString();
			    }
				break;
			    case MATPropertyKind::PK_ENABLE:
			    {
				    bEnabled = static_cast<bool>(entry.reference);
			    }
				break;
			    case MATPropertyKind::PK_BLEND_ENABLE:
			    {
				    bBlendEnabled = static_cast<bool>(entry.reference);
			    }
				break;
			    case MATPropertyKind::PK_ALPHA_TEST:
			    {
				    bAlphaTest = static_cast<bool>(entry.reference);
			    }
			    break;
			    case MATPropertyKind::PK_FOG_ENABLED:
			    {
				    bFogEnabled = static_cast<bool>(entry.reference);
			    }
			    break;
			    case MATPropertyKind::PK_Z_BIAS:
			    {
				    bZBias = static_cast<bool>(entry.reference);
			    }
			    break;
			    case MATPropertyKind::PK_OPACITY:
			    {
				    fOpacity = static_cast<float>(entry.reference);
			    }
			    break;
			    case MATPropertyKind::PK_Z_OFFSET:
			    {
				    fZOffset = static_cast<float>(entry.reference);
			    }
			    break;
			    case MATPropertyKind::PK_ALPHA_REFERENCE:
			    {
				    iAlphaReference = static_cast<uint32_t>(entry.reference);
			    }
			    break;
			    case MATPropertyKind::PK_CULL_MODE:
			    {
				    ZBioSeekGuard guard { binaryReader };
				    binaryReader->seek(entry.reference);

				    std::string temp = binaryReader->readCString();

				    if (temp == "DontCare")
				    {
					    cullMode = MATCullMode::CM_DontCare;
				    }
				    else if (temp == "OneSided")
				    {
					    cullMode = MATCullMode::CM_OneSided;
				    }
				    else if (temp == "TwoSided")
				    {
					    cullMode = MATCullMode::CM_TwoSided;
				    }
				    else
				    {
					    assert(false && "Unsupported mode!");
				    }
			    }
				break;
			    case MATPropertyKind::PK_BLEND_MODE:
			    {
				    ZBioSeekGuard guard { binaryReader };
				    binaryReader->seek(entry.reference);

				    std::string temp = binaryReader->readCString();

				    if (temp == "TRANS") blendMode = MATBlendMode::BM_TRANS;
				    else if (temp == "TRANS_ON_OPAQUE") blendMode = MATBlendMode::BM_TRANS_ON_OPAQUE;
				    else if (temp == "TRANSADD_ON_OPAQUE") blendMode = MATBlendMode::BM_TRANSADD_ON_OPAQUE;
				    else if (temp == "ADD_BEFORE_TRANS") blendMode = MATBlendMode::BM_ADD_BEFORE_TRANS;
				    else if (temp == "ADD_ON_OPAQUE") blendMode = MATBlendMode::BM_ADD_ON_OPAQUE;
				    else if (temp == "ADD") blendMode = MATBlendMode::BM_ADD;
				    else if (temp == "SHADOW") blendMode = MATBlendMode::BM_SHADOW;
				    else if (temp == "STATICSHADOW") blendMode = MATBlendMode::BM_STATICSHADOW;
				    else
				    {
					    assert(false && "Unsupported mode!");
				    }
			    }
			    break;
			    default:
				    break;
			}
		}

		return MATRenderState(std::move(name), bEnabled, bBlendEnabled, bAlphaTest, bFogEnabled, bZBias, fOpacity, fZOffset, iAlphaReference, cullMode, blendMode);
	}
}