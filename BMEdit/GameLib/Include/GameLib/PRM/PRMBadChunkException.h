#pragma once

#include <GameLib/PRM/PRMException.h>


namespace gamelib::prm
{
	class PRMBadChunkException : public PRMException
	{
	public:
		explicit PRMBadChunkException(int chunkIndex);

		[[nodiscard]] char const* what() const override;

	private:
		std::string m_errorMessage;
	};
}