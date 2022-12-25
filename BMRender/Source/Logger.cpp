#include <BMRender/Logger.h>


namespace bmr
{
	std::unique_ptr<ILogInterface> Logger::g_logInterface = nullptr;

	void Logger::setInterface(std::unique_ptr<ILogInterface> &&logInterface)
	{
		g_logInterface = std::move(logInterface);
	}

	void Logger::logMessage(Level logLevel, std::string &&message)
	{
		if (g_logInterface)
		{
			g_logInterface->logMessage(logLevel, std::move(message));
		}
	}
}