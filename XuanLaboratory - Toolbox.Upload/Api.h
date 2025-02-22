#pragma once
#include <Windows.h>

typedef NTSTATUS(NTAPI* pfnNtQueryInformationProcess)(
    _In_      HANDLE           ProcessHandle,
    _In_      UINT             ProcessInformationClass,
    _Out_     PVOID            ProcessInformation,
    _In_      ULONG            ProcessInformationLength,
    _Out_opt_ PULONG           ReturnLength
    ); 

typedef NTSTATUS(NTAPI* pfnNtWow64ReadVirtualMemory64)(
    IN HANDLE ProcessHandle,
    IN PVOID64 BaseAddress,
    OUT PVOID Buffer,
    IN ULONG64 Size,
    OUT PULONG64 NumberOfBytesRead
    );

