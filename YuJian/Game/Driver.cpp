#include "Game.h"
#include "Driver.h"
#include <My/Common/func.h>
#include <stdio.h>
#include <shellapi.h>
#include <ShlObj_core.h>

#define FSFILTER_DRIVER_NAME "DriverFs999"

// ...
Driver::Driver()
{
}

// 测试
int Driver::Test()
{
	return 0x12345678;
}

BOOL Driver::InstallFsFilter(const char* path, const char * lpszDriverPath, const char * lpszAltitude)
{
	char    szTempStr[MAX_PATH];
	HKEY    hKey;
	DWORD    dwData;
	char    szDriverImagePath[MAX_PATH];

	if (NULL == lpszDriverPath)
	{
		return FALSE;
	}
	//得到完整的驱动路径
	//GetFullPathNameA(lpszDriverPath, MAX_PATH, szDriverImagePath, NULL);

	sprintf_s(szDriverImagePath, "%s\\files\\%s", path, lpszDriverPath);
	//printf("szDriverImagePath:%s\n", szDriverImagePath);

	SC_HANDLE hServiceMgr = NULL;// SCM管理器的句柄
	SC_HANDLE hService = NULL;// NT驱动程序的服务句柄

	//打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		// OpenSCManager失败
		printf(" OpenSCManager失败\n");
		CloseServiceHandle(hServiceMgr);
		return FALSE;
	}

	// OpenSCManager成功  

	//创建驱动所对应的服务
	hService = CreateServiceA(hServiceMgr,
		FSFILTER_DRIVER_NAME,             // 驱动程序的在注册表中的名字
		FSFILTER_DRIVER_NAME,             // 注册表驱动程序的DisplayName 值
		SERVICE_ALL_ACCESS,         // 加载驱动程序的访问权限
		SERVICE_FILE_SYSTEM_DRIVER, // 表示加载的服务是文件系统驱动程序
		SERVICE_DEMAND_START,       // 注册表驱动程序的Start 值
		SERVICE_ERROR_IGNORE,       // 注册表驱动程序的ErrorControl 值
		szDriverImagePath,          // 注册表驱动程序的ImagePath 值
		"FSFilter Activity Monitor",// 注册表驱动程序的Group 值
		NULL,
		"FltMgr",                   // 注册表驱动程序的DependOnService 值
		NULL,
		NULL);

	if (hService == NULL)
	{
		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			//服务创建失败，是由于服务已经创立过
			printf("服务创建失败，是由于服务已经创立过\n");
			CloseServiceHandle(hService);       // 服务句柄
			CloseServiceHandle(hServiceMgr);    // SCM句柄
			return TRUE;
		}
		else
		{
			printf(" 服务创建失败 %d\n", GetLastError());
			CloseServiceHandle(hService);       // 服务句柄
			CloseServiceHandle(hServiceMgr);    // SCM句柄
			return FALSE;
		}
	}
	CloseServiceHandle(hService);       // 服务句柄
	CloseServiceHandle(hServiceMgr);    // SCM句柄

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances子健下的键值项 
	//-------------------------------------------------------------------------------------------------------
	strcpy(szTempStr, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat(szTempStr, FSFILTER_DRIVER_NAME);
	strcat(szTempStr, "\\Instances");
	if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, szTempStr, 0, (LPSTR)"", TRUE, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		printf("HKEY_LOCAL_MACHINE %s创建失败\n", FSFILTER_DRIVER_NAME);
		return FALSE;
	}
	// 注册表驱动程序的DefaultInstance 值 
	strcpy(szTempStr, FSFILTER_DRIVER_NAME);
	strcat(szTempStr, " Instance");
	if (RegSetValueExA(hKey, "DefaultInstance", 0, REG_SZ, (CONST BYTE*)szTempStr, (DWORD)strlen(szTempStr)) != ERROR_SUCCESS)
	{
		printf("HKEY_LOCAL_MACHINE Set失败\n");
		return FALSE;
	}
	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances\\DriverName Instance子健下的键值项 
	//-------------------------------------------------------------------------------------------------------
	strcpy(szTempStr, "SYSTEM\\CurrentControlSet\\Services\\");
	strcat(szTempStr, FSFILTER_DRIVER_NAME);
	strcat(szTempStr, "\\Instances\\");
	strcat(szTempStr, FSFILTER_DRIVER_NAME);
	strcat(szTempStr, " Instance");
	if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, szTempStr, 0, (LPSTR)"", TRUE, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		printf("HKEY_LOCAL_MACHINE %s %s创建失败\n", FSFILTER_DRIVER_NAME, FSFILTER_DRIVER_NAME);
		return FALSE;
	}
	// 注册表驱动程序的Altitude 值
	strcpy(szTempStr, lpszAltitude);
	if (RegSetValueExA(hKey, "Altitude", 0, REG_SZ, (CONST BYTE*)szTempStr, (DWORD)strlen(szTempStr)) != ERROR_SUCCESS)
	{
		printf("Altitude Set失败\n");
		return FALSE;
	}
	// 注册表驱动程序的Flags 值
	dwData = 0x0;
	if (RegSetValueExA(hKey, "Flags", 0, REG_DWORD, (CONST BYTE*)&dwData, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		printf("Altitude Set失败2\n");
		return FALSE;
	}
	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	return TRUE;
}

