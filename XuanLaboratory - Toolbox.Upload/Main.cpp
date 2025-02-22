#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(suppress : 4996)
#include <Windows.h>
#include <stdio.h>
#include <CommCtrl.h>
#include <winusb.h>
#include <TlHelp32.h>
#include "FunctionCall.h"
#include "Structures.h"
#include "IoCtrl.h"
#include "Console.h"
#include "VariableAndStatics.h"


/*
*Rights to sign: XuanLaboratory
*Project Name:   XuanLaboratory - Toolbox
*Project created:2024-08.
*Copyright (C) HouYuXuan 2024 - All Rights Reserved
*/

LRESULT CALLBACK Window_Main(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Window_Setup(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
BOOL __RegisterHotKeys(); 
BOOL __CreateSetupWindow();
DWORD _CreateWidgets(HINSTANCE hInstanceMain);
DWORD ParentPid; //父进程Pid 全局
HWND MainWindow; //主窗口 全局
HWND FuncSelect; //根选择夹 全局
HWND Tasklist; //进程列表 全局
HWND Svclist; //服务列表 
HWND SetupWnd; //设置窗口
HINSTANCE hInstMain; //控制台实例 全局
LONG ConsoleProc; //旧控制台消息回调地址
HFONT UniversalFont; //通用字体(微软雅黑)
char* CurrentDir;

int main(HINSTANCE hInstanceMain) {
	char* CmdLine = (char*)malloc(1024);
	CmdLine = GetCommandLineA();
	char* CurrentDir = (char*)malloc(512);
	GetCurrentDirectoryA(511, CurrentDir);
	printA(StrConnect(StrConnect("Command line:",CmdLine), "\n"), 1);
	printA(StrConnect(StrConnect("Current dir:", CurrentDir), "\n"), 1);
	if (strcmp(CmdLine, "--LaunchAsChildProcess") != 0) { //检查命令行
		memset(CmdLine, 0, 1024);
		memcpy(CmdLine, "--LaunchAsChildProcess", sizeof("--LaunchAsChildProcess"));
		char* BufferA = (char*)malloc(1024);
		GetModuleFileNameA(0, BufferA, 1024);
		STARTUPINFOA si = { 0 }; //初始化为0，必须要写
		PROCESS_INFORMATION pi;
		BOOL bRet = FALSE;
		si.cb = sizeof(si);    //cb大小设置为结构体大小
		si.dwFlags = STARTF_USESHOWWINDOW;  //指定wShowWindow成员有效
		si.wShowWindow = SW_SHOWNORMAL;
		bRet = CreateProcessA(BufferA, CmdLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
		
		if (bRet) {
			printA("Parent process mode.\n", 1);

			if (SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS)) {
				printA("Overwrite priority successfully!\n", 1);
			}
			do {
				
				printA("Initializing named pipe...\n", 1);
				HANDLE Pipe = CreateNamedPipeA("\\\\.\\pipe\\XuanLaboratory_IoPipe", PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE, 1, 1024, 1024, 0, NULL);
				if (Pipe == INVALID_HANDLE_VALUE) {
					printA("Failed to create named pipe! Info:", 3);
					printf("%s", TranslateGetLastErrorMsg(GetLastError()));
					break;
				}
				HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
				if (hEvent == 0) {
					printA("Failed to create event! Info:", 3);
					printf("%s", TranslateGetLastErrorMsg(GetLastError()));
					break;
				}
				OVERLAPPED ol = { 0 };
				ol.hEvent = hEvent;
				if (!ConnectNamedPipe(Pipe, &ol) && (GetLastError()!=ERROR_IO_PENDING)) {
					printA("Failed to wait child process to connect!\n", 3);
					CloseHandle(Pipe);
					CloseHandle(hEvent);
					break;
				}
				printA("Waiting...\n", 1);

				if (WaitForSingleObject(hEvent, INFINITE) == WAIT_FAILED) {
					printA("Failed to wait object!\n", 3);
					CloseHandle(Pipe);
					CloseHandle(hEvent);
					break;
				}

				DWORD rec = 0;
				char* Buffer = (char*)malloc(sizeof(DWORD));
				RtlZeroMemory(Buffer, strlen(Buffer));
				sprintf(Buffer, "%d", GetCurrentProcessId());
				if (!WriteFile(Pipe, Buffer, strlen(Buffer), &rec, 0)) {
					printA("Failed to send message to child process!\n", 3);
				}
				else {
					printA("Send process id to child process successfully\n", 1);
				}


			} while (false);

			//ShowWindowAsync(GetConsoleWindow(), 0);
			WaitForSingleObject(pi.hProcess, INFINITE);
			ShowWindowAsync(GetConsoleWindow(), 1);
			printA("Child process died,cleaning up...\n", 2);
			StopService("XuanLab_KernelModeService");
			UnInstallService("XuanLab_KernelModeService");

		}
		else {
			printA("Error while launching child process\n",3);
		}

	}else {
		printA("Child process mode.\n", 1);
		__ApiInit();
		if (WaitForSingleObject(CreateThread(0, 64, (LPTHREAD_START_ROUTINE)__EnvironmentCheck, 0, 0, 0), INFINITE) == WAIT_FAILED) {
			printA("Initial Error : Launch thread __EnvironmentCheck() failed!\n", 4);
			ExitProcess(-1);
		}
		__InternalInit();
		SetConsoleTitleA("XuanLaboratory - Toolbox | Copyright (C) XuanLaboratory 2024");
		printC("XuanLaboratory - ToolBox\n", C_WHITE);
		printC("Build 00000.n/a_beta.00000-0000\n", C_CYAN);
		printC("Copyright (C) XuanLab | Xuan. 2025\n\n\n", C_YELLOW);
		__CreateTimerUniversalTimer();
		_CreateWidgets(hInstanceMain);
		if (!__RegisterHotKeys()) {
			printA("Failed to register hotkeys!\n", 2);
		}

		//最后执行
		MSG MainWindowMessage;
		while (GetMessageW(&MainWindowMessage, NULL, 0, 0))        //从消息队列中获取消息
		{
			TranslateMessage(&MainWindowMessage);                 //将虚拟键消息转换为字符消息
			DispatchMessage(&MainWindowMessage);                  //分发到回调函数(过程函数)
			if (!IsWindow(MainWindow)) break;
		}
		
		if (IsModifyPidEnabled) {
			CLIENT_PROC_PROTECT_METHOD Protect = { 0 };
			Protect.ModifyPid = FALSE;
			if (!DeviceIoControl(hDriver, SET_CLIENT_PROC_PROTECTION_METHOD, &Protect, sizeof(CLIENT_PROC_PROTECT_METHOD), 0, 0, 0, 0)) {
				MessageBoxA(GetConsoleWindow(), "很抱歉,与驱动通信失败!\n请保存好所有文件后按任意键退出.", "错误", MB_OK | MB_ICONERROR);
				system("pause>nul");
			}
		}
	}
}



//这里有严重问题 还未修改

LRESULT CALLBACK Window_Main(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) { 
	DWORD dlgID = LOWORD(wParam); //控件ID
	DWORD codeID = HIWORD(wParam); //消息代码
	switch (Message)
	{
	case WM_INITDIALOG: {

		break;
	}

	case WM_DESTROY: {
		DestroyWindow(MainWindow);
		break;
	}
	case WM_NOTIFY: {
		if (dlgID == GetDlgCtrlID(Tasklist)) {
			LPNMHDR dlgMsg = (LPNMHDR)lParam;
			switch (dlgMsg->code)
			{
			case NM_CLICK: {
				UINT col = ListView_GetNextItem(Tasklist, -1, LVIS_SELECTED);
				WCHAR info[8];
				ListView_GetItemText(Tasklist, col, 1, info, 8);
				break;
			}

			default:
				break;
			}
			memset(dlgMsg, 0, sizeof(NMHDR));
			//break;
		}
		if (dlgID == GetDlgCtrlID(FuncSelect)) { //功能选择夹
			LPNMHDR dlgMsg = (LPNMHDR)lParam;
			switch (dlgMsg->code)
			{
			case NM_CLICK: {
				DWORD currentID = SendMessageA(FuncSelect, TCM_GETCURSEL, 0, 0);
				if (currentID == 0) {
					ShowWindowAsync(Tasklist, 1);
				}
				else {
					ShowWindowAsync(Tasklist, 0);
					Tasklist_RefreshProcListAndUpdate(Tasklist);
				}

				if (currentID == 1) {
					ShowWindowAsync(Svclist, 1);
				}
				else {
					ShowWindowAsync(Svclist, 0);
				}

				break;
			}

			default: 
				break;
			}
			memset(dlgMsg, 0, sizeof(NMHDR));
			break;
		}



		if (dlgID = GetDlgCtrlID(MainWindow)) { //主窗口
			
			break;
		}

	}
	case WM_HOTKEY: {
		switch (wParam)
		{
		case 19491001: {  //Ctrl+Alt+S
			__CreateSetupWindow();
			break;
		}
		case 19491002: { //Ctrl+Alt+H

		}
		default:
			printA("Unexpected message. (Is program injected?)\n", 3);
			break;
		}
	}

	default:
		return DefWindowProcW(hWnd, Message, wParam, lParam);
	}
	return false;
}


LRESULT CALLBACK Window_Setup(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	DWORD dlgID = LOWORD(wParam); //控件ID
	DWORD codeID = HIWORD(wParam); //消息代码
	switch (Message)
	{
	case WM_INITDIALOG: {

		break;
	}

	case WM_DESTROY: {

	}
	case WM_NOTIFY: {
		

	}

	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return false;
}



DWORD _CreateWidgets(HINSTANCE hInstanceMain) {
	printA("Attention:User interaction has been taken over by GUI,the console will only display log info.\n\n", 1);
	printA("Creating GUI...\n", 1);

	//注册主窗口
	hInstMain = hInstanceMain;
	WNDCLASSW Reg = { 0 }; //主窗口回调
	HINSTANCE MainWin = 0;
	Reg.cbClsExtra = 0;
	Reg.cbWndExtra = 0;
	Reg.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	Reg.hCursor = LoadCursorW(0, IDC_ARROW);
	Reg.hIcon = LoadIconW(0, IDI_APPLICATION);
	Reg.hInstance = hInstanceMain;
	Reg.lpfnWndProc = Window_Main;
	Reg.lpszClassName = L"XuanLab - Toolbox MainWindow";
	Reg.lpszMenuName = 0;
	Reg.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassW(&Reg);

	//注册设置窗口
	WNDCLASSW RegClass = { 0 }; //设置窗口回调
	RegClass.cbClsExtra = 0;
	RegClass.cbWndExtra = 0;
	RegClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	RegClass.hCursor = LoadCursorW(0, IDC_ARROW);
	RegClass.hIcon = LoadIconW(0, IDI_APPLICATION);
	RegClass.hInstance = hInstMain;
	RegClass.lpfnWndProc = Window_Setup;
	RegClass.lpszClassName = L"XuanLab - Setup";
	RegClass.lpszMenuName = 0;
	RegClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassW(&RegClass);

	MainWindow = CreateWindowA("XuanLab - Toolbox MainWindow", "XuanLab - Toolbox", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1300, 835, 0, 0, MainWin, 0);
	if (!MainWindow) {
		MessageBoxA(0, "Error:Create the main window failed!", "错误", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	switch (ProgramMode)
	{
	case Mode_Ring3_Only: {
		SetWindowTextA(MainWindow, "XuanLab - Toolbox | Ring3 Mode");
		break;
	}
	case Mode_Both: {
		SetWindowTextA(MainWindow, "XuanLab - Toolbox | Ring0-Ring3 Mode");
		break;
	}
	default:
		break;
	}

	//除去主窗口的最大化 最小化按钮
	HMENU MainMenu = GetSystemMenu(MainWindow, false);		// 复制或修改而访问窗口菜单
	RemoveMenu(MainMenu, SC_MAXIMIZE, MF_BYCOMMAND);	// 从指定菜单删除一个菜单项或分离一个子菜单
	//RemoveMenu(MainMenu, SC_MINIMIZE, MF_BYCOMMAND);
	DrawMenuBar(MainWindow);

	//初始化根选择夹
	FuncSelect = CreateWindowA("SysTabControl32", NULL, WS_VISIBLE | WS_CHILD | TCS_TABS, 0, 0, 1300, 820, MainWindow, 0, 0, 0);
	if (!FuncSelect) {
		MessageBoxA(MainWindow, "Error:Create the widget failed!", "错误", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}
	SendMessageA(FuncSelect, WM_SETFONT, (WPARAM)UniversalFont, 2); //设置选择夹字体
	TCITEMA Tab = { 0 };
	LPCSTR List[] = { "进程管理器","驱动服务","网络","文件系统","硬件","内核" };
	for (int i = 0; i < ARRAYSIZE(List); i++) {
		Tab.mask = TCIF_IMAGE | TCIF_TEXT;
		Tab.iImage = 0;
		Tab.pszText = (LPSTR)List[i];
		SendMessageA(FuncSelect, TCM_INSERTITEMA, i, (LPARAM)&Tab);
	}
	UpdateWindow(FuncSelect);

	//初始化进程列表
	//Tasklist = CreateWindowA("SysListView32", NULL, WS_VISIBLE | WS_CHILD | LVS_REPORT , 0, 21, 1280, 773, FuncSelect, 0, 0, 0);
	Tasklist = CreateWindowExA(0, WC_LISTVIEWA, NULL, WS_VISIBLE | WS_CHILD | LVS_REPORT , 0, 21, 1280, 773, FuncSelect, 0, 0, 0);
	if (!Tasklist) {
		MessageBoxA(MainWindow, "Error:Create the widget failed!", "错误", MB_OK | MB_ICONERROR);
	}
	ListView_SetExtendedListViewStyle(Tasklist, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
	LVCOLUMN TempLvc= { 0 }; //可复用
	TempLvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
	LPCWSTR Header[] = { L"进程名称",L"PID",L"线程计数",L"用户名",L"进程基地址",L"文件路径",L"内存占用",L"发布者",L"启动时间",L"网络"};
	DWORD len[] = { 140,90,90,120,160,260,80,120,100,90 };
	for (int i = 0; i < ARRAYSIZE(Header); i++) {
		TempLvc.pszText = (LPWSTR)Header[i];
		TempLvc.cx = len[i];
		TempLvc.iSubItem = i;
		ListView_InsertColumn(Tasklist, i, &TempLvc);
	}
	Tasklist_RefreshProcListAndUpdate(Tasklist);

	//初始化服务列表
	Svclist = CreateWindowA("SysListView32", NULL, WS_VISIBLE | WS_CHILD | LVS_REPORT, 0, 21, 1280, 773, FuncSelect, 0, 0, 0);
	if (!Svclist) {
		MessageBoxA(MainWindow, "Error:Create the widget failed!", "错误", MB_OK | MB_ICONERROR);
	}
	ListView_SetExtendedListViewStyle(Tasklist, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	memset(&TempLvc, 0, sizeof(LVCOLUMN));
	TempLvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
	LPCWSTR Header2[] = { L"服务名",L"描述",L"文件路径",L"状态",L"启动模式",L"登录用户名" };
	DWORD len2[] = { 150,190,200,80,100,100 };
	for (int i = 0; i < ARRAYSIZE(Header2); i++) {
		TempLvc.pszText = (LPWSTR)Header2[i];
		TempLvc.cx = len2[i];
		TempLvc.iSubItem = i;
		ListView_InsertColumn(Svclist, i, &TempLvc);
	}
	ShowWindowAsync(Svclist, 0);

	Driver_TerminateProcess(3692);
}



BOOL __RegisterHotKeys() {
	BOOL Stat = 1;
	Stat = Stat & RegisterHotKey(MainWindow, 19491001, MOD_CONTROL | MOD_ALT, 0x53); //Ctrl+Alt+S
	Stat = Stat & RegisterHotKey(MainWindow, 19491002, MOD_CONTROL | MOD_ALT, 0x49); //Ctrl+Alt+H
	return Stat;
}

BOOL __CreateSetupWindow() {
	SetupWnd = CreateWindowA("XuanLab - Setup", "XuanLab - Toolbox Setup", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 700, 470, MainWindow, 0, 0, 0);
	LoadLibraryA("ntdll.dll");
	
	return 1;
}

/*
	//初始化设置选择夹
	SetupWndFuncSelect = CreateWindowA("SysTabControl32", NULL, WS_VISIBLE | WS_CHILD | TCS_TABS, 0, 21, 1280, 773, FuncSelect, 0, 0, 0);
	if (!SetupWndFuncSelect) {
		MessageBoxA(MainWindow, "Error:Create the widget failed!", "错误", MB_OK | MB_ICONERROR);
	}

	SendMessageA(SetupWndFuncSelect, WM_SETFONT, (WPARAM)UniversalFont, 2); //设置选择夹字体
	TCITEMA Tab2 = { 0 };
	LPCSTR List2[] = { "通用","驱动","高级","Debug","更新","关于" };
	for (int i = 0; i < ARRAYSIZE(List2); i++) {
		Tab2.mask = TCIF_IMAGE | TCIF_TEXT;
		Tab2.iImage = 0;
		Tab2.pszText = (LPSTR)List2[i];
		SendMessageA(SetupWndFuncSelect, TCM_INSERTITEMA, i, (LPARAM)&Tab2);
	}
	UpdateWindow(SetupWndFuncSelect);
*/