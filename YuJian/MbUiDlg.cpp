
// MbUiDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MbUi.h"
#include "MbUiDlg.h"
#include "afxdialogex.h"
#include "Game/Driver.h"
#include "Game/HttpClient.h"
#include "Game/DownFile.h"
#include <fstream>

#include <My/Common/func.h>
#include <My/Common/Explode.h>
#include <My/Common/MachineID2.h>
#include <My/Driver/KbdMou.h>
#include <My/Db/Sqlite.h>
#include <My/Win32/PE.h>
#include <My/Win32/Peb.h>
#include <My/Common/C.h>
#include <My/Common/Des.h>

#include "Game/LoadLibraryR.h"
#include "Game/GetProcAddressR.h"

#include "Asm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MSG_CALLJS       (WM_USER+100)

#define GetGameProc(type, index) (type)g_dlg->GetGameProcAddress(index)

typedef void(*WINAPI Func_Game_NoArg)();
typedef void (*WINAPI Func_Game_Init)(HWND, const char*);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMbUiDlg 对话框


CMbUiDlg* g_dlg = nullptr;
CMbUiDlg::CMbUiDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MBUI_DIALOG, pParent)
{
	g_dlg = this;
	g_dlg->m_ConfPath[253] = 0x16;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

#ifndef _DEBUG
	char name[32] = { 'Z','w','S','e','t','I','n','f','o','r','m','a','t','i','o','n','T','h','r','e','a','d', 0 };
	int index = GetSysCallIndex(name);
	Asm_Nd(GetCurrentThread(), index);
#endif

}

void CMbUiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMbUiDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(MSG_CALLJS, &CMbUiDlg::OnCallJs)
	ON_WM_HOTKEY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CMbUiDlg 消息处理程序

BOOL CMbUiDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);

#ifndef _DEBUG // ZwSetInformationThread
	char syscall_name[32] = { 'Z','w','S','e','t','I','n','f','o','r','m','a','t','i','o','n','T','h','r','e','a','d',0 };
	int syscall_index_1 = GetSysCallIndex(syscall_name);
#endif

	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

#ifndef _DEBUG
	Asm_Nd((HANDLE)-2, 10);
	Asm_Nd((HANDLE)-2, 13);
	Asm_Nd(GetCurrentThread(), syscall_index_1);
#endif

	// TODO: 在此添加额外的初始化代码

	//RegisterHotKey(m_hWnd, 1001, NULL, 'P');
	//RegisterHotKey(m_hWnd, 1002, NULL, 'C');

	g_dlg->m_ConfPath[252] = 0x89;

#if 0
	bool adr = AdjustPrivileges();
	::printf("提权状态结果:%d\n", adr);

	DWORD adbs[10];
	DWORD adbs_l = SGetProcessIds(L"adb.exe", adbs, sizeof(adbs) / sizeof(DWORD));
	for (int i = 0; i < adbs_l; i++) {
		char cmd[64];
		sprintf_s(cmd, "taskkill /f /t /pid %d", adbs[i]);
		system(cmd);
	}
	adbs_l = SGetProcessIds(L"conhost.exe", adbs, sizeof(adbs) / sizeof(DWORD));
	for (int i = 0; i < adbs_l; i++) {
		char cmd[64];
		sprintf_s(cmd, "taskkill /f /t /pid %d", adbs[i]);
		//system(cmd);
	}
#endif

#if 0
	/* 添加自启项 */
	char reg[255];
	sprintf_s(reg, "reg add HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run /v fsauto /t FS_AUTO /d %s\\test.exe /f", m_ConfPath);
	system(reg);
#endif


#if 0
	AllocConsole();
	freopen("CON", "w", stdout);
#endif

#if 0
	AllocConsole();
	freopen("CON", "w", stdout);

	char syscall_name2[32] = { 'Z','w','S','e','t','I','n','f','o','r','m','a','t','i','o','n','T','h','r','e','a','d',0 };
	int index2 = GetSysCallIndex(syscall_name2);
	//Asm_Nd(GetCurrentThread(), index2);
	__int64 back_addr = Asm_Rip();
	printf("index:%d %lld %s %d\n", index2, back_addr, GetCommandLineA(), GetParentProcessID());
#endif
	m_pDriver = new Driver;
	m_pDriver->InstallDriver(NULL);
#ifndef _DEBUG
	m_pDriver->SetProtectPid(0);
#endif

#ifdef _DEBUG

#endif
#ifdef  _DEBUG
	AllocConsole();
	freopen("CON", "w", stdout);

	SHGetSpecialFolderPathA(0, m_ConfPath, CSIDL_DESKTOPDIRECTORY, 0);
	strcat(m_ConfPath, "\\YuJian");
	HWND error = ::FindWindow(NULL, L"Error");
	if (error) {
		HWND sure = ::FindWindowEx(error, NULL, NULL, L"确定");
		::printf("error:%08X %08X\n", DWORD(error), DWORD(sure));
		::SendMessage(error, WM_COMMAND, MAKEWPARAM(0x02, BN_CLICKED), (LPARAM)sure);
	}
