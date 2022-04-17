#include <GameLib/GMS/GMSGroupClusterInfo.h>
#include <ZBinaryReader.hpp>


namespace gamelib::gms
{
	GMSGroupClusterInfo::GMSGroupClusterInfo() = default;

	uint32_t GMSGroupClusterInfo::getClusterSize() const
	{
		return m_clusterSize;
	}

	void GMSGroupClusterInfo::deserialize(GMSGroupClusterInfo &groupClusterInfo, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		// Read cluster meta
		binaryReader->read<uint32_t, ZBio::Endianness::LE>(&groupClusterInfo.m_data[0], kClusterSize);

		// Calculate sum
		for (const auto &v: groupClusterInfo.m_data)
		{
			groupClusterInfo.m_clusterSize += v;
		}
	}
}