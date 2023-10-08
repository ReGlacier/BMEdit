#include <GameLib/MAT/MATSprite.h>
#include <GameLib/MAT/MATEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATSprite::MATSprite(std::string name, bool bUnk0, bool bUnk1)
	    : m_name(std::move(name)), m_bUnk0(bUnk0), m_bUnk1(bUnk1)
	{
	}

	MATSprite MATSprite::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name {};
		bool bUnk0 { false };
		bool bUnk1 { false };
		int lastUpdatedUnk { 0 };

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
				if (lastUpdatedUnk == 0) bUnk0 = static_cast<bool>(entry.reference);
				else if (lastUpdatedUnk == 1) bUnk1 = static_cast<bool>(entry.reference);

				++lastUpdatedUnk;
			}
		}

		return MATSprite(std::move(name), bUnk0, bUnk1);
	}

	const std::string& MATSprite::getName() const
	{
		return m_name;
	}

	bool MATSprite::getUnk0() const
	{
		return m_bUnk0;
	}

	bool MATSprite::getUnk1() const
	{
		return m_bUnk1;
	}
}