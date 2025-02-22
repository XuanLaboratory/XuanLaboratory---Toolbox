#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(suppress : 4996)
#include <Windows.h>
#include <stdio.h>
#include "FunctionCall.h"
#include "Structures.h"
#include "IoCtrl.h"
#include "Console.h"

LPCSTR StrConnect(LPCSTR a, LPCSTR b) {
	if (a == nullptr || b == nullptr) {
		return NULL;
	}
	char* Buffer = (char*)malloc(sizeof(char) * 1024);
	strcpy(Buffer, a);
	strcat(Buffer, b);
	return Buffer;
}

BOOL StrCompare(LPCSTR a, LPCSTR b) {
	if (a == nullptr || b == nullptr) {
		return NULL;
	}
	return strcmp(a, b) == 0;
}

char* TranslateGetLastErrorMsg(DWORD code) {
	LPVOID lpMsgBuf=nullptr;
	if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, code, MAKELANGID(LANG_CHINESE_SIMPLIFIED, SUBLANG_CHINESE_SIMPLIFIED), (char*)&lpMsgBuf, 0, 0) <= 0) {
		return nullptr;
	}
	char* Buffer = (char*)malloc(strlen((char*)lpMsgBuf));
	memset(Buffer, 0, strlen(Buffer));
	memcpy(Buffer, lpMsgBuf, strlen((char*)lpMsgBuf)-3);
	return Buffer;
}