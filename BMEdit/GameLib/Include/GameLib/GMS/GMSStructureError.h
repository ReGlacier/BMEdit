#pragma once

#include <stdexcept>


namespace gamelib::gms
{
	class GMSStructureError : public std::exception
	{
	public:
		explicit GMSStructureError(const char *message);
	};
}