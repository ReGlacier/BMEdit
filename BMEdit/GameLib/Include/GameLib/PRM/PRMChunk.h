#pragma once


#include <memory>

namespace gamelib::prm
{
	struct PRMChunk
	{
		std::unique_ptr<uint8_t[]> buffer { nullptr };
		std::size_t bufferSize { 0 };
	};
}