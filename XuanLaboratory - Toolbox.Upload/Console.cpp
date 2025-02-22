#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(suppress : 4996)
#include "FunctionCall.h"
#include <stdio.h>
#include <Windows.h>
#include "Console.h"

/*
Copyright (C) XuanLaboratory - 2024 All Rights Reserved
控制台操作
*/

void PrintC(void);
HANDLE WaitObj = 0;
CONSOLE_PRINTER_INFO* Info = (CONSOLE_PRINTER_INFO*)malloc(sizeof(CONSOLE_PRINTER_INFO)); //公共变量,懒得传参了 麻烦


void printA(const char* s, int color){
	WaitForSingleObject(WaitObj, INFINITE); //先检测有没有排队项目 有则等待
	Info->level = color;
	Info->msg = s;
	WaitObj = CreateThread(0, 512, (LPTHREAD_START_ROUTINE)PrintC, 0, 0, 0);
	WaitForSingleObject(WaitObj, INFINITE); //等待线程完成后退出 防止其他代码调用抢占命令行
}

void PrintC(void) {
	const char* h = "";
	int color = 2;
	switch (Info->level)
	{
	case 1:
		h = "INFO";
		color = C_GREEN;
		break;
	case 2:
		h = "WARN";
		color = C_YELLOW;
		break;
	case 3:
		h = "ERROR";
		color = C_LRED;
		break;
	case 4:
		h = "FATAL";
		color = C_LRED;
		break;
	}
	char* Buffer = (char*)malloc(32);
	sprintf(Buffer, "[%s][", GetSystemTime());
	printf("%s", Buffer);
	free(Buffer);
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
	printf(h);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
	printf("] ");
	printf(Info->msg);
}

void printW(const char* msg, int info, DWORD value) {
	printA(msg, info);
	printf("0x%x\n", value);
}

void printC(const char* msg, int color) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
	printf(msg);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
}
