#include <ntifs.h>

NTSTATUS File_ReadFile(LPCWSTR Path, LPCWSTR Data,ULONG BufferLen) {
	OBJECT_ATTRIBUTES Obj = { 0 };
	UNICODE_STRING uFileName;
	RtlInitUnicodeString(&uFileName, Path);
	InitializeObjectAttributes(&Obj, &uFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	NTSTATUS Stat = STATUS_SUCCESS;
	HANDLE hFile = 0;
	IO_STATUS_BLOCK ioblock = { 0 };
	//打开文件
	Stat = ZwCreateFile(&hFile, FILE_ALL_ACCESS, &Obj, &ioblock, 0, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_RANDOM_ACCESS | FILE_SYNCHRONOUS_IO_NONALERT, NULL, NULL);
	if (!NT_SUCCESS(Stat)) return ioblock.Status;
	ioblock = { 0 };

	//分配缓冲区
	FILE_STANDARD_INFORMATION fileinfo = { 0 };
	Stat = ZwQueryInformationFile(hFile, &ioblock, &fileinfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
	if (!NT_SUCCESS(Stat)|| BufferLen < fileinfo.AllocationSize.QuadPart) { 
		ZwClose(hFile); 
		return ioblock.Status; 
	}
	PVOID Buffer = ExAllocatePool(NonPagedPool, (SIZE_T)fileinfo.AllocationSize.QuadPart+1);
	RtlZeroMemory(Buffer, (SIZE_T)fileinfo.AllocationSize.QuadPart + 1);
	ioblock = { 0 };

	//读取文件
	Stat = ZwReadFile(hFile, NULL, NULL, NULL, &ioblock, Buffer, fileinfo.AllocationSize.QuadPart, 0, 0);
	if (!NT_SUCCESS(Stat)) {
		ExFreePool(Buffer);
		ZwClose(hFile);
		return ioblock.Status;
	}

	return Stat;

}