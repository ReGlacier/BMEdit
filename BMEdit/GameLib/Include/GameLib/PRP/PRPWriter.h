#pragma once

#include <cstdint>
#include <vector>

#include <GameLib/PRP/PRPInstruction.h>
#include <GameLib/PRP/PRPZDefines.h>


namespace gamelib::prp
{
	class PRPWriter
	{
	public:
		PRPWriter() = default;

		static void write(const PRPZDefines &definitions, const std::vector<PRPInstruction> &instructions, bool isRaw, std::vector<uint8_t> &outBuffer);
	};
}