#pragma once

#include <cstdint>

namespace gamelib {
	template <typename T>
	struct FromBytes {
		T operator()(uint8_t byte);
	};
}