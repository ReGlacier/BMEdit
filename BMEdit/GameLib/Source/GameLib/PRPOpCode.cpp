#include <GameLib/PRPOpCode.h>

namespace gamelib
{
	namespace prp
	{
		std::string to_string(PRPOpCode opCode)
		{
#define OPC(x) if (opCode == PRPOpCode::x) { return #x; }
			OPC(Array)
			OPC(BeginObject)
			OPC(Reference)
			OPC(Container)
			OPC(Char)
			OPC(Bool)
			OPC(Int8)
			OPC(Int16)
			OPC(Int32)
			OPC(Float32)
			OPC(Float64)
			OPC(String)
			OPC(RawData)
			OPC(Bitfield)
			OPC(EndArray)
			OPC(SkipMark)
			OPC(EndObject)
			OPC(EndOfStream)
			OPC(NamedArray)
			OPC(BeginNamedObject)
			OPC(NamedReference)
			OPC(NamedContainer)
			OPC(NamedChar)
			OPC(NamedBool)
			OPC(NamedInt8)
			OPC(NamedInt16)
			OPC(NamedInt32)
			OPC(NamedFloat32)
			OPC(NamedFloat64)
			OPC(NamedString)
			OPC(NamedRawData)
			OPC(NameBitfield)
			OPC(StringOrArray_E)
			OPC(StringOrArray_8E)
			OPC(StringArray)
			return "Undefined";
#undef OPC
		}
	}

	prp::PRPOpCode FromBytes<prp::PRPOpCode>::operator()(uint8_t byte)
	{
		using prp::PRPOpCode;

		if (byte == 0x0E) return PRPOpCode::StringOrArray_E;
		if (byte == 0x8E) return PRPOpCode::StringOrArray_8E;
		if (byte < static_cast<uint8_t>(PRPOpCode::Array) || byte > static_cast<uint8_t>(PRPOpCode::NameBitfield)) {
			return PRPOpCode::ERR_NO_TAG;
		}

		if (byte == 128 || (byte >= 16 && byte <= 123)) {
			return PRPOpCode::ERR_UNKNOWN;
		}

		return static_cast<PRPOpCode>(byte);
	}
}