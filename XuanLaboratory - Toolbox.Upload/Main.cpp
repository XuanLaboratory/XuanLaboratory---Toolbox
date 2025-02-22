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
DWORD ParentPid; //������Pid ȫ��
HWND MainWindow; //������ ȫ��
HWND FuncSelect; //��ѡ��� ȫ��
HWND Tasklist; //�����б� ȫ��
HWND Svclist; //�����б� 
HWND SetupWnd; //���ô���
HINSTANCE hInstMain; //����̨ʵ�� ȫ��
LONG ConsoleProc; //�ɿ���̨��Ϣ�ص���ַ
HFONT UniversalFont; //ͨ������(΢���ź�)
char* CurrentDir;

int main(HINSTANCE hInstanceMain) {
	char* CmdLine = (char*)malloc(1024);
	CmdLine = GetCommandLineA();
	char* CurrentDir = (char*)malloc(512);
	GetCurrentDirectoryA(511, CurrentDir);
	printA(StrConnect(StrConnect("Command line:",CmdLine), "\n"), 1);
	printA(StrConnect(StrConnect("Current dir:", CurrentDir), "\n"), 1);
	if (strcmp(CmdLine, "--LaunchAsChildProcess") != 0) { //���������
		memset(CmdLine, 0, 1024);
		memcpy(CmdLine, "--LaunchAsChildProcess", sizeof("--LaunchAsChildProcess"));
		char* BufferA = (char*)malloc(1024);
		GetModuleFileNameA(0, BufferA, 1024);
		STARTUPINFOA si = { 0 }; //��ʼ��Ϊ0������Ҫд
		PROCESS_INFORMATION pi;
		BOOL bRet = FALSE;
		si.cb = sizeof(si);    //cb��С����Ϊ�ṹ���С
		si.dwFlags = STARTF_USESHOWWINDOW;  //ָ��wShowWindow��Ա��Ч
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

		//���ִ��
		MSG MainWindowMessage;
		while (GetMessageW(&MainWindowMessage, NULL, 0, 0))        //����Ϣ�����л�ȡ��Ϣ
		{
			TranslateMessage(&MainWindowMessage);                 //���������Ϣת��Ϊ�ַ���Ϣ
			DispatchMessage(&MainWindowMessage);                  //�ַ����ص�����(���̺���)
			if (!IsWindow(MainWindow)) break;
		}
		
		if (IsModifyPidEnabled) {
			CLIENT_PROC_PROTECT_METHOD Protect = { 0 };
			Protect.ModifyPid = FALSE;
			if (!DeviceIoControl(hDriver, SET_CLIENT_PROC_PROTECTION_METHOD, &Protect, sizeof(CLIENT_PROC_PROTECT_METHOD), 0, 0, 0, 0)) {
				MessageBoxA(GetConsoleWindow(), "�ܱ�Ǹ,������ͨ��ʧ��!\n�뱣��������ļ���������˳�.", "����", MB_OK | MB_ICONERROR);
				system("pause>nul");
			}
		}
	}
}



//�������������� ��δ�޸�

