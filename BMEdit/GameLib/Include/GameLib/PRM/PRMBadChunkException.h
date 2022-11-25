#pragma once

#include <GameLib/PRM/PRMException.h>
#include <cstdint>


namespace gamelib::prm
{
	class PRMBadChunkException : public PRMException
	{
	public:
		explicit PRMBadChunkException(std::uint32_t chunkIndex);

		[[nodiscard]] char const* what() const override;

	private:
		std::string m_errorMessage;
	};
}