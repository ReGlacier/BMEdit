#pragma once

#include <BMRender/ILogInterface.h>
#include <memory>


namespace bmr
{
	class Logger
	{
		static std::unique_ptr<ILogInterface> g_logInterface;

	public:
		static void setInterface(std::unique_ptr<ILogInterface> &&logInterface);
		static void logMessage(Level logLevel, std::string &&message);
	};
}