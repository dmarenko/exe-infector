#include "copyIcon.h"

/*
	The purpose of this library is to copy the icon from one exe to another exe (and some file metadata like name, version, copyright).


	Every exe has resources in it. Using Resource Hacker, we can open various exe's and get an impression of how it looks.
	The resources we're interested in will mostly be the app icons (and a few others). We will use the Windows API to do so.

	Every exe generally has a few app icons (for different resolutions),
	   and maybe a few other icons which are used inside the software itself (e.g. menu icon).


	<source.exe> <destination.exe>

	Steps:

	1. Delete MUI resource in destination.
	
	See: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-updateresourcew

	MUI seems to be some kind of manifest which can have resource updating restrictions,
	  which seems to sometimes cause an attempt to delete a resource to fail.
	I've learned about this issue when trying to delete the icons from cmd.exe.
	In my testing, I couldn't delete the icons from cmd unless I deleted MUI first.
	
	2. Delete the resources icons and icon groups in destination.

	Every resource in an exe has a language value. This allows developers to have different resources for different languages.
	Looking at various exe's using Resource Hacker, the convention seems to be, that app icons always have a language a set (non-zero value).
	Common language value I've seen is 1033 which is US English: MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US) == 1033
	But icons which are used for the software itself, like a menu icon, the language value is set to zero.
	Following this convention, we only delete icons and group icons which have the language set to zero.
	  (so we don't delete icons which the program uses inside its user interface)
	There's no guarantee the copied icon resources from the source exe will perfectly overwrite the icon resources in the destination exe,
	  so the old icon resources might be mixed with the new icon resources, so it seems like a good idea to delete before copying.

	3. Copy from the source the icons, group icons, metadata (optional) into destination.

	The destination now has a clean slate, and doesn't have any previous icon data which could've interferred.
	While it is true the convention seems to be that there's a language set only for app icons, there can always be outliers.
	For deleting, I had no choice but to rely on the language value, but for copying the new icons over,
	  it seems better to just copy everything, regardless of language, in the case that the source exe
	  is an outlier and has language set to zero for its app icons.
	
*/

void DeleteIcon(PCHAR target) {
	HMODULE hFromModule;
	HANDLE hToHandle;

	// For reading resources you need to load the exe (for writing you only need BeginUpdateResource, UpdateResource, EndUpdateResource).
	// We will load the exe to find the resources we need to delete, and to do the deleting we'll use the 3 functions mentioned above.
	hFromModule = LoadLibraryEx(target, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hFromModule == NULL)
	{
		return;
	}

	hToHandle = BeginUpdateResource(target, FALSE);
	if (hToHandle == NULL)
	{
		FreeLibrary(hFromModule);
		return;
	}

	auto callback = [](HMODULE hModule, LPCTSTR lpType, LPTSTR lpName, LONG_PTR lParam) -> BOOL {
		EnumResourceLanguages(hModule, lpType, lpName, (ENUMRESLANGPROC)([](HMODULE hModule, LPCTSTR lpType, LPCTSTR lpName, WORD wLang, LONG_PTR lParam) -> BOOL {			
			if (wLang != 0 || strcmp(lpType, "MUI") == 0) { // (for MUI always delete, regardless of language)
				UpdateResource((HANDLE)lParam, lpType, lpName, wLang, NULL, NULL); // passing NULL, NULL means to delete the resource
			}
			return 1; // (returning 0 stops the enumeration)
		}), lParam);
		return 1;
	};

	// See: https://docs.microsoft.com/en-us/windows/win32/menurc/using-resources
	// All resources in an exe file can be traversed/enumerated in the following nested way:
	// We call EnumResourceTypes and for every enumerated type we call EnumResourceNames
	//   and for every enumerated name we call EnumResourceLanguages.
	// This will enumerate through all the resources in the exe.
	// I only need to enumerate through specific types (MUI, RT_ICON, RT_GROUP_ICON),
	//   so I do the enumeration only for "names -> langs" (instead of "types -> names -> langs").

	EnumResourceNames(hFromModule, "MUI", (ENUMRESNAMEPROC)callback, (LONG_PTR)hToHandle);
	// https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-updateresourcew
	EnumResourceNames(hFromModule, RT_ICON, (ENUMRESNAMEPROC)callback, (LONG_PTR)hToHandle);
	EnumResourceNames(hFromModule, RT_GROUP_ICON, (ENUMRESNAMEPROC)callback, (LONG_PTR)hToHandle);

	// See: https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-updateresourcea
	// "Before you call this function, make sure all file handles other than the one returned by BeginUpdateResource are closed."
	// In other words, we must call FreeLibrary BEFORE calling EndUpdateResource - otherwise updating can fail.
	FreeLibrary(hFromModule);
	EndUpdateResource(hToHandle, FALSE); // changes apply only after calling this function
	
}

void CopyIcon(PCHAR from, PCHAR to) {
	HMODULE hFromModule;
	HANDLE hToHandle;

	hFromModule = LoadLibraryEx(from, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hFromModule == NULL)
	{
		return;
	}

	hToHandle = BeginUpdateResource(to, FALSE);
	if (hToHandle == NULL)
	{
		FreeLibrary(hFromModule);
		return;
	}

	auto callback = [](HMODULE hModule, LPCTSTR lpType, LPTSTR lpName, LONG_PTR lParam) -> BOOL {
		EnumResourceLanguages(hModule, lpType, lpName, (ENUMRESLANGPROC)([](HMODULE hModule, LPCTSTR lpType, LPCTSTR lpName, WORD wLang, LONG_PTR lParam) -> BOOL {
			HRSRC hRes = FindResourceEx(hModule, lpType, lpName, wLang);
			UpdateResource((HANDLE)lParam, lpType, lpName, wLang, LockResource(LoadResource(hModule, hRes)), SizeofResource(hModule, hRes));
			return 1;
		}), lParam);
		return 1;
	};

	EnumResourceNames(hFromModule, RT_ICON, (ENUMRESNAMEPROC)callback, (LONG_PTR)hToHandle);
	EnumResourceNames(hFromModule, RT_GROUP_ICON, (ENUMRESNAMEPROC)callback, (LONG_PTR)hToHandle);
	// Copies file details - name, description, version, copyright, etc. (this is optional, but makes the imitation exe look more like the original)
	// There's probably no need to delete RT_VERSION before setting it -- in Resource Hacker, the convention seems to be a single resource entry for it,
	//   so it can just be replaced (for icons it's better to delete them completely first)
	EnumResourceNames(hFromModule, RT_VERSION, (ENUMRESNAMEPROC)callback, (LONG_PTR)hToHandle);

	FreeLibrary(hFromModule);

	EndUpdateResource(hToHandle, FALSE);

}

void ReplaceIcon(PCHAR from, PCHAR to) {
	DeleteIcon(to); // delete icon data in target (creates clean slate to work in)
	CopyIcon(from, to); // copy the icon data to target
}
