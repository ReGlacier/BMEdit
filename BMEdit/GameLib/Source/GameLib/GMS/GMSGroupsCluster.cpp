#include <GameLib/GMS/GMSGroupsCluster.h>
#include <GameLib/GMS/GMSStructureError.h>
#include <ZBinaryReader.hpp>


namespace gamelib::gms
{
	GMSGroupsCluster::GMSGroupsCluster() = default;

	const std::vector<GMSGroupClusterInfo> &GMSGroupsCluster::getClusters() const
	{
		return m_clusters;
	}

	void GMSGroupsCluster::deserialize(GMSGroupsCluster &cluster, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		// Read size
		const auto clustersCount = binaryReader->read<uint32_t, ZBio::Endianness::LE>();

		if (!clustersCount)
		{
			throw GMSStructureError("Invalid GMS: empty clusters size not allowed (at least one group (ROOT) should exists!)");
		}

		cluster.m_clusters.resize(clustersCount + 1);

		for (int clusterIndex = 0; clusterIndex < clustersCount; ++clusterIndex)
		{
			auto &currentCluster = cluster.m_clusters[clusterIndex];

			// Read cluster
			GMSGroupClusterInfo::deserialize(currentCluster, binaryReader);
		}
	}
}