#if 0
	Sleep(5000);

	int n = 0;
	keybd_event(VK_F5, 0, 0, NULL); 
	while (++n < 30) {
		SetCursorPos(1000 + (n*5), 500 + (n*5));
		Sleep(1000);
	}
	keybd_event(VK_F5, 0, KEYEVENTF_KEYUP, NULL);
#endif
#if 0
	LRESULT result = 0;
	int x = 300, y = 300;
	HWND hwnd = (HWND)::FindWindow(nullptr, L"【魔域】");
	hwnd = ::FindWindowEx(hwnd, NULL, NULL, NULL);

	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	HMODULE h3drole = EnumModuleBaseAddr(pid, L"3drole.dll");
	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	PVOID xPtr = (BYTE*)h3drole + 0x0F60C3C;
	PVOID yPtr = (BYTE*)h3drole + 0x0F60C38;
	DWORD data[2] = { 0, 0 };
	SIZE_T read_length = 0;

	while (true) {
		ReadProcessMemory(hProcess, yPtr, data, sizeof(data), &read_length);
		printf("pos:%d,%d.\n", data[1], data[0]);
		Sleep(1000);
	}
#endif

	char key[] = "1234567890";
	char str[] = "test123";
	char desStr[128], desStr2[128];
	DesEncrypt(desStr, key, str, strlen(str));
	DesDecrypt(desStr2, key, desStr, strlen(desStr), true);
	//printf("%s %d\n", desStr, strlen(desStr));
	//printf("%s %d\n", desStr2, strlen(desStr2));

	int index = GetSysCallIndex("ZwSetInformationThread");
	printf("index:%d\n", index);
	//Asm_Nd(GetCurrentThread(), index);

	pfnNtQuerySetInformationThread f = (pfnNtQuerySetInformationThread)GetNtdllProcAddress("ZwSetInformationThread");
	//NTSTATUS sta = f(GetCurrentThread(), ThreadHideFromDebugger, NULL, 0);

	int cpuid[16];
	memset(cpuid, 0, sizeof(cpuid));
	__cpuidex(cpuid, 0, 0);
	printf("%08X %08X %08X %08X\n", cpuid[0], cpuid[1], cpuid[2], cpuid[3]);
	__cpuidex(&cpuid[4], 1, 0); // 1,2索引忽略 取cpuid[4],cpuid[7]
	printf("%08X %08X %08X %08X\n", cpuid[4], cpuid[5], cpuid[6], cpuid[7]);
	__cpuidex(&cpuid[8], 1, 1);
	printf("%08X %08X %08X %08X\n", cpuid[8], cpuid[9], cpuid[10], cpuid[11]);
	__cpuidex(&cpuid[12], 0x80000001, 0);
	printf("%08X %08X %08X %08X\n", cpuid[12], cpuid[13], cpuid[14], cpuid[15]);

	// while (true);
#else
#if 1
	pfnNtQuerySetInformationThread f = (pfnNtQuerySetInformationThread)GetNtdllProcAddress("ZwSetInformationThread");
	NTSTATUS sta = f(GetCurrentThread(), ThreadHideFromDebugger, NULL, 0);
	//::printf("sta:%d\n", sta);
#endif
	GetCurrentDirectoryA(MAX_PATH, m_ConfPath);
#endif //  _DEBUG

#ifndef x64
#else
#ifdef _DEBUG
#if 1
	//m_hGameModule = LoadLibrary(L"C:\\Users\\fucan\\Desktop\\MNQ-9Star\\vs\\x64\\Game.dll");
	CString game_dll_name = L"C:\\Users\\12028\\Desktop\\工具\\Vs\\x64\\sound.dll";
	//game_dll_name = L"C:\\Users\\12028\\Desktop\\工具\\Vs\\x64\\sound-e";
	LoadGameModule(game_dll_name, true);
	printf("m_hGameModule:%p %p\n", m_hGameModule, time);

	int _tm = time(nullptr);
#else
	m_hGameModule = LoadLibrary(L"C:\\Users\\fucan\\Desktop\\MNQ-9Star\\vs\\x64\\XiaoAo.dll");
#endif
#else
	WCHAR desktop_path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, desktop_path);
	CString dll;
	dll = desktop_path;
	dll += L"\\KGMusic\\sound";
	LoadGameModule(dll, false);

	CString tip;
	tip.Format(L"(%d)\n", GetLastError());

	//AfxMessageBox(dll);
	//AfxMessageBox(tip);
	if (m_hGameModule) {
		//AfxMessageBox(L"加载成功Game");
	}
	else{
		//AfxMessageBox(L"加载失败Game");
	}
	
#endif
	
#endif

	CRect rect(0, 0, MyRand(750, 760), MyRand(720, 750));
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE);
	GetClientRect(rect);
	jsBindFunction("CallCpp", js_Func, 1);
	m_web = wkeCreateWebWindow(WKE_WINDOW_TYPE_CONTROL, *this, 0, 0, rect.Width(), rect.Height());
	//wkeLoadURL(m_web, "https://www.baidu.com");
	wchar_t html[MAX_PATH];
