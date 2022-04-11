#pragma once

#include <cstdint>
#include <string>
#include <GameLib/FromBytes.h>


namespace gamelib::prp {
	enum class PRPOpCode : unsigned int {
		Array = 0x1,
		BeginObject = 0x2,
		Reference = 0x3,
		Container = 0x4,
		Char = 0x5,
		Bool = 0x6,
		Int8 = 0x7,
		Int16 = 0x8,
		Int32 = 0x9,
		Float32 = 0xA,
		Float64 = 0xB,
		String = 0xC,
		RawData = 0xD,
		Bitfield = 0x10,
		EndArray = 0x7C,
		SkipMark = 0x7D,
		EndObject = 0x7E,
		EndOfStream = 0x7F,
		NamedArray = 0x81,
		BeginNamedObject = 0x82,
		NamedReference = 0x83,
		NamedContainer = 0x84,
		NamedChar = 0x85,
		NamedBool = 0x86,
		NamedInt8 = 0x87,
		NamedInt16 = 0x88,
		NamedInt32 = 0x89,
		NamedFloat32 = 0x8A,
		NamedFloat64 = 0x8B,
		NamedString = 0x8C,
		NamedRawData = 0x8D,
		NameBitfield = 0x8F,
		StringOrArray_E = 0xE,
		StringOrArray_8E = 0x8E,
		StringArray = 0xF,
		// System
		ERR_UNKNOWN = 0xFFFD,
		ERR_NO_TAG = 0xFFFE
	};

	std::string to_string(PRPOpCode opCode);
}

namespace gamelib {
	template <>
	struct FromBytes<prp::PRPOpCode>
	{
		prp::PRPOpCode operator()(uint8_t bytes);
	};
}