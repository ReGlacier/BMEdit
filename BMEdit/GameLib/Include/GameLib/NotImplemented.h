#pragma once

#include <stdexcept>


namespace gamelib
{
	class NotImplemented final : public std::exception
	{
	public:
		NotImplemented();
		explicit NotImplemented(const char *message);
	};
}