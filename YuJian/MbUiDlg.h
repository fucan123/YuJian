
// MbUiDlg.h: 头文件
//

#pragma once
#include "mb/wke.h"

#define MSG_ADDLOG        1
#define MSG_SETTEXT       2
#define MSG_ADDTABLEROW   3
#define MSG_FILLTABLE     4
#define MSG_SETSETTING    5
#define MSG_ALERT         6
#define MSG_UPSTATUSTEXT  100
#define MSG_UPVER_OK      101
#define MSG_UPSTEP_OK     102
#define MSG_VERIFY_OK     200

typedef struct export_dll_func
{
	PVOID IsVaid;
	PVOID Relase;
	PVOID Pause;
	PVOID IsLogin;
	PVOID InstallDll;
	PVOID OpenGame;
	PVOID CloseGame;
	PVOID InTeam;
	PVOID PutSetting;
	PVOID GetInCard;
	PVOID VerifyCard;
	PVOID SelectFBRecord;
} ExportDllFunc;

struct my_msg {
	int  op;
	char id[32];
	char text[64];
	wchar_t text_w[64];
	char cla[32];
	int  value[10];
	int  table_text;
	int  status_text;
};

#define COPY_MSG(msg, _op, _id, _text, _cla, _value) { \
    ZeroMemory(&msg, sizeof(my_msg)); \
    msg.op = _op; \
    if (_id) strcpy(msg.id, _id); \
	if (_text) strcpy(msg.text, _text); \
    if (_cla) strcpy(msg.cla, _cla); \
	msg.value[0] = _value; \
}

#define COPY_MSGP(p, _op, _id, _text, _cla, _value) { \
    ZeroMemory(p, sizeof(my_msg)); \
    p->op = _op; \
    if (_id) strcpy(p->id, _id); \
	if (_text) strcpy(p->text, _text); \
    if (_cla) strcpy(p->cla, _cla); \
	p->value[0] = _value; \
}

#define COPY_MSG_W(msg, _op, _id, _text_w, _cla, _value) { \
    ZeroMemory(&msg, sizeof(my_msg)); \
    msg.op = _op; \
    if (_id) strcpy(msg.id, _id); \
	if (_text_w) wcscpy(msg.text_w, _text_w); \
    if (_cla) strcpy(msg.cla, _cla); \
	msg.value[0] = _value; \
}

#define COPY_MSGP(p, _op, _id, _text_w, _cla, _value) { \
    ZeroMemory(p, sizeof(my_msg)); \
    p->op = _op; \
    if (_id) strcpy(p->id, _id); \
	if (_text_w) wcscpy(p->text_w, _text_w); \
    if (_cla) strcpy(p->cla, _cla); \
	p->value[0] = _value; \
}

class Driver;
// CMbUiDlg 对话框
class CMbUiDlg : public CDialogEx
{
// 构造
public:
	CMbUiDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MBUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	void    LoadGameModule(CString& name, bool is_debug);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	// 调用JS
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	int     GetSysCallIndex(const char* name);

	LRESULT OnCallJs(WPARAM w, LPARAM l);
	void    AddLog(my_msg* pMsg);
	void    SetText(my_msg* pMsg);
	void    AddTableRow(my_msg* pMsg);
	void    FillTable(my_msg* pMsg);
	void    SetSetting(my_msg* pMsg);
	void    Alert(my_msg* pMsg);
	void    UpdateStatusText(my_msg* pMsg);
	void    UpVerOk(my_msg* pMsg);
	void    UpStepOk(my_msg* pMsg);
	void    VerifyOk(my_msg* pMsg);

	// 获取游戏模块函数
	FARPROC GetGameProcAddress(int index);
public:
	// 文档加载完成
	static void WKE_CALL_TYPE DocumentReadyCallback(wkeWebView webView, void* param);

	// js调用函数
	static jsValue JS_CALL js_Func(jsExecState es);
	// 设置程序标题
	jsValue SetTitle(jsExecState es);
	// 打开帐号菜单 返回是否登录
	jsValue OpenMenu(jsExecState es);
	// 安装dll驱动
	jsValue InstallDll(jsExecState es);
	// 打开游戏
	jsValue OpenGame(jsExecState es);
	// 关闭游戏
	jsValue CloseGame(jsExecState es);
	// 设置入队
	jsValue InTeam(jsExecState es);
	// 设置
	jsValue PutSetting(jsExecState es);
	// 转移卡号本机
	jsValue GetInCard(jsExecState es);
	// 验证卡号
	jsValue VerifyCard(jsExecState es);
	// 查询副本记录
	jsValue FBRecord(jsExecState es);
	// 游戏内部喊话
	jsValue Talk(jsExecState es);
	// 线程
	static DWORD WINAPI Thread(LPVOID param);
	// 更新版本号
	static DWORD WINAPI UpdateVer(LPVOID);
	// 更新流程文件
	static DWORD WINAPI UpdateStep(LPVOID);
public:
	// 配置路径
	char m_ConfPath[255];
	HMODULE m_hGameModule;
	ExportDllFunc m_DllFunc;
	Driver* m_pDriver;

	wkeWebView  m_web;
	jsExecState m_es;
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnClose();
};
