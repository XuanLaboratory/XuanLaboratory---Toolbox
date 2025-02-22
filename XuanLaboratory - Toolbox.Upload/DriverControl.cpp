#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(suppress : 4996)
#include <Windows.h>
#include <stdio.h>
#include <VersionHelpers.h>
#include "FunctionCall.h"
#include "Structures.h"
#include "IoCtrl.h"
#include "Console.h"
#include "VariableAndStatics.h"

/*
ע�� DeviceIoControl����ֵ��NTSTATUS����
*/

CLIENT_PROC_PROTECT_METHOD Protect; //In file

DWORD ProgramMode; //Global
HANDLE hDriver; //Global
BOOL IsModifyPidEnabled = FALSE;

LPCSTR Path = NULL; //����·��

VOID __MontiorDriverInfo(void);

BOOL __InitializeDriver() {
	//��ж�ط���,�����°�װ
	StopService("XuanLab_KernelModeService");
	UnInstallService("XuanLab_KernelModeService");
	Path = ReadConfigString(Cfg, "DriverOptions", "DriverPath");
	//��װ��������,����:�ں�����;�Զ�����;����ȼ�:��������
	if (!FileExistsStaus(Path)) {
		printA("Invalid file,please select a valid driver file.\n", 3);
		BOOL Break = 0;
		char* PathRet;
		while (!Break) {
			PathRet = CreateFileSelectDlg("Ĭ�ϵ��ļ��޷��ҵ�,��ָ��һ����Ч�������ļ�.\n(Ĭ����ΪXuanLaboratoryToolbox-Ring0Driver.sys");
			if (PathRet == nullptr) {
				if (MessageBoxA(GetConsoleWindow(), "��û��ָ�������ļ�,Ҫ�˳���?\n", "����", MB_YESNO | MB_ICONWARNING) == IDYES) {
					return 0;
				}
				else {
					continue;
				}
			}
			else {
				if (CheckIfExecutable(PathRet)) {
					printA("Saving path to registry...\n", 1);
					if (!WriteConfigString(Cfg,"DriverOptions","DriverPath",PathRet)) {
						printA("Failed to save path to registry!\n", 3);
					}
					break;
				}
				else {
					MessageBoxA(GetConsoleWindow(), "��Ч���ļ�,������ָ��", "����", MB_ICONERROR | MB_OK);
					continue;
				}
			}

		}
		
	}
	printA("Installing service...\n", 1);
	BOOL Status = InstallService("XuanLab_KernelModeService", "XuanLab_KernelModeService", Path, "XuanLaboratory - Toolbox Ring 0 Driver", SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_CRITICAL);
	if (!Status) {
		printA("Installing the service failed! ", 3);
		printf("%s\n", TranslateGetLastErrorMsg(GetLastError()));
		return false;
	}

	printA("Launching service...\n", 1);
	Status = LaunchService("XuanLab_KernelModeService");
	if (!Status) {
		printA("Launch the service failed,maybe you need to install the certificate of this program. Info:", 3);
		printf("%s\n\n", TranslateGetLastErrorMsg(GetLastError()));
		UnInstallService("XuanLab_KernelModeService");
		DWORD Btn = MessageBoxA(GetConsoleWindow(), "δ��������������,���������Ϊû�д򿪵���ģʽ���ر�����ǩ��,Ҳ�����Ǳ�ɱ�������ֹ��,����ϵͳ����������.\n����Գ��Դ򿪵���ģʽ���ر�����ǩ��\n\n���[��]�����г���", "����", MB_YESNO | MB_ICONERROR);
		if (Btn==IDYES) {
			if (EnableNoSignedDriverLoad()) {
				printA("Configuration modified successfully,please reboot to apply.\n", 1);
				if (MessageBoxA(GetConsoleWindow(), "���óɹ�,Ҫ����������Ӧ�ø�����,����ǰ,Ӧ�������������ĵ�\n�Ƿ�����?", "����", MB_YESNO | MB_ICONWARNING) == IDYES) {
					ExitWindowsEx(EWX_REBOOT, EWX_FORCE);
				}
			}
			else {
				printA("Failed to modify configuration.\n", 2);
			}
		}
		return false;
	}
	//����������
	printA("Checking symbol link...\n", 1);
	hDriver = CreateFileA("\\\\.\\XuanLabKernelModeDriver", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDriver == INVALID_HANDLE_VALUE) {
		printA("Symbol Link not found,driver maybe failed to initialize!\n", 2);
		StopService("XuanLab_KernelModeService");
		UnInstallService("XuanLab_KernelModeService");
		SetWindowTextA(GetConsoleWindow(), "XuanLaboratory - Toolbox | Ring3 Mode");
		ProgramMode = Mode_Ring3_Only;
		return false;
	}
	ProgramMode = Mode_Both;
	DRIVER_INIT_INFO DrvInit = { 0 };
	DrvInit.ClientProcessId = GetCurrentProcessId();
	DrvInit.ClientParentPid = ParentPid;
	DrvInit.ClientHandle = (ULONG)GetCurrentProcess();
	DrvInit.IfDeleteFile = FALSE;
	DrvInit.MajorVerInfo = Ver.dwMajor;

	Protect = { 0 };
	Protect.ModifyPid = FALSE;
	Protect.ObRegisterCallback = FALSE;
	Protect.SSDT_Hook = FALSE;
	if (ReadConfigBoolean(Cfg,"DriverOptions","SelfProtect")) {
		Protect.ObRegisterCallback = TRUE;
	}
	else {
		printA("Client self protection disabled\n", 2);
	}

	if (ReadConfigBoolean(Cfg,"DriverOptions","SelfProtectByModifyPid")) {
		DWORD ret = 0;
		ret = MessageBoxA(GetConsoleWindow(), "����:��������ͨ���޸�������̱�ʶ���ķ�������������,��δ�ָ�ǰ��ֹ���̺�ᵼ��ϵͳ����.\n����ֱ�ӹر�,����ᵼ������!\nһ��Ҫ������֮ǰ�������������ļ�,���������ʧ!\n\n���[��]����.", "���ܾ���", MB_YESNO | MB_ICONWARNING);
		if (ret == IDYES) {
			ret = MessageBoxA(GetConsoleWindow(), "��󾯸�:�Ƿ�Ҫͨ���޸�������̱�ʶ���ķ�������������?", "����", MB_YESNO | MB_ICONWARNING);
			if (ret == IDYES) {
				Protect.ModifyPid = TRUE;
				IsModifyPidEnabled = TRUE;
				HMENU MainMenu = GetSystemMenu(GetConsoleWindow(), false);		// ���ƻ��޸Ķ����ʴ��ڲ˵�
				RemoveMenu(MainMenu, SC_CLOSE, MF_BYCOMMAND);	// ��ָ���˵�ɾ��һ���˵�������һ���Ӳ˵�
				DrawMenuBar(MainWindow);
			}
		}
	}

	printA("Configuring driver...\n", 1);

	if (!DeviceIoControl(hDriver, INIT_DRIVER, &DrvInit, sizeof(DrvInit), 0, 0, 0, 0)) {
		printA("Configuring driver failed!\n", 3);
	}

	if (!DeviceIoControl(hDriver, SET_CLIENT_PROC_PROTECTION_METHOD, &Protect, sizeof(Protect), 0, 0, 0, 0)) {
		printA("Send protection config to driver failed!\n", 3);
	}
	return true;
}

