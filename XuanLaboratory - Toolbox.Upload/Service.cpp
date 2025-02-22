#include <iostream>
#include <Windows.h>
#include "FunctionCall.h"



BOOL InstallService(LPCSTR svcName, LPCSTR dispName, LPCSTR lpFilePath, LPCSTR lpDesc, DWORD svcType, DWORD svcLaunchMethod, DWORD svcErrorLevel) {
	char szFilePath[MAX_PATH] = { 0 };
	SC_HANDLE hSCManager = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL) {
		return false;
	}
	GetFullPathNameA(lpFilePath, MAX_PATH, szFilePath, NULL);
	SC_HANDLE hSvc = CreateServiceA(hSCManager, svcName, dispName, SERVICE_ALL_ACCESS, svcType, svcLaunchMethod, svcErrorLevel, szFilePath, NULL, NULL, NULL, NULL, NULL);
	if (hSvc == NULL) {
		return false;
	}
	SetServiceDescription(svcName, lpDesc);
	CloseServiceHandle(hSvc);
	CloseServiceHandle(hSCManager);
	return true;
}

BOOL LaunchService(LPCSTR svcName) {
	SC_HANDLE hSCManager = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL) {
		return false;
	}
	SC_HANDLE hSvc = OpenServiceA(hSCManager, svcName, SERVICE_ALL_ACCESS);
	if (hSvc == NULL) {
		return false;
	}
	BOOL ret = StartServiceA(hSvc, 0, 0);
	CloseServiceHandle(hSvc);
	CloseServiceHandle(hSCManager);
	return ret;
}

BOOL StopService(LPCSTR svcName) {
	SC_HANDLE hSCManager = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL) {
		return false;
	}
	SC_HANDLE hSvc = OpenServiceA(hSCManager, svcName, SERVICE_ALL_ACCESS);
	if (hSvc == NULL) {
		return false;
	}
	SERVICE_STATUS Stat;
	BOOL ret = ControlService(hSvc, SERVICE_CONTROL_STOP, &Stat);
	CloseServiceHandle(hSvc);
	CloseServiceHandle(hSCManager);
	return ret;
}

BOOL UnInstallService(LPCSTR svcName) {
	SC_HANDLE hSCManager = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL) {
		return false;
	}
	SC_HANDLE hSvc = OpenServiceA(hSCManager, svcName, SERVICE_ALL_ACCESS);
	if (hSvc == NULL) {
		return false;
	}
	BOOL ret = DeleteService(hSvc);
	CloseServiceHandle(hSvc);
	CloseServiceHandle(hSCManager);
	return ret;
}

BOOL SetServiceDescription(LPCSTR svcName, LPCSTR Description) {
	SC_HANDLE hSCMangager = OpenSCManagerA(0, 0, GENERIC_EXECUTE);
	if (hSCMangager == 0) {
		return false;
	}
	SC_HANDLE hSvc = OpenServiceA(hSCMangager, svcName, SERVICE_CHANGE_CONFIG);
	if (hSvc == 0) {
		return false;
	}
	BOOL ret = ChangeServiceConfig2A(hSvc, 1, &Description);
	CloseServiceHandle(hSvc);
	CloseServiceHandle(hSCMangager);
	return ret;
}

DWORD GetServiceStatus(LPCSTR svcName) {
	SC_HANDLE hSCMangager = OpenSCManagerA(0, 0, GENERIC_EXECUTE);
	if (hSCMangager == 0) {
		return -1;
	}
	SC_HANDLE hSvc = OpenServiceA(hSCMangager, svcName, SERVICE_CHANGE_CONFIG);
	if (hSvc == 0) {
		return -1;
	}

	SERVICE_STATUS Stat = { 0 };
	QueryServiceStatus(hSvc, &Stat);
	return Stat.dwCurrentState;
}