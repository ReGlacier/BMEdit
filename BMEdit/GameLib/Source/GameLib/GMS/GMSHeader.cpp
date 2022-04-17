#include <GameLib/GMS/GMSHeader.h>
#include <GameLib/GMS/GMSStructureError.h>
#include <GameLib/GMS/GMSSectionOffsets.h>
#include <GameLib/BinaryReaderSeekScope.h>
#include <ZBinaryReader.hpp>
#include <array>


namespace gamelib::gms
{
	GMSHeader::GMSHeader() = default;

	const GMSEntries &GMSHeader::getEntries() const
	{
		return m_geomEntities;
	}

	const GMSGeomStats &GMSHeader::getGeomStats() const
	{
		return m_geomStats;
	}

	const GMSGroupsCluster &GMSHeader::getGeomClusters() const
	{
		return m_geomClusters;
	}

	void GMSHeader::deserialize(GMSHeader &header, ZBio::ZBinaryReader::BinaryReader *gmsFileReader, ZBio::ZBinaryReader::BinaryReader *bufFileReader)
	{
		//TODO: https://github.com/ReGlacier/ReHitmanTools/issues/3#issuecomment-769654029

		{
			// Validate format
			BinaryReaderSeekScope rootScope { gmsFileReader };
			gmsFileReader->seek(4);

			static constexpr std::array<uint32_t, 3> kExpectedSignature = { 0, 0, 4 };
			std::array<uint32_t, 3> sections = { 0, 0, 0 };

			gmsFileReader->read<uint32_t, ZBio::Endianness::LE>(sections.data(), 3);

			if (sections != kExpectedSignature)
			{
				// Invalid format, stop deserialization
				throw GMSStructureError("Invalid GMS format: expected 0x4=0, 0x8=0, 0xC=4");
			}
		}

		{
			// Check physics data tag (for Hitman Blood Money should be 0xFFFFFFFFu)
			BinaryReaderSeekScope physicsScope { gmsFileReader };
			gmsFileReader->seek(GMSSectionOffsets::LEGACY_PHYSICS_DATA);

			constexpr uint32_t kValidPhysicsOffset = 0xFFFFFFFFu;

			const auto physicsOffset = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();
			if (physicsOffset != kValidPhysicsOffset)
			{
				throw GMSStructureError("Invalid GMS format: unsupported pointer to physics data (expected 0xFFFFFFFF)");
			}
		}

		{
			// Read clusters
			BinaryReaderSeekScope depthLevelsScope { gmsFileReader };
			gmsFileReader->seek(GMSSectionOffsets::CLUSTER_INFO);

			const auto clustersRegionOffset = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();
			gmsFileReader->seek(clustersRegionOffset);

			GMSGroupsCluster::deserialize(header.m_geomClusters, gmsFileReader);
		}

		{
			// Read Entities
			BinaryReaderSeekScope entitiesScope { gmsFileReader };
			gmsFileReader->seek(GMSSectionOffsets::ENTITIES);

			const auto geomTableOffset = gmsFileReader->read<uint32_t, ZBio::Endianness::LE>();
			gmsFileReader->seek(geomTableOffset);

			GMSEntries::deserialize(header.m_geomEntities, gmsFileReader, bufFileReader);
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