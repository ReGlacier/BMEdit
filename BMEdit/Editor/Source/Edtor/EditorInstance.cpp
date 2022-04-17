#include <Editor/EditorInstance.h>
#include <Include/IO/ZIPLevelAssetProvider.h>
#include <GameLib/GMS/GMSStructureError.h>
#include <GameLib/PRP/PRPStructureError.h>
#include <GameLib/TypeNotFoundException.h>
#include <BMEditMainWindow.h>
#include <QApplication>


namespace editor {
	EditorInstance::EditorInstance() : QObject(nullptr)
	{
	}

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

	void EditorInstance::openLevelFromZIP(const std::string &path)
	{
		auto savedLevel = std::move(m_currentLevel);

		auto provider = std::make_unique<ZIPLevelAssetProvider>(path);
		if (!provider)
		{
			levelLoadFailed(QString("Unable to load file %1").arg(QString::fromStdString(path)));
			return;
		}

		if (!provider->isValid())
		{
			levelLoadFailed(QString("Invalid ZIP fs instance!"));
			return;
		}

		m_currentLevel = std::make_unique<gamelib::Level>(std::move(provider));
		if (!m_currentLevel)
		{
			if (savedLevel)
			{
				m_currentLevel = std::move(savedLevel);
			}
			levelLoadFailed(QString("Unable to allocate memory for level"));
			return;
		}

		try {
			if (!m_currentLevel->loadSceneData())
			{
				if (savedLevel)
				{
					m_currentLevel = std::move(savedLevel);
				}
				else
				{
					m_currentLevel = nullptr; // cleanup level to avoid work with invalid level instance
				}
				levelLoadFailed(QString("Unable to load scene data!"));
				return;
			}

			levelLoadSuccess();
		}
		catch (const gamelib::gms::GMSStructureError &gmsStructureError)
		{
			if (savedLevel)
			{
				m_currentLevel = std::move(savedLevel);
			}
			levelLoadFailed(QString("Error in GMS structure: %1").arg(gmsStructureError.what()));
		}
		catch (const gamelib::prp::PRPStructureError &prpStructureError)
		{
			if (savedLevel)
			{
				m_currentLevel = std::move(savedLevel);
			}
			levelLoadFailed(QString("Error in PRP structure: %1").arg(prpStructureError.what()));
		}
		catch (const gamelib::TypeNotFoundException &typeNotFoundException)
		{
			if (savedLevel)
			{
				m_currentLevel = std::move(savedLevel);
			}
			levelLoadFailed(QString("Unable to locate requried type %1").arg(typeNotFoundException.what()));
		}
	}

	const gamelib::Level *EditorInstance::getActiveLevel()
	{
		return m_currentLevel.get();
	}

	void EditorInstance::closeLevel()
	{
		m_currentLevel = nullptr;
	}

	std::unique_ptr<gamelib::Level> EditorInstance::takeLevel()
	{
		return std::move(m_currentLevel);
	}

	void EditorInstance::restoreLevel(std::unique_ptr<gamelib::Level> &&level)
	{
		m_currentLevel = std::move(level);
	}
}