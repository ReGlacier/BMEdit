#include <GameLib/PRP/PRPOpCode.h>

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
		bool areOpCodesHasSameKind(PRPOpCode first, PRPOpCode second)
		{
			if (first == second)
			{
				return true;
			}

#define SAME(a, b) if ((first == (a)) && (second == (b)) || (first == (b)) && (second == (a))) { return true; }
			SAME(PRPOpCode::Array, PRPOpCode::NamedArray);
			SAME(PRPOpCode::BeginObject, PRPOpCode::BeginNamedObject);
			SAME(PRPOpCode::Container, PRPOpCode::NamedContainer);
			SAME(PRPOpCode::Char, PRPOpCode::NamedChar);
			SAME(PRPOpCode::Bool, PRPOpCode::NamedBool);
			SAME(PRPOpCode::Int8, PRPOpCode::NamedInt8);
			SAME(PRPOpCode::Int16, PRPOpCode::NamedInt16);
			SAME(PRPOpCode::Int32, PRPOpCode::NamedInt32);
			SAME(PRPOpCode::Float32, PRPOpCode::NamedFloat32);
			SAME(PRPOpCode::Float64, PRPOpCode::NamedFloat64);
			SAME(PRPOpCode::String, PRPOpCode::NamedString);
			SAME(PRPOpCode::RawData, PRPOpCode::NamedRawData);
			SAME(PRPOpCode::Reference, PRPOpCode::NamedReference);
			SAME(PRPOpCode::Bitfield, PRPOpCode::NameBitfield);
			SAME(PRPOpCode::StringOrArray_E, PRPOpCode::StringOrArray_8E);
#undef SAME
			return false;
		}

		PRPOpCode fromString(const std::string &asString)
		{
#define OPSW(x) if (asString.find(#x) != std::string::npos) return PRPOpCode::x;
			OPSW(Array)
			OPSW(BeginObject)
			OPSW(Reference)
			OPSW(Container)
			OPSW(Char)
			OPSW(Bool)
			OPSW(Int8)
			OPSW(Int16)
			OPSW(Int32)
			OPSW(Float32)
			OPSW(Float64)
			OPSW(String)
			OPSW(RawData)
			OPSW(Bitfield)
			OPSW(EndArray)
			OPSW(SkipMark)
			OPSW(EndObject)
			OPSW(EndOfStream)
			OPSW(NamedArray)
			OPSW(BeginNamedObject)
			OPSW(NamedReference)
			OPSW(NamedContainer)
			OPSW(NamedChar)
			OPSW(NamedBool)
			OPSW(NamedInt8)
			OPSW(NamedInt16)
			OPSW(NamedInt32)
			OPSW(NamedFloat32)
			OPSW(NamedFloat64)
			OPSW(NamedString)
			OPSW(NamedRawData)
			OPSW(NameBitfield)
			OPSW(StringOrArray_E)
			OPSW(StringOrArray_8E)
			OPSW(StringArray)
#undef OPSW
			return PRPOpCode::ERR_UNKNOWN;
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