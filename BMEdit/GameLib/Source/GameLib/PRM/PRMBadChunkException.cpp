#include <GameLib/PRM/PRMBadChunkException.h>
#include <string>


namespace gamelib::prm
{
	PRMBadChunkException::PRMBadChunkException(std::uint32_t chunkIndex) : PRMException()
	{
		m_errorMessage = "Bad chunk #" + std::to_string(chunkIndex);
	}

	const char *PRMBadChunkException::what() const
	{
		return m_errorMessage.data();
	}
}