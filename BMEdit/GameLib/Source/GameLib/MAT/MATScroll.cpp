#include <GameLib/MAT/MATEntries.h>
#include <GameLib/MAT/MATScroll.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATScroll::MATScroll(std::string name, bool bEnabled, std::vector<float>&& speedVector)
		: m_name(std::move(name)), m_bEnabled(bEnabled), m_vfSpeed(std::move(speedVector))
	{
	}

	MATScroll MATScroll::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name {};
		bool bEnabled { false };
		std::vector<float> speedVector {};

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
			else if (entry.kind == MATPropertyKind::PK_SCROLL_SPEED)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				assert(speedVector.empty() && "It should be empty here!");

				speedVector.resize(entry.containerCapacity);
				binaryReader->read<float, ZBio::Endianness::LE>(speedVector.data(), entry.containerCapacity);
			}
			else
			{
				assert(false && "Unprocessed case");
			}
		}

		return MATScroll(std::move(name), bEnabled, std::move(speedVector));
	}
}