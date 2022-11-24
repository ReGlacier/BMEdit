#pragma once

#include <GameLib/PRM/PRMException.h>


namespace gamelib::prm
{
	class PRMBadFile : public PRMException
	{
	public:
		explicit PRMBadFile(const std::string& reason);

		[[nodiscard]] char const* what() const override;
	private:
		std::string m_message;
	};
}