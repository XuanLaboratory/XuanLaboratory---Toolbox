#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(suppress : 4996)
#include <Windows.h>
#include <VersionHelpers.h>
#include <stdio.h>
#include "FunctionCall.h"
#include "Structures.h"
#include "IoCtrl.h"
#include "Console.h"

typedef void(__stdcall* RtlGetNtVersionNumbersCall)(DWORD*, DWORD*, DWORD*);

VERSION_INFO GetVersionRtl(void) {
    BYTE* sharedUserData = (BYTE*)0x7FFE0000;
    ULONG uMajorVer, uMinorVer, uBuildNum;
    memcpy(&uMajorVer, (sharedUserData + 0x26c), sizeof(ULONG));
    memcpy(&uMinorVer, (sharedUserData + 0x270), sizeof(ULONG));
    memcpy(&uBuildNum, (sharedUserData + 0x260), sizeof(ULONG));
    VERSION_INFO Ver = { 0 };
    char* Buffer = (char*)malloc(128);
    sprintf(Buffer, "%d-%d-%d", *(ULONG*)(sharedUserData + 0x26c), *(ULONG*)(sharedUserData + 0x270), *(ULONG*)(sharedUserData + 0x260));
    Ver.dwMajor = uMajorVer;
    Ver.dwMinor = uMinorVer;
    Ver.dwBuild = uBuildNum;
    if (uBuildNum >= 22000) Ver.dwMajor = 11;
    Ver.String = Buffer;
    return Ver;
}

char* GetSystemTime() {
    SYSTEMTIME SysTime;
    GetLocalTime(&SysTime);
    char* Buffer = (char*)malloc(128);
    sprintf(Buffer, "%02d:%02d:%02d.%03d", SysTime.wHour, SysTime.wMinute, SysTime.wSecond, SysTime.wMilliseconds);
    return Buffer;
}

char* GetSystemTimeFull() {
    SYSTEMTIME SysTime;
    GetLocalTime(&SysTime);
    char* Buffer = (char*)malloc(256);
    sprintf(Buffer, "%04d-%02d-%02d %02d:%02d:%02d.%03d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond, SysTime.wMilliseconds);
    return Buffer;
}

BOOL EnableNoSignedDriverLoad() {
    BOOL Stat = true;
    Stat = Stat & (DWORD)ShellExecuteW(0, L"runas", L"BCDEdit.exe", L"/debug on", NULL, SW_SHOWNORMAL);
    Stat = Stat & (DWORD)ShellExecuteW(0, L"runas", L"BCDEdit.exe", L"/set testsigning on", NULL, SW_SHOWNORMAL);
    Stat = Stat & (DWORD)ShellExecuteW(0, L"runas", L"BCDEdit.exe", L"/set nointegritychecks on", NULL, SW_SHOWNORMAL);
    return Stat;
}

BOOL DisableNoSignedDriverLoad() {
    BOOL Stat = true;
    Stat = Stat & (DWORD)ShellExecuteW(0, L"runas", L"BCDEdit.exe", L"/debug off", NULL, SW_SHOWNORMAL);
    Stat = Stat & (DWORD)ShellExecuteW(0, L"runas", L"BCDEdit.exe", L"/set testsigning off", NULL, SW_SHOWNORMAL);
    Stat = Stat & (DWORD)ShellExecuteW(0, L"runas", L"BCDEdit.exe", L"/set nointegritychecks off", NULL, SW_SHOWNORMAL);
    return Stat;
}