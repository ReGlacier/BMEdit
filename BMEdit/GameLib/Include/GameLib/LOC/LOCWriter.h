#pragma once

#include <GameLib/LOC/LOCTreeNode.h>
#include <cstdint>
#include <string>


namespace gamelib::loc
{
	class LOCWriter
	{
	public:
		static void write(const LOCTreeNode::Ptr& root, std::vector<uint8_t> &outBuffer);
	};
}