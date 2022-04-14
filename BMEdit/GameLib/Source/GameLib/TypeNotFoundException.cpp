#include <GameLib/TypeNotFoundException.h>

namespace gamelib
{
	TypeNotFoundException::TypeNotFoundException(std::string message)
		: std::exception(""), m_message(std::move(message))
	{
	}

	const char *TypeNotFoundException::what() const
	{
		return m_message.data();
	}
}