#include <Editor/EditorInstance.h>
#include <Editor/ZIPLevelAssetProvider.h>
#include <BMEditMainWindow.h>
#include <QApplication>


namespace editor {
	EditorInstance &EditorInstance::getInstance()
	{
		static EditorInstance g_editor;
		return g_editor;
	}

	int EditorInstance::run(int argc, char **argv)
	{
		//TODO: Support no-gui mode here
		QScopedPointer<QApplication> app(new QApplication(argc, argv));
		QScopedPointer<QMainWindow> mainWindow(new BMEditMainWindow());

		mainWindow->show();
		return app->exec();
	}

	bool EditorInstance::openLevelFromZIP(const std::string &path)
	{
		auto provider = std::make_unique<ZIPLevelAssetProvider>(path);
		if (!provider)
		{
			return false;
		}

		if (!provider->isValid())
		{
			return false;
		}

		m_currentLevel = std::make_unique<gamelib::Level>(std::move(provider));
		if (!m_currentLevel)
		{
			return false;
		}

		if (!m_currentLevel->loadSceneData())
		{
			m_currentLevel = nullptr; // cleanup level to avoid work with invalid level instance
			return false;
		}

		return true;
	}

	const gamelib::Level *EditorInstance::getActiveLevel()
	{
		return m_currentLevel.get();
	}

	void EditorInstance::closeLevel()
	{
		m_currentLevel = nullptr;
	}
}