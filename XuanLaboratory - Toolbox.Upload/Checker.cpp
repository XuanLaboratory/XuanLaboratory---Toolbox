#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(suppress : 4996)
#define _WIN32_WINNT 0x6000
#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>
#include "VariableAndStatics.h"
#include "Api.h"
#include "FunctionCall.h"
#include "Structures.h"
#include "IoCtrl.h"
#include "Console.h"

VERSION_INFO Ver;

VOID __EnvironmentCheck(void) { 
	printA("Checking environment...\n", 1); //Part1
	Ver = GetVersionRtl();
	if (Ver.dwMajor < 10) {
		printA("Error:Unsupported OS!!!\n", 4);
		char* Buffer = (char*)malloc(512);
		sprintf(Buffer, "����:�㵱ǰ��ϵͳ�汾ΪWindows%d,������Ĳ���ϵͳҪ��ΪWindows10������.\n�뽫��Ĳ���ϵͳ������Windows10���������б�����!\n\n������ʹ��,�뵥��[��](��ʱ�ں�ģʽ�޷�����),������[��]�˳�.", Ver.dwMajor);
		if (MessageBoxA(GetConsoleWindow(), Buffer, "����", MB_YESNO) == IDNO) {
			ExitProcess(-4);
		}
		else {
			WriteConfigBoolean(Cfg, "DriverOptions", "LoadDriver", false);
		}
	}
	printA("Detecting debuggers...\n", 1); //Part2

	NTSTATUS Stat = 0;
	ULONG_PTR IsDebugger = 0;
	BOOL IsDbg = 0;
	DWORD len = 0;
	do {
		Stat = NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugPort, &IsDebugger, sizeof(ULONG_PTR), &len); //ProcessDebugPort
		if (IsDebugger) {
			printA("ProcessDebugPort detected!\n", 2);
			IsDbg = 1;
			break;
		}
		Stat = NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugObjectHandle, &IsDebugger, sizeof(ULONG_PTR), &len); //ProcessDebugObjectHandle
		if (IsDebugger) {
			printA("ProcessDebugObjectHandle detected!\n", 2);
			IsDbg = 1;
			break;
		}
		Stat = NtQueryInformationProcess(GetCurrentProcess(), ProcessDebugFlags, &IsDebugger, sizeof(ULONG_PTR), &len); //ProcessDebugFlags
		if (IsDebugger) {
			printA("ProcessDebugFlags detected!\n", 2);
			IsDbg = 1;
			break;
		}
	} while (0);
	if (IsDbg) {
		printA("Checking parent process information...\n", 1);
		DWORD ParentID = QueryParentProcessID(GetCurrentProcessId());
		if (ParentID >= 0) {
			printW("The PID of parent process is ", 1, ParentID);
			DWORD Btn = MessageBoxA(0, "ע��:�����ϵͳ�м�⵽�˵��������ұ���������������������.\n��������������з�����ȹ���,����ȷ���Ƿ�Ϊ���������еĵ���?\n�������,���ǽ����븸���̲���ֹ��.", "����", MB_YESNO | MB_ICONWARNING);
			if (Btn == IDNO) {
				printA("Detaching...\n", 1);
				char* CmdLine = (char*)malloc(1024);
				memcpy(CmdLine, "--LaunchAsChildProcess", sizeof("--LaunchAsChildProcess"));
				char* BufferA = (char*)malloc(1024);
				GetModuleFileNameA(0, BufferA, 1024);
				STARTUPINFOA si = { 0 }; //��ʼ��Ϊ0������Ҫд
				PROCESS_INFORMATION pi;
				BOOL bRet = FALSE;
				si.cb = sizeof(si);    //cb��С����Ϊ�ṹ���С
				si.dwFlags = STARTF_USESHOWWINDOW;  //ָ��wShowWindow��Ա��Ч
				si.wShowWindow = SW_SHOWNORMAL;
				bRet = CreateProcessA(BufferA, CmdLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
				if (bRet) {
					printA("Create new process successfully,quitting...\n", 1);
				}
			}
		}
	}
	HMODULE EnumModules[8192];
	DWORD Size = 0;
	EnumProcessModules(GetCurrentProcess(), EnumModules, sizeof(EnumModules), &Size);
	printA("DLL total count is ", 1);
	printf("%d\n", Size / sizeof(HMODULE));
	printf("=========================================List of modules=========================================\n");
	for (int i = 0; i < Size / sizeof(HMODULE); i++) {
		char DllName[1024];
		GetModuleFileNameA(EnumModules[i], DllName, 1024);
		printf("%03d %s\n", i, DllName);
	}
	printf("=========================================End of the list=========================================\n\n");
}
