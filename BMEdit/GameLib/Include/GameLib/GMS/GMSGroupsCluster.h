#pragma once

#include <GameLib/GMS/GMSGroupClusterInfo.h>

#include <vector>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::gms
{
	class GMSGroupsCluster
	{
		friend class GMSHeader;
	public:
		GMSGroupsCluster();

		[[nodiscard]] const std::vector<GMSGroupClusterInfo> &getClusters() const;

		static void deserialize(GMSGroupsCluster &cluster, ZBio::ZBinaryReader::BinaryReader *binaryReader);
	private:
		std::vector<GMSGroupClusterInfo> m_clusters;
	};
}