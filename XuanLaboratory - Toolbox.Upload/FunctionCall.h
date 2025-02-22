#include <Windows.h>
#include "Structures.h"

//Service.cpp
BOOL InstallService(LPCSTR svcName, LPCSTR dispName, LPCSTR lpFilePath, LPCSTR lpDesc, DWORD svcType, DWORD svcLaunchMethod, DWORD svcErrorLevel); //安装指定的服务
BOOL UnInstallService(LPCSTR svcName); //删除指定的服务
BOOL LaunchService(LPCSTR svcName); //启动指定的服务
BOOL StopService(LPCSTR svcName); //停止指定的服务
BOOL SetServiceDescription(LPCSTR, LPCSTR); //设置指定服务描述
DWORD GetServiceStatus(LPCSTR svcName); //获取服务状态

//DriverControl.cpp
BOOL __InitializeDriver();
BOOL Driver_TerminateProcess(DWORD Pid); //R0结束进程
BOOL Driver_TerminateProcessTree(DWORD Pid); //R0结束进程树
BOOL Driver_SuspendProcess(DWORD Pid); //R0挂起进程
BOOL Driver_ResumeProcess(DWORD Pid); //R0恢复进程
BOOL Driver_WriteProcessMemory(DWORD Pid, PVOID pBaseAddr, PVOID pSourceAddr, ULONG len); //R0写进程内存
BOOL Driver_ReadProcessMemory(DWORD Pid, PVOID pBaseAddr, ULONG ReadLen, PVOID pBuffer, ULONG BufferLen); //R0读进程内存
BOOL Driver_QueryProcessPebData(DWORD Pid, PEB64* Peb); //R0获取进程PEB信息

//Registry.cpp
//HKEY Registry_OpenKey(LPCSTR Path, HKEY HKey);
//char* Registry_ReadStringKey(HKEY Hkey, LPCSTR Path, LPCSTR Subkey);
//BOOL Registry_WriteStringKey(HKEY Hkey, LPCSTR Path, LPCSTR Subkey, LPCSTR value);
//BOOL Registry_DeleteKey(HKEY HKey, LPCSTR Path, LPCSTR Subkey);
//BOOL Registry_WriteDwordKey(HKEY Hkey, LPCSTR Path, LPCSTR Subkey, DWORD value);

//Console.cpp
void printC(const char* msg, int color);
void printW(const char* msg, int info, DWORD value); //输出日志(以HEX的形式打印值)
void printA(const char* msg, int info); //输出日志

//FileIO.cpp
LPCSTR File_ReadFileFull(LPCSTR); //读取文件全部数据
BOOL File_WriteFile(LPCSTR FileName, LPCSTR Info); //写入文件(直接覆盖)
BOOL File_WriteFileAttach(LPCSTR FileName, LPCSTR Info); //写入文件(追加模式)
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
BOOL Widget_Checkbox_IsChecked(HWND hWnd); //选择框是否被选中
void Widget_Checkbox_SetStat(HWND hWnd, BOOL Check); //设置选择框状态
BOOL Widget_SetTitle(HWND hWnd, const char* Str); //设置控件标题
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
DWORD __CreateTimerUniversalTimer(); //创建定时器