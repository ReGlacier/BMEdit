#pragma once

#include <cstdint>


namespace gamelib::mat
{
	enum class MATCullMode : uint8_t
	{
		CM_DontCare,
		CM_OneSided,
		CM_TwoSided
	};
}