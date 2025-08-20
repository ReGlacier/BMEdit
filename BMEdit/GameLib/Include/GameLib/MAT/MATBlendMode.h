#pragma once

#include <cstdint>


namespace gamelib::mat
{
	enum class MATBlendMode : uint8_t
	{
		BM_TRANS,
		BM_TRANS_ON_OPAQUE,
		BM_TRANSADD_ON_OPAQUE,
		BM_ADD_BEFORE_TRANS,
		BM_ADD_ON_OPAQUE,
		BM_ADD,
		BM_SHADOW,
		BM_STATICSHADOW
	};
}