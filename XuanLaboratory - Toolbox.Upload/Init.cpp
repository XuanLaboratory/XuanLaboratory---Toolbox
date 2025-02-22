#include <Windows.h>
#include <stdio.h>
#include <CommCtrl.h>
#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "FunctionCall.h"
#include "Structures.h"
#include "IoCtrl.h"
#include "Console.h"
#include "VariableAndStatics.h"

pfnNtQueryInformationProcess NtQueryInformationProcess;
pfnNtWow64ReadVirtualMemory64 NtWow64ReadVirtualMemory64;

DWORD __PluginInit(void);
DWORD __DebugInit(void);

DWORD __InternalInit(void) {
	printA("Initializing...\n", 1);




	printA("Raising privileges...\n", 1);
    HANDLE token_handle;
    if (OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&token_handle)){
		LUID luid;
		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
			TOKEN_PRIVILEGES tkp;
			tkp.PrivilegeCount = 1;
			tkp.Privileges[0].Luid = luid;
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges(token_handle, FALSE, &tkp, sizeof(tkp), NULL, NULL)) {
				printA("Raise Privileges failed!\n",2);
			}
		}
    }
	
	printA("Overwriting priority...\n", 1);
	if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)) {
		printA("Overwrite priority failed!\n", 2);
	}

	printA("Searching for config file in current dir...\n", 1);
	HANDLE hCfg = CreateFileA(Cfg, FILE_READ_ONLY, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	BOOL cValid = hCfg == INVALID_HANDLE_VALUE;
	if (cValid) {
		printA("Config file not found,re-configuring...\n", 2); //重置配置
		
		
		WriteConfigBoolean(Cfg, "Init", "LoadPlugin", false);
		WriteConfigBoolean(Cfg, "Init", "Debug", false);
		WriteConfigInt(Cfg, "Init", "Privilege", REALTIME_PRIORITY_CLASS);
		File_WriteFileAttach(Cfg, "\n\n");
		WriteConfigBoolean(Cfg, "DriverOptions", "LoadDriver", false);
		WriteConfigBoolean(Cfg, "DriverOptions", "SelfProtect", false);
		WriteConfigBoolean(Cfg, "DriverOptions", "SelfProtectByModifyPid", false);

	}
	printA("Connecting to parent process...\n", 1);
	do {
		if (WaitNamedPipeA("\\\\.\\pipe\\XuanLaboratory_IoPipe", NMPWAIT_WAIT_FOREVER) == 0) {
			printA("Connect to parent process failed : No pipe avaliable.\n", 3);
			break;
		}
		HANDLE Pipe = CreateFileA("\\\\.\\pipe\\XuanLaboratory_IoPipe", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (Pipe == INVALID_HANDLE_VALUE) {
			printA("Failed to open pipe!\n", 3);
			break;
		}
		DWORD rec = 0;
		char* Buffer = (char*)malloc(sizeof(DWORD));
		if (!ReadFile(Pipe, Buffer, strlen(Buffer), &rec, 0)) {
			printA("Failed to read message from pipe!\n", 3);
			break;
		}
		ParentPid = atoi(Buffer);
		printA("Parent process id is ", 1);
		printf("%d\n", ParentPid);

	} while (false);


	if (ReadConfigBoolean(Cfg,"DriverOptions","LoadDriver")) {
		printA("Loading driver...\n", 1);
		__InitializeDriver();
	}
	if (ReadConfigBoolean(Cfg, "Init", "LoadPlugin")) {
		printA("Initializing plugin manager...\n", 1);
		__PluginInit();
	}
	
	if (ReadConfigBoolean(Cfg, "Init", "Debug")) {
		__DebugInit();
	}
	

	//初始化字体
	UniversalFont = CreateFontW(-12, -6, 0, 0, 100, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, L"微软雅黑"); //通用字体
	if (!UniversalFont) {
		printA("Failed to create new font,using default font.\n", 3);
		MessageBoxA(GetConsoleWindow(), "错误:未能创建字体!\n\n使用默认字体", "错误", MB_OK);
		UniversalFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	}


	return 0;
}


DWORD __PluginInit(void) {
	printA("Searching for plugins in current directory...\n", 1);
	SetCurrentDirectoryA(".");
	WIN32_FIND_DATAA fd = { 0 };
	HANDLE hEnum = FindFirstFileA("*.*", &fd);
	DWORD pluginCount = 0;
	if (hEnum == INVALID_HANDLE_VALUE) {
		printA("Failed to enum file in current dir!\n", 3);
		return GetLastError();
	}
	do {
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if (strcmp(PathFindExtensionA(fd.cFileName), ".dll") == 0) { //判断后缀名是否为.dll
				if (CheckIfExecutable(fd.cFileName)) { //判断文件头
					pluginCount++;
				}
				else {
					printA(StrConnect(fd.cFileName, " is not a valid executable file,ignored.\n"), 2);
				}
			}
		}

	} while (FindNextFileA(hEnum, &fd));
	return 0;
}


DWORD __DebugInit(void) {
	printA("Debug initializing...\n", 1);

	return 0;
}

DWORD __ApiInit(void) {
	printA("Initializing api...\n", 1);
	do {
		NtQueryInformationProcess = NULL;
		HMODULE hNtDll = LoadLibraryA("ntdll.dll");
		if (!hNtDll) {
			printA("Failed to load ntdll.dll\n", 3);
			break;
		}
		NtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess");
		NtWow64ReadVirtualMemory64 = (pfnNtWow64ReadVirtualMemory64)GetProcAddress(hNtDll, "NtWow64ReadVirtualMemory64");

		return 0;
	} while (0);



	return GetLastError();
}

