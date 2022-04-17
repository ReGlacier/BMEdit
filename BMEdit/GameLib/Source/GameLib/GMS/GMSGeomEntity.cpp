#include <GameLib/GMS/GMSGeomEntity.h>
#include <ZBinaryReader.hpp>


namespace gamelib::gms
{
	GMSGeomEntity::GMSGeomEntity() = default;

	const std::string &GMSGeomEntity::getName() const
	{
		return m_name;
	}

	uint32_t GMSGeomEntity::getTypeId() const
	{
		return m_typeId;
	}

	uint32_t GMSGeomEntity::getInstanceId() const
	{
		return m_instanceId;
	}

	uint32_t GMSGeomEntity::getColiBits() const
	{
		return m_coliBits;
	}

	uint32_t GMSGeomEntity::getDepthLevel() const
	{
		return m_depthLevel;
	}

	uint32_t GMSGeomEntity::getParentGeomIndex() const
	{
		return m_parentGeomIndex;
	}

	bool GMSGeomEntity::isInheritedOfGeom() const
	{
		return m_typeId && ((m_typeId & 0x100000u) != 0u);
	}

	void GMSGeomEntity::deserialize(GMSGeomEntity &entity, uint32_t depthLevel, ZBio::ZBinaryReader::BinaryReader *gmsBinaryReader, ZBio::ZBinaryReader::BinaryReader *bufBinaryReader)
	{
		entity.m_depthLevel = depthLevel;

		// Read name
		const auto nameOffset = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		bufBinaryReader->seek(nameOffset);
		entity.m_name = bufBinaryReader->readCString();

		// Read unk4 & unk8
		entity.m_unk4 = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entity.m_unk8 = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// Read primitive id
		entity.m_primitiveId = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// Read unk10
		entity.m_unk10 = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// Read typeId
		entity.m_typeId = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// Read unk18
		entity.m_unk18 = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// Read coliBits
		entity.m_coliBits = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// Read unk20, 24, 28, 2C
		entity.m_unk20 = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entity.m_unk24 = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entity.m_unk28 = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entity.m_unk2C = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// Read instanceId
		entity.m_instanceId = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();

		// Read unk34, 38, 3C
		entity.m_unk34.u32 = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entity.m_unk38 = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		entity.m_unk3C = gmsBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}
}