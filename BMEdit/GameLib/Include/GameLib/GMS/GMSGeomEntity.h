#pragma once

#include <cstdint>
#include <string>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::gms
{
	class GMSGeomEntity
	{
	public:
		GMSGeomEntity();

		[[nodiscard]] const std::string &getName() const;
		[[nodiscard]] uint32_t getTypeId() const;
		[[nodiscard]] uint32_t getInstanceId() const;
		[[nodiscard]] uint32_t getColiBits() const;
		[[nodiscard]] uint32_t getDepthLevel() const;

		static void deserialize(GMSGeomEntity &entity, uint32_t depthLevel, ZBio::ZBinaryReader::BinaryReader *gmsBinaryReader, ZBio::ZBinaryReader::BinaryReader *bufBinaryReader);
	private:
		uint32_t m_depthLevel {};
		std::string m_name {};
		uint32_t m_unk4 { };
		uint32_t m_unk8 { };
		uint32_t m_primitiveId { };
		uint32_t m_unk10 { };
		uint32_t m_typeId { };
		uint32_t m_unk18 { };
		uint32_t m_coliBits { };
		uint32_t m_unk20 { };
		uint32_t m_unk24 { };
		uint32_t m_unk28 { };
		uint32_t m_unk2C { };
		uint32_t m_instanceId { };
		uint32_t m_unk34 { };
		uint32_t m_unk38 { };
		uint32_t m_unk3C { };
	};
}