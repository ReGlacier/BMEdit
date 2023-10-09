#include <GameLib/MAT/MATOption.h>
#include <GameLib/MAT/MATEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATOption::MATOption(std::string name, bool bEnabled, MATValU&& valU)
		: m_name(std::move(name)), m_bEnabled(bEnabled), m_valU(std::move(valU))
	{
	}

	const std::string& MATOption::getName() const
	{
		return m_name;
	}

	bool MATOption::isEnabled() const
	{
		return m_bEnabled;
	}

	const MATValU& MATOption::getValU() const
	{
		return m_valU;
	}

	MATOption MATOption::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name {};
		bool bEnabled = false;
		MATValU valU {};

		for (int i = 0; i < propertiesCount; i++)
		{
			MATPropertyEntry entry;
			MATPropertyEntry::deserialize(entry, binaryReader);

			if (auto kind = entry.kind; kind == MATPropertyKind::PK_NAME)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				name = binaryReader->readCString();
			}
			else if (kind == MATPropertyKind::PK_ENABLE)
			{
				bEnabled = static_cast<bool>(entry.reference);
			}
			else if (kind == MATPropertyKind::PK_VAL_U)
			{
				valU = MATValU::makeFromStream(binaryReader, entry);
			}
			else
			{
				assert(false && "Unprocessed entry!");
			}
		}

		return MATOption(std::move(name), bEnabled, std::move(valU));
	}
}