#pragma once

#include <GameLib/LOC/LOCTreeNode.h>
#include <cstdint>
#include <string>


namespace gamelib::loc
{
	class LOCReader
	{
	public:
		LOCReader();

		bool parse(const uint8_t *locFileBuffer, int64_t locFileSize);

		[[nodiscard]] const LOCTreeNode::Ptr& getRoot() const;

	private:
		LOCTreeNode::Ptr m_pRoot { nullptr };
	};
}