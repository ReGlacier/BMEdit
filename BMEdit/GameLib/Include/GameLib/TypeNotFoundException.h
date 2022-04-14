#pragma once

#include <stdexcept>
#include <string>


namespace gamelib
{
	class TypeNotFoundException : public std::exception
	{
	public:
		explicit TypeNotFoundException(std::string message);

		[[nodiscard]] char const *what() const override;

	private:
		std::string m_message;
	};
}