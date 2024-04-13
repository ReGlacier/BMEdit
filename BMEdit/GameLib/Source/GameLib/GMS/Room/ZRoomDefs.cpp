#include <GameLib/GMS/Room/ZRoomDefs.h>
#include <glm/gtc/type_ptr.hpp>
#include <ZBinaryReader.hpp>


namespace gamelib::gms::room
{
	void ZRoomExit::deserialize(ZRoomExit& eXit, ZBio::ZBinaryReader::BinaryReader* bufBinaryReader)
	{
		bufBinaryReader->read<float, ZBio::Endianness::LE>(glm::value_ptr(eXit.unkVec0), 3);
		bufBinaryReader->read<float, ZBio::Endianness::LE>(glm::value_ptr(eXit.unkVec1), 3);
		bufBinaryReader->read<float, ZBio::Endianness::LE>(glm::value_ptr(eXit.unkVec2), 3);
		bufBinaryReader->read<float, ZBio::Endianness::LE>(glm::value_ptr(eXit.unkVec3), 3);
		eXit.iRoomREF = bufBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		eXit.unk1C    = bufBinaryReader->read<uint8_t, ZBio::Endianness::LE>();
		eXit.unk1D    = bufBinaryReader->read<uint8_t, ZBio::Endianness::LE>();
		eXit.unkFlags = bufBinaryReader->read<uint8_t, ZBio::Endianness::LE>();
		eXit.unk1F    = bufBinaryReader->read<uint8_t, ZBio::Endianness::LE>();
	}

	void ZRoomExit::deserialize(ZRoomExit& eXit, const Span<uint8_t>& byteBufferSpan)
	{
		if (byteBufferSpan.size() < sizeof(ZRoomExit))
		{
			assert(byteBufferSpan.size() >= sizeof(ZRoomExit) && "Too small span buffer");
			return;
		}

		ZBio::ZBinaryReader::BinaryReader binaryReader(reinterpret_cast<const char*>(byteBufferSpan.data()), byteBufferSpan.size());
		ZRoomExit::deserialize(eXit, &binaryReader);
	}

	void ZRoomNeighbor::deserialize(ZRoomNeighbor& neighbor, ZBio::ZBinaryReader::BinaryReader* bufBinaryReader)
	{
		neighbor.rRoomREF = bufBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		neighbor.unk4 = bufBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
		neighbor.unk8 = bufBinaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}
}