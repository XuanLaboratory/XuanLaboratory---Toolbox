#include <ntifs.h>
#include <ntddk.h>
#include <ntimage.h>
#include <ntstrsafe.h>
#include "FunctionCall.h"

OB_PREOP_CALLBACK_STATUS OnPreOpenProcess(PVOID, POB_PRE_OPERATION_INFORMATION Info);
void OnPreProcessCreateCallbackEx(HANDLE ParentProc, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO Info);

NTSTATUS OnRegistryNotifyCallback(PVOID Context, PVOID Arg1, PVOID Arg2);
PVOID ProcessNotifyCallbackHandle;
BOOLEAN ProcessCreateNotifyStat;
BOOLEAN ProcessNotifyCallbackStat;

void UnRegNotifyCallbacks(void) {
	if (ProcessNotifyCallbackStat) ObUnRegisterCallbacks(ProcessNotifyCallbackHandle);
	if (ProcessCreateNotifyStat) PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)OnPreProcessCreateCallbackEx, TRUE);
}


NTSTATUS __ProtectClientByRegisterCallback() {
	NTSTATUS Status = STATUS_ACCESS_DENIED;
	//初始化注册信息 （注册进程通知回调 开始)
	OB_CALLBACK_REGISTRATION ObCallbackReg = { 0 };
	OB_OPERATION_REGISTRATION ObOperationReg = { 0 };
	RtlZeroMemory(&ObCallbackReg, sizeof(ObCallbackReg));
	RtlZeroMemory(&ObOperationReg, sizeof(ObOperationReg));

	//初始化ObCallbackReg
	ObCallbackReg.Version = OB_FLT_REGISTRATION_VERSION;
	ObCallbackReg.Altitude = RTL_CONSTANT_STRING(L"439999.19491001");
	ObCallbackReg.OperationRegistrationCount = 1;
	ObCallbackReg.RegistrationContext = nullptr;
	ObCallbackReg.OperationRegistration = &ObOperationReg;

	//初始化ObOperationReg
	ObOperationReg.ObjectType = PsProcessType;
	ObOperationReg.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	ObOperationReg.PreOperation = OnPreOpenProcess;

	Status = ObRegisterCallbacks(&ObCallbackReg, &ProcessNotifyCallbackHandle);
	ProcessNotifyCallbackStat = (Status == STATUS_SUCCESS);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	Status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)OnPreProcessCreateCallbackEx, FALSE);
	ProcessCreateNotifyStat = NT_SUCCESS(Status);
	if (!NT_SUCCESS(Status)) {
		return Status;
	}


	
	return STATUS_SUCCESS;
}

OB_PREOP_CALLBACK_STATUS OnPreOpenProcess(PVOID, POB_PRE_OPERATION_INFORMATION Info) {
	if (SelfProtect) {
		PEPROCESS Process = (PEPROCESS)Info->Object;
		ULONG Pid = HandleToULong(PsGetProcessId(Process));
		if ((Pid == ClientProcessId || Pid == ClientParentProcessId) && SelfProtect) {
			Info->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_ALL_ACCESS;
		}
	}
	return OB_PREOP_SUCCESS;
}


void OnPreProcessCreateCallbackEx(HANDLE ParentProc, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO Info) {
	if (Info == NULL) { //进程退出通知
		if ((ULONG)ProcessId == ClientProcessId || (ULONG)ProcessId == ClientParentProcessId) {
			UNICODE_STRING SvcName;
			RtlInitUnicodeString(&SvcName, L"XuanLab_KernelModeService");
			DbgPrint(("STAT:%x\n"),ZwUnloadDriver(&SvcName));
		}
	}
	else { //进程创建通知
		DbgPrint(("PProc:%d Proc:%d Cmdline=%ws"), ParentProc, ProcessId, Info->CommandLine->Buffer);
		Info->CreationStatus = STATUS_NOT_ALLOWED_ON_SYSTEM_FILE;
	}
}



NTSTATUS EnumAddressOfLib(UNICODE_STRING DllFileName, PVOID Buffer) { //注意!!! 这个只能给客户端用
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE hFile = NULL;
	HANDLE hSection = NULL;
	PVOID pBaseAddress = NULL;
	//DbgPrint(("Buffer=%ws\n"), DllFileName.Buffer);
	DLL_EXPORT_INFO_NT* Opt = (DLL_EXPORT_INFO_NT*)Buffer;


	// 内存映射文件
	status = KernelMapFile(DllFileName, &hFile, &hSection, &pBaseAddress);
	if (!NT_SUCCESS(status))
	{
		DbgPrint(("FAILED WITH:%x\n"), status);
		return status;
	}
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBaseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress);
	ULONG ulNumberOfNames = pExportTable->NumberOfNames;
	PULONG lpNameArray = (PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfNames);
	PCHAR lpName = NULL;

	for (ULONG i = 0; i < ulNumberOfNames; i++)
	{
		lpName = (PCHAR)((PUCHAR)pDosHeader + lpNameArray[i]);
		USHORT uHint = *(USHORT*)((PUCHAR)pDosHeader + pExportTable->AddressOfNameOrdinals + 2 * i);
		ULONG ulFuncAddr = *(PULONG)((PUCHAR)pDosHeader + pExportTable->AddressOfFunctions + 4 * uHint);
		PVOID lpFuncAddr = (PVOID)((PUCHAR)pDosHeader + ulFuncAddr);
		Opt[i].Address = (ULONG64)lpFuncAddr;
		Opt[i].Index = i;
		DbgPrint(("Function export:%s\n"), lpName);
		memcpy(Opt[i].FunctionName, lpName, 48);
	}
	ZwUnmapViewOfSection(NtCurrentProcess(), pBaseAddress);
	ZwClose(hSection);
	ZwClose(hFile);
	return 0;
}