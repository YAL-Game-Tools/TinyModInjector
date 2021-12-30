/// @author YellowAfterlife

#include "stdafx.h"
#include <ShlObj.h>
#include <ShlObj_core.h>
#include <dinput.h>

static bool ready;
static bool verbose;
static HMODULE DirectInput8_base;

typedef HRESULT WINAPI DirectInput8Create_t(HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID* ppvOut,
	LPUNKNOWN punkOuter
);

static wchar_t* title = L"TinyModInjector (posing as dinput8.dll)";

static DirectInput8Create_t* DirectInput8Create_base;

static void showMessage(wchar_t* msg, UINT uType = MB_ICONERROR) {
	MessageBoxW(NULL, msg, title, MB_OK | uType);
}

static void showFolderPathError(HRESULT hr) {
	wchar_t buf[1024];
	wsprintfW(buf, L"Failed to get system folder path using SHGetFolderPathA!"
		L"\nHRESULT is %d (check MSDN for error codes).", hr);
	showMessage(buf);
}

static void showLibraryLoadError(wchar_t* path, DWORD gle) {
	wchar_t buf[1024];
	wsprintfW(buf, L"Failed to load the real dinput8.dll using LoadLibraryW!"
		L"\nPath is \"%s\""
		L"\nGetLastError is %d (check MSDN for error codes)."
		L"\n(copy the real DLL over there if there isn't one?)", path, gle);
	showMessage(buf);
}

static void showFunctionLoadError(wchar_t* path, DWORD gle) {
	wchar_t buf[1024];
	wsprintfW(buf, L"Failed to locate the real DirectInput8Create function!"
		L"\nDLL path is \"%s\""
		L"\nGetLastError is %d (check MSDN for error codes)."
		L"\n(perhaps this DLL is corrupted?)", path, gle);
	showMessage(buf);
}

static void showFindFileFirstError(DWORD gle) {
	wchar_t buf[1024];
	wsprintfW(buf, L"Failed to start looking for mod DLLs using FindFirstFileW!"
		L"\nGetLastError is %d (check MSDN for error codes)."
		L"\n(perhaps the NativeMods folder doesn't exist?)", gle);
	showMessage(buf);
}

static void showModLoadError(wchar_t* path, DWORD gle) {
	wchar_t buf[1024];
	wsprintfW(buf, L"Failed to load a mod using LoadLibraryW!"
		L"\nPath is \"%s\""
		L"\nGetLastError is %d (check MSDN for error codes)."
		L"\n(you can move/delete mods that don't work)", path, gle);
	showMessage(buf);
}

template<typename Tc>
void fakestrncat(Tc* dst, const Tc* src, size_t dstSize) {
	size_t dstLen = 0;
	while (dst[dstLen]) dstLen++;
	if (dstLen >= dstSize - 1) return;

	size_t srcLen = 0;
	while (src[srcLen]) srcLen++;
	if (srcLen == 0) return;

	size_t toCopy = srcLen;
	if (dstLen + toCopy + 1 > dstSize) {
		toCopy = dstSize - dstLen - 1;
	}
	memcpy_arr(dst + dstLen, src, toCopy);
	dst[dstLen + toCopy] = 0;
}
template<typename Tc, size_t size>
inline void fakestrncat(Tc(&dst)[size], const Tc* src) {
	fakestrncat(dst, src, size);
}

static HMODULE loadBaseDLL() {
	ready = true;
	wchar_t path[MAX_PATH];
	int csidl;
	#ifndef _WIN64
	csidl = CSIDL_SYSTEMX86;
	#else
	csidl = CSIDL_SYSTEM;
	#endif
	auto hr = SHGetFolderPathW(NULL, csidl, NULL, 0, path);
	if (hr != S_OK) {
		showFolderPathError(hr);
		return NULL;
	}
	fakestrncat(path, L"\\dinput8.dll");
	auto hm = LoadLibraryW(path);
	if (hm == NULL) {
		showLibraryLoadError(path, GetLastError());
		return NULL;
	}
	DirectInput8_base = hm;
	auto f = (DirectInput8Create_t*)GetProcAddress(hm, "DirectInput8Create");
	if (f == nullptr) showFunctionLoadError(path, GetLastError());
	DirectInput8Create_base = f;
	return hm;
}

static WIN32_FIND_DATAW findData; // too big for stack huh?
static wchar_t modsLoaded[1024];
static void showModsLoaded(int count) {
	wchar_t buf[1024];
	wsprintfW(buf, L"%d %s have been loaded:%s", count, count == 1 ? L"mod" : L"mods", modsLoaded);
	showMessage(buf, MB_ICONINFORMATION);
}
static void loadMods() {
	auto hFind = FindFirstFileW(L"NativeMods\\*.dll", &findData);
	wchar_t loadPath[MAX_PATH];
	if (hFind == INVALID_HANDLE_VALUE) {
		showFindFileFirstError(GetLastError());
		return;
	}
	if (verbose) modsLoaded[0] = 0;
	int modCount = 0;
	do {
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			loadPath[0] = 0;
			fakestrncat(loadPath, L"NativeMods\\");
			fakestrncat(loadPath, findData.cFileName);
			auto hm = LoadLibraryW(loadPath);
			if (hm == NULL) {
				showModLoadError(loadPath, GetLastError());
			} else if (verbose) {
				fakestrncat(modsLoaded, L"\n");
				fakestrncat(modsLoaded, loadPath);
				modCount++;
			}
		}
	} while (FindNextFileW(hFind, &findData));
	FindClose(hFind);
	if (verbose) showModsLoaded(modCount);
}

extern "C" HRESULT WINAPI DirectInput8Create(
	HINSTANCE hinst,
	DWORD dwVersion,
	REFIID riidltf,
	LPVOID * ppvOut,
	LPUNKNOWN punkOuter
) {
	if (!ready) {
		if (verbose) {
			showMessage(L"DirectInput8Create is being called for the first time, loading the real dinput8.dll"
				#ifdef TMI_DELAY
				L" and mods"
				#endif
				L"...", MB_ICONINFORMATION);
		}
		loadBaseDLL();
		#ifdef TMI_DELAY
		loadMods();
		#endif
	}
	if (DirectInput8Create_base != nullptr) {
		return DirectInput8Create_base(hinst, dwVersion, riidltf, ppvOut, punkOuter);
	} else return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_OLD_WIN_VERSION);
}

template<typename T>
static bool fakestreq(const T* s1, const T* s2) {
	for (size_t i = 0;; i++) {
		auto c1 = s1[i], c2 = s2[i];
		if (c1 != c2) return false;
		if (c1 == 0) return true;
	}
}

static void init() {
	ready = false;
	DirectInput8_base = NULL;
	DirectInput8Create_base = NULL;
	verbose = false;

	//
	int nArgs;
	auto szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (szArglist != nullptr) {
		for (int i = 0; i < nArgs; i++) {
			if (fakestreq(szArglist[i], L"-tmi-debug")) {
				verbose = true;
			}
		}
	}
	LocalFree(szArglist);
	if (!verbose && GetFileAttributesW(L"tmi-debug.txt") != INVALID_FILE_ATTRIBUTES) verbose = true;
	if (verbose) {
		showMessage(L"The DLL has loaded! (DLL_PROCESS_ATTACH)", MB_ICONINFORMATION);
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			init();
			#ifndef TMI_DELAY
			loadMods();
			#endif
			break;
		case DLL_PROCESS_DETACH:
			//
			break;
	}
	return TRUE;
}