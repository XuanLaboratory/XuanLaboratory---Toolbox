#include <Windows.h>
#include "Structures.h"

//Service.cpp
BOOL InstallService(LPCSTR svcName, LPCSTR dispName, LPCSTR lpFilePath, LPCSTR lpDesc, DWORD svcType, DWORD svcLaunchMethod, DWORD svcErrorLevel); //��װָ���ķ���
BOOL UnInstallService(LPCSTR svcName); //ɾ��ָ���ķ���
BOOL LaunchService(LPCSTR svcName); //����ָ���ķ���
BOOL StopService(LPCSTR svcName); //ָֹͣ���ķ���
BOOL SetServiceDescription(LPCSTR, LPCSTR); //����ָ����������
DWORD GetServiceStatus(LPCSTR svcName); //��ȡ����״̬

//DriverControl.cpp
BOOL __InitializeDriver();
BOOL Driver_TerminateProcess(DWORD Pid); //R0��������
BOOL Driver_TerminateProcessTree(DWORD Pid); //R0����������
BOOL Driver_SuspendProcess(DWORD Pid); //R0�������
BOOL Driver_ResumeProcess(DWORD Pid); //R0�ָ�����
BOOL Driver_WriteProcessMemory(DWORD Pid, PVOID pBaseAddr, PVOID pSourceAddr, ULONG len); //R0д�����ڴ�
BOOL Driver_ReadProcessMemory(DWORD Pid, PVOID pBaseAddr, ULONG ReadLen, PVOID pBuffer, ULONG BufferLen); //R0�������ڴ�
BOOL Driver_QueryProcessPebData(DWORD Pid, PEB64* Peb); //R0��ȡ����PEB��Ϣ

//Registry.cpp
//HKEY Registry_OpenKey(LPCSTR Path, HKEY HKey);
//char* Registry_ReadStringKey(HKEY Hkey, LPCSTR Path, LPCSTR Subkey);
//BOOL Registry_WriteStringKey(HKEY Hkey, LPCSTR Path, LPCSTR Subkey, LPCSTR value);
//BOOL Registry_DeleteKey(HKEY HKey, LPCSTR Path, LPCSTR Subkey);
//BOOL Registry_WriteDwordKey(HKEY Hkey, LPCSTR Path, LPCSTR Subkey, DWORD value);

//Console.cpp
void printC(const char* msg, int color);
void printW(const char* msg, int info, DWORD value); //�����־(��HEX����ʽ��ӡֵ)
void printA(const char* msg, int info); //�����־

//FileIO.cpp
LPCSTR File_ReadFileFull(LPCSTR); //��ȡ�ļ�ȫ������
BOOL File_WriteFile(LPCSTR FileName, LPCSTR Info); //д���ļ�(ֱ�Ӹ���)
BOOL File_WriteFileAttach(LPCSTR FileName, LPCSTR Info); //д���ļ�(׷��ģʽ)
BOOL FileExistsStaus(LPCSTR path);
char* CreateFileSelectDlg(const char* Title);
BOOL CheckIfExecutable(const char* Path);
LPCSTR ReadConfigString(LPCSTR Path, LPCSTR Section, LPCSTR Key);
BOOL ReadConfigBoolean(LPCSTR Path, LPCSTR Section, LPCSTR Key);
DWORD ReadConfigDword(LPCSTR Path, LPCSTR Section, LPCSTR Key);
BOOL WriteConfigString(LPCSTR Path, LPCSTR Section, LPCSTR Key, LPCSTR Data);
BOOL WriteConfigBoolean(LPCSTR Path, LPCSTR Section, LPCSTR Key, BOOL b);
BOOL WriteConfigInt(LPCSTR Path, LPCSTR Section, LPCSTR Key, DWORD v);

//Process.cpp
DWORD QueryParentProcessID(DWORD Pid);
BOOL RaiseProcessPermission(DWORD Pid);

//Checker.cpp
VOID __EnvironmentCheck(void);

//MessageFunction.cpp
LPCSTR StrConnect(LPCSTR a, LPCSTR b);
BOOL StrCompare(LPCSTR a, LPCSTR b);
char* TranslateGetLastErrorMsg(DWORD code);

//WindowModuleCtrl.cpp
DWORD Window_ChangeColor(HWND hWnd, DWORD Color);
BOOL Widget_Checkbox_IsChecked(HWND hWnd); //ѡ����Ƿ�ѡ��
void Widget_Checkbox_SetStat(HWND hWnd, BOOL Check); //����ѡ���״̬
BOOL Widget_SetTitle(HWND hWnd, const char* Str); //���ÿؼ�����
BOOL Widget_HideWindow(HWND hWnd, BOOL Hide);
DWORD Tasklist_RefreshProcListAndUpdate(HWND Target);

//System.cpp
VERSION_INFO GetVersionRtl(void);
char* GetSystemTime();
char* GetSystemTimeFull();
BOOL EnableNoSignedDriverLoad();
BOOL DisableNoSignedDriverLoad();

//Init.cpp
DWORD __ApiInit(void);
DWORD __InternalInit(void);

//Timer.cpp
DWORD __CreateTimerUniversalTimer(); //������ʱ��