#ifdef _DEBUG
	wsprintfW(html, L"E:\\CPP\\YuJian\\html\\static\\index.html");
#else
	wsprintfW(html, L"%hs\\Lyric\\static\\index.html", m_ConfPath);
#endif
	char web_title[16];
	RandStr(web_title, 6, 15, MyRand(750, 1098));
	wkeSetWindowTitle(m_web, web_title);

	wkeLoadFileW(m_web, html);
	wkeShowWindow(m_web, TRUE);

	m_es = wkeGlobalExec(m_web);
	
	wkeOnDocumentReady(m_web, DocumentReadyCallback, this);

	//mouse_event(MOUSEEVENTF_MOVE| MOUSEEVENTF_ABSOLUTE, 1000 * 65536 / GetSystemMetrics(SM_CXSCREEN), 50 * 65536 / GetSystemMetrics(SM_CYSCREEN), 0, 0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 加载游戏模块
void CMbUiDlg::LoadGameModule(CString& name, bool is_debug)
{
	//AfxMessageBox(L"1");
	if (is_debug) {
		m_hGameModule = LoadLibrary(name);
		return;
	}

	//AfxMessageBox(L"2");
	CString tmp;
	WCHAR MyDir[_MAX_PATH];

	HANDLE handle = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		//AfxMessageBox(name);
		return;
	}

	DWORD size = GetFileSize(handle, NULL);
	DWORD r_size = 0; // WIN7一定需要这个参数
	BYTE* buffer = new BYTE[size], *out = new BYTE[size];

	if (!buffer || !out) {
		AfxMessageBox(L"!buffer || !ou");
	}

	//AfxMessageBox(L"3");
	if (!ReadFile(handle, buffer, size, &r_size, NULL)) {
		//AfxMessageBox(L"无法读取游戏模块");
		goto end;
	}

	CloseHandle(handle);
	//AllocConsole();
	//freopen("CON", "w", stdout);

	//AfxMessageBox(L"4");
	m_pDriver->DecodeDll(buffer, out, size);

	::SHGetSpecialFolderPathW(this->GetSafeHwnd(), MyDir, CSIDL_DESKTOP, 0);
	tmp = L"C:\\Windows";
	tmp += L"\\System32";
	tmp += L"\\tmp.bak";

	//AfxMessageBox(tmp);

	
	//printf("%c%c %c%c\n", buffer[0], buffer[1], out[0], out[1]);

	handle = CreateFile(tmp, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		//AfxMessageBox(L"!handle2");
		goto end;
	}

	DWORD w_size = 0;
	if (!WriteFile(handle, out, size, &w_size, NULL)) {
		//AfxMessageBox(L"!WriteFile");
		goto end;
	}
		
	CloseHandle(handle);

	m_hGameModule = LoadLibrary(tmp);
	if (!m_hGameModule) {
		//printf("m_hGameModule:%0p, %d\n", m_hGameModule, GetLastError());
	}
end:
	delete buffer;
	delete out;
}

void CMbUiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMbUiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMbUiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CMbUiDlg::GetSysCallIndex(const char * name)
{
	int index = -1;
	char* addr = (char*)GetNtdllProcAddress(name);
	for (int i = 0; i < 0x60; i++) {
		char v = addr[i] & 0xff;
		//printf("v:%02X\n", v&0xff);
		if ((v&0xff) == 0xcc || (v & 0xff) == 0xc3)
			break;

		if ((v&0xff) == 0xB8) { // mov eax,...
			//printf("v2:%02X\n", addr[i + 3]&0xff);
			index = *(int*)&addr[i + 1];
			//printf("v2:%02X %08X\n", addr[i + 3] & 0xff, index);
			break;
		}
	}
	return index;
}

// 调用JS
LRESULT CMbUiDlg::OnCallJs(WPARAM w, LPARAM l)
{
	my_msg* msg = (my_msg*)w;
	//::printf("OnAddLog:%08X\n", msg);

	//printf("msg:%d %s\n", msg->op, msg->text);
	//AfxMessageBox(msg->text_w);
	switch (msg->op)
	{
	case MSG_ADDLOG:
		AddLog(msg);
		break;
	case MSG_SETTEXT:
		SetText(msg);
		break;
	case MSG_ADDTABLEROW:
		AddTableRow(msg);
		break;
	case MSG_FILLTABLE:
		FillTable(msg);
		break;
	case MSG_SETSETTING:
		SetSetting(msg);
		break;
	case MSG_ALERT:
		Alert(msg);
		break;
	case MSG_UPSTATUSTEXT:
		UpdateStatusText(msg);
		break;
	case MSG_UPVER_OK:
		UpVerOk(msg);
		break;
	case MSG_UPSTEP_OK:
		UpStepOk(msg);
		break;
	case MSG_VERIFY_OK:
		VerifyOk(msg);
		break;
	default:
		break;
	}

	if (l) {
		::printf("delete msg %p\n", msg);
		//delete msg;
	}

	return 0;
}