BOOL Driver_TerminateProcess(DWORD Pid) {
	if (hDriver == INVALID_HANDLE_VALUE) return 0;
	PROCESS_OPERATE_INFO Info = { 0 };
	Info.ProcessId = Pid;
	Info.Operation = ProcessTerminate;
	BOOL Ret = DeviceIoControl(hDriver, PROCESS_OPERATION, &Info, sizeof(Info), 0, 0, 0, 0);
	return Ret;
}

BOOL Driver_SuspendProcess(DWORD Pid) {
	if (hDriver == INVALID_HANDLE_VALUE) return 0;
	PROCESS_OPERATE_INFO Info = { 0 };
	Info.ProcessId = Pid;
	Info.Operation = ProcessSuspend;
	BOOL Ret = DeviceIoControl(hDriver, PROCESS_OPERATION, &Info, sizeof(Info), 0, 0, 0, 0);
	return Ret;
}

BOOL Driver_ResumeProcess(DWORD Pid) {
	if (hDriver == INVALID_HANDLE_VALUE) return 0;
	PROCESS_OPERATE_INFO Info = { 0 };
	Info.ProcessId = Pid;
	Info.Operation = ProcessResume;
	BOOL Ret = DeviceIoControl(hDriver, PROCESS_OPERATION, &Info, sizeof(Info), 0, 0, 0, 0);
	return Ret;
}

