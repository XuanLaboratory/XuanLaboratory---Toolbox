#pragma once
#include <ntddk.h>
#include "Define.h"

NTSTATUS DriverCreate(PDEVICE_OBJECT pdo, PIRP Irp);
NTSTATUS DriverClose(PDEVICE_OBJECT pdo, PIRP Irp);
NTSTATUS DriverDeviceControl(PDEVICE_OBJECT pdo, PIRP Irp);

//Main.cpp
void SetDriverLastMsg(char* msg, ULONG level); //设置驱动最后一条消息

//ClientProcess.cpp
void UnRegNotifyCallbacks(void); 
NTSTATUS __ProtectClientByRegisterCallback();
NTSTATUS EnumAddressOfLib(UNICODE_STRING DllFileName, PVOID Buffer);

//Process.cpp
NTSTATUS UniversalProcessOperation(PROCESS_OPERATE_INFO* Info); //进程操作
BOOLEAN IsProcessIdValid(ULONG Pid); //Pid是否有效
NTSTATUS ModifyProcessPid(ULONG Pid, ULONG PidOld, BOOLEAN Opt); //进行保护操作时，PidOld将忽略

//WindowsKernel.cpp
NTSTATUS KernelInformationQueryDispatch(KERNEL_INFO_QUERY_DISPATCHER* Info);

NTSTATUS KernelMapFile(UNICODE_STRING FileName, HANDLE* phFile, HANDLE* phSection, PVOID* ppBaseAddress);
ULONG64 GetAddressFromFunction(UNICODE_STRING DllFileName, PCHAR pszFunctionName);