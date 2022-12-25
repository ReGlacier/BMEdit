#include <GameLib/PRM/PRMVertexFormatDetails.h>
#include <GameLib/PRM/PRMChunk.h>
#include <cstdint>


#define BM_MAKE_FEATURE(f) static_cast<std::uint8_t>((f))

namespace gamelib::prm
{
	PRMVertexFormatDetails::PRMVertexFormatDetails(const PRMChunk *chunk)
	{
		if (chunk)
		{
			detectFeatures(chunk);
		}
	}

	std::size_t PRMVertexFormatDetails::getVerticesCount() const
	{
		return m_verticesCount;
	}

	bool PRMVertexFormatDetails::isFeatureSupported(PRMVertexFormatFeature feature) const
	{
		return static_cast<bool>(m_features & static_cast<std::uint8_t>(feature));
	}

	std::optional<std::size_t> PRMVertexFormatDetails::getFeatureDataOffsetInBuffer(PRMVertexFormatFeature feature) const
	{
		if (!isFeatureSupported(feature))
			return std::nullopt;

		return m_data.at(static_cast<std::size_t>(feature)).offset;
	}

	std::optional<std::size_t> PRMVertexFormatDetails::getFeatureDataStrideInBuffer(PRMVertexFormatFeature feature) const
	{
		if (!isFeatureSupported(feature))
			return std::nullopt;

		return m_data.at(static_cast<std::size_t>(feature)).stride;
	}

	std::optional<VertexFeatureDataFormat> PRMVertexFormatDetails::getFeatureDataFormatInBuffer(PRMVertexFormatFeature feature) const
	{
		if (!isFeatureSupported(feature))
			return std::nullopt;

		return m_data.at(static_cast<std::size_t>(feature)).dataFormat;
	}

	void PRMVertexFormatDetails::detectFeatures(const PRMChunk *chunk)
	{
		switch (chunk->getKind())
		{
			case PRMChunkRecognizedKind::CRK_ZERO_CHUNK:
			case PRMChunkRecognizedKind::CRK_INDEX_BUFFER:
			case PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER:
			case PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER:
				return;
			case PRMChunkRecognizedKind::CRK_VERTEX_BUFFER:
			{
				auto vbh = chunk->getVertexBufferHeader();

				// See https://github.com/HHCHunter/Hitman-BloodMoney/blob/master/TOOLS/PRMConverter/Source/PRMConvert.cpp#L31 for format details
				switch (vbh->vertexFormat)
				{
					case PRMVertexBufferFormat::VBF_VERTEX_10:
					{
						m_features = BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_POSITION);
						auto& posFeature = m_data[BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_POSITION)];

						posFeature.offset = 0;
						posFeature.stride = static_cast<std::size_t>(vbh->vertexFormat);
					}
					break;
					case PRMVertexBufferFormat::VBF_VERTEX_24:
					{
						m_features = BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_POSITION) | BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_UV);

						auto& posFeature = m_data[BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_POSITION)];
						posFeature.offset = 0;

						auto& uvFeature = m_data[BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_UV)];
						uvFeature.offset = 0xC; // 12 bytes, 3 floats * 4 bytes per float

						uvFeature.stride = posFeature.stride = static_cast<std::size_t>(vbh->vertexFormat);
					}
					break;
					case PRMVertexBufferFormat::VBF_VERTEX_28:
					{
						m_features = BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_POSITION) | BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_UV);

						auto& posFeature = m_data[BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_POSITION)];
						posFeature.offset = 0;

						auto& uvFeature = m_data[BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_UV)];
						uvFeature.offset = 20; // 20 bytes, pos (12) + unk[2] (8)

						uvFeature.stride = posFeature.stride = static_cast<std::size_t>(vbh->vertexFormat);
					}
					break;
					case PRMVertexBufferFormat::VBF_VERTEX_34:
					{
						m_features = BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_POSITION) | BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_UV);

						auto& posFeature = m_data[BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_POSITION)];
						posFeature.offset = 0;

						auto& uvFeature = m_data[BM_MAKE_FEATURE(PRMVertexFormatFeature::VFF_UV)];
						uvFeature.offset = 36; // 12 (pos) + unk1[3] + unk2[3] = 12 + 12 + 12 = 36

						uvFeature.stride = posFeature.stride = static_cast<std::size_t>(vbh->vertexFormat);
					}
					break;
					case PRMVertexBufferFormat::VBF_UNKNOWN_VERTEX:
						assert(false);
						return;
				}

				m_verticesCount = chunk->getBufferSize() / static_cast<std::size_t>(vbh->vertexFormat);
			}
			break;
		}
	}
}

#undef BM_MAKE_FEATURE