BOOL Driver_TerminateProcessTree(DWORD Pid) {
	if (hDriver == INVALID_HANDLE_VALUE) return 0;
	PROCESS_OPERATE_INFO Info = { 0 };
	Info.ProcessId = Pid;
	Info.Operation = ProcessTerminateProcTree;
	BOOL Ret = DeviceIoControl(hDriver, PROCESS_OPERATION, &Info, sizeof(Info), 0, 0, 0, 0);
	return Ret;
}

BOOL Driver_WriteProcessMemory(DWORD Pid,PVOID pBaseAddr,PVOID pSourceAddr,ULONG len) {
	if (hDriver == INVALID_HANDLE_VALUE) return 0;
	PROCESS_OPERATE_INFO Info = { 0 };
	Info.ProcessId = Pid;
	Info.Operation = ProcessWriteMemory;
	Info.BufferIn = pBaseAddr;
	Info.BufferOut = pSourceAddr;
	Info.BufferOutSize = len;
	BOOL Ret = DeviceIoControl(hDriver, PROCESS_OPERATION, &Info, sizeof(Info), 0, 0, 0, 0);
	return Ret;
}

BOOL Driver_ReadProcessMemory(DWORD Pid, PVOID pBaseAddr, ULONG ReadLen, PVOID pBuffer, ULONG BufferLen) {
	if (hDriver == INVALID_HANDLE_VALUE) return 0;
	PROCESS_OPERATE_INFO Info = { 0 };
	Info.ProcessId = Pid;
	Info.Operation = ProcessReadMemory;
	Info.BufferIn = pBaseAddr;
	Info.BufferInSize = ReadLen;
	Info.BufferOut = pBuffer;
	Info.BufferOutSize = BufferLen;
	BOOL Ret = DeviceIoControl(hDriver, PROCESS_OPERATION, &Info, sizeof(Info), 0, 0, 0, 0);
	return Ret;
}

BOOL Driver_QueryProcessPebData(DWORD Pid,PEB64* Peb) {
	if (hDriver == INVALID_HANDLE_VALUE) return 0;
	PROCESS_OPERATE_INFO Info = { 0 };
	Info.ProcessId = Pid;
	Info.Operation = ProcessQueryPebData;
	Info.BufferOut = Peb;
	Info.BufferOutSize = sizeof(PEB64);
	BOOL Ret = DeviceIoControl(hDriver, PROCESS_OPERATION, &Info, sizeof(Info), 0, 0, 0, 0);
	return Ret;
}

BOOL Demo(void) {
	KERNEL_INFO_QUERY_DISPATCHER Info = { 0 };
	Info.Operation = KernelRequestDllExportAddress;
	WCHAR dll[37] = L"\\??\\C:\\Windows\\System32\\ntdll.dll";
	Info.BufferIn = &dll;
	DLL_EXPORT_INFO_NT* Array = (DLL_EXPORT_INFO_NT*)malloc(sizeof(DLL_EXPORT_INFO_NT) * 4096);
	Info.BufferOut = Array;
	BOOL Ret = DeviceIoControl(hDriver, KERNEL_INFO_QUERY, &Info, sizeof(Info), 0, 0, 0, 0);
	for (int i = 0; i < 1000; i++) {
		printf("%s - %d - %p\n", Array[i].FunctionName, Array[i].Index, Array[i].Address);

	}
	return Ret;
}