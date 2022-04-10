#pragma once

namespace editor {
	class EditorInstance {
	public:
		EditorInstance() = default;

		int run(int argc, char** argv);
	};
}