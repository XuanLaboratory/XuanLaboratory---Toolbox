#include <ntifs.h>
#include <ntddk.h>
#include "IoCtrl.h"
#include "FunctionCall.h"


BOOLEAN LockSymbolLink; //是否锁定符号链接 全局变量 
BOOLEAN SelfProtect = FALSE;
ULONG ClientProcessId; //客户端Id 全局变量
ULONG ClientParentProcessId; //客户端父进程Id 全局变量
ULONG MajorWindowsVer;

PEPROCESS ClientModifyPid; //修改客户端Pid

NTSTATUS DriverCreate(PDEVICE_OBJECT pdo, PIRP Irp) { //仅允许一个被打开的的句柄
	if (LockSymbolLink) {
		DbgPrint("Found another process want to open driver,access denied.\n");
		Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return STATUS_ACCESS_DENIED;
	}
	else {
		Irp->IoStatus.Status = STATUS_SUCCESS;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		LockSymbolLink = TRUE;
		return STATUS_SUCCESS;
	}

}

NTSTATUS DriverClose(PDEVICE_OBJECT pdo, PIRP Irp) {
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	LockSymbolLink = FALSE;
	return STATUS_SUCCESS;
}


NTSTATUS DriverDeviceControl(PDEVICE_OBJECT pdo, PIRP Irp) {
	UNREFERENCED_PARAMETER(pdo); //这里不要该参数	
	NTSTATUS Stat = STATUS_SUCCESS; //默认为成功
	if (IsDriverUnloading) { //判断驱动是否正在卸载
		Stat = STATUS_ACCESS_DENIED;
		Irp->IoStatus.Status = Stat;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return Stat;
	}
	PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(Irp);

	ULONG len = 0; //返回的长度
	ULONG BufferSizeIn = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength; //获取缓冲区长度
	ULONG BufferSizeOut = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

	switch (IoStackLocation->Parameters.DeviceIoControl.IoControlCode)
	{
	case INIT_DRIVER: {
		if (BufferSizeIn % sizeof(DRIVER_INIT_INFO) != 0) { //判断大小
			Stat = STATUS_INVALID_BUFFER_SIZE;
			break;
		}
		DRIVER_INIT_INFO* Init = (DRIVER_INIT_INFO*)Irp->AssociatedIrp.SystemBuffer; //驱动初始化
		ClientProcessId = Init->ClientProcessId; //设置客户端Id
		ClientParentProcessId = Init->ClientParentPid;
		MajorWindowsVer = Init->MajorVerInfo;
		
		break;
	}
	case GET_DRIVER_LAST_MSG: { //获取驱动最后一条消息 带结构
		PVOID ClientBuffer = Irp->AssociatedIrp.SystemBuffer;
		if (BufferSizeOut % sizeof(DRIVER_LAST_MSG) != 0) {
			Stat = STATUS_INVALID_BUFFER_SIZE;
			break;
		}
		DRIVER_LAST_MSG* Msg = (DRIVER_LAST_MSG*)ClientBuffer;
		memcpy(Msg->Buffer, DrvMsg.Buffer, 128);
		Msg->Type = DrvMsg.Type;
		len += sizeof(DRIVER_LAST_MSG);
		break;

	}

	case PROCESS_OPERATION: {
		PVOID ClientBuffer = Irp->AssociatedIrp.SystemBuffer;
		if (BufferSizeIn % sizeof(PROCESS_OPERATE_INFO) != 0) {
			Stat = STATUS_INVALID_BUFFER_SIZE;
			break;
		}
		PROCESS_OPERATE_INFO* Info = (PROCESS_OPERATE_INFO*)ClientBuffer;
		Stat = UniversalProcessOperation(Info);
		len += sizeof(PROCESS_OPERATE_INFO);
		break;

	}

	case SET_CLIENT_PROC_PROTECTION_METHOD: {
		CLIENT_PROC_PROTECT_METHOD* Method = (CLIENT_PROC_PROTECT_METHOD*)Irp->AssociatedIrp.SystemBuffer;
		if (BufferSizeIn % sizeof(CLIENT_PROC_PROTECT_METHOD) != 0) {
			Stat = STATUS_INVALID_BUFFER_SIZE;
			break;
		}
		Stat = ModifyProcessPid(ClientProcessId, ClientProcessId, Method->ModifyPid);
		Stat = Stat & ModifyProcessPid(ClientParentProcessId, ClientParentProcessId, Method->ModifyPid);
		SelfProtect = Method->ObRegisterCallback;
		len = +sizeof(CLIENT_PROC_PROTECT_METHOD);
		break;
	}

	case KERNEL_INFO_QUERY: {
		if (BufferSizeIn % sizeof(KERNEL_INFO_QUERY_DISPATCHER) != 0) {
			Stat = STATUS_INVALID_BUFFER_SIZE;
			break;
		}
		KERNEL_INFO_QUERY_DISPATCHER* Info = (KERNEL_INFO_QUERY_DISPATCHER*)Irp->AssociatedIrp.SystemBuffer;
		Stat = KernelInformationQueryDispatch(Info);
		len = +sizeof(KERNEL_INFO_QUERY_DISPATCHER);
		break;
	}

	default: {
		Stat = STATUS_INVALID_DEVICE_REQUEST; //未知的控制代码
		break; }
	}
	Irp->IoStatus.Status = Stat;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Stat;
}




NTSTATUS DeviceIoControlDispatcher(PIRP Irp, ULONG* IoLen) {
	PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	ULONG BufferSizeIn = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
	ULONG BufferSizeOut = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
	do {
		if (BufferSizeIn % sizeof(DEVICE_IO_CTRL_DISPATCHER) != 0) {
			return STATUS_INVALID_BUFFER_SIZE;
		}
		DEVICE_IO_CTRL_DISPATCHER* Dispatcher = (DEVICE_IO_CTRL_DISPATCHER*)Irp->AssociatedIrp.SystemBuffer;

		switch (Dispatcher->MajorFunctionCode)
		{
		case InitDriver: {
			if (Dispatcher->BufferInSize % sizeof(DRIVER_INIT_INFO) != 0) { //判断大小
				return STATUS_INVALID_BUFFER_SIZE;
			}

			DRIVER_INIT_INFO* Init = (DRIVER_INIT_INFO*)Irp->AssociatedIrp.SystemBuffer; //驱动初始化
			ClientProcessId = Init->ClientProcessId; //设置客户端Id
			ClientParentProcessId = Init->ClientParentPid;
			MajorWindowsVer = Init->MajorVerInfo;

			break;
		}
		default:
			return STATUS_INVALID_DEVICE_REQUEST;
		}
	} while (0);

	
}