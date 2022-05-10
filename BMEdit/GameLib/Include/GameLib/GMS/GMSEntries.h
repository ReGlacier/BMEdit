#pragma once

#include <GameLib/GMS/GMSGeomEntity.h>
#include <vector>

namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::gms
{
	class GMSEntries
	{
		friend class GMSHeader;

	public:
		GMSEntries();

		[[nodiscard]] const std::vector<GMSGeomEntity> &getGeomEntities() const;

		static void deserialize(GMSEntries &entries, ZBio::ZBinaryReader::BinaryReader *gmsFileReader, ZBio::ZBinaryReader::BinaryReader *bufFileReader);

	private:
		std::vector<GMSGeomEntity> m_entities;
	};
}