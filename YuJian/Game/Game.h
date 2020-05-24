#include "GameServer.h"
#include <Windows.h>
#include <vector>
#include <fstream>

#define P2DW(v) (*(DWORD*)(v))       // ת��DWORD��ֵ
#define P2INT(v) (*(int*)(v))        // ת��int��ֵ

#define LOG(v) m_pGame->AddUILog(v,nullptr)
#define LOGVARP(p,...) { sprintf_s(p,__VA_ARGS__);LOG(p); }
#define LOGVARN(n,...) {char _s[n]; _s[n-1]=0; LOGVARP(_s,__VA_ARGS__); }

#define LOG2(v,cla) m_pGame->AddUILog(v,cla)
#define LOGVARP2(p,cla,...) { sprintf_s(p,__VA_ARGS__);LOG2(p,cla); }
#define LOGVARN2(n,cla,...) {char _s[n]; _s[n-1]=0; LOGVARP2(_s,cla,__VA_ARGS__); }

#define MAKESPOS(v) (int(v/m_fScale))

#define ACCSTA_INIT       0x1000 // �ʺŻ�δ��¼
#define ACCSTA_READY      0x0001 // ׼����¼
#define ACCSTA_LOGIN      0x0002 // ���ڵ�¼
#define ACCSTA_ONLINE     0x0004 // �ʺ�����
#define ACCSTA_OPENFB     0x0008 // ���ڿ�������
#define ACCSTA_ATFB       0x0010 // �Ƿ��ڸ���
#define ACCSTA_COMPLETED  0x0200 // �Ѿ�û������ˢ��
#define ACCSTA_OFFLINE    0x0100 // �ʺ�����

#define MAX_ACCOUNT_LOGIN 5      // ���֧�ֶ����˺�����

struct my_msg2 {
	char id[32];
	char text[64];
	char cla[32];
	int  value;
	int  value2;
};

