#include <Editor/EditorInstance.h>
#include <BMEditMainWindow.h>
#include <QApplication>


namespace editor {
	int EditorInstance::run(int argc, char **argv)
	{
		//TODO: Support no-gui mode here
		QScopedPointer<QApplication> app(new QApplication(argc, argv));
		QScopedPointer<QMainWindow> mainWindow(new BMEditMainWindow());

		mainWindow->show();
		return app->exec();
	}
}