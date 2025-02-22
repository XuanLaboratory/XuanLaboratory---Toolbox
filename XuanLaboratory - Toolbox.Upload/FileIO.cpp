#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(suppress : 4996)
#include "FunctionCall.h"
#include <ShlObj.h>
#include <CommCtrl.h>
#include <stdio.h>
#include <Windows.h>
#pragma comment(lib,"comctl32.lib")
#include <WinTrust.h>
#pragma comment(lib,"wintrust.lib");
#include <wincrypt.h>
#pragma comment(lib,"crypt32.lib");


/*
Copyright (C) XuanLaboratory - 2024 All Rights Reserved
文件操作
*/

#define fn  "FileIO.cpp"

//常规文件IO

LPCSTR File_ReadFileFull(LPCSTR FileName) {
	HANDLE hFile = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		return NULL;
	}
	DWORD len = GetFileSize(hFile, 0);
	char* Buffer = (char*)malloc(len);
	DWORD read = 0;
	BOOL Status = ReadFile(hFile, Buffer, len, &read, 0);
	CloseHandle(hFile);
	if (Status) {
		LPCSTR data = Buffer;
		return data;
	}
	else {
		return NULL;
	}
}

BOOL File_WriteFile(LPCSTR FileName,LPCSTR Info) {
	HANDLE hFile = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	DWORD WriteLen = 0;
	DWORD Datalen = lstrlenA(Info);
	BOOL Stat = WriteFile(hFile, Info ,Datalen , &WriteLen, 0);
	CloseHandle(hFile);
	return Stat;
}

BOOL File_WriteFileAttach(LPCSTR FileName, LPCSTR Info) {
	HANDLE hFile = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		hFile = CreateFileA(FileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile== INVALID_HANDLE_VALUE) {
			return false;
		}
	}
	DWORD WriteLen = 0;
	DWORD Datalen = lstrlenA(Info);
	SetFilePointer(hFile, 0, 0, FILE_END);
	BOOL Stat = WriteFile(hFile, Info, Datalen, &WriteLen, 0);
	CloseHandle(hFile);
	return Stat;
}


LPCSTR ReadConfigString(LPCSTR Path, LPCSTR Section, LPCSTR Key) {
	char* Buffer = (char*)malloc(1024);
	GetPrivateProfileStringA(Section, Key, "<null>", Buffer, 1024, Path);
	return Buffer;
}

BOOL ReadConfigBoolean(LPCSTR Path, LPCSTR Section, LPCSTR Key) {
	char* Buffer = (char*)malloc(5);
	GetPrivateProfileStringA(Section, Key, "false", Buffer, 5, Path);
	if (strcmp(Buffer, "true") == 0) return TRUE;
	return FALSE;
}

DWORD ReadConfigDword(LPCSTR Path, LPCSTR Section, LPCSTR Key) {
	return GetPrivateProfileIntA(Section, Key, 0, Path);
}

BOOL WriteConfigString(LPCSTR Path, LPCSTR Section, LPCSTR Key, LPCSTR Data) {
	DWORD len = strlen(Data);
	return WritePrivateProfileStringA(Section, Key, Data, Path);
}

BOOL WriteConfigBoolean(LPCSTR Path, LPCSTR Section, LPCSTR Key, BOOL b) {
	char* Buffer = (char*)malloc(4);
	RtlZeroMemory(Buffer, strlen(Buffer));
	memset(Buffer, 0, 5);
	if (b) {
		sprintf(Buffer, "%s", "true");
	}
	else
	{
		sprintf(Buffer, "%s", "true");
	}
	return WritePrivateProfileStringA(Section, Key, Buffer, Path);
}

BOOL WriteConfigInt(LPCSTR Path, LPCSTR Section, LPCSTR Key, DWORD v) {
	char* Buffer = (char*)malloc(sizeof(DWORD));
	sprintf(Buffer, "%d", v);
	return WritePrivateProfileStringA(Section, Key, Buffer, Path);
}

//其他功能

BOOL FileExistsStaus(LPCSTR path)
{
	if (path == nullptr) return false;

	HANDLE hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	CloseHandle(hFile);
	return true;
}

char* CreateFileSelectDlg(const char* Title) {
	LPITEMIDLIST pil = NULL;
	INITCOMMONCONTROLSEX InitCtrls = { 0 };
	char szBuf[4096] = { 0 };
	BROWSEINFOA bi = { 0 };
	bi.hwndOwner = NULL;
	bi.iImage = 0;
	bi.lParam = NULL;
	bi.lpfn = NULL;
	bi.lpszTitle = Title;
	bi.pszDisplayName = szBuf;
	bi.ulFlags = BIF_BROWSEINCLUDEFILES;

	InitCommonControlsEx(&InitCtrls);
	pil = SHBrowseForFolderA(&bi);
	if (NULL != pil){
		SHGetPathFromIDListA(pil, szBuf);
		return szBuf;
	}
	return nullptr;
}

BOOL CheckIfExecutable(const char* Path) { // -1=失败 0=否 1=是
	HANDLE hFile = CreateFileA(Path, FILE_READ_ACCESS, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (!hFile) return false;
	char mzHeader[2] = { 0 };
	DWORD bytesRead = 0;
	BOOL result = ReadFile(hFile, mzHeader, 2, &bytesRead, nullptr);
	if (!result || bytesRead != 2 || mzHeader[0] != 'M' || mzHeader[1] != 'Z') { //检验MZ头
		CloseHandle(hFile);
		return false;
	}
	
	DWORD peHeaderOffset = 0;
	SetFilePointer(hFile, 0x3C, nullptr, FILE_BEGIN); //检验PE头
	result = ReadFile(hFile, &peHeaderOffset, sizeof(peHeaderOffset), &bytesRead, nullptr);
	if (!result || bytesRead != sizeof(peHeaderOffset)) {
		CloseHandle(hFile);
		return false;
	}

	SetFilePointer(hFile, peHeaderOffset, nullptr, FILE_BEGIN);
	char peHeader[4] = { 0 };
	result = ReadFile(hFile, peHeader, 4, &bytesRead, nullptr);
	CloseHandle(hFile);

	return (result && bytesRead == 4 && peHeader[0] == 'P' && peHeader[1] == 'E' && peHeader[2] == '\0' && peHeader[3] == '\0');
}


