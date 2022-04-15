#include <GameLib/GMS/GMSGeomStats.h>
#include <GameLib/TypeRegistry.h>
#include <ZBinaryReader.hpp>
#include <sstream>


namespace gamelib::gms
{
	GMSGeomStats::GMSGeomStats() = default;

	const std::vector<GMSGeomStats::Entry> &GMSGeomStats::getStatEntries() const
	{
		return m_statEntries;
	}

	void GMSGeomStats::deserialize(GMSGeomStats &stats, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		// Read stats count
		const auto statEntriesCount = binaryReader->read<uint32_t, ZBio::Endianness::LE>();

		stats.m_statEntries.resize(statEntriesCount);

		const auto &registry = TypeRegistry::getInstance();

		for (uint32_t statEntryIndex = 0; statEntryIndex < statEntriesCount; ++statEntryIndex)
		{
			auto &currentEntry = stats.m_statEntries[statEntryIndex];

			currentEntry.typeId = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
			currentEntry.count  = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
			currentEntry.unk    = binaryReader->read<uint32_t, ZBio::Endianness::LE>();

			{
				std::stringstream ss;
				ss << "0x" << std::hex << std::uppercase << currentEntry.typeId;
				const auto typeHash = ss.str();

				currentEntry.typeInfo = registry.findTypeByHash(typeHash);
			}
		}
	}
}