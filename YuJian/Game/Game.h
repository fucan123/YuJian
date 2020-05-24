#include "GameServer.h"
#include <Windows.h>
#include <vector>
#include <fstream>

#define P2DW(v) (*(DWORD*)(v))       // 转成DWORD数值
#define P2INT(v) (*(int*)(v))        // 转成int数值

#define LOG(v) m_pGame->AddUILog(v,nullptr)
#define LOGVARP(p,...) { sprintf_s(p,__VA_ARGS__);LOG(p); }
#define LOGVARN(n,...) {char _s[n]; _s[n-1]=0; LOGVARP(_s,__VA_ARGS__); }

#define LOG2(v,cla) m_pGame->AddUILog(v,cla)
#define LOGVARP2(p,cla,...) { sprintf_s(p,__VA_ARGS__);LOG2(p,cla); }
#define LOGVARN2(n,cla,...) {char _s[n]; _s[n-1]=0; LOGVARP2(_s,cla,__VA_ARGS__); }

#define MAKESPOS(v) (int(v/m_fScale))

#define ACCSTA_INIT       0x1000 // 帐号还未登录
#define ACCSTA_READY      0x0001 // 准备登录
#define ACCSTA_LOGIN      0x0002 // 正在登录
#define ACCSTA_ONLINE     0x0004 // 帐号在线
#define ACCSTA_OPENFB     0x0008 // 正在开启副本
#define ACCSTA_ATFB       0x0010 // 是否在副本
#define ACCSTA_COMPLETED  0x0200 // 已经没有项链刷了
#define ACCSTA_OFFLINE    0x0100 // 帐号下线

#define MAX_ACCOUNT_LOGIN 5      // 最多支持多少账号在线

struct my_msg2 {
	char id[32];
	char text[64];
	char cla[32];
	int  value;
	int  value2;
};

typedef struct _account_
{
	char   Name[32];       // 帐号
	char   Password[32];   // 密码
	char   Role[16];       // 角色名称
	char   SerBig[32];     // 游戏大区
	char   SerSmall[32];   // 游戏小区
	int    RoleNo;
	int    XL;             // 项链数量 
	int    IsGetXL;        // 是否已经获取了项链
	int    IsReady;        // 是否已准备
	int    IsBig;
	int    OfflineLogin;   // 是否掉线重连
	float  Scale;
	HWND   Wnd;
	RECT   Rect;
	int    Status;
	char   StatusStr[16];
	SOCKET Socket;
	int    LoginTime;     // 登录时间
	int    PlayTime;      // 进入时间
	int    AtFBTime;      // 进入副本时间
	int    LastTime;      // 最后通信时间
	int    Flag;          // 
	int    Index;
} Account;

using namespace std;

class Sqlite;
class JsCall;
class WebList;
class Home;
class Driver;
class GameServer;

class Game
{
public:
	// ...
	Game(JsCall* p, char* account_ctrl_id, char* log_ctrl_id, const char* conf_path, HWND hWnd);
	// ...
	~Game();

	// 监听
	void Listen(USHORT port);
	// 登录
	void Login(Account* p);
	// 进入到登录界面
	void GoLoginUI(int left, int top);
	// 输入帐号密码登录
	void Input(Account* p);
	// 自动登号
	bool AutoLogin();
	// 设置登号类型
	void SetLoginFlag(int flag);
	// 获取需要登录的数量
	int GetLoginCount();
	// 检查登录超时的帐号
	int CheckLoginTimeOut();
	// 设置状态-全部
	void SetAllStatus(int status);
	// 设置准备
	void SetReady(Account* p, int v);
	// 设置状态
	void SetStatus(Account* p, int status, bool update_text=false);
	// 设置SOKCET
	void SetSocket(Account* p, SOCKET s);
	// 设置Flag
	void SetFlag(Account* p, int flag);
	// 设置已完成
	void SetCompleted(Account* p);
	// 获取帐号总数
	int GetAccountCount();
	// 获取正在刷副本帐号数量
	int GetAtFBCount();
	// 获取所有在线账号数量
	int GetOnLineCount();
	// 帐号获取
	Account* GetAccount(const char* name);
	// 获取帐号
	Account* GetAccount(int index);
	// 获取帐号[根据状态]
	Account* GetAccountByStatus(int status);
	// 获取最多项链帐号
	Account* GetMaxXLAccount(Account** last=nullptr);
	// 获取准备登录的帐号
	Account* GetReadyAccount();
	// 获取下一个要登录的帐号
	Account* GetNextLoginAccount();
	// 获取帐号
	Account* GetAccountBySocket(SOCKET s);
	// 获取大号帐号
	Account* GetBigAccount();
	// 获取大号SOCKET
	SOCKET   GetBigSocket();
	// 设置已经入队
	void SetInTeam(int index);
	// 关闭游戏
	void CloseGame(int index);
	// 设置已准备好帐号数量
	void SetReadyCount(int v);
	// 设置已准备好帐号数量
	int AddReadyCount(int add = 1);
	// 对比帐号状态
	bool CheckStatus(Account* p, int status);
	// 是否自动登录
	bool IsAutoLogin();
	// 是否已全部准备好
	bool IsAllReady();
	// 帐号是否在登录
	bool IsLogin(Account* p);
	// 帐号是否在线
	bool IsOnline(Account* p);
	// 获取状态字符
	char* GetStatusStr(Account* p);

