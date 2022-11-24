#include <GameLib/PRM/PRMBadFile.h>


namespace gamelib::prm
{
	PRMBadFile::PRMBadFile(const std::string& reason) : PRMException()
	{
		m_message = "Bad file: " + reason;
	}

	const char *PRMBadFile::what() const
	{
		return m_message.data();
	}
}