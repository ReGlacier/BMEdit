#include <Editor/EditorInstance.h>
#include <memory>

int main(int argc, char** argv) {
	return editor::EditorInstance::getInstance().run(argc, argv);
}