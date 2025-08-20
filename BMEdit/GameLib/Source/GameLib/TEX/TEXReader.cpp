#include <GameLib/TEX/TEXReader.h>
#include <ZBinaryReader.hpp>


namespace gamelib::tex
{
	bool TEXReader::parse(const uint8_t *texFileBuffer, int64_t texFileSize)
	{
		if (!texFileBuffer || !texFileSize)
			return false;

		{
			ZBio::ZBinaryReader::BinaryReader reader{
			    reinterpret_cast<const char *>(texFileBuffer),
			    static_cast<int64_t>(texFileSize)};

			// Read header
			TEXHeader::deserialize(m_header, &reader);
		}

		if (m_header.m_texturesPoolOffset >= texFileSize || m_header.m_cubeMapsPoolOffset >= texFileSize)
			return false; // Invalid file

		// Read textures pool
		{
			ZBio::ZBinaryReader::BinaryReader reader{
				reinterpret_cast<const char*>(&texFileBuffer[m_header.m_texturesPoolOffset]),
				static_cast<int64_t>(texFileSize)
			};

			reader.read<uint32_t, ZBio::Endianness::LE>(m_texturesPool.data(), static_cast<int64_t>(m_texturesPool.size()));
		}

		///NOTE: For PS4 support see https://github.com/pavledev/GlacierTEXEditor/blob/master/GlacierTEXEditor/Form1.cs#L261
		// Now read entries
		m_entries.clear();

		for (const uint32_t& entryOffset : m_texturesPool)
		{
			if (entryOffset != 0)
			{
				// Do read
				ZBio::ZBinaryReader::BinaryReader reader{
					reinterpret_cast<const char*>(&texFileBuffer[entryOffset]),
					static_cast<int64_t>(texFileSize - entryOffset) // Technically we must set some constants, but who cares?
				};

				TEXEntry& entry = m_entries.emplace_back();
				entry.m_offset = entryOffset;
				TEXEntry::deserialize(entry, &reader);
			}
			else
			{
				if (m_entries.empty())
				{
					++m_countOfEmptyOffsets;
				}
			}
		}

		// Read cube maps pool #2
		{
			ZBio::ZBinaryReader::BinaryReader reader{
			    reinterpret_cast<const char*>(&texFileBuffer[m_header.m_cubeMapsPoolOffset]),
			    static_cast<int64_t>(texFileSize)
			};

			reader.read<uint32_t, ZBio::Endianness::LE>(m_cubeMapsPool.data(), static_cast<int64_t>(m_cubeMapsPool.size()));
		}

		// Read cubemaps
		{
			for (auto& entry : m_entries)
			{
				// A right way to get cube maps working in game
				if (!(entry.m_flags & TEXEntryFlags::TEX_EF_IsCubeMap))
					continue;

				const uint32_t offset = m_cubeMapsPool[entry.m_index];

				ZBio::ZBinaryReader::BinaryReader reader{
				    reinterpret_cast<const char*>(&texFileBuffer[offset]), static_cast<int64_t>(texFileSize)
				};

				TEXCubeMaps::deserialize(entry.m_cubeMaps, &reader);
			}
		}

		return true;
	}
}