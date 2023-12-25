#pragma once

#include <ZBinaryReader.hpp>
#include <cassert>

// Just a few helpers for BinaryIO library
namespace gamelib
{
	struct ZBioHelpers
	{
		/**
		 * @brief Move position by offset from current position (pos += offset)
		 * @param reader
		 * @param offset
		 */
		static void seekBy(ZBio::ZBinaryReader::BinaryReader* reader, int64_t offset)
		{
			if (!reader)
			{
				assert(reader != nullptr);
				return;
			}

			const int64_t finalPos = offset + reader->tell();
			if (finalPos < 0 || finalPos > reader->size())
			{
				assert(false && "Out of bounds!");
				return;
			}

			reader->seek(finalPos);
		}
	};

	struct ZBioSeekGuard
	{
		ZBioSeekGuard() = delete;
		ZBioSeekGuard(const ZBioSeekGuard&) = delete;
		ZBioSeekGuard(ZBioSeekGuard&&) = delete;
		ZBioSeekGuard& operator=(const ZBioSeekGuard&) = delete;
		ZBioSeekGuard& operator=(ZBioSeekGuard&&) = delete;

		explicit ZBioSeekGuard(ZBio::ZBinaryReader::BinaryReader* reader)
		{
			m_reader = reader;

			if (reader)
			{
				m_seekTo = reader->tell();
			}
		}

		~ZBioSeekGuard()
		{
			if (m_reader)
			{
				m_reader->seek(m_seekTo);
				m_reader = nullptr;
			}
		}

	private:
		ZBio::ZBinaryReader::BinaryReader* m_reader { nullptr };
		int64_t m_seekTo { 0 };
	};
}