#pragma once

#include <GameLib/GMS/GMSGeomEntity.h>
#include <GameLib/GMS/GMSGeomStats.h>
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

		static void deserialize(GMSHeader &header, ZBio::ZBinaryReader::BinaryReader *binaryReader, ZBio::ZBinaryReader::BinaryReader *bufFileReader);

	private:
		//https://github.com/ReGlacier/ReHitmanTools/issues/3#issuecomment-769654029
		std::vector<GMSGeomEntity> m_geomEntities {};
		GMSGeomStats m_geomStats {};
	};
}