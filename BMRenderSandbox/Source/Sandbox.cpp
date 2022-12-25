#include <QApplication>
#include <QSurfaceFormat>
#include <BMRender/Logger.h>
#include <BMRenderSandbox/SandboxLogger.h>

#include <SandboxMainWindow.h>


int main(int argc, char** argv)
{
	QApplication application(argc, argv);

	bmr::Logger::setInterface(std::make_unique<SandboxLogger>());

	QSurfaceFormat format;
	format.setVersion(3, 3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);

	SandboxMainWindow window;
	window.show();

	return QGuiApplication::exec();
}