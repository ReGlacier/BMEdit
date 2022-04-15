#include <GameLib/GMS/GMSHeader.h>
#include <GameLib/GMS/GMSStructureError.h>
#include <GameLib/GMS/GMSSectionOffsets.h>
#include <GameLib/BinaryReaderSeekScope.h>
#include <ZBinaryReader.hpp>


namespace gamelib::gms
{
	GMSHeader::GMSHeader() = default;

	void GMSHeader::deserialize(GMSHeader &header, ZBio::ZBinaryReader::BinaryReader *gmsFileReader, ZBio::ZBinaryReader::BinaryReader *bufFileReader)
	{
		//TODO: https://github.com/ReGlacier/ReHitmanTools/issues/3#issuecomment-769654029
		{
			// Read Entities
			BinaryReaderSeekScope entitiesScope { gmsFileReader };
			gmsFileReader->seek(GMSSectionOffsets::ENTITIES);

			const auto geomTableOffset = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();
			gmsFileReader->seek(geomTableOffset);

			struct GeomDescription
			{
				uint32_t declarationOffset { 0 };
				uint32_t unk4 { 0 };
			};

			const auto entitiesCount = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();
			header.m_geomEntities.reserve(entitiesCount);

			for (int entId = 0; entId < entitiesCount; ++entId)
			{
				BinaryReaderSeekScope entityScope { gmsFileReader };

				constexpr int kEntrySize = 8;

				const auto geomAddress = geomTableOffset + 4 + (entId * kEntrySize);
				gmsFileReader->seek(geomAddress);

				GeomDescription desc {};
				desc.declarationOffset = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();
				desc.unk4 = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();

				auto &currentGeomEntry = header.m_geomEntities.emplace_back();

				{
					BinaryReaderSeekScope geomScope { gmsFileReader };

					const auto depth = (desc.declarationOffset >> 24u);
					const auto realOffset = 4 * (desc.declarationOffset & 0xFFFFFFu);
					gmsFileReader->seek(realOffset);

					GMSGeomEntity::deserialize(currentGeomEntry, depth, gmsFileReader, bufFileReader);
				}
			}
		}

		{
			// Read GeomStats
			BinaryReaderSeekScope entitiesScope { gmsFileReader };
			gmsFileReader->seek(GMSSectionOffsets::GEOM_STATS);

			const auto geomStatsOffset = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();
			gmsFileReader->seek(geomStatsOffset);

			GMSGeomStats::deserialize(header.m_geomStats, gmsFileReader);
		}
	}
}