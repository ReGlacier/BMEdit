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
}