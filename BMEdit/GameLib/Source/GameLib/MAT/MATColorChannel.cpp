#include <GameLib/MAT/MATColorChannel.h>
#include <GameLib/MAT/MATEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATColorChannel::MATColorChannel(std::string name, bool bEnabled, MATValU&& color)
		: m_name(std::move(name)), m_bEnabled(bEnabled), m_color(color)
	{
	}

	MATColorChannel MATColorChannel::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name {};
		bool bEnabled { false };
		MATValU color;

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
				// Read ValU
				color = MATValU::makeFromStream(binaryReader, entry);
			}
			else
			{
				assert(false && "Unprocessed entry!");
			}
		}

		return MATColorChannel(std::move(name), bEnabled, std::move(color));
	}

	const std::string& MATColorChannel::getName() const
	{
		return m_name;
	}

	bool MATColorChannel::isEnabled() const
	{
		return m_bEnabled;
	}

	const MATValU& MATColorChannel::getColor() const
	{
		return m_color;
	}
}