#include <GameLib/MAT/MATSubClass.h>
#include <GameLib/MAT/MATEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATSubClass MATSubClass::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name {}, oTyp {}, sTyp {};
		std::vector<MATLayer> layers {};
		std::vector<std::string> valI {};

		for (int i = 0; i < propertiesCount; i++)
		{
			MATPropertyEntry entry;
			MATPropertyEntry::deserialize(entry, binaryReader);

			if (entry.kind == MATPropertyKind::PK_NAME)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				name = binaryReader->readCString();
			}
			else if (entry.kind == MATPropertyKind::PK_OTYP)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				oTyp = binaryReader->readCString();
			}
			else if (entry.kind == MATPropertyKind::PK_STYP)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				sTyp = binaryReader->readCString();
			}
			else if (entry.kind == MATPropertyKind::PK_LAYER)
			{
				// Read layer
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				layers.emplace_back(MATLayer::makeFromStream(binaryReader, entry.containerCapacity));
			}
			else if (entry.kind == MATPropertyKind::PK_VAL_I)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				valI.emplace_back(binaryReader->readCString());
			}
			else
			{
				assert(false && "Unprocessed entry!");
			}
		}

		return MATSubClass(std::move(name), std::move(oTyp), std::move(sTyp), std::move(layers));
	}
}