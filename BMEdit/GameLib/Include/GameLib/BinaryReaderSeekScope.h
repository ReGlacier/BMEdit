#pragma once

#include <cstdint>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib
{
	class BinaryReaderSeekScope
	{
	public:
		explicit BinaryReaderSeekScope(ZBio::ZBinaryReader::BinaryReader *binaryReader);
		~BinaryReaderSeekScope();

		BinaryReaderSeekScope(const BinaryReaderSeekScope &) = delete;
		BinaryReaderSeekScope(BinaryReaderSeekScope &&) = delete;
		BinaryReaderSeekScope &operator=(const BinaryReaderSeekScope &) = delete;
		BinaryReaderSeekScope &operator=(BinaryReaderSeekScope &&) = delete;

	private:
		ZBio::ZBinaryReader::BinaryReader *m_binaryReader { nullptr };
		int64_t m_offset { 0 };
	};
}