#pragma once

#include <stdexcept>
#include <string>


namespace gamelib::gms
{
	class GMSStructureError : public std::exception
	{
	public:
		explicit GMSStructureError(std::string message);

		[[nodiscard]] char const *what() const noexcept override;
	private:
		std::string m_message;
	};
}