	// 发送卡住重启时间
	int SendQiaZhuS(int second);
	// 发送给大号
	int SendToBig(SOCKET_OPCODE opcode, bool clear=false);
	// 发送给大号
	int SendToBig(SOCKET_OPCODE opcode, int v, bool clear = true);

	// 检查数据库
	void CheckDB();
	// 更新物品信息
	void UpdateDBItem(const char* account, const char* item_name);
	// 更新刷副本次数
	void UpdateDBFB(int count=-1);

	// 更新刷副本次数文本
	void UpdateFBCountText(int lx, bool add=true);
	// 更新掉线复活次数文本
	void UpdateOffLineAllText(int offline, int reborn);

	// 写入日记
	void WriteLog(const char* log);

	// 读取配置文件
	DWORD ReadConf();
	// 读取帐号设置
	bool  ReadAccountSetting(const char* data);
	// 读取其它设置
	void  ReadSetting(const char* data);

	// 自动关机
	void AutoShutDown();
	// 定时关机
	bool ClockShutDown(int flag=0);
	// 关机
	void ShutDown();
	// 当前时间是否在此时间
	bool IsInTime(int s_hour, int s_minute, int e_hour, int e_minute);

	// 设置
	void PutSetting(wchar_t* name, int v);
	// 设置
	void PutSetting(wchar_t* name, wchar_t* v);
	// 打开游戏
	void OpenGame(int index);
	// 喊话
	void Talk(wchar_t* text, int type);
	// 注入DLL
	void InstallDll();
	// 自动登号
	void AutoPlay(int index, bool stop);
	// 添加帐号
	void AddAccount(Account* account);
	// 验证卡号
	void VerifyCard(wchar_t* card);
	// 更新程序版本
	void UpdateVer();

	// 更新帐号状态
	void UpdateAccountStatus(Account * account);
	// 更新帐号在线时长
	void UpdateAccountPlayTime(Account * account);
	// 输入日记到UI界面
	void AddUILog(char text[], char cla[], bool ui = false);
	// 更新
	void UpdateText(int row, int col, char text[]);
public:
	// 设置
	struct
	{
		char GamePath[32];   // 游戏路径
		char GameFile[32];   // 游戏客户端文件
		char SerBig[32];     // 游戏大区
		char SerSmall[32];   // 游戏小区
		int  LoginTimeOut;   // 登录超时允许时间
		int  TimeOut;        // 游戏超时时间
		int  ReConnect;      // 是否断线重连
		int  AutoLoginNext;  // 是否自动登录帐号
		int  LogoutByGetXL;  // 领完项链是否自动退出
		int  ShutDownNoXL;   // 刷完没有项链自动关机

		int  ShutDown_SH;    // 定时关机[开始小时]
		int  ShutDown_SM;    // 定时关机[开始分钟]
		int  ShutDown_EH;    // 定时关机[结束小时]
		int  ShutDown_EM;    // 定时关机[结束分钟]

		int  OffLine_SH;     // 定时关机[开始小时]
		int  OffLine_SM;     // 定时关机[开始分钟]
		int  OffLine_EH;     // 定时关机[结束小时]
		int  OffLine_EM;     // 定时关机[结束分钟]

		int  AutoLogin_SH;   // 定时登录[开始小时]
		int  AutoLogin_SM;   // 定时登录[开始分钟]
		int  AutoLogin_EH;   // 定时登录[结束小时]
		int  AutoLogin_EM;   // 定时登录[结束分钟]

		int  NoAutoSelect;   // 不自动选择游戏区
		int  TalkOpen;       // 是否自动喊话    
		int  IsDebug;        // 是否显示调试信息
		
	} m_Setting;

	// 自身
	Game* m_pGame;
	// 数据库
	Sqlite* m_pSqlite;

	// JsCall
	JsCall*  m_pJsCall;
	// 帐号列表控件
	WebList* m_pAccoutCtrl;
	// 日记列表控件
	WebList* m_pLogCtrl;
	// Home
	Home* m_pHome;
	// Driver
	Driver* m_pDriver;

	// UI窗口
	HWND m_hUIWnd;
	// 更新版本线程
	LPTHREAD_START_ROUTINE m_funcUpdateVer;
	// 消息
	my_msg2 m_Msg;

	float m_fScale;

	// 监听游戏信息
	GameServer* m_pServer;

	// 帐号列表
	std::vector<Account*> m_AccountList;

	// 登号类型 -2停止登 -1全部 其他账号列表索引
	int m_iLoginFlag = -2;

	// 当前登号索引
	int m_iLoginIndex = 0;
	// 需要登录帐号数量
	int m_iLoginCount = 0;
	// 已经准备好帐号数量
	int m_iReadyCount = 0;
	// 上次开启副本帐号索引
	int m_iOpenFBIndex = -1;

	char m_chPath[255];
	ofstream m_LogFile;

	bool m_bLock = false;
};