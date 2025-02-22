#include <ntifs.h>
#include <ntddk.h>
#include "FunctionCall.h"

UNICODE_STRING DeviceName, SymbolLink;
DRIVER_LAST_MSG DrvMsg = { 0 };
BOOLEAN IsDriverUnloading = false;

void DriverUnload_Function(PDRIVER_OBJECT DriverObject);
NTSTATUS __Init(PDRIVER_OBJECT DriverObject);


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS Status = STATUS_FAILED_DRIVER_ENTRY; //初始状态
	RtlInitUnicodeString(&DeviceName, L"\\Device\\XuanLabKernelModeDriver"); //初始化设备
	RtlInitUnicodeString(&SymbolLink, L"\\??\\XuanLabKernelModeDriver"); //初始化符号链接

	IoDeleteSymbolicLink(&SymbolLink); //首先删除一次符号链接

	Status = IoCreateDevice(DriverObject, 64, &DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DriverObject->DeviceObject); //创建设备
	if (!NT_SUCCESS(Status)) {
		IoDeleteDevice(DriverObject->DeviceObject);
		return Status;
	}

	Status = IoCreateSymbolicLink(&SymbolLink, &DeviceName); //创建符号链接
	if (!NT_SUCCESS(Status)) {
		IoDeleteSymbolicLink(&SymbolLink);
		IoDeleteDevice(DriverObject->DeviceObject);
		return Status;
	}

	//设置例程
	DriverObject->DriverUnload = DriverUnload_Function;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDeviceControl;


	//在此之后开始设置最后一条消息
	((PLDR_DATA)DriverObject->DriverSection)->Flags |= 0x20; //过掉MmVerifyCallbackFunction

	if(__Init(DriverObject)){
		SetDriverLastMsg("Driver initialized successfully!\n", 1);
	}
	return STATUS_SUCCESS;
}



void DriverUnload_Function(PDRIVER_OBJECT DriverObject) {
	NTSTATUS Status = STATUS_FAIL_CHECK;
	IsDriverUnloading = true;	
	SetDriverLastMsg("Driver is unloading\n", 2);
	UnRegNotifyCallbacks(); //反注册进程打开回调
	Status = IoDeleteSymbolicLink(&SymbolLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}


void SetDriverLastMsg(char* msg,ULONG level) {
	if (msg == nullptr) return;
	if (strlen(msg) > 127) return; //DRIVER_LAST_MSG中缓冲区大小为128
	if (!(level > 0 && level < 5)) return;
	DrvMsg.Type = level;
	memcpy(DrvMsg.Buffer, msg, strlen(msg));
}



NTSTATUS __Init(PDRIVER_OBJECT DriverObject) {
	NTSTATUS Stat = STATUS_SUCCESS;
	Stat = __ProtectClientByRegisterCallback();
	if (!NT_SUCCESS(Stat)) {
		SetDriverLastMsg("Failed to register process notify!\n", 2);

	}

	
	return Stat;
}