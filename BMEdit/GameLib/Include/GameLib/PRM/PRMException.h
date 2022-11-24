#pragma once

#include <stdexcept>


namespace gamelib::prm
{
	class PRMException : public std::exception
	{
	public:
		using std::exception::exception;
	};
}