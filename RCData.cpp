#include "RCData.h"
#include "utils.h"
#include <string>

// Useful demonstration: https://www.codeproject.com/articles/4945/updateresource

void PutFileInRCData(PCHAR file, PCHAR target, int id) {
	HANDLE hFile;
	DWORD dwFileSize;
	DWORD dwBytesRead;
	LPBYTE lpBuffer = NULL;

	hFile = CreateFile(file, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		dwFileSize = GetFileSize(hFile, NULL);
		lpBuffer = new BYTE[dwFileSize];

		if (ReadFile(hFile, lpBuffer, dwFileSize, &dwBytesRead, NULL) != FALSE) {
			HANDLE hResource = BeginUpdateResource(target, FALSE);
			if (hResource != NULL) {
				if (UpdateResource(hResource, RT_RCDATA, MAKEINTRESOURCE(id), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPVOID)lpBuffer, dwFileSize) != FALSE) {
					EndUpdateResource(hResource, FALSE);
				}
			}
		}

		delete[] lpBuffer;
		CloseHandle(hFile);
	}
}

void RunFileInRCData(PCHAR target, int id, BOOL isPython) {
	HMODULE hLibrary;
	HRSRC hResource = NULL;
	HGLOBAL hResourceLoaded;
	LPBYTE lpBuffer = NULL;

	hLibrary = LoadLibraryEx(target, NULL, LOAD_LIBRARY_AS_DATAFILE);

	if (hLibrary != NULL) {
		hResource = FindResource(hLibrary, MAKEINTRESOURCE(id), RT_RCDATA);
		if (hResource != NULL) {
			hResourceLoaded = LoadResource(hLibrary, hResource);
			if (hResourceLoaded != NULL) {
				lpBuffer = (LPBYTE)LockResource(hResourceLoaded);
				if (lpBuffer != NULL)
				{
					DWORD dwFileSize;
					DWORD dwBytesWritten;
					HANDLE hFile;

					dwFileSize = SizeofResource(hLibrary, hResource);
					
					std::string tmpFile;
					GetTemporaryFilePath(isPython ? "pyw" : "exe", tmpFile);

					hFile = CreateFile(tmpFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

					if (hFile != INVALID_HANDLE_VALUE) {
						WriteFile(hFile, lpBuffer, dwFileSize, &dwBytesWritten, NULL);
						CloseHandle(hFile);

						if (isPython) {
							char cmd[MAX_PATH] = "pythonw \"";
							strcat_s(cmd, tmpFile.c_str());
							strcat_s(cmd, "\"");
							SystemNoShell(cmd);
						} else {
							// 1st param for start is title; just pass empty string for no title
							// start <title> /d <cwd> <file>
							char cmd[MAX_PATH] = "start \"\" /d \"%cd%\" \"";
							strcat_s(cmd, tmpFile.c_str());
							strcat_s(cmd, "\"");
							SystemNoShell(cmd);
						}
					}
				}
			}
		}

		FreeLibrary(hLibrary);
	}
}