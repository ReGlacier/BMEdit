#include <GameLib/PRPOpCode.h>

namespace gamelib {
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