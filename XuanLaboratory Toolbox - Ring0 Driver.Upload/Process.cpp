#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include "FunctionCall.h"

BOOLEAN WriteProcessMemory(PEPROCESS proc, PVOID pBaseAddress, PVOID pWriteData, SIZE_T writeDataSize);
BOOLEAN ReadProcessMemory(PEPROCESS proc, PVOID pBaseAddress, PVOID pClientBuffer, SIZE_T readDataSize, SIZE_T BufferSize);
BOOLEAN GetProcessPeb(PEPROCESS pProc, PVOID pPeb);

NTSTATUS UniversalProcessOperation(PROCESS_OPERATE_INFO* Info) {
	ULONG Pid = Info->ProcessId;
	NTSTATUS StatusClient = Info->Status;
	PVOID BufferIn = Info->BufferIn;
	ULONG BufferInSize = Info->BufferInSize;
	PVOID BufferOut = Info->BufferOut;
	ULONG BufferOutSize = Info->BufferOutSize;

	//先获取到进程句柄 方便使用
	HANDLE hProcess;
	OBJECT_ATTRIBUTES ObjAttr;
	CLIENT_ID ClientId = { 0 };
	NTSTATUS Status;
	ClientId.UniqueProcess = (HANDLE)Pid;
	InitializeObjectAttributes(&ObjAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
	Status = ZwOpenProcess(&hProcess, GENERIC_ALL, &ObjAttr, &ClientId);
	if (!NT_SUCCESS(Status)) { //如果失败返回
		return Status;
	}

	PEPROCESS Proc = { 0 };
	Status = PsLookupProcessByProcessId((HANDLE)Pid, &Proc);
	if (!NT_SUCCESS(Status)) { //如果失败返回
		ObDereferenceObject(Proc);
		return Status;
	}

	switch (Info->Operation)
	{
	case ProcessTerminate: {
		Status = ZwTerminateProcess(hProcess, STATUS_SUCCESS);
		if (!NT_SUCCESS(Status)) {
			ZwClose(hProcess);
			return Status;
		}
		break;
	}

	case ProcessTerminateProcTree: {
		PEPROCESS Proc = { 0 };
		Status = PsLookupProcessByProcessId((HANDLE)Pid, &Proc);
		if (!NT_SUCCESS(Status)) break;		
		HANDLE ProcessTree[128] = { 0 };
		HANDLE hParent = PsGetProcessInheritedFromUniqueProcessId(Proc); //第一次准备
		ULONG i = 0;
		//搜索目标进程的父进程直到完毕
		while (IsProcessIdValid((ULONG)hParent)) {
			ProcessTree[i] = hParent;
			Proc = { 0 };
			Status = PsLookupProcessByProcessId(hParent, &Proc);
			if (!NT_SUCCESS(Status)) break;
			hParent = PsGetProcessInheritedFromUniqueProcessId(Proc);
			i++;
		}


		i = 0;
		PROCESS_OPERATE_INFO Op = { 0 };
		Op.Operation = ProcessTerminate;
		//结束列表中的进程
		while (ProcessTree[i] != 0) {
			Op.ProcessId = (ULONG)ProcessTree[i];
			Status = UniversalProcessOperation(&Op);
			if (!NT_SUCCESS(Status)) break;

		}
		RtlZeroMemory(&ProcessTree, sizeof(ProcessTree));
		break;
	}

	case ProcessSuspend: {
		Status = PsSuspendProcess(Proc);
		break;
	}

	case ProcessResume: {
		Status = PsResumeProcess(Proc);
		break;
	}
	
	case ProcessWriteMemory: {
		if (!WriteProcessMemory(Proc, BufferIn, BufferOut, BufferOutSize)) Status = STATUS_UNSUCCESSFUL;
		break;
	}

	case ProcessReadMemory: {
		if(!ReadProcessMemory(Proc,BufferIn,BufferOut,BufferInSize,BufferOutSize)) Status = STATUS_UNSUCCESSFUL;
		break;
	}

	case ProcessQueryPebData: {
		PVOID Buffer = ExAllocatePool(NonPagedPool, sizeof(PEB64));
		if (!GetProcessPeb(Proc, Buffer)) {
			Status = STATUS_UNSUCCESSFUL;
			ExFreePool(Buffer);
			break;
		}
		KAPC_STATE kapc = { 0 };
		PEPROCESS client = { 0 };
		Status = PsLookupProcessByProcessId((HANDLE)ClientProcessId, &client);
		if (!NT_SUCCESS(Status)) {
			ObDereferenceObject(client);
			ExFreePool(Buffer);
			break;
		}
		BOOLEAN isWow64 = PsGetProcessWow64Process(Proc);
		SIZE_T pebSize = isWow64 ? sizeof(PEB32) : sizeof(PEB64);

		KeStackAttachProcess(client, &kapc);
		if (BufferOutSize < pebSize) {
			Status = STATUS_INVALID_BUFFER_SIZE;
			KeUnstackDetachProcess(&kapc);
			ObDereferenceObject(client);
			ExFreePool(Buffer);
			break;
		}
		RtlCopyMemory(BufferOut, Buffer, pebSize);
		KeUnstackDetachProcess(&kapc);
		ObDereferenceObject(client);
		ExFreePool(Buffer);
		break;
	}

	default:
		Status = STATUS_INVALID_DEVICE_REQUEST;
		break;

	}
	ObDereferenceObject(Proc);
	ZwClose(hProcess);
	StatusClient = Status;
	return Status;
}


BOOLEAN IsProcessIdValid(ULONG Pid) {
	HANDLE hProcess;
	OBJECT_ATTRIBUTES ObjAttr;
	CLIENT_ID ClientId = { 0 };
	NTSTATUS Status;
	ClientId.UniqueProcess = (HANDLE)Pid;
	InitializeObjectAttributes(&ObjAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
	Status = ZwOpenProcess(&hProcess, GENERIC_ALL, &ObjAttr, &ClientId);
	if (!NT_SUCCESS(Status)) { //如果失败返回
		return false;
	}
	ZwClose(hProcess);
	return true;
}

BOOLEAN WriteProcessMemory(PEPROCESS proc,PVOID pBaseAddress, PVOID pWriteData, SIZE_T writeDataSize){
	KAPC_STATE kapc = { 0 };
	PVOID Buffer = ExAllocatePool(NonPagedPool, writeDataSize);
	RtlZeroMemory(Buffer, writeDataSize);
	RtlCopyMemory(Buffer, pWriteData, writeDataSize);
	KeStackAttachProcess(proc, &kapc);
	if (MmIsAddressValid(pBaseAddress)) {
		RtlCopyMemory(pBaseAddress, Buffer, writeDataSize);
		KeUnstackDetachProcess(&kapc);
		ExFreePool(Buffer);
		return TRUE;
	}
	else {
		KeUnstackDetachProcess(&kapc);
		ExFreePool(Buffer);
		return FALSE;
	}
}

BOOLEAN ReadProcessMemory(PEPROCESS proc, PVOID pBaseAddress, PVOID pClientBuffer, SIZE_T readDataSize,SIZE_T BufferSize) { //客户端Pid由DRIVER_INIT提供
	if (readDataSize > BufferSize) return FALSE;
	KAPC_STATE kapc = { 0 };
	PVOID Buffer = ExAllocatePool(NonPagedPool, readDataSize);
	RtlZeroMemory(Buffer, readDataSize);
	KeStackAttachProcess(proc, &kapc); //附加目标进程

	if (MmIsAddressValid(pBaseAddress)) {
		RtlCopyMemory(Buffer, pBaseAddress, readDataSize);
		KeUnstackDetachProcess(&kapc);
	}
	else {
		KeUnstackDetachProcess(&kapc);
		ExFreePool(Buffer);
		return FALSE;
	}
	kapc = { 0 };
	PEPROCESS client = { 0 };
	if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)ClientProcessId, &client))) {
		ObDereferenceObject(client);
		ExFreePool(Buffer);
		return FALSE;
	}
	KeStackAttachProcess(client, &kapc); //附加客户端进程,写入数据
	if (MmIsAddressValid(pClientBuffer)) {
		RtlCopyMemory(pClientBuffer, Buffer, readDataSize);
		KeUnstackDetachProcess(&kapc);
		ExFreePool(Buffer);
		ObDereferenceObject(client);
		return TRUE;
	}
	else {
		KeUnstackDetachProcess(&kapc);
		ExFreePool(Buffer);
		ObDereferenceObject(client);
		return FALSE;
	}

}

