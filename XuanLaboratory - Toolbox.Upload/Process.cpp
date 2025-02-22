#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(suppress : 4996)
#include <Windows.h>
#include <stdio.h>
#include "FunctionCall.h"
#include "Structures.h"
#include "IoCtrl.h"
#include "Console.h"
#include "VariableAndStatics.h"
#include "Api.h"

DWORD QueryParentProcessID(DWORD Pid) {
	HANDLE hProcess;
	PROCESS_BASIC_INFORMATION ProcessBasicInfo = { 0 };
	if (Pid == GetCurrentProcessId()) {
		hProcess = GetCurrentProcess();
	}
	else {
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, Pid);
		if (hProcess == 0) {
			return -1;
		}

	}

	DWORD ParentID;
	DWORD len = 0;
	NtQueryInformationProcess(hProcess, ProcessBasicInformation, (PVOID)&ProcessBasicInfo, sizeof(PROCESS_BASIC_INFORMATION), &len);
	return (ULONG_PTR)ProcessBasicInfo.InheritedFromUniqueProcessId;
}


BOOL RaiseProcessPermission(DWORD Pid) {
	HANDLE token_handle;
	if (OpenProcessToken(OpenProcess(PROCESS_ALL_ACCESS,0,Pid), TOKEN_ALL_ACCESS, &token_handle)) {
		LUID luid;
		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
			TOKEN_PRIVILEGES tkp;
			tkp.PrivilegeCount = 1;
			tkp.Privileges[0].Luid = luid;
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if (AdjustTokenPrivileges(token_handle, FALSE, &tkp, sizeof(tkp), NULL, NULL)) {
				return true;
			}
		}
	}
	return false;
}


/*
BOOL QueryProcessPebData(DWORD Pid, PEBU* Peb) {
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, Pid);
	if (hProc == INVALID_HANDLE_VALUE) return false;
	PROCESS_BASIC_INFORMATION64 pbi = { 0 };
	NTSTATUS ret = NtQueryInformationProcess(hProc, ProcessBasicInformation, &pbi, sizeof(pbi), 0);
	printf("NT=%x\n",ret);
	if (ret == 0) {
		SIZE_T rLen = 0;
		printf("DBG=%p\n", pbi.PebBaseAddress);
		DWORD mempt = 0;
		ret = NtWow64ReadVirtualMemory64(hProc, (PVOID64)pbi.PebBaseAddress, Peb, sizeof(PEBU), 0);
		printf("RM=%d\n", ret);
		return ret;
	}
	return false;
}
*/

