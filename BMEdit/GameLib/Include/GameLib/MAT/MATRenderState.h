#pragma once

#include <GameLib/MAT/MATBlendMode.h>
#include <GameLib/MAT/MATCullMode.h>
#include <GameLib/MAT/MATValU.h>
#include <string>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	class MATRenderState
	{
	public:
		MATRenderState(std::string name,
		               bool bEnabled, bool bBlendEnabled, bool bAlphaTest, bool bFogEnabled, bool bZBias,
		               float fOpacity, float fZOffset,
		               uint32_t iAlphaReference,
		               MATCullMode cullMode, MATBlendMode blendMode,
		               MATValU&& valU):
			   	m_name(std::move(name)),
   				m_bEnabled(bEnabled), m_bEnableBlend(bBlendEnabled), m_bAlphaTest(bAlphaTest), m_bFogEnabled(bFogEnabled), m_bZBias(bZBias),
			   	m_fOpacity(fOpacity), m_fZOffset(fZOffset),
			   	m_iAlphaReference(iAlphaReference),
			   	m_eCullMode(cullMode), m_eBlendMode(blendMode),
				m_valU(std::move(valU))
		{
		}

		static MATRenderState makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount);

		[[nodiscard]] const std::string& getName() const { return m_name; }
		[[nodiscard]] bool isEnabled() const { return m_bEnabled; }
		[[nodiscard]] bool isBlendEnabled() const { return m_bEnableBlend; }
		[[nodiscard]] bool isAlphaTestEnabled() const { return m_bAlphaTest; }
		[[nodiscard]] bool isFogEnabled() const { return m_bFogEnabled; }
		[[nodiscard]] bool hasZBias() const { return m_bZBias; }
		[[nodiscard]] uint32_t getAlphaReference() const { return m_iAlphaReference; }
		[[nodiscard]] float getOpacity() const { return m_fOpacity; }
		[[nodiscard]] float getZOffset() const { return m_fZOffset; }
		[[nodiscard]] MATCullMode getCullMode() const { return m_eCullMode; }
		[[nodiscard]] MATBlendMode getBlendMode() const { return m_eBlendMode; }

	private:
		std::string m_name {};
		bool m_bEnabled { false };
		bool m_bEnableBlend { false };
		bool m_bAlphaTest { false };
		bool m_bFogEnabled { false };
		bool m_bZBias { false };
		MATCullMode m_eCullMode { MATCullMode::CM_DontCare };
		MATBlendMode m_eBlendMode { MATBlendMode::BM_TRANS };
		uint32_t m_iAlphaReference { 0u };
		float m_fOpacity { 1.f };
		float m_fZOffset { .0f };
		MATValU m_valU {};
	};
}