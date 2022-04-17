#pragma once

#include <QObject>

#include <GameLib/Level.h>


namespace editor {
	class EditorInstance : public QObject {
		Q_OBJECT

		EditorInstance();

	public:
		EditorInstance(const EditorInstance &) = delete;
		EditorInstance(EditorInstance &&) = delete;
		EditorInstance& operator=(const EditorInstance &) = delete;
		EditorInstance& operator=(EditorInstance &&) = delete;

		static EditorInstance &getInstance();

		int run(int argc, char** argv);

		void openLevelFromZIP(const std::string &path);
		void closeLevel();

		const gamelib::Level *getActiveLevel();

		[[nodiscard]] std::unique_ptr<gamelib::Level> takeLevel();
		void restoreLevel(std::unique_ptr<gamelib::Level> &&level);

	signals:
		void levelLoadSuccess();
		void levelLoadProgressChanged(int totalPercentsProgress, const QString &currentOperationTag);
		void levelLoadFailed(const QString &reason);

	private:
		std::unique_ptr<gamelib::Level> m_currentLevel;
	};
}