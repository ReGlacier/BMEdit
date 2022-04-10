#include <Editor/EditorInstance.h>

int main(int argc, char** argv) {
	editor::EditorInstance instance {};
	return instance.run(argc, argv);
}