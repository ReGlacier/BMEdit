#include <GameLib/MAT/MATColorChannel.h>
#include <GameLib/MAT/MATEntries.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>
#include <utility>


namespace gamelib::mat
{
	MATColorChannel::MATColorChannel(std::string name, bool bEnabled, MATColorRGBA&& color)
		: m_name(std::move(name)), m_bEnabled(bEnabled), m_color(color)
	{
	}

	MATColorChannel MATColorChannel::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		std::string name {};
		bool bEnabled { false };
		MATColorRGBA color;

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
				// We will jump to unmarked region (raw buffer)
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				if (entry.containerCapacity == 3)
				{
					// RGB
					if (entry.valueType == MATValueType::PT_UINT32)
					{
						color.emplace<glm::ivec3>(
						    binaryReader->read<uint32_t, ZBio::Endianness::LE>(),
						    binaryReader->read<uint32_t, ZBio::Endianness::LE>(),
						    binaryReader->read<uint32_t, ZBio::Endianness::LE>()
						);
					}
					else if (entry.valueType == MATValueType::PT_FLOAT)
					{
						color.emplace<glm::vec3>(
						    binaryReader->read<float, ZBio::Endianness::LE>(),
						    binaryReader->read<float, ZBio::Endianness::LE>(),
						    binaryReader->read<float, ZBio::Endianness::LE>()
						);
					}
				}
				else if (entry.containerCapacity == 4)
				{
					// RGBA
					if (entry.valueType == MATValueType::PT_UINT32)
					{
						color.emplace<glm::ivec4>(
						    binaryReader->read<uint32_t, ZBio::Endianness::LE>(),
						    binaryReader->read<uint32_t, ZBio::Endianness::LE>(),
						    binaryReader->read<uint32_t, ZBio::Endianness::LE>(),
						    binaryReader->read<uint32_t, ZBio::Endianness::LE>()
						);
					}
					else if (entry.valueType == MATValueType::PT_FLOAT)
					{
						color.emplace<glm::vec4>(
						    binaryReader->read<float, ZBio::Endianness::LE>(),
						    binaryReader->read<float, ZBio::Endianness::LE>(),
						    binaryReader->read<float, ZBio::Endianness::LE>(),
						    binaryReader->read<float, ZBio::Endianness::LE>()
						);
					}
				}
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

	const MATColorRGBA& MATColorChannel::getColor() const
	{
		return m_color;
	}
}