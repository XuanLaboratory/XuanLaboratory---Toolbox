#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(suppress : 4996)
#include "FunctionCall.h"
#include <stdio.h>
#include <Windows.h>
#include "Console.h"

/*
Copyright (C) XuanLaboratory - 2024 All Rights Reserved
����̨����
*/

void PrintC(void);
HANDLE WaitObj = 0;
CONSOLE_PRINTER_INFO* Info = (CONSOLE_PRINTER_INFO*)malloc(sizeof(CONSOLE_PRINTER_INFO)); //��������,���ô����� �鷳


void printA(const char* s, int color){
	WaitForSingleObject(WaitObj, INFINITE); //�ȼ����û���Ŷ���Ŀ ����ȴ�
	Info->level = color;
	Info->msg = s;
	WaitObj = CreateThread(0, 512, (LPTHREAD_START_ROUTINE)PrintC, 0, 0, 0);
	WaitForSingleObject(WaitObj, INFINITE); //�ȴ��߳���ɺ��˳� ��ֹ�������������ռ������
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
