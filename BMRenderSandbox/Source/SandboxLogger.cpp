#include <BMRenderSandbox/SandboxLogger.h>
#include <QDebug>


void SandboxLogger::logMessage(bmr::Level level, std::string &&message)
{
	switch (level)
	{
		case bmr::Level::LL_INFO:
		    qInfo() << QString::fromStdString(message);
		    break;
		case bmr::Level::LL_WARNING:
		    qWarning() << QString::fromStdString(message);
		    break;
		case bmr::Level::LL_ERROR:
		    qCritical() << QString::fromStdString(message);
		    break;
		case bmr::Level::LL_ASSERT:
		    qCritical() << QString::fromStdString(message);
		    assert(false);
		    break;
	}
}