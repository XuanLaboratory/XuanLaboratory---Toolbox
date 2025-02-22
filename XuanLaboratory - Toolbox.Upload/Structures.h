#pragma once
#include <Windows.h>

typedef VOID (NTAPI* PPS_POST_PROCESS_INIT_ROUTINEU) (VOID);

typedef LONG KPRIORITY, * PKPRIORITY;

typedef enum _PROCESSINFOCLASS
{
    ProcessBasicInformation, 					// 0x0
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,							// 0x7
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    ProcessDeviceMap,
    ProcessSessionInformation,
    ProcessForegroundInformation,
    ProcessWow64Information, 					// 0x1A
    ProcessImageFileName, 						// 0x1B
    ProcessLUIDDeviceMapsEnabled,
    ProcessBreakOnTermination,
    ProcessDebugObjectHandle,					// 0x1E
    ProcessDebugFlags, 							// 0x1F
    ProcessHandleTracing,
    ProcessIoPriority,
    ProcessExecuteFlags,
    ProcessResourceManagement,
    ProcessCookie,
    ProcessImageInformation,
    ProcessCycleTime,
    ProcessPagePriority,
    ProcessInstrumentationCallback,
    ProcessThreadStackAllocation,
    ProcessWorkingSetWatchEx,
    ProcessImageFileNameWin32,
    ProcessImageFileMapping,
    ProcessAffinityUpdateMode,
    ProcessMemoryAllocationMode,
    ProcessGroupInformation,
    ProcessTokenVirtualizationEnabled,
    ProcessConsoleHostProcess,
    ProcessWindowInformation,
    ProcessHandleInformation,
    ProcessMitigationPolicy,
    ProcessDynamicFunctionTableInformation,
    ProcessHandleCheckingMode,
    ProcessKeepAliveCount,
    ProcessRevokeFileHandles,
    ProcessWorkingSetControl,
    ProcessHandleTable,
    ProcessCheckStackExtentsMode,
    ProcessCommandLineInformation,
    ProcessProtectionInformation,
    ProcessMemoryExhaustion,
    ProcessFaultInformation,
    ProcessTelemetryIdInformation,
    ProcessCommitReleaseInformation,
    ProcessDefaultCpuSetsInformation,
    ProcessAllowedCpuSetsInformation,
    ProcessSubsystemProcess,
    ProcessJobMemoryInformation,
    ProcessInPrivate,
    ProcessRaiseUMExceptionOnInvalidHandleClose,
    ProcessIumChallengeResponse,
    ProcessChildProcessInformation,
    ProcessHighGraphicsPriorityInformation,
    ProcessSubsystemInformation,
    ProcessEnergyValues,
    ProcessActivityThrottleState,
    ProcessActivityThrottlePolicy,
    ProcessWin32kSyscallFilterInformation,
    ProcessDisableSystemAllowedCpuSets,
    ProcessWakeInformation,
    ProcessEnergyTrackingState,
    ProcessManageWritesToExecutableMemory,
    ProcessCaptureTrustletLiveDump,
    ProcessTelemetryCoverage,
    ProcessEnclaveInformation,
    ProcessEnableReadWriteVmLogging,
    ProcessUptimeInformation,
    ProcessImageSection,
    ProcessDebugAuthInformation,
    ProcessSystemResourceManagement,
    ProcessSequenceNumber,
    ProcessLoaderDetour,
    ProcessSecurityDomainInformation,
    ProcessCombineSecurityDomainsInformation,
    ProcessEnableLogging,
    ProcessLeapSecondInformation,
    ProcessFiberShadowStackAllocation,
    ProcessFreeFiberShadowStackAllocation,
    MaxProcessInfoClass
} PROCESSINFOCLASS;

typedef struct _PEB_LDR_DATAU {
    BYTE Reserved1[8];
    PVOID Reserved2[3];
    LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATAU, * PPEB_LDR_DATAU;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2)] USHORT* Buffer;
#else // MIDL_PASS
    _Field_size_bytes_part_opt_(MaximumLength, Length) PWCH   Buffer;
#endif // MIDL_PASS
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;
typedef const UNICODE_STRING* PCUNICODE_STRING;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength), length_is(Length)]
#endif // MIDL_PASS
    _Field_size_bytes_part_opt_(MaximumLength, Length) PCHAR Buffer;
} STRING;
typedef STRING* PSTRING;
typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;