// 写入日记信息
void CMbUiDlg::AddLog(my_msg* pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "AddLog");
	jsValue vs[3];
	vs[0] = jsNull();
	if (pMsg->text[0]) {
		vs[1] = jsString(es, pMsg->text);
	}
	else {
		vs[1] = jsStringW(es, pMsg->text_w);
	}

	if (pMsg->cla)
		vs[2] = jsString(es, pMsg->cla);
	else
		vs[2] = jsNull();

	jsCallGlobal(es, f, vs, sizeof(vs)/sizeof(jsValue));
}

// 设置html文字
void CMbUiDlg::SetText(my_msg* pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f, vs[5];
	int argv = 0;

	if (pMsg->table_text) {
		f = jsGetGlobal(es, "UpdateTableText");
		vs[0] = jsString(es, pMsg->id);
		vs[1] = jsInt(pMsg->value[0]);
		vs[2] = jsInt(pMsg->value[1]);
		if (pMsg->text[0]) vs[3] = jsString(es, pMsg->text);
		else vs[3] = jsStringW(es, pMsg->text_w);
		argv = 4;
	}
	else if (pMsg->status_text) {
		f = jsGetGlobal(es, "UpdateStatusText");
		if (pMsg->text[0]) vs[0] = jsString(es, pMsg->text);
		else vs[0] = jsStringW(es, pMsg->text_w);
		vs[1] = jsInt(pMsg->value[0]);
		argv = 2;
	}
	else {
		f = jsGetGlobal(es, "SetText");
		vs[0] = jsString(es, pMsg->id);
		if (pMsg->text[0]) vs[1] = jsString(es, pMsg->text);
		else vs[1] = jsStringW(es, pMsg->text_w);
		argv = 2;
	}
	
	jsCallGlobal(es, f, vs, argv);
}

// 更新html文字
void CMbUiDlg::AddTableRow(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "AddTableRow");
	jsValue vs[4];
	vs[0] = jsString(es, pMsg->id);
	vs[1] = jsInt(0);
	vs[2] = jsInt(pMsg->value[0]);
	if (pMsg->text[0]) vs[3] = jsString(es, pMsg->text);
	else vs[3] = jsStringW(es, pMsg->text_w);

	jsCallGlobal(es, f, vs, sizeof(vs) / sizeof(jsValue));
}

void  CMbUiDlg::FillTable(my_msg* pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "FillTableRow");
	jsValue vs[3];
	vs[0] = jsNull();
	vs[1] = jsInt(0);
	vs[2] = jsInt(0);

	jsCallGlobal(es, f, vs, sizeof(vs) / sizeof(jsValue));
}

// 设置
void CMbUiDlg::SetSetting(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "SetSetting");
	jsValue vs[3];
	vs[0] = jsString(es, pMsg->id);
	vs[1] = jsInt(pMsg->value[0]);
	vs[2] = jsInt(pMsg->value[1]);

	jsCallGlobal(es, f, vs, sizeof(vs) / sizeof(jsValue));
}

// Alert
void CMbUiDlg::Alert(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "ShowMsg");
	jsValue vs[3];
	if (pMsg->text[0]) vs[0] = jsString(es, pMsg->text);
	else vs[0] = jsStringW(es, pMsg->text_w);
	vs[1] = jsStringW(es, L"提示");
	vs[2] = jsInt(pMsg->value[0]);

	jsCallGlobal(es, f, vs, sizeof(vs) / sizeof(jsValue));
}

// 更新状态栏文字
void CMbUiDlg::UpdateStatusText(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "UpdateStatusText");
	jsValue vs[2];
	if (pMsg->text[0]) vs[0] = jsString(es, pMsg->text);
	else vs[0] = jsStringW(es, pMsg->text_w);
	vs[1] = jsInt(pMsg->value[0]);

	jsCallGlobal(es, f, vs, sizeof(vs) / sizeof(jsValue));
}

// 更新版本号完成
void CMbUiDlg::UpVerOk(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "UpdateVerOk");

	jsCallGlobal(es, f, nullptr, 0);
}

// 更新版本号完成
void CMbUiDlg::UpStepOk(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "UpdateStepOk");

	jsCallGlobal(es, f, nullptr, 0);
}

// 验证成功啦
void CMbUiDlg::VerifyOk(my_msg * pMsg)
{
	jsExecState es = wkeGlobalExec(m_web);
	jsValue f = jsGetGlobal(es, "VerifyOk");

	jsCallGlobal(es, f, nullptr, 0);
}

// 获取游戏模块函数
FARPROC CMbUiDlg::GetGameProcAddress(int index)
{
	__int64* p = (__int64*)((PBYTE)&m_DllFunc + (index * sizeof(PVOID)));
	return (FARPROC)(*p);
}