typedef struct _account_
{
	char   Name[32];       // �ʺ�
	char   Password[32];   // ����
	char   Role[16];       // ��ɫ����
	char   SerBig[32];     // ��Ϸ����
	char   SerSmall[32];   // ��ϷС��
	int    RoleNo;
	int    XL;             // �������� 
	int    IsGetXL;        // �Ƿ��Ѿ���ȡ������
	int    IsReady;        // �Ƿ���׼��
	int    IsBig;
	int    OfflineLogin;   // �Ƿ��������
	float  Scale;
	HWND   Wnd;
	RECT   Rect;
	int    Status;
	char   StatusStr[16];
	SOCKET Socket;
	int    LoginTime;     // ��¼ʱ��
	int    PlayTime;      // ����ʱ��
	int    AtFBTime;      // ���븱��ʱ��
	int    LastTime;      // ���ͨ��ʱ��
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

	// ����
	void Listen(USHORT port);
	// ��¼
	void Login(Account* p);
	// ���뵽��¼����
	void GoLoginUI(int left, int top);
	// �����ʺ������¼
	void Input(Account* p);
	// �Զ��Ǻ�
	bool AutoLogin();
	// ���õǺ�����
	void SetLoginFlag(int flag);
	// ��ȡ��Ҫ��¼������
	int GetLoginCount();
	// ����¼��ʱ���ʺ�
	int CheckLoginTimeOut();
	// ����״̬-ȫ��
	void SetAllStatus(int status);
	// ����׼��
	void SetReady(Account* p, int v);
	// ����״̬
	void SetStatus(Account* p, int status, bool update_text=false);
	// ����SOKCET
	void SetSocket(Account* p, SOCKET s);
	// ����Flag
	void SetFlag(Account* p, int flag);
	// ���������
	void SetCompleted(Account* p);
	// ��ȡ�ʺ�����
	int GetAccountCount();
	// ��ȡ����ˢ�����ʺ�����
	int GetAtFBCount();
	// ��ȡ���������˺�����
	int GetOnLineCount();
	// �ʺŻ�ȡ
	Account* GetAccount(const char* name);
	// ��ȡ�ʺ�
	Account* GetAccount(int index);
	// ��ȡ�ʺ�[����״̬]
	Account* GetAccountByStatus(int status);
	// ��ȡ��������ʺ�
	Account* GetMaxXLAccount(Account** last=nullptr);
	// ��ȡ׼����¼���ʺ�
	Account* GetReadyAccount();
	// ��ȡ��һ��Ҫ��¼���ʺ�
	Account* GetNextLoginAccount();
	// ��ȡ�ʺ�
	Account* GetAccountBySocket(SOCKET s);
	// ��ȡ����ʺ�
	Account* GetBigAccount();
	// ��ȡ���SOCKET
	SOCKET   GetBigSocket();
	// �����Ѿ����
	void SetInTeam(int index);
	// �ر���Ϸ
	void CloseGame(int index);
	// ������׼�����ʺ�����
	void SetReadyCount(int v);
	// ������׼�����ʺ�����
	int AddReadyCount(int add = 1);
	// �Ա��ʺ�״̬
	bool CheckStatus(Account* p, int status);
	// �Ƿ��Զ���¼
	bool IsAutoLogin();
	// �Ƿ���ȫ��׼����
	bool IsAllReady();
	// �ʺ��Ƿ��ڵ�¼
	bool IsLogin(Account* p);
	// �ʺ��Ƿ�����
	bool IsOnline(Account* p);
	// ��ȡ״̬�ַ�
	char* GetStatusStr(Account* p);

	// ���Ϳ�ס����ʱ��
	int SendQiaZhuS(int second);
	// ���͸����
	int SendToBig(SOCKET_OPCODE opcode, bool clear=false);
	// ���͸����
	int SendToBig(SOCKET_OPCODE opcode, int v, bool clear = true);

	// ������ݿ�
	void CheckDB();
	// ������Ʒ��Ϣ
	void UpdateDBItem(const char* account, const char* item_name);
	// ����ˢ��������
	void UpdateDBFB(int count=-1);

	// ����ˢ���������ı�
	void UpdateFBCountText(int lx, bool add=true);
	// ���µ��߸�������ı�
	void UpdateOffLineAllText(int offline, int reborn);

	// д���ռ�
	void WriteLog(const char* log);

	// ��ȡ�����ļ�
	DWORD ReadConf();
	// ��ȡ�ʺ�����
	bool  ReadAccountSetting(const char* data);
	// ��ȡ��������
	void  ReadSetting(const char* data);

	// �Զ��ػ�
	void AutoShutDown();
	// ��ʱ�ػ�
	bool ClockShutDown(int flag=0);
	// �ػ�
	void ShutDown();
	// ��ǰʱ���Ƿ��ڴ�ʱ��
	bool IsInTime(int s_hour, int s_minute, int e_hour, int e_minute);

	// ����
	void PutSetting(wchar_t* name, int v);
	// ����
	void PutSetting(wchar_t* name, wchar_t* v);
	// ����Ϸ
	void OpenGame(int index);
	// ����
	void Talk(wchar_t* text, int type);
	// ע��DLL
	void InstallDll();
	// �Զ��Ǻ�
	void AutoPlay(int index, bool stop);
	// ����ʺ�
	void AddAccount(Account* account);
	// ��֤����
	void VerifyCard(wchar_t* card);
	// ���³���汾
	void UpdateVer();

	// �����ʺ�״̬
	void UpdateAccountStatus(Account * account);
	// �����ʺ�����ʱ��
	void UpdateAccountPlayTime(Account * account);
	// �����ռǵ�UI����
	void AddUILog(char text[], char cla[], bool ui = false);
	// ����
	void UpdateText(int row, int col, char text[]);
public:
	// ����
	struct
	{
		char GamePath[32];   // ��Ϸ·��
		char GameFile[32];   // ��Ϸ�ͻ����ļ�
		char SerBig[32];     // ��Ϸ����
		char SerSmall[32];   // ��ϷС��
		int  LoginTimeOut;   // ��¼��ʱ����ʱ��
		int  TimeOut;        // ��Ϸ��ʱʱ��
		int  ReConnect;      // �Ƿ��������
		int  AutoLoginNext;  // �Ƿ��Զ���¼�ʺ�
		int  LogoutByGetXL;  // ���������Ƿ��Զ��˳�
		int  ShutDownNoXL;   // ˢ��û�������Զ��ػ�

		int  ShutDown_SH;    // ��ʱ�ػ�[��ʼСʱ]
		int  ShutDown_SM;    // ��ʱ�ػ�[��ʼ����]
		int  ShutDown_EH;    // ��ʱ�ػ�[����Сʱ]
		int  ShutDown_EM;    // ��ʱ�ػ�[��������]

		int  OffLine_SH;     // ��ʱ�ػ�[��ʼСʱ]
		int  OffLine_SM;     // ��ʱ�ػ�[��ʼ����]
		int  OffLine_EH;     // ��ʱ�ػ�[����Сʱ]
		int  OffLine_EM;     // ��ʱ�ػ�[��������]

		int  AutoLogin_SH;   // ��ʱ��¼[��ʼСʱ]
		int  AutoLogin_SM;   // ��ʱ��¼[��ʼ����]
		int  AutoLogin_EH;   // ��ʱ��¼[����Сʱ]
		int  AutoLogin_EM;   // ��ʱ��¼[��������]

		int  NoAutoSelect;   // ���Զ�ѡ����Ϸ��
		int  TalkOpen;       // �Ƿ��Զ�����    
		int  IsDebug;        // �Ƿ���ʾ������Ϣ
		
	} m_Setting;

	// ����
	Game* m_pGame;
	// ���ݿ�
	Sqlite* m_pSqlite;

	// JsCall
	JsCall*  m_pJsCall;
	// �ʺ��б�ؼ�
	WebList* m_pAccoutCtrl;
	// �ռ��б�ؼ�
	WebList* m_pLogCtrl;
	// Home
	Home* m_pHome;
	// Driver
	Driver* m_pDriver;

	// UI����
	HWND m_hUIWnd;
	// ���°汾�߳�
	LPTHREAD_START_ROUTINE m_funcUpdateVer;
	// ��Ϣ
	my_msg2 m_Msg;

	float m_fScale;

	// ������Ϸ��Ϣ
	GameServer* m_pServer;

	// �ʺ��б�
	std::vector<Account*> m_AccountList;

	// �Ǻ����� -2ֹͣ�� -1ȫ�� �����˺��б�����
	int m_iLoginFlag = -2;

	// ��ǰ�Ǻ�����
	int m_iLoginIndex = 0;
	// ��Ҫ��¼�ʺ�����
	int m_iLoginCount = 0;
	// �Ѿ�׼�����ʺ�����
	int m_iReadyCount = 0;
	// �ϴο��������ʺ�����
	int m_iOpenFBIndex = -1;

	char m_chPath[255];
	ofstream m_LogFile;

	bool m_bLock = false;
};