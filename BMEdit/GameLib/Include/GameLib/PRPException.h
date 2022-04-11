#pragma once

#include <stdexcept>
#include <string>

#include <GameLib/PRPRegionID.h>


namespace gamelib::prp
{
	class PRPException : public std::exception
	{
	public:
		PRPException(const std::string &message, PRPRegionID region, int opCodeIndex = -1);

		[[nodiscard]] char const* what() const override;

	private:
		std::string m_errorMessage;
	};
}