// 文档加载完成
void WKE_CALL_TYPE CMbUiDlg::DocumentReadyCallback(wkeWebView webView, void* param)
{
	//AfxMessageBox(L"DocumentReadyCallback");
	g_dlg->m_ConfPath[251] = 0x99;
#if 1
	CMbUiDlg* p = (CMbUiDlg*)param;
	jsExecState es = wkeGlobalExec(webView);
	jsValue f = jsGetGlobal(es, "test");
	if (f) {
		//AfxMessageBox(L"jsGetGlobal");
	}
	jsValue vs[2];
	vs[0] = jsInt(123);
	vs[1] = jsStringW(es, L"法克油");
	jsCallGlobal(es, f, vs, 2);
#endif
	if (!g_dlg->m_hGameModule) {
		CString msg;
		msg.Format(L"无法加载游戏模块(%d)！！！", GetLastError());
		//AfxMessageBox(msg);
		return;
	}

	Func_Game_Init Game_Init = Func_Game_Init(GetProcAddress(g_dlg->m_hGameModule, "EntryIn"));
	if (Game_Init) {
		//printf("%p\n", Game_Init);

		ExportDllFunc** sp = (ExportDllFunc**)&g_dlg->m_ConfPath[230];
		*sp = (ExportDllFunc*)&g_dlg->m_DllFunc;
		printf("ExportDllFunc:%p\n", *sp);
		g_dlg->m_ConfPath[250] = 0xCB;
		Game_Init(g_dlg->m_hWnd, g_dlg->m_ConfPath);
	}

	CreateThread(NULL, NULL, Thread, param, NULL, NULL);
}

// js调用函数
jsValue JS_CALL CMbUiDlg::js_Func(jsExecState es)
{
	::printf("参数数量:%d\n", jsArgCount(es));
	if (jsArgType(es, 0) != JSTYPE_STRING)
		return 0;

	const utf8* func_name = jsToString(es, jsArg(es, 0));
	::printf("%s\n", jsToString(es, jsArg(es, 0)));
	if (strcmp("set_title", func_name) == 0)
		return g_dlg->SetTitle(es);
	if (strcmp("open_menu", func_name) == 0)
		return g_dlg->OpenMenu(es);
	if (strcmp("start", func_name) == 0)
		return g_dlg->InstallDll(es);
	if (strcmp("open_game", func_name) == 0)
		return g_dlg->OpenGame(es);
	if (strcmp("close_game", func_name) == 0)
		return g_dlg->CloseGame(es);
	if (strcmp("in_team", func_name) == 0)
		return g_dlg->InTeam(es);
	if (strcmp("put_setting", func_name) == 0)
		return g_dlg->PutSetting(es);
	if (strcmp("getin_card", func_name) == 0)
		return g_dlg->GetInCard(es);
	if (strcmp("verify_card", func_name) == 0)
		return g_dlg->VerifyCard(es);
	if (strcmp("get_process", func_name) == 0)
		return g_dlg->GetProcess(es);
	if (strcmp("hide_process", func_name) == 0)
		return g_dlg->HideProcess(es);
	if (strcmp("fb_record", func_name) == 0)
		return g_dlg->FBRecord(es);
	if (strcmp("talk", func_name) == 0)
		return g_dlg->Talk(es);
	if (strcmp("update_ver", func_name) == 0) {
		CreateThread(NULL, NULL, UpdateVer, g_dlg, NULL, NULL);
		return jsInt(0);
	}
	if (strcmp("update_step", func_name) == 0) {
		CreateThread(NULL, NULL, UpdateStep, g_dlg, NULL, NULL);
		return jsInt(0);
	}

	CString str;
	str.Format(L"参数数量:%d", jsArgCount(es));
	return jsStringW(es, str);
}

// 设置程序标题
jsValue CMbUiDlg::SetTitle(jsExecState es)
{
	SetWindowText(jsToStringW(es, jsArg(es, 1)));
	return jsInt(0);
}

// 打开帐号菜单 返回是否登录
jsValue CMbUiDlg::OpenMenu(jsExecState es)
{
	//printf("OpenMenu:%d\n", jsToInt(es, jsArg(es, 1)));
	typedef int (*WINAPI Func_Game_IsLogin)(int index); // Game_IsLogin
	return jsInt((GetGameProc(Func_Game_IsLogin, 3))(jsToInt(es, jsArg(es, 1))));
}

// 安装dll驱动
jsValue CMbUiDlg::InstallDll(jsExecState es)
{
	typedef int (*WINAPI Func_Game_InstallDll)(); // Game_InstallDll
	return jsInt((GetGameProc(Func_Game_InstallDll, 4))());
}

// 打开游戏
jsValue CMbUiDlg::OpenGame(jsExecState es)
{
	typedef int(*WINAPI Func_Game_OpenGame)(int, int); // Game_OpenGame
	return jsInt((GetGameProc(Func_Game_OpenGame, 5))(jsToInt(es, jsArg(es, 1)), jsToInt(es, jsArg(es, 2))));
}