LRESULT CALLBACK Window_Main(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) { 
	DWORD dlgID = LOWORD(wParam); //�ؼ�ID
	DWORD codeID = HIWORD(wParam); //��Ϣ����
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
		if (dlgID == GetDlgCtrlID(FuncSelect)) { //����ѡ���
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



		if (dlgID = GetDlgCtrlID(MainWindow)) { //������
			
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
	DWORD dlgID = LOWORD(wParam); //�ؼ�ID
	DWORD codeID = HIWORD(wParam); //��Ϣ����
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

	//ע��������
	hInstMain = hInstanceMain;
	WNDCLASSW Reg = { 0 }; //�����ڻص�
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

	//ע�����ô���
	WNDCLASSW RegClass = { 0 }; //���ô��ڻص�
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
		MessageBoxA(0, "Error:Create the main window failed!", "����", MB_OK | MB_ICONERROR);
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

	//��ȥ�����ڵ���� ��С����ť
	HMENU MainMenu = GetSystemMenu(MainWindow, false);		// ���ƻ��޸Ķ����ʴ��ڲ˵�
	RemoveMenu(MainMenu, SC_MAXIMIZE, MF_BYCOMMAND);	// ��ָ���˵�ɾ��һ���˵�������һ���Ӳ˵�
	//RemoveMenu(MainMenu, SC_MINIMIZE, MF_BYCOMMAND);
	DrawMenuBar(MainWindow);

	//��ʼ����ѡ���
	FuncSelect = CreateWindowA("SysTabControl32", NULL, WS_VISIBLE | WS_CHILD | TCS_TABS, 0, 0, 1300, 820, MainWindow, 0, 0, 0);
	if (!FuncSelect) {
		MessageBoxA(MainWindow, "Error:Create the widget failed!", "����", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}
	SendMessageA(FuncSelect, WM_SETFONT, (WPARAM)UniversalFont, 2); //����ѡ�������
	TCITEMA Tab = { 0 };
	LPCSTR List[] = { "���̹�����","��������","����","�ļ�ϵͳ","Ӳ��","�ں�" };
	for (int i = 0; i < ARRAYSIZE(List); i++) {
		Tab.mask = TCIF_IMAGE | TCIF_TEXT;
		Tab.iImage = 0;
		Tab.pszText = (LPSTR)List[i];
		SendMessageA(FuncSelect, TCM_INSERTITEMA, i, (LPARAM)&Tab);
	}
	UpdateWindow(FuncSelect);

	//��ʼ�������б�
	//Tasklist = CreateWindowA("SysListView32", NULL, WS_VISIBLE | WS_CHILD | LVS_REPORT , 0, 21, 1280, 773, FuncSelect, 0, 0, 0);
	Tasklist = CreateWindowExA(0, WC_LISTVIEWA, NULL, WS_VISIBLE | WS_CHILD | LVS_REPORT , 0, 21, 1280, 773, FuncSelect, 0, 0, 0);
	if (!Tasklist) {
		MessageBoxA(MainWindow, "Error:Create the widget failed!", "����", MB_OK | MB_ICONERROR);
	}
	ListView_SetExtendedListViewStyle(Tasklist, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT );
	LVCOLUMN TempLvc= { 0 }; //�ɸ���
	TempLvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
	LPCWSTR Header[] = { L"��������",L"PID",L"�̼߳���",L"�û���",L"���̻���ַ",L"�ļ�·��",L"�ڴ�ռ��",L"������",L"����ʱ��",L"����"};
	DWORD len[] = { 140,90,90,120,160,260,80,120,100,90 };
	for (int i = 0; i < ARRAYSIZE(Header); i++) {
		TempLvc.pszText = (LPWSTR)Header[i];
		TempLvc.cx = len[i];
		TempLvc.iSubItem = i;
		ListView_InsertColumn(Tasklist, i, &TempLvc);
	}
	Tasklist_RefreshProcListAndUpdate(Tasklist);

	//��ʼ�������б�
	Svclist = CreateWindowA("SysListView32", NULL, WS_VISIBLE | WS_CHILD | LVS_REPORT, 0, 21, 1280, 773, FuncSelect, 0, 0, 0);
	if (!Svclist) {
		MessageBoxA(MainWindow, "Error:Create the widget failed!", "����", MB_OK | MB_ICONERROR);
	}
	ListView_SetExtendedListViewStyle(Tasklist, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	memset(&TempLvc, 0, sizeof(LVCOLUMN));
	TempLvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
	LPCWSTR Header2[] = { L"������",L"����",L"�ļ�·��",L"״̬",L"����ģʽ",L"��¼�û���" };
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
	//��ʼ������ѡ���
	SetupWndFuncSelect = CreateWindowA("SysTabControl32", NULL, WS_VISIBLE | WS_CHILD | TCS_TABS, 0, 21, 1280, 773, FuncSelect, 0, 0, 0);
	if (!SetupWndFuncSelect) {
		MessageBoxA(MainWindow, "Error:Create the widget failed!", "����", MB_OK | MB_ICONERROR);
	}

	SendMessageA(SetupWndFuncSelect, WM_SETFONT, (WPARAM)UniversalFont, 2); //����ѡ�������
	TCITEMA Tab2 = { 0 };
	LPCSTR List2[] = { "ͨ��","����","�߼�","Debug","����","����" };
	for (int i = 0; i < ARRAYSIZE(List2); i++) {
		Tab2.mask = TCIF_IMAGE | TCIF_TEXT;
		Tab2.iImage = 0;
		Tab2.pszText = (LPSTR)List2[i];
		SendMessageA(SetupWndFuncSelect, TCM_INSERTITEMA, i, (LPARAM)&Tab2);
	}
	UpdateWindow(SetupWndFuncSelect);
*/