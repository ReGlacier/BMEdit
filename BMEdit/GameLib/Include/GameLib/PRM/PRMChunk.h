#pragma once


#include <memory>

namespace gamelib::prm
{
	using PRMRawChunk = uint8_t*;
	using PRMChunk = std::unique_ptr<uint8_t[]>;
}