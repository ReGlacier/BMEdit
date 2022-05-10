#pragma once

#include <array>
#include <cstdint>

namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::gms
{
	class GMSGroupClusterInfo
	{
		friend class GMSHeader;
	public:
		GMSGroupClusterInfo();

		[[nodiscard]] uint32_t getClusterSize() const;

		static void deserialize(GMSGroupClusterInfo &groupClusterInfo, ZBio::ZBinaryReader::BinaryReader *binaryReader);

	private:
		static constexpr int kClusterSize = 24;
		using ClusterData = std::array<uint32_t, kClusterSize>;

		ClusterData m_data {};
		uint32_t m_clusterSize { 0u };
	};
}