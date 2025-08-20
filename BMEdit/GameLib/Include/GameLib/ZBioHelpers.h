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

	template <typename T>
	concept TSeekable = requires (T t)
	{
		t.seek((int64_t)1337);
		t.tell();
	};

	template <typename T = ZBio::ZBinaryReader::BinaryReader> requires (TSeekable<T>)
	struct ZBioSeekGuard
	{
		ZBioSeekGuard() = delete;
		ZBioSeekGuard(const ZBioSeekGuard&) = delete;
		ZBioSeekGuard(ZBioSeekGuard&&) = delete;
		ZBioSeekGuard& operator=(const ZBioSeekGuard&) = delete;
		ZBioSeekGuard& operator=(ZBioSeekGuard&&) = delete;

		explicit ZBioSeekGuard(T* seekable)
		{
			m_seekable = seekable;

			if (seekable)
			{
				m_seekTo = seekable->tell();
			}
		}

		~ZBioSeekGuard()
		{
			if (m_seekable)
			{
				m_seekable->seek(m_seekTo);
				m_seekable = nullptr;
			}
		}

	private:
		T* m_seekable { nullptr };
		int64_t m_seekTo { 0 };
	};
}