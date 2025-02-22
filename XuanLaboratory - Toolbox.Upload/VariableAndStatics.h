#pragma once
#include <Windows.h>
#include "Structures.h"
#include "Api.h"

#define Cfg ".\\XuanLab.ini" //默认配置文件

extern DWORD ParentPid;
extern DWORD ProgramMode;
extern BOOL IsModifyPidEnabled;
extern HANDLE hDriver;
extern char* CurrentDir;

extern HWND MainWindow;
extern HWND FuncSelect;
extern HWND Tasklist;
extern HINSTANCE hInstMain;
extern HFONT UniversalFont;

extern DWORD HeartBeatCount;
extern HANDLE Timer_Queue;
extern HANDLE Timer_TskList;

extern VERSION_INFO Ver;

extern pfnNtQueryInformationProcess NtQueryInformationProcess; //自导出API NtQueryInformationProcess
extern pfnNtWow64ReadVirtualMemory64 NtWow64ReadVirtualMemory64; 