#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1
#pragma warning(suppress : 4996)
#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <CommCtrl.h>
#include <Psapi.h>
#include "FunctionCall.h"
#include "Structures.h"
#include "IoCtrl.h"
#include "Console.h"

DWORD Window_ChangeColor(HWND hWnd, DWORD Color) {
	return SendMessageA(hWnd, 1033, 1, Color);
}

BOOL Widget_Checkbox_IsChecked(HWND hWnd) {
	return !(SendMessageA(hWnd, 240, 0, 0) == 0);
}


void Widget_Checkbox_SetStat(HWND hWnd, BOOL Check) {
	SendMessageA(hWnd, 241, (int)Check, 0);
}


BOOL Widget_SetTitle(HWND hWnd, const char* Str) {
	return SendMessageA(hWnd, 12, 0, (LPARAM)Str) == 0;
}


BOOL Widget_HideWindow(HWND hWnd, BOOL Hide) {
	return ShowWindowAsync(hWnd, Hide);
}

DWORD Tasklist_RefreshProcListAndUpdate(HWND Target) {
	LVITEMW Processinfo = { 0 };
	Processinfo.mask = LVIF_TEXT;
	HANDLE Proclst = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 PE32 = { 0 };
	PE32.dwSize = sizeof(PE32); 
	BOOL ProclstEnd = Process32First(Proclst, &PE32); //枚举进程状态
	DWORD Snp = 0; //计数
	DWORD Retlen1 = 512; //QueryFullProcessImageNameW
	HANDLE ProcessHandle = 0; //PID
	HANDLE TokenHandle = 0; //TokenHandle
	WCHAR Buffer[512];
	PEB64 peb = { 0 };
	while (ProclstEnd) {

		Processinfo.pszText = (LPWSTR)PE32.szExeFile;
		Processinfo.iItem = Snp;
		Processinfo.iSubItem = 0;
		SendMessage(Target, LVM_INSERTITEM, 0, LPARAM(&Processinfo));
		
		swprintf(Processinfo.pszText, L"%d", PE32.th32ProcessID);
		Processinfo.iSubItem = 1;
		SendMessage(Target, LVM_SETITEM, 0, LPARAM(&Processinfo));

		swprintf(Processinfo.pszText, L"%04d", PE32.cntThreads);
		Processinfo.iSubItem = 2;
		SendMessage(Target, LVM_SETITEM, 0, LPARAM(&Processinfo));

		if (Driver_QueryProcessPebData(PE32.th32ProcessID, &peb)) {
			swprintf(Processinfo.pszText, L"0x%p", peb.ImageBaseAddress);
			Processinfo.iSubItem = 4;
			SendMessage(Target, LVM_SETITEM, 0, LPARAM(&Processinfo));
		}
		else {
			memcpy(Processinfo.pszText, L"不支持", sizeof(L"不支持"));
		}


		Processinfo.iSubItem = 5;
		ProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, PE32.th32ProcessID);
		if (GetProcessImageFileName(ProcessHandle,Buffer,512)) {
			Processinfo.pszText = Buffer;
		}
		else {
			memcpy(Processinfo.pszText, L"拒绝访问", sizeof(L"拒绝访问"));
		}

		CloseHandle(ProcessHandle);
		SendMessage(Target, LVM_SETITEM, 0, LPARAM(&Processinfo));
		memset(Buffer, 0, sizeof(Buffer));


		Snp++;
		ProclstEnd = Process32Next(Proclst, &PE32);
	}

	ProclstEnd = Process32First(Proclst, &PE32);
	return Snp;
}