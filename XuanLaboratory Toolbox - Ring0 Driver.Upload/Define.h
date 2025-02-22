#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include "Peb.h"
#include "OffsetList.h"

typedef unsigned char       BYTE;

//Structures

typedef struct _DRIVER_INIT_INFO {
	ULONG ClientProcessId; //客户端PID
	ULONG ClientParentPid;
	ULONG ClientHandle;
	BOOLEAN IfDeleteFile;
	ULONG MajorVerInfo;
}DRIVER_INIT_INFO, * PDRIVER_INIT_INFO;

typedef struct _DRIVER_LAST_MSG {
	ULONG Type; //1=Info 2=Warn 3=Error 4=Fatal
	char Buffer[128];
}DRIVER_LAST_MSG, * PDRIVER_LAST_MSG;

typedef struct _PROCESS_OPERATE_INFO {
	ULONG ProcessId;
	ULONG Operation;
	PVOID BufferIn;
	ULONG BufferInSize;
	PVOID BufferOut;
	ULONG BufferOutSize;
	NTSTATUS Status;
}PROCESS_OPERATE_INFO, * PPROCESS_OPERATE_INFO;

typedef struct _DEVICE_IO_CTRL_DISPATCHER {
	int MajorFunctionCode;
	int MinorFunctionCode;
	PVOID BufferIn;
	ULONG BufferInSize;
	PVOID BufferOut;
	ULONG BufferOutSize;
	ULONG Parameter1;
	ULONG Parameter2;
}DEVICE_IO_CTRL_DISPATCHER, * PDEVICE_IO_CTRL_DISPATCHER;

typedef enum _DeviceIoCtrlDispatcher {
	InitDriver,
	ProcessOperation,
	SetClientProcessProtect,
	QueryKernelInformation,
};

typedef enum _ProcessOperateInfo {
	ProcessTerminate,
	ProcessTerminateProcTree,
	ProcessSuspend,
	ProcessResume,
	ProcessWriteMemory,
	ProcessReadMemory,
	ProcessQueryPebData,
}ProcessOperateInfo;

typedef struct _KERNEL_INFO_QUERY_DISPATCHER {
	ULONG Operation;
	PVOID BufferIn;
	ULONG BufferInSize;
	PVOID BufferOut;
	ULONG BufferOutSize;
}KERNEL_INFO_QUERY_DISPATCHER, * PKERNEL_INFO_QUERY_DISPATCHER;

typedef enum _KernelInfoQueryOpt {
	KernelRequestDllExportAddress,
}KernelInfoQueryOpt;

typedef struct _DLL_EXPORT_INFO_NT {
	CHAR FunctionName[48];
	ULONG64 Address;
	ULONG Index;
}DLL_EXPORT_INFO_NT, * PDLL_EXPORT_INFO_NT;

typedef struct _CLIENT_PROC_PROTECT_METHOD {
	BOOLEAN ObRegisterCallback;
	BOOLEAN SSDT_Hook;
	BOOLEAN ModifyPid;
}CLIENT_PROC_PROTECT_METHOD, * PCLIENT_PROC_PROTECT_METHOD;


#ifdef _WIN64
typedef struct _LDR_DATA{
	 LIST_ENTRY listEntry;//16
	 ULONG64 __Undefined1;//8
	 ULONG64 __Undefined2;//8
	 ULONG64 __Undefined3;//8
	 ULONG64 NonPagedDebugInfo;//8
	 ULONG64 DllBase;//8
	 ULONG64 EntryPoint;//8
	 ULONG SizeOfImage;//4
	 UNICODE_STRING path;//16
	 UNICODE_STRING name;//16
	 ULONG   Flags;
}LDR_DATA, * PLDR_DATA;
#else
typedef struct _LDR_DATA{
	LIST_ENTRY listEntry;
	ULONG unknown1;
	ULONG unknown2;
	ULONG unknown3;
	ULONG unknown4;
	ULONG unknown5;
	ULONG unknown6;
	ULONG unknown7;
	UNICODE_STRING path;
	UNICODE_STRING name;
	ULONG   Flags;
}LDR_DATA, * PLDR_DATA;
#endif

//Global Variables
extern BOOLEAN LockSymbolLink;
extern BOOLEAN SelfProtect;
extern ULONG ClientProcessId;
extern ULONG ClientParentProcessId;
extern DRIVER_LAST_MSG DrvMsg; //驱动消息
extern BOOLEAN IsDriverUnloading;
extern ULONG MajorWindowsVer;


//API
extern "C" NTKERNELAPI NTSTATUS PsSuspendProcess(PEPROCESS proc);    //暂停进程
extern "C" NTKERNELAPI NTSTATUS PsResumeProcess(PEPROCESS proc);    //恢复进程
extern "C" NTKERNELAPI HANDLE PsGetProcessInheritedFromUniqueProcessId(IN PEPROCESS Process); //未公开进行导出 获取父进程Pid
extern "C" NTKERNELAPI NTSTATUS ZwQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
extern "C" NTKERNELAPI NTSTATUS ZwSetInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength);
extern "C" NTKERNELAPI PVOID PsGetProcessPeb(_In_ PEPROCESS Process);
extern "C" NTKERNELAPI BOOLEAN PsGetProcessWow64Process(PEPROCESS Process);