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
注意 DeviceIoControl返回值由NTSTATUS决定
*/

CLIENT_PROC_PROTECT_METHOD Protect; //In file

DWORD ProgramMode; //Global
HANDLE hDriver; //Global
BOOL IsModifyPidEnabled = FALSE;

LPCSTR Path = NULL; //驱动路径

VOID __MontiorDriverInfo(void);

BOOL __InitializeDriver() {
	//先卸载服务,再重新安装
	StopService("XuanLab_KernelModeService");
	UnInstallService("XuanLab_KernelModeService");
	Path = ReadConfigString(Cfg, "DriverOptions", "DriverPath");
	//安装驱动程序,类型:内核驱动;自动启动;错误等级:致命错误
	if (!FileExistsStaus(Path)) {
		printA("Invalid file,please select a valid driver file.\n", 3);
		BOOL Break = 0;
		char* PathRet;
		while (!Break) {
			PathRet = CreateFileSelectDlg("默认的文件无法找到,请指定一个有效的驱动文件.\n(默认名为XuanLaboratoryToolbox-Ring0Driver.sys");
			if (PathRet == nullptr) {
				if (MessageBoxA(GetConsoleWindow(), "还没有指定驱动文件,要退出吗?\n", "警告", MB_YESNO | MB_ICONWARNING) == IDYES) {
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
					MessageBoxA(GetConsoleWindow(), "无效的文件,请重新指定", "错误", MB_ICONERROR | MB_OK);
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
		DWORD Btn = MessageBoxA(GetConsoleWindow(), "未能启动驱动程序,这可能是因为没有打开调试模式并关闭驱动签名,也可能是被杀毒软件阻止了,请检查系统环境后重试.\n你可以尝试打开调试模式并关闭驱动签名\n\n点击[是]将进行尝试", "错误", MB_YESNO | MB_ICONERROR);
		if (Btn==IDYES) {
			if (EnableNoSignedDriverLoad()) {
				printA("Configuration modified successfully,please reboot to apply.\n", 1);
				if (MessageBoxA(GetConsoleWindow(), "设置成功,要立即重启以应用更改吗,重启前,应保存好你的所有文档\n是否重启?", "警告", MB_YESNO | MB_ICONWARNING) == IDYES) {
					ExitWindowsEx(EWX_REBOOT, EWX_FORCE);
				}
			}
			else {
				printA("Failed to modify configuration.\n", 2);
			}
		}
		return false;
	}
	//检测符号链接
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
		ret = MessageBoxA(GetConsoleWindow(), "警告:你启用了通过修改自身进程标识符的方法来保护进程,在未恢复前终止进程后会导致系统蓝屏.\n请勿直接关闭,否则会导致蓝屏!\n一定要在启用之前保存好你的所有文件,以免造成损失!\n\n点击[是]继续.", "功能警告", MB_YESNO | MB_ICONWARNING);
		if (ret == IDYES) {
			ret = MessageBoxA(GetConsoleWindow(), "最后警告:是否要通过修改自身进程标识符的方法来保护进程?", "警告", MB_YESNO | MB_ICONWARNING);
			if (ret == IDYES) {
				Protect.ModifyPid = TRUE;
				IsModifyPidEnabled = TRUE;
				HMENU MainMenu = GetSystemMenu(GetConsoleWindow(), false);		// 复制或修改而访问窗口菜单
				RemoveMenu(MainMenu, SC_CLOSE, MF_BYCOMMAND);	// 从指定菜单删除一个菜单项或分离一个子菜单
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