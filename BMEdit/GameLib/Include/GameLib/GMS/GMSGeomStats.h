#pragma once

#include <cstdint>
#include <vector>

#include <GameLib/Type.h>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::gms
{
	class GMSGeomStats
	{
	public:
		struct Entry
		{
			uint32_t typeId { 0 };
			const Type *typeInfo { nullptr }; // Recognized at runtime
			uint32_t count { 0 };
			uint32_t unk { 0 }; // Unknown, saved to future usage
		};

		GMSGeomStats();

		[[nodiscard]] const std::vector<Entry> &getStatEntries() const;

		static void deserialize(GMSGeomStats &stats, ZBio::ZBinaryReader::BinaryReader *binaryReader);
	private:
		std::vector<Entry> m_statEntries;
	};
}