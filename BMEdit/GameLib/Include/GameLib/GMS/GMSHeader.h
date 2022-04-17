#pragma once

#include <GameLib/GMS/GMSGeomEntity.h>
#include <GameLib/GMS/GMSGeomStats.h>
#include <GameLib/GMS/GMSGroupsCluster.h>
#include <GameLib/GMS/GMSEntries.h>
#include <cstdint>
#include <vector>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::gms
{
	class GMSHeader
	{
	public:
		GMSHeader();

		[[nodiscard]] const GMSEntries &getEntries() const;
		[[nodiscard]] const GMSGeomStats &getGeomStats() const;
		[[nodiscard]] const GMSGroupsCluster &getGeomClusters() const;

		static void deserialize(GMSHeader &header, ZBio::ZBinaryReader::BinaryReader *binaryReader, ZBio::ZBinaryReader::BinaryReader *bufFileReader);
	private:
		GMSEntries m_geomEntities {};
		GMSGeomStats m_geomStats {};
		GMSGroupsCluster m_geomClusters {};
	};
}