// 关闭游戏
jsValue CMbUiDlg::CloseGame(jsExecState es)
{
	typedef int(*WINAPI Func_Game_CloseGame)(int index); // Game_CloseGame
	return jsInt((GetGameProc(Func_Game_CloseGame, 6))(jsToInt(es, jsArg(es, 1))));
}

// 设置入队
jsValue CMbUiDlg::InTeam(jsExecState es)
{
	typedef int(*WINAPI Func_Game_InTeam)(int index); // Game_InTeam
	return jsInt((GetGameProc(Func_Game_InTeam, 7))(jsToInt(es, jsArg(es, 1))));
}

// 设置
jsValue CMbUiDlg::PutSetting(jsExecState es)
{
	typedef int(*WINAPI Func_Game_PutSetting)(const wchar_t*, int); // Game_PutSetting
	return jsInt((GetGameProc(Func_Game_PutSetting, 8))(
		jsToStringW(es, jsArg(es, 1)),
		jsToInt(es, jsArg(es, 2)))
	);
}

// 转移卡号本机
jsValue CMbUiDlg::GetInCard(jsExecState es)
{
	typedef int(*WINAPI Func_Game_GetInCard)(const wchar_t*); // Game_GetInCard
	return jsInt((GetGameProc(Func_Game_GetInCard, 9))(jsToStringW(es, jsArg(es, 1))));
}

// 验证卡号
jsValue CMbUiDlg::VerifyCard(jsExecState es)
{
	typedef int(*WINAPI Func_Game_VerifyCard)(const wchar_t*, const wchar_t*); // Game_VerifyCard
	return jsInt
	(
		(GetGameProc(Func_Game_VerifyCard, 10))
		(
			jsToStringW(es, jsArg(es, 1)),
			jsToStringW(es, jsArg(es, 2))
		)
	);
}

struct jsProcess {
	jsExecState es;
	jsValue v;
	jsValue object;
	int     length;
};

// 获取进程信息
jsValue CMbUiDlg::GetProcess(jsExecState es)
{
	jsProcess pro;
	pro.es = es;
	pro.v = jsEmptyArray(es);
	pro.object = 0;
	pro.length = 0;

	//EnumWindows(EnumWindowsProc, (LPARAM)&pro);
	// 进程数量
	DWORD count = 0;
	// 定义进程信息结构
	PROCESSENTRY32 pe32 = { sizeof(pe32) };
	// 创建系统当前进程快照
	HANDLE hProcessShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessShot == INVALID_HANDLE_VALUE)
		return 0;

	if (Process32First(hProcessShot, &pe32)) {
		do {
			pro.object = jsEmptyObject(es);
			jsSet(pro.es, pro.object, "id", jsInt(pe32.th32ProcessID));
			jsSet(pro.es, pro.object, "title", jsStringW(es, pe32.szExeFile));
			jsSetAt(pro.es, pro.v, pro.length, pro.object);

			pro.length++;

		} while (Process32Next(hProcessShot, &pe32));
	}

	CloseHandle(hProcessShot);

	return pro.v;
}

// 隐藏进程信息
jsValue CMbUiDlg::HideProcess(jsExecState es)
{
	int id = jsToInt(es, jsArg(es, 1));
	m_pDriver->SetProtectPid(id);
	return jsInt(1);
}

BOOL CMbUiDlg::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	jsProcess* p = (jsProcess*)lParam;

	wchar_t text[256] = { 0 };
	//AfxMessageBox(text);
	::GetWindowTextW(hWnd, text, sizeof(text));
	if (wcslen(text) && wcscmp(L"Default IME", text)) {
		p->object = jsEmptyObject(p->es);

		DWORD dwPid;
		GetWindowThreadProcessId(hWnd, &dwPid);
		jsSet(p->es, p->object, "id", jsInt(dwPid));
		jsSet(p->es, p->object, "title", jsStringW(p->es, text));
		jsSetAt(p->es, p->v, p->length, p->object);

		p->length++;
	}

	return TRUE;
}

// 查询副本记录
jsValue CMbUiDlg::FBRecord(jsExecState es)
{
	typedef int(*WINAPI Func_Game_SelectFBRecord)(char***, int*); // Game_SelectFBRecord
	int col = 0;
	char** result = nullptr;
	int row = (GetGameProc(Func_Game_SelectFBRecord, 11))(&result, &col);

	jsValue v = jsEmptyArray(es);
	for (int i = 1; i <= row; i++) {
		int index = i * col;

		jsValue object = jsEmptyObject(es);
		jsSet(es, object, "id", jsInt(atoi(result[index])));
		jsSet(es, object, "start_time", jsInt(atoi(result[index + 1])));
		jsSet(es, object, "end_time", jsInt(atoi(result[index + 2])));
		jsSet(es, object, "time_long", jsInt(atoi(result[index + 3])));
		jsSet(es, object, "status", jsInt(atoi(result[index + 4])));
		jsSetAt(es, v, i - 1, object);
	}
	return v;
}

