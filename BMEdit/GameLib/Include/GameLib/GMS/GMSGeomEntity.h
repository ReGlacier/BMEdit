#pragma once

#include <cstdint>
#include <string>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::gms
{
	enum ECollisionMask : uint32_t
	{
		COLIMASK_All = 1,
		COLIMASK_Background = 2,
		COLIMASK_Shot = 4,
		COLIMASK_WaterGlass = 8,
		COLIMASK_NoWalk = 16,
		COLIMASK_Sight = 32,
		COLIMASK_Hero = 64,
		COLIMASK_Camera = 128,
		COLIMASK_NPC = 256


		/// That masks means that entity attached to some ZROOM and we've able to find room by some "hint"
		/// 0x40040000
		/// 0x40000000
	};

	class GMSGeomEntity
	{
		///----------
		/// FRIENDS
		///----------
		friend class GMSEntries;
		friend class GMSHeader;

	public:
		static constexpr uint32_t kInvalidParent = 0xFFFFFFEEu;

		GMSGeomEntity();

		[[nodiscard]] const std::string &getName() const;
		[[nodiscard]] uint32_t getTypeId() const;
		[[nodiscard]] uint32_t getInstanceId() const;
		[[nodiscard]] uint32_t getColiBits() const;
		[[nodiscard]] uint32_t getParentGeomIndex() const;
		[[nodiscard]] bool isInheritedOfGeom() const;
		[[nodiscard]] bool isRootOfGroup() const;
		[[nodiscard]] uint32_t getRelativeDepthLevel() const;
		[[nodiscard]] uint32_t getGeomFlags() const;

		static void deserialize(GMSGeomEntity &entity, ZBio::ZBinaryReader::BinaryReader *gmsBinaryReader, ZBio::ZBinaryReader::BinaryReader *bufBinaryReader);

	private:
		///---------------
		/// RUNTIME DATA
		///---------------
		uint32_t m_parentGeomIndex { GMSGeomEntity::kInvalidParent };
		uint32_t m_geomFlags { 0u };

		///--------------------
		/// DESERIALIZED DATA
		///--------------------
		std::string m_name {};
		uint32_t m_unk4 { };
		uint32_t m_unk8 { };
		uint32_t m_primitiveId { };
		uint32_t m_unk10 { };
		uint32_t m_typeId { };
		uint32_t m_unk18 { };
		uint32_t m_coliBits {}; // +1C. NOTE: ECollisionMask contains all possible & expected values
		uint32_t m_unk20 { };
		uint32_t m_unk24 { };
		uint32_t m_unk28 { };
		uint32_t m_unk2C { };
		uint32_t m_instanceId { };
		union {
			struct {
				uint8_t unk0;
				uint8_t priority;
				uint8_t unk2;
				uint8_t unk3;
			} data;
			uint8_t u8_4[4];
			uint32_t u32 { 0u };
		} m_unk34;
		uint32_t m_unk38 { };
		uint32_t m_unk3C { };

		static_assert(sizeof(m_unk34) == sizeof(uint32_t), "Compile error: bad size of m_unk34!");
	};
}