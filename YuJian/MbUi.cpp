
// MbUi.cpp: 定义应用程序的类行为。
//

#include "stdafx.h"
#include "MbUi.h"
#include "MbUiDlg.h"
#include "mb/wke.h"
#include <My/Common/func.h>
#include <My/Common/C.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMbUiApp

BEGIN_MESSAGE_MAP(CMbUiApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMbUiApp 构造

CMbUiApp::CMbUiApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CMbUiApp 对象

CMbUiApp theApp;


// CMbUiApp 初始化

BOOL CMbUiApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	char desktop_path[MAX_PATH];
	SHGetSpecialFolderPathA(0, desktop_path, CSIDL_DESKTOPDIRECTORY, 0);
	strcat(desktop_path, "\\YuJian");
	if (!IsDirExistA(desktop_path)) {
		AfxMessageBox(L"请勿修改文件夹名字.");
		return FALSE;
	}

#ifndef x64
	wkeSetWkeDllPath(L"E:\\下载\\miniblink-200101\\node.dll");
#else
#ifdef _DEBUG
	//wkeSetWkeDllPath(L"E:\\下载\\miniblink-200101\\miniblink_x64.dll");
	ModifyWebWndClass("C:\\Users\\fucan\\Desktop\\YuJian");
	wkeSetWkeDllPath(L"C:\\Users\\fucan\\Desktop\\YuJian\\files\\web.dll");
#else
	char path_a[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, path_a);
	ModifyWebWndClass(path_a);

	wchar_t path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);
	CString dll = path;
	dll += L"\\files\\web.dll";
	//AfxMessageBox(dll);
	wkeSetWkeDllPath(dll);
#endif
#endif
	//测试001
	int wke = wkeInitialize();
	CString wkeStr;
	wkeStr.Format(L"wke:%d", wke);
	//AfxMessageBox(wkeStr);

	CMbUiDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

// HHH
void CMbUiApp::ModifyWebWndClass(const char * path)
{
	char old_file[256], new_file[256];
	sprintf_s(old_file, "%s\\files\\miniblink_x64.dll", path);
	sprintf_s(new_file, "%s\\files\\web.dll", path);
	FILE* _fopen = fopen(old_file, "rb+");
	FILE* _fwrite = fopen(new_file, "wb+");
	::fseek(_fopen, 0, SEEK_END); //定位到文件末
	int file_len = ftell(_fopen); //文件长度
	::fseek(_fopen, 0, SEEK_SET);

	for (int i = 0; i < file_len;) {
		::fseek(_fopen, i, SEEK_SET);

		char data[1024];
		memset(data, 0, sizeof(data));

		size_t size = ::fread(data, 1, sizeof(data), _fopen);
		int old_i = i;
		i += size;

		if (old_i < 0x019E7188 && i > 0x019E7188) {
			int index = 0x019E7188 - old_i;
			memset(&data[index], 0, 12 * 2);
			int num = MyRand(6, 12);
			for (int j = 0; j < num; j++) {
				int index2 = index + j * 2;
				int v = MyRand(1, 2, j);
				if (v == 1)
					data[index2] = MyRand('a', 'z', j);
				else
					data[index2] = MyRand('A', 'Z', j);

				//::printf("%08X:%02X\n", index2, data[index2] & 0xff);
			}
		}

		int write_size = i > file_len ? i - file_len : sizeof(data);
		size_t size2 = ::fwrite(data, 1, size, _fwrite);
		if (size > 0) {
			//::printf("%d, %d\n", (int)size, (int)size2);
		}
		else {
			i += 1;
		}
		//
	}

	::fclose(_fopen);
	::fclose(_fwrite);
}


