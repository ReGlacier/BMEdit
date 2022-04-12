#include <GameLib/NotImplemented.h>


namespace gamelib
{
	NotImplemented::NotImplemented()
		: std::exception("This feature not implemented yet")
	{
	}

	NotImplemented::NotImplemented(const char *message)
		: std::exception(message)
	{
	}
}