// 启动文件过滤保护
BOOL Driver::StartFsFilter()
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		printf("schManager=NULL %d\n", GetLastError());
		CloseServiceHandle(schManager);
		return FALSE;
	}
	schService = OpenServiceA(schManager, FSFILTER_DRIVER_NAME, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		printf("schService=NULL %d\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	if (!StartService(schService, 0, NULL))
	{
		printf("!StartService %d\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		if (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
		{
			// 服务已经开启
			return TRUE;
		}
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

// 停止文件保护驱动
BOOL Driver::StopFsFilter()
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;
	bool            bStopped = false;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		return FALSE;
	}
	schService = OpenServiceA(schManager, FSFILTER_DRIVER_NAME, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}
	if (!ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus) && (svcStatus.dwCurrentState != SERVICE_STOPPED))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

// 删除文件保护驱动
BOOL Driver::DeleteFsFilter()
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
	{
		return FALSE;
	}
	schService = OpenServiceA(schManager, FSFILTER_DRIVER_NAME, SERVICE_ALL_ACCESS);
	if (NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}
	ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus);
	if (!DeleteService(schService))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}
	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

// 安装驱动
bool Driver::InstallDriver(const char* path)
{
	wchar_t file[_MAX_PATH];
	::SHGetSpecialFolderPath(NULL, file, CSIDL_DESKTOP, 0);
	wcscat(file, L"\\9星2\\files\\firenet.sys");

	if (!IsFileExist(file)) {
		//printf("缺少必需文件:firenet.sys");
		//m_pJsCall->ShowMsg("缺少必需文件:firenet.sys", "文件不存在", 2);
		m_bIsInstallDll = false;
		char kill[32];
		sprintf_s(kill, "taskkill /f /t /pid %d", GetCurrentProcessId());
		system(kill);
		TerminateProcess(GetCurrentProcess(), 4);
		return false;
	}

	bool is_try = false;
_try_install_:
	if (m_SysDll.Install(L"firenet_safe", L"safe fire", file)) {
		//printf("InstallDriver Ok.\n");
		return true;
	}
	else {
		if (!is_try) {
			is_try = true;
#if 1
			Delete(L"firenet_safe");
#else
			ShellExecuteA(NULL, "open", "cmd", "/C sc stop firenet_safe", NULL, SW_HIDE);
			ShellExecuteA(NULL, "open", "cmd", "/C sc delete firenet_safe", NULL, SW_HIDE);
#endif
			//printf("InstallDriver Failed. Try Agin\n");
			goto _try_install_;
		}
		else {
			m_SysDll.UnInstall();
			char kill[32];
			sprintf_s(kill, "taskkill /f /t /pid %d", GetCurrentProcessId());
			system(kill);
			TerminateProcess(GetCurrentProcess(), 4);
			//LOG2(L"安装驱动失败, 请重启本程序再尝试.", "red");
		}

		//MessageBox(NULL, "安装驱动失败", "提示", MB_OK);
		return true;
	}
}


// 设置保护进程ID
void Driver::SetProtectPid(DWORD pid)
{
	pid = pid ? pid : GetCurrentProcessId();
	HANDLE hDevice = CreateFileA("\\\\.\\CrashDumpUpload",
		NULL,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		return;
	}

	char    out;
	DWORD   in_buffer[2] = { pid, GetParentProcessID() };
	DWORD	returnLen;
	BOOL result = DeviceIoControl(
		hDevice,
		IOCTL_SET_PROTECT_PID,
		in_buffer,
		sizeof(in_buffer),
		&out,
		sizeof(out),
		&returnLen,
		NULL);

	CloseHandle(hDevice);
}

// 解密DLL
bool Driver::DecodeDll(BYTE* in, BYTE* out, DWORD size)
{
	HANDLE hDevice = CreateFileA("\\\\.\\CrashDumpUpload",
		NULL,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("hDevice == INVALID_HANDLE_VALUE\n");
		return false;
	}

	DWORD	returnLen;
	BOOL result = DeviceIoControl(
		hDevice,
		IOCTL_DECODE_DLL,
		in,
		size,
		out,
		size,
		&returnLen,
		NULL);

	CloseHandle(hDevice);
	return true;
}

// 蓝屏
void Driver::BB()
{
	HANDLE hDevice = CreateFileA("\\\\.\\CrashDumpUpload",
		NULL,
		NULL,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		return;
	}

	DWORD v = 0;
	char	output;
	DWORD	returnLen;
	BOOL result = DeviceIoControl(
		hDevice,
		IOCTL_BSOD,
		&v,
		4,
		&output,
		sizeof(char),
		&returnLen,
		NULL);

	CloseHandle(hDevice);
}

// 删除驱动服务
bool Driver::Delete(const wchar_t* name)
{
	if (wcsstr(name, L"firenet_safe")) {
		HANDLE hDevice = CreateFileA("\\\\.\\CrashDumpUpload", NULL, NULL, NULL,OPEN_EXISTING,NULL, NULL);

		if (hDevice == INVALID_HANDLE_VALUE) {
			goto _unstall_;
		}

		DWORD v = 0;
		char	output;
		DWORD	returnLen;
		BOOL result = DeviceIoControl(hDevice, IOCTL_SAFE_UNSTALL, &v, 4, &output, sizeof(char), &returnLen, NULL);

		CloseHandle(hDevice);
	}

_unstall_:
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == schManager)
		return false;

	schService = OpenService(schManager, name, SERVICE_ALL_ACCESS);
	if (NULL == schService) {
		CloseServiceHandle(schManager);
		return false;
	}
	if (!ControlService(schService, SERVICE_CONTROL_STOP, &svcStatus)) {
		//LOG2(L"!ControlService", "red");
		//return false;
	}

	if (!DeleteService(schService)) {
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return false;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);
	return true;
}
