#include "utils.h"
#include <windows.h>

void SystemNoShell(const char *cmd) {
	STARTUPINFOA startup_info = { 0 };
	LPSTARTUPINFOA p_startup_info = &startup_info;
	PROCESS_INFORMATION proc_info = { 0 };
	LPPROCESS_INFORMATION p_proc_info = &proc_info;

	char cmd_path[MAX_PATH];
	GetEnvironmentVariableA("COMSPEC", cmd_path, MAX_PATH);

	char cmd_args[MAX_PATH] = { 0 };
	strcat_s(cmd_args, " /c ");
	strcat_s(cmd_args, cmd);

	bool process_created = CreateProcess(
		cmd_path,
		cmd_args,
		NULL,
		NULL,
		FALSE,
		DETACHED_PROCESS,
		NULL,
		NULL,
		p_startup_info,
		p_proc_info
	);

	/*if (WaitForSingleObject(proc_info.hThread, INFINITE) > 0) {
		WaitForSingleObject(proc_info.hProcess, INFINITE); // wait till process completes
		CloseHandle(proc_info.hProcess); // avoid memory leak
	}*/
}

void GetTemporaryFilePath(std::string fileExtension, std::string& filePath)
{
	// Useful link: https://www.codeproject.com/tips/314160/generate-temporary-files-with-any-extension

	TCHAR lpszTempPath[MAX_PATH] = { 0 };
	GetTempPath(MAX_PATH, lpszTempPath); // 1st param is length of buffer

	TCHAR lpszFilePath[MAX_PATH] = { 0 };
	GetTempFileName(lpszTempPath, NULL, 0, lpszFilePath); // <path>\<pre><uuuu>.TMP
	filePath = lpszFilePath;
	std::remove(filePath.c_str()); // delete .tmp file -- was only used to get the temp filename
	filePath.replace(filePath.find(".tmp") + 1, fileExtension.length(), fileExtension); // <path>\<pre><uuuu>.<new-extension>
}
