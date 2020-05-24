#pragma once
#include <devioctl.h>
#include <My/Driver/Sys.h>

#define IOCTL_SET_INJECT_X86DLL \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_SET_INJECT_X64DLL \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEL_INJECT_X86DLL \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEL_INJECT_X64DLL \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x903, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_SET_PROTECT_PID \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x100, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_SET_PROTECT_VBOX_PID \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x101, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_SET_HIDE_PID \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x102, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DECODE_DLL \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x200, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SAFE_UNSTALL \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0xf00, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_BSOD \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0xfff, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

class Driver
{
public:
	// ...
	Driver();

	// 测试
	int  Test();
	// 安装文件过滤驱动
	BOOL InstallFsFilter(const char* path, const char* lpszDriverPath, const char* lpszAltitude);
	// 启动文件过滤保护
	BOOL StartFsFilter();
	// 停止文件保护驱动
	BOOL StopFsFilter();
	// 删除文件保护驱动
	BOOL DeleteFsFilter();
	// 安装驱动
	bool InstallDriver(const char* path);
	// 设置保护进程ID
	void SetProtectPid(DWORD pid);
	// 解密Dll
	bool DecodeDll(BYTE* in, BYTE* out, DWORD size);
	// 蓝屏
	void BB();
	// 删除驱动服务
	bool Delete(const wchar_t* name);
public:

	bool  m_bIsInstallDll = false;
	Sys m_SysDll;
};