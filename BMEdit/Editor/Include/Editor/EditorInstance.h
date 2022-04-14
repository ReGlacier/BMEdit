#pragma once

#include <GameLib/Level.h>


namespace editor {
	class EditorInstance {
		EditorInstance() = default;

	public:
		EditorInstance(const EditorInstance &) = delete;
		EditorInstance(EditorInstance &&) = delete;
		EditorInstance& operator=(const EditorInstance &) = delete;
		EditorInstance& operator=(EditorInstance &&) = delete;

		static EditorInstance &getInstance();

		int run(int argc, char** argv);

		bool openLevelFromZIP(const std::string &path);
		void closeLevel();

		const gamelib::Level *getActiveLevel();
	private:
		std::unique_ptr<gamelib::Level> m_currentLevel;
	};
}