#include <ntifs.h>
#include <ntddk.h>
#include "IoCtrl.h"
#include "FunctionCall.h"


BOOLEAN LockSymbolLink; //�Ƿ������������� ȫ�ֱ��� 
BOOLEAN SelfProtect = FALSE;
ULONG ClientProcessId; //�ͻ���Id ȫ�ֱ���
ULONG ClientParentProcessId; //�ͻ��˸�����Id ȫ�ֱ���
ULONG MajorWindowsVer;

PEPROCESS ClientModifyPid; //�޸Ŀͻ���Pid

NTSTATUS DriverCreate(PDEVICE_OBJECT pdo, PIRP Irp) { //������һ�����򿪵ĵľ��
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
	UNREFERENCED_PARAMETER(pdo); //���ﲻҪ�ò���	
	NTSTATUS Stat = STATUS_SUCCESS; //Ĭ��Ϊ�ɹ�
	if (IsDriverUnloading) { //�ж������Ƿ�����ж��
		Stat = STATUS_ACCESS_DENIED;
		Irp->IoStatus.Status = Stat;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return Stat;
	}
	PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(Irp);

	ULONG len = 0; //���صĳ���
	ULONG BufferSizeIn = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength; //��ȡ����������
	ULONG BufferSizeOut = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

	switch (IoStackLocation->Parameters.DeviceIoControl.IoControlCode)
	{
	case INIT_DRIVER: {
		if (BufferSizeIn % sizeof(DRIVER_INIT_INFO) != 0) { //�жϴ�С
			Stat = STATUS_INVALID_BUFFER_SIZE;
			break;
		}
		DRIVER_INIT_INFO* Init = (DRIVER_INIT_INFO*)Irp->AssociatedIrp.SystemBuffer; //������ʼ��
		ClientProcessId = Init->ClientProcessId; //���ÿͻ���Id
		ClientParentProcessId = Init->ClientParentPid;
		MajorWindowsVer = Init->MajorVerInfo;
		
		break;
	}
	case GET_DRIVER_LAST_MSG: { //��ȡ�������һ����Ϣ ���ṹ
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
		Stat = STATUS_INVALID_DEVICE_REQUEST; //δ֪�Ŀ��ƴ���
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
			if (Dispatcher->BufferInSize % sizeof(DRIVER_INIT_INFO) != 0) { //�жϴ�С
				return STATUS_INVALID_BUFFER_SIZE;
			}

			DRIVER_INIT_INFO* Init = (DRIVER_INIT_INFO*)Irp->AssociatedIrp.SystemBuffer; //������ʼ��
			ClientProcessId = Init->ClientProcessId; //���ÿͻ���Id
			ClientParentProcessId = Init->ClientParentPid;
			MajorWindowsVer = Init->MajorVerInfo;

			break;
		}
		default:
			return STATUS_INVALID_DEVICE_REQUEST;
		}
	} while (0);

	
}