#include <GameLib/PRPDefinitionType.h>

namespace gamelib {
	prp::PRPDefinitionType FromBytes<prp::PRPDefinitionType>::operator()(uint8_t byte)
	{
		if (byte == 2) return prp::PRPDefinitionType::Array_Int32;
		if (byte == 3) return prp::PRPDefinitionType::Array_Float32;
		if (byte == 0xC) return prp::PRPDefinitionType::StringRef_1;
		if (byte == 0xE) return prp::PRPDefinitionType::StringRef_2;
		if (byte == 0x10) return prp::PRPDefinitionType::StringRef_3;
		if (byte == 0x11) return prp::PRPDefinitionType::StringRefTab;
		return prp::PRPDefinitionType::ERR_UNKNOWN;
	}
}