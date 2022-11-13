#include <Editor/EditorInstance.h>
#include <QtGlobal>
#include <memory>

#ifdef Q_OS_WIN
#include <Windows.h>

struct Win32EntryPointArgumentsCollector
{
	int argc { 0 };
	char** argv { nullptr };

	Win32EntryPointArgumentsCollector();
	~Win32EntryPointArgumentsCollector();

private:
	LPWSTR* m_collectedArgsW;
};

Win32EntryPointArgumentsCollector::Win32EntryPointArgumentsCollector()
{
	m_collectedArgsW = CommandLineToArgvW(GetCommandLine(), &argc);

	argv = reinterpret_cast<char**>(std::malloc(argc * sizeof(char*)));
	for (int i = 0; i < argc; ++i)
	{
		const auto len = wcslen(m_collectedArgsW[i]) + 1;
		argv[i] = reinterpret_cast<char*>(std::malloc(sizeof(char*) * len));

		std::size_t numChConverted = 0;
		wcstombs_s(&numChConverted, &argv[i][0], len, m_collectedArgsW[i], len);
	}

	LocalFree(m_collectedArgsW);
}

Win32EntryPointArgumentsCollector::~Win32EntryPointArgumentsCollector()
{
	for (int i = 0; i < argc; ++i)
	{
		std::free(argv[i]);
		argv[i] = nullptr;
	}

	std::free(argv);
	argv = nullptr;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine, int nCmdShow) {
	Win32EntryPointArgumentsCollector entryPointArgs {};
	return editor::EditorInstance::getInstance().run(entryPointArgs.argc, entryPointArgs.argv);
}
#else
int main(int argc, char** argv) {
	return editor::EditorInstance::getInstance().run(argc, argv);
}
#endif