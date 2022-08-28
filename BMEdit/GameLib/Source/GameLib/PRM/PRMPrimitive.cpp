#include <GameLib/PRM/PRMPrimitive.h>
#include <ZBinaryReader.hpp>


namespace gamelib::prm
{
	void PRMPrimitive::deserialize(gamelib::prm::PRMPrimitive &primitive, ZBio::ZBinaryReader::BinaryReader *binaryReader)
	{
		auto &[bonesOffset, primPacKType, kind, textureId, unk6, nextVarOff, unkC, unkD, unkE, currentVarIdx, unk10, matIdx, totalVars] = primitive;

		bonesOffset = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		primPacKType = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		kind = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		textureId = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		unk6 = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		nextVarOff = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
		unkC = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		unkD = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		unkE = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		currentVarIdx = binaryReader->read<uint8_t, ZBio::Endianness::LE>();
		unk10 = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		matIdx = binaryReader->read<uint16_t, ZBio::Endianness::LE>();
		totalVars = binaryReader->read<uint32_t, ZBio::Endianness::LE>();
	}
}