typedef struct _STRING32 {
    USHORT   Length;
    USHORT   MaximumLength;
    ULONG  Buffer;
} STRING32;
typedef STRING32* PSTRING32;

typedef STRING32 UNICODE_STRING32;
typedef UNICODE_STRING32* PUNICODE_STRING32;

//User mode peb def
typedef struct _RTL_USER_PROCESS_PARAMETERSU {
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERSU, * PRTL_USER_PROCESS_PARAMETERSU;

typedef struct _PEBU {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATAU Ldr;
    PRTL_USER_PROCESS_PARAMETERSU ProcessParameters;
    PVOID Reserved4[3];
    PVOID AtlThunkSListPtr;
    PVOID Reserved5;
    ULONG Reserved6;
    PVOID Reserved7;
    ULONG Reserved8;
    ULONG AtlThunkSListPtr32;
    PVOID Reserved9[45];
    BYTE Reserved10[96];
    PPS_POST_PROCESS_INIT_ROUTINEU PostProcessInitRoutine;
    BYTE Reserved11[128];
    PVOID Reserved12[1];
    ULONG SessionId;
} PEBU, * PPEBU;

//end

typedef struct _PROCESS_BASIC_INFORMATION
{
    NTSTATUS ExitStatus;
    PPEBU PebBaseAddress;
    KAFFINITY AffinityMask;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, * PPROCESS_BASIC_INFORMATION;

typedef PROCESS_BASIC_INFORMATION* PPROCESS_BASIC_INFORMATION;


typedef struct _PROCESS_BASIC_INFORMATION64 {
    NTSTATUS ExitStatus;
    UINT32 Reserved0;
    UINT64 PebBaseAddress;
    UINT64 AffinityMask;
    UINT32 BasePriority;
    UINT32 Reserved1;
    UINT64 UniqueProcessId;
    UINT64 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION64;


typedef struct _PROCESS_BASIC_INFORMATION32 {
    NTSTATUS ExitStatus;
    UINT32 PebBaseAddress;
    UINT32 AffinityMask;
    UINT32 BasePriority;
    UINT32 UniqueProcessId;
    UINT32 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION32;


//Krnl mode peb def
typedef struct _CURDIR              // 2 elements, 0x18 bytes (sizeof) 
{
    /*0x000*/     struct _UNICODE_STRING DosPath; // 3 elements, 0x10 bytes (sizeof) 
    /*0x010*/     VOID* Handle;
}CURDIR, * PCURDIR;

typedef struct _RTL_DRIVE_LETTER_CURDIR // 4 elements, 0x18 bytes (sizeof) 
{
    /*0x000*/     UINT16       Flags;
    /*0x002*/     UINT16       Length;
    /*0x004*/     ULONG32      TimeStamp;
    /*0x008*/     struct _STRING DosPath;             // 3 elements, 0x10 bytes (sizeof) 
}RTL_DRIVE_LETTER_CURDIR, * PRTL_DRIVE_LETTER_CURDIR;

typedef enum _SYSTEM_DLL_TYPE  // 7 elements, 0x4 bytes
{
    PsNativeSystemDll = 0 /*0x0*/,
    PsWowX86SystemDll = 1 /*0x1*/,
    PsWowArm32SystemDll = 2 /*0x2*/,
    PsWowAmd64SystemDll = 3 /*0x3*/,
    PsWowChpeX86SystemDll = 4 /*0x4*/,
    PsVsmEnclaveRuntimeDll = 5 /*0x5*/,
    PsSystemDllTotalTypes = 6 /*0x6*/
}SYSTEM_DLL_TYPE, * PSYSTEM_DLL_TYPE;

typedef struct _EWOW64PROCESS        // 3 elements, 0x10 bytes (sizeof) 
{
    /*0x000*/     VOID* Peb;
    /*0x008*/     UINT16       Machine;
    /*0x00A*/     UINT8        _PADDING0_[0x2];
    /*0x00C*/     enum _SYSTEM_DLL_TYPE NtdllType;
}EWOW64PROCESS, * PEWOW64PROCESS;

typedef struct _RTL_USER_PROCESS_PARAMETERS                // 37 elements, 0x440 bytes (sizeof) 
{
    /*0x000*/     ULONG32      MaximumLength;
    /*0x004*/     ULONG32      Length;
    /*0x008*/     ULONG32      Flags;
    /*0x00C*/     ULONG32      DebugFlags;
    /*0x010*/     VOID* ConsoleHandle;
    /*0x018*/     ULONG32      ConsoleFlags;
    /*0x01C*/     UINT8        _PADDING0_[0x4];
    /*0x020*/     VOID* StandardInput;
    /*0x028*/     VOID* StandardOutput;
    /*0x030*/     VOID* StandardError;
    /*0x038*/     struct _CURDIR CurrentDirectory;                       // 2 elements, 0x18 bytes (sizeof)   
    /*0x050*/     struct _UNICODE_STRING DllPath;                        // 3 elements, 0x10 bytes (sizeof)   
    /*0x060*/     struct _UNICODE_STRING ImagePathName;                  // 3 elements, 0x10 bytes (sizeof)   
    /*0x070*/     struct _UNICODE_STRING CommandLine;                    // 3 elements, 0x10 bytes (sizeof)   
    /*0x080*/     VOID* Environment;
    /*0x088*/     ULONG32      StartingX;
    /*0x08C*/     ULONG32      StartingY;
    /*0x090*/     ULONG32      CountX;
    /*0x094*/     ULONG32      CountY;
    /*0x098*/     ULONG32      CountCharsX;
    /*0x09C*/     ULONG32      CountCharsY;
    /*0x0A0*/     ULONG32      FillAttribute;
    /*0x0A4*/     ULONG32      WindowFlags;
    /*0x0A8*/     ULONG32      ShowWindowFlags;
    /*0x0AC*/     UINT8        _PADDING1_[0x4];
    /*0x0B0*/     struct _UNICODE_STRING WindowTitle;                    // 3 elements, 0x10 bytes (sizeof)   
    /*0x0C0*/     struct _UNICODE_STRING DesktopInfo;                    // 3 elements, 0x10 bytes (sizeof)   
    /*0x0D0*/     struct _UNICODE_STRING ShellInfo;                      // 3 elements, 0x10 bytes (sizeof)   
    /*0x0E0*/     struct _UNICODE_STRING RuntimeData;                    // 3 elements, 0x10 bytes (sizeof)   
    /*0x0F0*/     struct _RTL_DRIVE_LETTER_CURDIR CurrentDirectores[32];
    /*0x3F0*/     UINT64       EnvironmentSize;
    /*0x3F8*/     UINT64       EnvironmentVersion;
    /*0x400*/     VOID* PackageDependencyData;
    /*0x408*/     ULONG32      ProcessGroupId;
    /*0x40C*/     ULONG32      LoaderThreads;
    /*0x410*/     struct _UNICODE_STRING RedirectionDllName;             // 3 elements, 0x10 bytes (sizeof)   
    /*0x420*/     struct _UNICODE_STRING HeapPartitionName;              // 3 elements, 0x10 bytes (sizeof)   
    /*0x430*/     UINT64* DefaultThreadpoolCpuSetMasks;
    /*0x438*/     ULONG32      DefaultThreadpoolCpuSetMaskCount;
    /*0x43C*/     UINT8        _PADDING2_[0x4];
}RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB_LDR_DATA                            // 9 elements, 0x58 bytes (sizeof) 
{
    /*0x000*/     ULONG32      Length;
    /*0x004*/     UINT8        Initialized;
    /*0x005*/     UINT8        _PADDING0_[0x3];
    /*0x008*/     VOID* SsHandle;
    /*0x010*/     struct _LIST_ENTRY InLoadOrderModuleList;           // 2 elements, 0x10 bytes (sizeof) 
    /*0x020*/     struct _LIST_ENTRY InMemoryOrderModuleList;         // 2 elements, 0x10 bytes (sizeof) 
    /*0x030*/     struct _LIST_ENTRY InInitializationOrderModuleList; // 2 elements, 0x10 bytes (sizeof) 
    /*0x040*/     VOID* EntryInProgress;
    /*0x048*/     UINT8        ShutdownInProgress;
    /*0x049*/     UINT8        _PADDING1_[0x7];
    /*0x050*/     VOID* ShutdownThreadId;
}PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB64
{
    UCHAR InheritedAddressSpace;
    UCHAR ReadImageFileExecOptions;
    UCHAR BeingDebugged;
    UCHAR BitField;
    ULONG64 Mutant;
    ULONG64 ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    ULONG64 SubSystemData;
    ULONG64 ProcessHeap;
    ULONG64 FastPebLock;
    ULONG64 AtlThunkSListPtr;
    ULONG64 IFEOKey;
    ULONG64 CrossProcessFlags;
    ULONG64 UserSharedInfoPtr;
    ULONG SystemReserved;
    ULONG AtlThunkSListPtr32;
    ULONG64 ApiSetMap;
} PEB64, * PPEB64;

#pragma pack(4)
typedef struct _PEB32
{
    UCHAR InheritedAddressSpace;
    UCHAR ReadImageFileExecOptions;
    UCHAR BeingDebugged;
    UCHAR BitField;
    ULONG Mutant;
    ULONG ImageBaseAddress;
    ULONG Ldr;
    ULONG ProcessParameters;
    ULONG SubSystemData;
    ULONG ProcessHeap;
    ULONG FastPebLock;
    ULONG AtlThunkSListPtr;
    ULONG IFEOKey;
    ULONG CrossProcessFlags;
    ULONG UserSharedInfoPtr;
    ULONG SystemReserved;
    ULONG AtlThunkSListPtr32;
    ULONG ApiSetMap;
} PEB32, * PPEB32;

typedef struct _PEB_LDR_DATA32
{
    ULONG Length;
    BOOLEAN Initialized;
    ULONG SsHandle;
    LIST_ENTRY32 InLoadOrderModuleList;
    LIST_ENTRY32 InMemoryOrderModuleList;
    LIST_ENTRY32 InInitializationOrderModuleList;
    ULONG EntryInProgress;
} PEB_LDR_DATA32, * PPEB_LDR_DATA32;

typedef struct _LDR_DATA_TABLE_ENTRY32
{
    LIST_ENTRY32 InLoadOrderLinks;
    LIST_ENTRY32 InMemoryOrderModuleList;
    LIST_ENTRY32 InInitializationOrderModuleList;
    ULONG DllBase;
    ULONG EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING32 FullDllName;
    UNICODE_STRING32 BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union
    {
        LIST_ENTRY32 HashLinks;
        ULONG SectionPointer;
    }u1;
    ULONG CheckSum;
    union
    {
        ULONG TimeDateStamp;
        ULONG LoadedImports;
    }u2;
    ULONG EntryPointActivationContext;
    ULONG PatchInformation;
} LDR_DATA_TABLE_ENTRY32, * PLDR_DATA_TABLE_ENTRY32;

#pragma pack()

//end



typedef struct _VERSION_INFO {
    DWORD dwMajor;
    DWORD dwMinor;
    DWORD dwBuild;
    char* String;
}VERSION_INFO, * PVERSION_INFO;

typedef enum _ProgramMode {
	Mode_Ring3_Only,
	Mode_Ring0_Only,
	Mode_Both
};

typedef struct _CONSOLE_PRINTER_INFO {
    const char* msg;
    DWORD level;
    DWORD value;
}CONSOLE_PRINTER_INFO, * PCONSOLE_PRINTER_INFO;

typedef struct _DRIVER_INIT_INFO {
    ULONG ClientProcessId; //¿Í»§¶ËPID
    ULONG ClientParentPid;
    ULONG ClientHandle;
    BOOLEAN IfDeleteFile;
    ULONG MajorVerInfo;
}DRIVER_INIT_INFO, * PDRIVER_INIT_INFO;


typedef struct _CLIENT_PROC_PROTECT_METHOD {
    BOOLEAN ObRegisterCallback;
    BOOLEAN SSDT_Hook;
    BOOLEAN ModifyPid;
}CLIENT_PROC_PROTECT_METHOD, * PCLIENT_PROC_PROTECT_METHOD;

typedef struct _DRIVER_LAST_MSG {
    ULONG Type; //1=Info 2=Warn 3=Error 4=Fatal
    CHAR Buffer[128];
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