BOOLEAN GetProcessPeb(PEPROCESS pProc, PVOID pPeb) {
	if (pPeb == nullptr) return FALSE;

	PPEB pPebUser = (PPEB)PsGetProcessPeb(pProc);
	if (pPebUser == NULL) return FALSE;

	BOOLEAN isWow64 = PsGetProcessWow64Process(pProc) != NULL;
	SIZE_T pebSize = isWow64 ? sizeof(PEB32) : sizeof(PEB64);

	KAPC_STATE kapc;
	KeStackAttachProcess(pProc, &kapc);
	BOOLEAN success = FALSE;
	__try {
		ProbeForRead(pPebUser, pebSize, sizeof(UCHAR));
		RtlCopyMemory(pPeb, pPebUser, pebSize);
		success = TRUE;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		success = FALSE;
	}
	KeUnstackDetachProcess(&kapc);
	return success;
}


NTSTATUS ModifyProcessPid(ULONG Pid,ULONG PidOld, BOOLEAN Opt) {
	/*
	Opt: TRUE=保护 FALSE=取消
	Under Win10 : 0x440
	Under Win11 : 0x1d0
	*/

	PEPROCESS Proc = { 0 };
	NTSTATUS Stat = PsLookupProcessByProcessId((HANDLE)Pid, &Proc);
	if (!NT_SUCCESS(Stat)) {
		ObDereferenceObject(Proc);
		return Stat;
	}
	BYTE* PeAddr = (BYTE*)Proc;
	ULONG Offset = 0;
	ULONG Seed = 1949;
	if (MajorWindowsVer == 10) Offset = 0x440;
	if (MajorWindowsVer == 11) Offset = 0x1d0;

	HANDLE Rep = (HANDLE)(RtlRandom(&Seed) * 2 + 1); //随机生成一个Pid,为奇数

	if (Opt) {
		memcpy((PeAddr + Offset), &Rep, sizeof(HANDLE));
		ObDereferenceObject(Proc);
		return Stat;
	}
	else {
		if (Pid % 2 != 0) { //检测PID是否有效
			Stat = STATUS_INVALID_PARAMETER_2;
			ObDereferenceObject(Proc);
		}
		Rep = (HANDLE)PidOld;
		memcpy((PeAddr + Offset), &Rep, sizeof(HANDLE));
		ObDereferenceObject(Proc);
		return Stat;

	}
}