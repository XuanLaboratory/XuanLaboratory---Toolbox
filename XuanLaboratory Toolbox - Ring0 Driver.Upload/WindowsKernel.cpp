#include <ntifs.h>
#include <ntimage.h>
#include <ntstrsafe.h>
#include "FunctionCall.h"

NTSTATUS KernelInformationQueryDispatch(KERNEL_INFO_QUERY_DISPATCHER* Info) { //ֱ�ӶԿͻ��˳����ڴ����д��
	KAPC_STATE kapc = { 0 };
	PEPROCESS Proc = { 0 };
	NTSTATUS Stat = PsLookupProcessByProcessId((HANDLE)ClientProcessId, &Proc);
	KeStackAttachProcess(Proc, &kapc);
	DbgPrint("Dispatcher call!\n");
	switch (Info->Operation)
	{
	case KernelRequestDllExportAddress: {
		UNICODE_STRING TarDll = { 0 };
		RtlInitUnicodeString(&TarDll, (PCWSTR)Info->BufferIn);
		Stat = EnumAddressOfLib(TarDll, Info->BufferOut);
		break;
	}
	default:
		Stat = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}


	KeUnstackDetachProcess(&kapc);
	ObDereferenceObject(Proc);
	return Stat;
}


NTSTATUS KernelMapFile(UNICODE_STRING FileName, HANDLE* phFile, HANDLE* phSection, PVOID* ppBaseAddress){
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE hFile = NULL;
	HANDLE hSection = NULL;
	OBJECT_ATTRIBUTES objectAttr = { 0 };
	IO_STATUS_BLOCK iosb = { 0 };
	PVOID pBaseAddress = NULL;
	SIZE_T viewSize = 0;

	// �����ļ�Ȩ��
	InitializeObjectAttributes(&objectAttr, &FileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	// ���ļ�
	status = ZwOpenFile(&hFile, GENERIC_READ, &objectAttr, &iosb, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	// �����ڶ���
	status = ZwCreateSection(&hSection, SECTION_MAP_READ | SECTION_MAP_WRITE, NULL, 0, PAGE_READWRITE, 0x1000000, hFile);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		return status;
	}
	// ӳ�䵽�ڴ�
	status = ZwMapViewOfSection(hSection, NtCurrentProcess(), &pBaseAddress, 0, 1024, 0, &viewSize, ViewShare, MEM_TOP_DOWN, PAGE_READWRITE);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hSection);
		ZwClose(hFile);
		return status;
	}

	// ��������
	*phFile = hFile;
	*phSection = hSection;
	*ppBaseAddress = pBaseAddress;

	return status;
}


ULONG64 GetAddressFromFunction(UNICODE_STRING DllFileName, PCHAR pszFunctionName){
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE hFile = NULL;
	HANDLE hSection = NULL;
	PVOID pBaseAddress = NULL;

	// �ڴ�ӳ���ļ�
	status = KernelMapFile(DllFileName, &hFile, &hSection, &pBaseAddress);
	if (!NT_SUCCESS(status))
	{
		return 0;
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

		if (_strnicmp(pszFunctionName, lpName, strlen(pszFunctionName)) == 0)
		{
			ZwUnmapViewOfSection(NtCurrentProcess(), pBaseAddress);
			ZwClose(hSection);
			ZwClose(hFile);

			return (ULONG64)lpFuncAddr;
		}
	}
	ZwUnmapViewOfSection(NtCurrentProcess(), pBaseAddress);
	ZwClose(hSection);
	ZwClose(hFile);
	return 0;
}


