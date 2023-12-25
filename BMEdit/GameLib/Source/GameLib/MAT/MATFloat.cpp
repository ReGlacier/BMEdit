#include <GameLib/MAT/MATFloat.h>
#include <GameLib/MAT/MATEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATFloat::MATFloat(std::string name, bool bEnabled, MATValU&& valU)
		: m_name(std::move(name)), m_bEnabled(bEnabled), m_valU(std::move(valU))
	{
	}

	MATFloat MATFloat::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name {};
		bool bEnabled { false };
		MATValU valU {};


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
			else if (entry.kind == MATPropertyKind::PK_ENABLE)
			{
				bEnabled = static_cast<bool>(entry.reference);
			}
			else if (entry.kind == MATPropertyKind::PK_VAL_U)
			{
				// Make ValU by right way
				valU = MATValU::makeFromStream(binaryReader, entry);
			}
			else
			{
				assert(false && "Unprocessed case");
			}
		}

		return MATFloat(std::move(name), bEnabled, std::move(valU));
	}
}