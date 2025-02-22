#define _CRT_SECURE_NO_WARNINGS 1
#define _CRT_NON_CONFORMING_SWPRINTFS 1
#pragma warning(suppress : 4996)
#include <Windows.h>
#include <stdio.h>
#include <CommCtrl.h>
#include <winusb.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include "VariableAndStatics.h"
#include "FunctionCall.h"
#include "Structures.h"
#include "IoCtrl.h"
#include "Console.h"


HANDLE Timer_Queue;
HANDLE Timer_TskList;
HANDLE Timer_GetDriverMsg;
HANDLE Timer_HeartBeat; //Internal
DWORD HeartBeatCountOld;
DWORD HeartBeatCount; //Global

DRIVER_LAST_MSG DriverMsg = { 0 };
DRIVER_LAST_MSG MsgOld = { 0 };

VOID CALLBACK Tasklist_Refresher(PVOID lpParam, BOOLEAN TimerOrWaitFired);
VOID CALLBACK HeartBeatCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired);
VOID CALLBACK GetDriverMsg(PVOID lpParam, BOOLEAN TimerOrWaitFired);

DWORD __CreateTimerUniversalTimer() {
	BOOL Stat = 1;
	Timer_Queue = CreateTimerQueue();
	Stat = Stat & CreateTimerQueueTimer(&Timer_TskList, Timer_Queue, Tasklist_Refresher, 0, 0, 1000, WT_EXECUTEDEFAULT);
	Stat = Stat & CreateTimerQueueTimer(&Timer_HeartBeat, Timer_Queue, HeartBeatCallback, 0, 0, 1000, WT_EXECUTEDEFAULT);
	Stat = Stat & CreateTimerQueueTimer(&Timer_GetDriverMsg, Timer_Queue, GetDriverMsg, 0, 0, 10, WT_EXECUTEDEFAULT);

	return 1;
}

VOID CALLBACK GetDriverMsg(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
	if (DeviceIoControl(hDriver, GET_DRIVER_LAST_MSG, 0, 0, &DriverMsg, sizeof(DriverMsg), 0, 0)) {
		if ((strcmp(DriverMsg.Buffer, MsgOld.Buffer) != 0) || (DriverMsg.Type > 1)) {

			printA(DriverMsg.Buffer, DriverMsg.Type);
		}
		memcpy(&MsgOld, &DriverMsg, sizeof(DriverMsg));
	}
}


VOID CALLBACK Tasklist_Refresher(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
	
}

VOID CALLBACK HeartBeatCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
	HeartBeatCount++;
	if (HeartBeatCount == 120) {
		printA("Program's heartbeat.\n", 1);
		HeartBeatCount = 0;
	}
}

VOID CALLBACK HeartBeatChecker(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
	if (HeartBeatCount > 120|| HeartBeatCount==HeartBeatCountOld) {
		MessageBoxA(GetConsoleWindow(), "警告:发现计数器错误,程序是否还有响应?\n\n请点击[是]以继续.", "警告", MB_OK | MB_ICONWARNING);
		HeartBeatCount = 0;

	}
	HeartBeatCountOld = HeartBeatCount;
}

