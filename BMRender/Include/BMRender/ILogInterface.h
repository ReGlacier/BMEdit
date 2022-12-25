#pragma once

#include <string>
#include <memory>


namespace bmr
{
	enum class Level
	{
		LL_INFO,
		LL_WARNING,
		LL_ERROR,
		LL_ASSERT
	};

	class ILogInterface
	{
	public:
		virtual ~ILogInterface() noexcept = default;

		virtual void logMessage(Level level, std::string &&message) = 0;
	};
}