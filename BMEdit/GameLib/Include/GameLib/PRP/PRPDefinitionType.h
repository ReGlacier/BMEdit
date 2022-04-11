#pragma once

#include <cstdint>
#include <GameLib/FromBytes.h>


namespace gamelib::prp {
	enum class PRPDefinitionType : unsigned short {
		Array_Int32 = 2,
		Array_Float32 = 3,
		StringRef_1 = 0xC,
		StringRef_2 = 0xE,
		StringRef_3 = 0x10,
		StringRefTab = 0x11,
		ERR_UNKNOWN = 0xFFFF
	};
}

namespace gamelib {
	template <>
	struct FromBytes<prp::PRPDefinitionType>
	{
		prp::PRPDefinitionType operator()(uint8_t byte);
	};
}