// 游戏内部喊话
jsValue CMbUiDlg::Talk(jsExecState es)
{
	typedef int(*WINAPI Func_Game_Talk)(const char* text, int type); // Talk
	Func_Game_Talk func = (Func_Game_Talk)GetProcAddress(m_hGameModule, "Talk");
	func(jsToString(es, jsArg(es, 1)), jsToInt(es, jsArg(es, 2)));

	return jsInt(168);
}

// 线程
DWORD WINAPI CMbUiDlg::Thread(LPVOID param)
{
	while (true) {
		my_msg msg;
		COPY_MSG_W(msg, 1, nullptr, L"text w ", "cb", 0);
		//::PostMessage(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(800);
	}
	
	return 0;
}

#define DOWNURL "http://137.59.149.38/upv"

// 更新版本号
DWORD WINAPI CMbUiDlg::UpdateVer(LPVOID)
{
	my_msg msg;
	msg.op = MSG_UPVER_OK;

	typedef bool(*WINAPI Func_Game_IsValid)();
	if (!((GetGameProc(Func_Game_IsValid, 0))())) {
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(100);
		::printf("!((GetGameProc(Func_Game_IsValid, 0))()))");
		return 0;
	}

	printf("检查更新版本\n");
	CString path;
	path.Format(L"/upv?ver=%d=", time(nullptr));
	//AfxMessageBox(path);
	std::string result;
	HttpClient http;
	http.Request(L"137.59.149.38", path.GetBuffer(), result);
	//printf("%ws %s\n", path, result.c_str());
	Explode explode("|", result.c_str());
	if (explode.GetCount() < 7) {
		::MessageBox(NULL, L"检查失败, 请重试.", L"检查更新", MB_OK);
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(100);
		return 0;
	}

	std::string ver;
	char ver_file[255];
	sprintf_s(ver_file, "%s\\ver.ini", g_dlg->m_ConfPath);
	ifstream fr(ver_file, fstream::in);
	getline(fr, ver);
	if (!fr.is_open()) {
		printf("没有ver:%s\n", ver_file);
		ver = "UI.1.0.0|ld2.1.0.0|Game.1.0.0|ld.1.0.0|pixel.1.0.0|html.1.0.0|ldNews.1.0.0|team.1.0.0|pos.1.0.0";
	}

	Explode test("|", ver.c_str());
	if (test.GetCount() != 9) {
		ver = "UI.1.0.0|ld2.1.0.0|Game.1.0.0|ld.1.0.0|pixel.1.0.0|html.1.0.0|ldNews.1.0.0|team.1.0.0|pos.1.0.0";
	}

	Explode arr("|", ver.c_str());

	::printf("%s\n%s\n", result.c_str(), ver.c_str());

	msg.op = MSG_SETTEXT;
	msg.status_text = 1;
	msg.value[0] = 1;
	bool update = false;
	CString csMsg = L"";
	char url[256], machine_id[33], host[256];
	MachineID2 mac;
	mac.GetMachineID(machine_id);
	machine_id[32] = 0;
	sprintf_s(host, "%s?%d&machine_id=%s&game=1&file", DOWNURL, time(nullptr), machine_id);
	//printf("update 机器码:%s.\n", machine_id);

	if (strcmp(arr[1], explode[1]) != 0) {
		update = true;
		csMsg = L"更新完成, 程序重启动后生效.";
		printf("下载ld2-e\n");
		wcscpy(msg.text_w, L"下载ld2-e");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(100);
		sprintf_s(url, "%s=ld2-e", host);
		DownFile(url, "KGMusic/ld2-e", NULL);
	}
	if (strcmp(arr[2], explode[2]) != 0) {
		update = true;
		csMsg = L"更新完成, 程序重启动后生效.";
		printf("下载sound\n");
		wcscpy(msg.text_w, L"下载sound");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(100);
		sprintf_s(url, "%s=sound", host);
		DownFile(url, "KGMusic/sound", NULL);
	}
	if (strcmp(arr[3], explode[3]) != 0) {
		update = true;
		csMsg = L"更新完成, 程序重启动后生效.";
		printf("下载ld-e\n");
		wcscpy(msg.text_w, L"下载ld-e");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(100);
		sprintf_s(url, "%s=ld-e", host);
		DownFile(url, "KGMusic/ld-e", NULL);
	}
	if (strcmp(arr[4], explode[4]) != 0) {
		update = true;
		csMsg = L"更新完成, 程序重启动后生效.";
		printf("下载p.ini\n");
		wcscpy(msg.text_w, L"下载p.ini");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(100);
		sprintf_s(url, "%s=p.ini", host);
		DownFile(url, "Cache/p.ini", NULL);
	}
	if (strcmp(arr[5], explode[5]) != 0) {
		update = true;
		csMsg = L"更新完成, 程序重启动后生效.";

		printf("下载index.html\n");
		wcscpy(msg.text_w, L"下载index.html...");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, NULL);
		Sleep(100);
		sprintf_s(url, "%s=html/static/index.html", host);
		DownFile(url, "Lyric/static/index.html", NULL);

		printf("下载main.css\n");
		wcscpy(msg.text_w, L"下载main.css...");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, NULL);
		Sleep(100);
		sprintf_s(url, "%s=html/static/main.css", host);
		DownFile(url, "Lyric/static/main.css", NULL);

		printf("下载main.js\n");
		wcscpy(msg.text_w, L"下载main.js...");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, NULL);
		Sleep(100);
		sprintf_s(url, "%s=html/static/main.js", host);
		DownFile(url, "Lyric/static/main.js", NULL);
	}
	if (strcmp(arr[6], explode[6]) != 0) {
		update = true;
		csMsg = L"更新完成, 计算机重启动后生效(sys).";
		printf("下载ldNews.sys\n");
		wcscpy(msg.text_w, L"下载ldNews.sys");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Driver dvr;
		dvr.Delete(L"net2020");
		Sleep(100);
		sprintf_s(url, "%s=ldNews.sys", host);
		DownFile(url, "KGMusic/ldNews.sys", NULL);
	}
	if (strcmp(arr[7], explode[7]) != 0) {
		update = true;
		csMsg = L"更新完成, 重启程序生效.";
		printf("下载3dmark.dll\n");
		wcscpy(msg.text_w, L"下载3dmark.dll");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(100);
		sprintf_s(url, "%s=3dmark.dll", host);
		DownFile(url, "KGMusic/3dmark.dll", NULL);
	}
	if (strcmp(arr[8], explode[8]) != 0) {
		update = true;
		csMsg = L"更新完成, 重启程序后生效.";
		
		wcscpy(msg.text_w, L"下载n.ini");
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
		Sleep(100);
		sprintf_s(url, "%s=n.ini", host);
		printf("下载n.ini %s\n", url);
		DownFile(url, "Cache/n.ini", NULL);
	}
	if (strcmp(arr[0], explode[0]) != 0) {
		char param[128];
		sprintf_s(param, "御剑.exe %s=御剑.exe", host);

		fr.close();

		ofstream fw;
		fw.open(ver_file);
		fw << result;
		fw.close();

		ShellExecuteA(NULL, "open", "down.exe", param, g_dlg->m_ConfPath, SW_SHOWNORMAL);

		if (strcmp(arr[2], explode[2]) == 0) {
			ExitProcess(0);
			return 0;
		}
	}

	my_msg msg2;
	msg2.op = MSG_UPVER_OK;
	PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg2, 0);
	Sleep(100);

	if (update) {
		my_msg msg3;
		msg3.op = MSG_ALERT;
		msg3.value[0] = 1;
		wcscpy(msg3.text_w, csMsg);
		PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg3, 0);
	}

	fr.close();

	ofstream fw;
	fw.open(ver_file);
	fw << result;
	fw.close();

	return 0;
}

