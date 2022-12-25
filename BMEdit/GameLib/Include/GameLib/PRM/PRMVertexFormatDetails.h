#pragma once

#include <GameLib/PRM/PRMVertexFormatFeature.h>
#include <optional>
#include <cstdint>
#include <map>


namespace gamelib::prm
{
	class PRMChunk;

	enum class VertexFeatureDataFormat
	{
		VFDF_FLOAT32,
		VFDF_INT32
	};

	class PRMVertexFormatDetails
	{
	public:
		PRMVertexFormatDetails() = default;
		explicit PRMVertexFormatDetails(const PRMChunk *chunk);

		[[nodiscard]] std::size_t getVerticesCount() const;

		// Support level
		[[nodiscard]] bool isFeatureSupported(PRMVertexFormatFeature feature) const;
		[[nodiscard]] std::optional<std::size_t> getFeatureDataOffsetInBuffer(PRMVertexFormatFeature feature) const;
		[[nodiscard]] std::optional<std::size_t> getFeatureDataStrideInBuffer(PRMVertexFormatFeature feature) const;
		[[nodiscard]] std::optional<VertexFeatureDataFormat> getFeatureDataFormatInBuffer(PRMVertexFormatFeature feature) const;

	private:
		void detectFeatures(const PRMChunk *chunk);

	private:
		struct FeatureBufferData
		{
			std::size_t offset { 0 };
			std::size_t stride { 0 };
			VertexFeatureDataFormat dataFormat { VertexFeatureDataFormat::VFDF_FLOAT32 };
		};

		std::size_t m_verticesCount { 0 };
		std::uint8_t m_features { 0u };
		std::map<std::uint8_t, FeatureBufferData> m_data;
	};
}