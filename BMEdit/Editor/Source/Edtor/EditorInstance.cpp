#include <Editor/EditorInstance.h>
#include <AssetProvider/ZIPLevelAssetProvider.h>
#include <GameLib/GMS/GMSStructureError.h>
#include <GameLib/PRP/PRPStructureError.h>
#include <GameLib/Scene/SceneObjectVisitorException.h>
#include <GameLib/TypeNotFoundException.h>
#include <BMEditMainWindow.h>

#include <QApplication>
#include <QFile>


namespace editor {
	class LevelBackup
	{
		// Pointers
		std::unique_ptr<gamelib::Level> *m_sourceLevelInstance;
		std::string *m_sourceLevelPath;

		// Holders
		std::unique_ptr<gamelib::Level> m_levelInstance;
		std::string m_levelPath;

		// Flags
		bool m_shouldRestore{ true };

	public:
		LevelBackup(std::unique_ptr<gamelib::Level> *levelInstance, std::string *levelPath)
			: m_sourceLevelInstance(levelInstance), m_sourceLevelPath(levelPath)
		{
			m_levelInstance = std::move(*levelInstance);
			m_levelPath = std::move(*levelPath);
		}

		~LevelBackup()
		{
			if (m_shouldRestore)
			{
				*m_sourceLevelInstance = std::move(m_levelInstance);
				*m_sourceLevelPath = std::move(m_levelPath);
			}
		}

		LevelBackup(const LevelBackup&) = delete;
		LevelBackup(LevelBackup&&) = delete;
		LevelBackup& operator=(const LevelBackup&) = delete;
		LevelBackup& operator=(LevelBackup&&) = delete;

		void decline()
		{
			m_shouldRestore = false;
		}
	};

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
		Q_INIT_RESOURCE(BMEdit);

		//TODO: Support no-gui mode here
		QScopedPointer<QApplication> app(new QApplication(argc, argv));
		QScopedPointer<QMainWindow> mainWindow(new BMEditMainWindow());

		mainWindow->show();
		return app->exec();
	}

	void EditorInstance::openLevelFromZIP(const std::string &path)
	{
		LevelBackup levelBackup { &m_currentLevel, &m_currentLevelPath };

		auto provider = std::make_unique<ap::ZIPLevelAssetProvider>(path);
		if (!provider)
		{
			levelLoadFailed(QString("Unable to load file %1").arg(QString::fromStdString(path)));
			return;
		}

		if (!provider->isValid())
		{
			levelLoadFailed(QString("Invalid ZIP file instance!"));
			return;
		}

		m_currentLevel = std::make_unique<gamelib::Level>(std::move(provider));
		if (!m_currentLevel)
		{
			levelLoadFailed(QString("Unable to allocate memory for level"));
			return;
		}

#ifndef BMEDIT_DEBUG
		try
#endif
		{
			if (!m_currentLevel->loadSceneData())
			{
				levelLoadFailed(QString("Unable to load scene data!"));
				return;
			}

			levelBackup.decline(); // Destroy previous instance of level
			m_currentLevelPath = path; // Store path to level
			levelLoadSuccess();
		}
#ifndef BMEDIT_DEBUG
		catch (const gamelib::gms::GMSStructureError &gmsStructureError)
		{
			levelLoadFailed(QString("Error in GMS structure: %1").arg(gmsStructureError.what()));
		}
		catch (const gamelib::prp::PRPStructureError &prpStructureError)
		{
			levelLoadFailed(QString("Error in PRP structure: %1").arg(prpStructureError.what()));
		}
		catch (const gamelib::TypeNotFoundException &typeNotFoundException)
		{
			levelLoadFailed(QString("Unable to locate requried type %1").arg(typeNotFoundException.what()));
		}
		catch (const gamelib::scene::SceneObjectVisitorException &sceneObjectException)
		{
			levelLoadFailed(QString("Unable to visit geom on scene: %1").arg(sceneObjectException.what()));
		}
		catch (const std::runtime_error &runtimeFailure)
		{
			levelLoadFailed(QString("RUNTIME ERROR: %1").arg(runtimeFailure.what()));
		}
#endif
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

	void EditorInstance::exportAsset(gamelib::io::AssetKind assetKind)
	{
//		auto provider = std::make_unique<ap::ZIPLevelAssetProvider>(m_currentLevelPath);
//		if (!provider)
//		{
//			emit exportAssetFailed(QString("Failed to open '%1' ZIP file").arg(QString::fromStdString(m_currentLevelPath)));
//			return;
//		}
//
//		if (!provider->isValid() || !provider->isEditable())
//		{
//			emit exportAssetFailed(QString("Invalid IO provider state (not valid or not editable) of ZIP file '%1'").arg(QString::fromStdString(m_currentLevelPath)));
//			return;
//		}
//
//		//TODO: Rewrite
//		std::array<uint8_t, 10> bytes = {
//		    'H', 'E', 'L', 'L', '0', ' ', 'I', 'O', 'I', '!'
//		};
//
//		const auto result = provider->saveAsset(assetKind, gamelib::Span(bytes));
//
//		if (!result)
//		{
//			emit exportAssetFailed(QString("saveAsset() failed"));
//		}
//		else
//		{
//			emit exportAssetSuccess(assetKind, QString("N/A"));
//		}
	}

	bool EditorInstance::exportPRP(const QString &filePath)
	{
		std::vector<uint8_t> prpFileBuffer;

		if (!m_currentLevel)
		{
			return false;
		}

		QFile prpFile(filePath);
		if (!prpFile.open(QIODeviceBase::OpenModeFlag::WriteOnly | QIODeviceBase::OpenModeFlag::Truncate | QIODeviceBase::OpenModeFlag::Unbuffered))
		{
			return false;
		}

		m_currentLevel->dumpAsset(gamelib::io::AssetKind::PROPERTIES, prpFileBuffer);

		if (prpFileBuffer.empty())
		{
			return false;
		}

		QByteArray raw(reinterpret_cast<const char*>(prpFileBuffer.data()), static_cast<qsizetype>(prpFileBuffer.size()));
		prpFile.write(raw);

		return true;
	}
}