// 更新流程文件
DWORD WINAPI CMbUiDlg::UpdateStep(LPVOID)
{
	my_msg msg;
	msg.op = MSG_SETTEXT;
	msg.status_text = 1;
	msg.value[0] = 1;

	char url[256], machine_id[33], host[256];
	MachineID2 mac;
	mac.GetMachineID(machine_id);
	machine_id[32] = 0;
	sprintf_s(host, "%s?%d&machine_id=%s&game=1&file", DOWNURL, time(nullptr), machine_id);
	
	wcscpy(msg.text_w, L"下载高配-1.txt");
	PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
	Sleep(100);
	sprintf_s(url, "%s=高配-1.txt", host);
	DownFile(url, "全秒版/高配-1.txt", NULL);

	wcscpy(msg.text_w, L"下载低配-1.txt");
	PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
	Sleep(100);
	sprintf_s(url, "%s=低配-1.txt", host);
	DownFile(url, "全秒版/低配-1.txt", NULL);

	wcscpy(msg.text_w, L"下载高配-无宠物.txt");
	PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg, 0);
	Sleep(100);
	sprintf_s(url, "%s=高配-无宠物.txt", host);
	DownFile(url, "全秒版/高配-无宠物.txt", NULL);

	my_msg msg2;
	msg2.op = MSG_UPSTEP_OK;
	PostMessageA(g_dlg->m_hWnd, MSG_CALLJS, (WPARAM)&msg2, 0);

	Sleep(100);

	return 0;
}


void CMbUiDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	typedef int(*WINAPI Func_Game_Pause)(bool);
	switch (nHotKeyId)
	{
	case 1001:
		//::printf("OnHotKey:1001\n");
		(GetGameProc(Func_Game_Pause, 2))(true);
		break;
	case 1002:
		//::printf("OnHotKey:1002\n");
		(GetGameProc(Func_Game_Pause, 2))(false);
		break;
	}

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}


void CMbUiDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//typedef int(*WINAPI Func_Game_Relase)();
	//(GetGameProc(Func_Game_Relase, 1))();

	UnregisterHotKey(m_hWnd, 1001);
	UnregisterHotKey(m_hWnd, 1002);

	CDialogEx::OnClose();
}
