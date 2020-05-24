#include "../Web/JsCall.h"
#include "../Web/WebList.h"
#include "Game.h"
#include "Driver.h"
#include "Home.h"
#include <ShlObj_core.h>
#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <My/Common/mystring.h>
#include <My/Common/OpenTextFile.h>
#include <My/Common/Explode.h>
#include <My/Driver/KbdMou.h>
#include <My/Db/Sqlite.h>


// ...
Game::Game(JsCall * p, char* account_ctrl_id, char* log_ctrl_id, const char* conf_path, HWND hWnd)
{
	m_pGame = this;
	m_hUIWnd = hWnd;
	strcpy(m_chPath, conf_path);

	char db_file[255];
	strcpy(db_file, m_chPath);
	strcat(db_file, "\\data\\data.db");
	m_pSqlite = new Sqlite(db_file);

	m_iOpenFBIndex = -1;

	m_pJsCall = p;
	m_pAccoutCtrl = new WebList;
	m_pAccoutCtrl->Init(m_pJsCall, "table_1");
	m_pLogCtrl = new WebList;
	m_pLogCtrl->Init(m_pJsCall, "log_ul");

	m_pHome = new Home;
	m_pDriver = new Driver(this);
	m_pServer = new GameServer(this);
	m_pServer->SetSelf(m_pServer);

	CheckDB();

	ZeroMemory(&m_Setting, sizeof(m_Setting));
}

// ...
Game::~Game()
{
	if (m_pSqlite)
		m_pSqlite->Close();

	delete m_pSqlite;
	m_pSqlite = nullptr;
}

// ����
void Game::Listen(USHORT port)
{
	m_pServer->Listen(port);
}

// ��¼
void Game::Login(Account * p)
{
	if (!IsAutoLogin() || !p)
		return;

	WM_COMMAND;
	SetStatus(p, ACCSTA_LOGIN);
	UpdateAccountStatus(p);
	return;
	// [esp + 0C] != 012FB && [esp + 08] != 0200 && [esp + 08] != 0201
	GoLoginUI(p->Rect.left, p->Rect.top);
	Input(p);
}

// ���뵽��¼����
void Game::GoLoginUI(int left, int top)
{
	if (!IsAutoLogin())
		return;

	// [esp + 08] != 0464 && [esp + 08] != 0200 && [esp + 08] != 0201
	// [esp + 08] != 0738 && [esp + 08] != 0282
	// [esp+0x08]!=0x281&&[esp+0x08]!=0x282&&[esp+0x08]!=0x400&&[esp+0x08]!=0x87&&[esp+0x08]!=0x111
	// [esp+0x08]!=0xD6&&[esp+0x08]!=0x36E&&[esp+0x08]!=0xC5&&[esp+0x08]!=0x30&&[esp+0x08]!=0xCF&&[esp+0x08]!=0xB1&&[esp+0x08]!=0xB7&&[esp+0x08]!=0xB0
	LOGVARN(16, "ѡ������");
	// 506,550 500,507 515,395 
	Sleep(1500);
	Drv_LeftClick(MAKESPOS(left + 506), MAKESPOS(top + 550));  // ������ʽ��
	printf("���:%d.%d\n", left + 506, top + 550);
	Sleep(1500);
	Drv_LeftClick(MAKESPOS(left + 500), MAKESPOS(top + 507));  // ѡ�����
	printf("���:%d.%d\n", left + 500, top + 507);
	Sleep(1500);
	Drv_LeftClick(MAKESPOS(left + 515), MAKESPOS(top + 355));  // ѡ��С��
	printf("���:%d.%d\n", left + 515, top + 355);
	
}

// �����ʺ������¼
void Game::Input(Account* p)
{
	if (!IsAutoLogin())
		return;

	char del[15];
	for (int i = 0; i < sizeof(del); i++) {
		del[i] = VK_BACK;
	}
	// 300,265 300,305

	char log[32];
#if 1
	LOGVARP(log, "�����ʺ�: %s", p->Name);
	Sleep(1500);
	Drv_LeftClick(MAKESPOS(p->Rect.left + 300), MAKESPOS(p->Rect.top + 265));  // �ʺ������
	Sleep(1000);
	Drv_Input(del,sizeof(del)); // ����ʺ�
	Sleep(100);
	Drv_Input(p->Name, strlen(p->Name)); // �����ʺ�
#endif

	LOGVARP(log, "��������");
	Sleep(1000);
	Drv_LeftClick(MAKESPOS(p->Rect.left + 300), MAKESPOS(p->Rect.top + 305));  // ���������
	Sleep(1000);
	Drv_Input(p->Password, strlen(p->Password)); // ��������
	Sleep(500);

	LOGVARP(log, "������Ϸ");
	Drv_LeftClick(MAKESPOS(p->Rect.left + 265), MAKESPOS(p->Rect.top + 430));  // ����
	//Drv_Input(VK_RETURN);
}

// �Զ��Ǻ�
bool Game::AutoLogin()
{
	char log[64];
	if (!IsAutoLogin())
		return false;

	if (m_iLoginCount == 0) { // û��Ҫ��¼���ʺ�
		LOGVARP(log, "û��Ҫ��¼���ʺ�\n");
		return false;
	}
		
	if (m_iLoginFlag >= 0 && m_iLoginIndex != m_iLoginFlag) { // ֻ��һ���˺�
		LOGVARP(log, "ֻ��һ���˺�\n");
		return false;
	}

	int login_count = GetOnLineCount();
	if (login_count >= MAX_ACCOUNT_LOGIN) {
		printf("�ѵ�¼��Ϸ�˺�����:%d\n", GetOnLineCount());
		LOGVARP(log, "�ѵ�¼��Ϸ�˺�����:%d\n", GetOnLineCount());
		return false;
	}	
	
	Account* p = GetAccount(m_iLoginIndex);
	if (!p) // ������
		return false;
	if (IsLogin(p) || (p->IsBig && m_Setting.LogoutByGetXL)) { // ���ڵ�¼���ѵ�¼ ���Ų�������
		m_iLoginIndex++;
		return AutoLogin(); // ��¼��һ��
	}

	p->LoginTime = time(nullptr);
	SetStatus(p, ACCSTA_READY);
	UpdateAccountStatus(p);
	m_iLoginIndex++;
	LOGVARN(64, "������Ϸ, ׼����¼%s[%d/%d]", p->Name, login_count + 1, m_iLoginCount);

	printf("%s%s\n", m_Setting.GamePath, m_Setting.GameFile);
	ShellExecuteA(NULL, "open", m_Setting.GameFile, NULL, m_Setting.GamePath, SW_SHOWNORMAL);
	return true;
}

// ���õǺ�����
void Game::SetLoginFlag(int flag)
{
	m_iLoginFlag = flag;
	m_iLoginIndex = flag >= 0 ? flag : 0;
	m_iLoginCount = GetLoginCount();
	if (flag != -2) {
		LOGVARN(32, "��Ҫ��¼�ʺ�����:%d\n", m_iLoginCount);
	}
}

// ��ȡ��Ҫ��¼������
int Game::GetLoginCount()
{
	if (m_iLoginFlag >= 0)
		return !IsLogin(m_AccountList[m_iLoginFlag]) ? 1 : 0;

	int max = 5;
	int count = 0;
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (IsLogin(m_AccountList[i])) {
			max--;
		}
		else {
			if (max == count) // ��¼�����ﵽ���
				break;
			count++;
		}
			
	}
	return count;
}

// ����¼��ʱ���ʺ�
int Game::CheckLoginTimeOut()
{
	// ��ʱ�ػ�
	if (ClockShutDown(1))
		return 0;
	// ��ʱ����
	if (IsInTime(m_Setting.OffLine_SH, m_Setting.OffLine_SM, m_Setting.OffLine_EH, m_Setting.OffLine_EM)) {
		if (GetOnLineCount() > 0) {
			m_pServer->SendToOther(0, SCK_CLOSE, true);
			return 0;
		}
	}
	// ��ʱ��¼
	if (IsInTime(m_Setting.AutoLogin_SH, m_Setting.AutoLogin_SM, m_Setting.AutoLogin_EH, m_Setting.AutoLogin_EM)) {
		if (GetOnLineCount() < MAX_ACCOUNT_LOGIN) {
			AutoPlay(-1, false);
			return 0;
		}
	}

	int count = 0, now_time = time(nullptr);
	for (int i = 0; i < m_AccountList.size(); i++) {
		Account* p = m_AccountList[i];
		if (p->LoginTime && 
			CheckStatus(p, ACCSTA_READY | ACCSTA_LOGIN)) {
			if ((now_time - p->LoginTime) > m_Setting.LoginTimeOut) {
				LOGVARN(64, "%s ��¼��ʱ", p->Name);
				m_pServer->Send(p->Socket, SCK_CLOSE, true);
				count++;
			}
		}
		if (p->PlayTime && p->Status != ACCSTA_COMPLETED) {
			UpdateAccountPlayTime(p);
			Sleep(500);
		}
		if (0 && IsOnline(p) && p->LastTime) { // ����
			m_pServer->Send(p->Socket, SCK_PING, true);
			if (m_Setting.TimeOut && (now_time - p->LastTime) > m_Setting.TimeOut) {
				LOGVARN(64, "%s �Ѿ�%d��û����Ӧ", p->Name, now_time - p->LastTime);
				m_pServer->Send(p->Socket, SCK_CLOSE);
			}
		}
	}
	return count;
}

// ����״̬-ȫ��
void Game::SetAllStatus(int status)
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		Account* p = m_AccountList[i];
		p->Status = status;
		GetStatusStr(p);
		UpdateAccountStatus(p);
	}
}

// ����׼��
void Game::SetReady(Account * p, int v)
{
	p->IsReady = v;
}

// ����״̬
void Game::SetStatus(Account* p, int status, bool update_text)
{
	if (p == nullptr)
		return;

	p->Status = status;
	GetStatusStr(p);
	if (update_text) {
		UpdateAccountStatus(p);
	}
}

// ����SOKCET
void Game::SetSocket(Account * p, SOCKET s)
{
	p->Socket = s;
}

// ����Flag
void Game::SetFlag(Account * p, int flag)
{
	p->Flag = flag;
}

// ���������
void Game::SetCompleted(Account* p)
{
	LOGVARN2(64, "green", "%s[%s] �Ѿ����", p->Name, p->Role);

	SetStatus(p, ACCSTA_COMPLETED);
	UpdateAccountStatus(p);
}

// ��ȡ�ʺ�����
int Game::GetAccountCount()
{
	return m_AccountList.size();
}

// ��ȡ����ˢ�����ʺ�����
int Game::GetAtFBCount()
{
	int count = 0;
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->Status == ACCSTA_ATFB)
			count++;
	}
	return count;
}

// ��ȡ���������˺�����
int Game::GetOnLineCount()
{
	int count = 0;
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (IsOnline(m_AccountList[i]))
			count++;
	}
	return count;
}

// �ʺ��Ƿ������б�
Account* Game::GetAccount(const char* name)
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (strcmp(name, m_AccountList[i]->Name) == 0)
			return m_AccountList[i];
	}
	return nullptr;
}

// ��ȡ�ʺ�
Account * Game::GetAccount(int index)
{
	return index < m_AccountList.size() ? m_AccountList[index] : nullptr;
}

// ��ȡ�ʺ�[����״̬]
Account * Game::GetAccountByStatus(int status)
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->Status & status) {
			LOGVARN(64, "��ȡ�ʺ�:%s[%08X|%08X]", m_AccountList[i]->Name, m_AccountList[i]->Status, status);
			return m_AccountList[i];
		}
	}
	return nullptr;
}

// ��ȡ��������ʺ� last=���Ƚϵ��ʺ�
Account* Game::GetMaxXLAccount(Account** last)
{
	if (last) *last = nullptr;

	Account* p = nullptr;
	int value = 0;
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (!IsLogin(m_AccountList[i])) // û������
			continue;

		if (i != m_iOpenFBIndex && m_AccountList[i]->XL > value) { // �ϴο��������ȼ����ں���
			p = m_AccountList[i];
			value = p->XL;
			printf("%s������%d\n", p->Name, p->XL);
		}
		if (last && !m_AccountList[i]->IsBig) // �������Ƚ��ʺ�
			*last = m_AccountList[i];
	}
	if (!p && m_iOpenFBIndex > -1) { // û���ҵ�, ������һ����
		p = m_AccountList[m_iOpenFBIndex];
		if (p && p->XL == 0) // ��һ����û��������
			p = nullptr;
	}
	if (p) {
		m_iOpenFBIndex = p->Index; // ���濪���������ʺ�����
	}
	return p;
}

// ��ȡ���ڵ�¼���ʺ�
Account* Game::GetReadyAccount()
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->Status == ACCSTA_READY)
			return m_AccountList[i];
	}
	return nullptr;
}

// ��ȡ��һ��Ҫ��¼���ʺ�
Account* Game::GetNextLoginAccount()
{
	if (!m_Setting.AutoLoginNext)
		return nullptr;

	printf("GetAccountByStatus(ACCSTA_INIT | ACCSTA_OFFLINE)\n");
	Account* p = GetAccountByStatus(ACCSTA_INIT | ACCSTA_OFFLINE);
	printf("GetAccountByStatus(ACCSTA_INIT | ACCSTA_OFFLINE) ���\n");
	if (!p) {
		printf("for (int i = 0; i < m_AccountList.size(); i++) \n");
		for (int i = 0; i < m_AccountList.size(); i++) {
			if (CheckStatus(m_AccountList[i], ACCSTA_COMPLETED)) { // ��ɵ�
				tm t;
				time_t play_time = m_AccountList[i]->PlayTime;
				gmtime_s(&t, &play_time);

				tm t2;
				time_t now_time = time(nullptr);
				gmtime_s(&t2, &now_time);

				if (t.tm_mday != t2.tm_mday) { // �ʺ������������¼��, �����ٴε�¼
					printf(" �����ʺ�״̬%d %d %d,%d\n", t.tm_mday, t2.tm_mday, (int)play_time, (int)now_time);
					p = m_AccountList[i];
					SetStatus(p, ACCSTA_INIT); // �����ʺ�״̬
					break;
				}
			}
		}
		printf("for (int i = 0; i < m_AccountList.size(); i++)  ���\n");
	}

	return p;
}

// ��ȡ�ʺ�
Account* Game::GetAccountBySocket(SOCKET s)
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->Socket == s)
			return m_AccountList[i];
	}
	return nullptr;
}

// ��ȡ����ʺ�
Account* Game::GetBigAccount()
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->IsBig)
			return m_AccountList[i];
	}
	return nullptr;
}

// ��ȡ���SOCKET
SOCKET Game::GetBigSocket()
{
	Account* p = GetBigAccount();
	return p ? p->Socket : 0;
}

// �����Ѿ����
void Game::SetInTeam(int index)
{
	Account* p = GetAccount(index);
	if (p) {
		m_pServer->CanInFB(p, nullptr, 0);
	}
}

// �ر���Ϸ
void Game::CloseGame(int index)
{
	Account* p = GetAccount(index);
	if (p) {
		printf("���͹ر�:%s\n", p->Name);
		p->OfflineLogin = 0;
		m_pServer->Send(p->Socket, SCK_CLOSE, true);
	}
}

// ������׼�����ʺ�����
void Game::SetReadyCount(int v)
{
	m_iReadyCount = v;
}

// ������׼�����ʺ�����
int Game::AddReadyCount(int add)
{
	m_iReadyCount += add;
	return m_iReadyCount;
}

// �Ա��ʺ�״̬
bool Game::CheckStatus(Account* p, int status)
{
	return p ? (p->Status & status) : false;
}

// �Ƿ��Զ���¼
bool Game::IsAutoLogin()
{
	return m_iLoginFlag != -2;
}

// �Ƿ���ȫ��׼����
bool Game::IsAllReady()
{
	int count = 0;
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->IsReady)
			count++;
	}

	return count >= 5 || count >= m_AccountList.size();
}

// �ʺ��Ƿ��ڵ�¼
bool Game::IsLogin(Account * p)
{
	return CheckStatus(p, 0xff);
}

// �ʺ��Ƿ�����
bool Game::IsOnline(Account * p)
{
	return CheckStatus(p, 0xff);
}

// ��ȡ״̬�ַ�
char* Game::GetStatusStr(Account* p)
{
	if (p->Status == ACCSTA_INIT)
		return strcpy(p->StatusStr, "δ��¼");
	if (p->Status == ACCSTA_READY)
		return strcpy(p->StatusStr, "���ڴ���Ϸ...");
	if (p->Status == ACCSTA_LOGIN)
		return strcpy(p->StatusStr, "���ڵ�¼...");
	if (p->Status == ACCSTA_ONLINE)
		return strcpy(p->StatusStr, "����");
	if (p->Status == ACCSTA_OPENFB)
		return strcpy(p->StatusStr, "���ڿ�������...");
	if (p->Status == ACCSTA_ATFB)
		return strcpy(p->StatusStr, "����ˢ����...");
	if (p->Status == ACCSTA_COMPLETED)
		return strcpy(p->StatusStr, "�����");
	if (p->Status == ACCSTA_OFFLINE)
		return strcpy(p->StatusStr, "����");
	return p->Name;
}

// ���Ϳ�ס����ʱ��
int Game::SendQiaZhuS(int second)
{
	if (second > 0) {
		return SendToBig(SCK_QIAZHUS, second, true);
	}
	return 0;
}

// ���͸����
int Game::SendToBig(SOCKET_OPCODE opcode, bool clear)
{
	return m_pServer->Send(GetBigSocket(), opcode, clear);
}

// ���͸����
int Game::SendToBig(SOCKET_OPCODE opcode, int v, bool clear)
{
	if (clear)
		m_pServer->m_Server.ClearSendString();

	m_pServer->m_Server.SetInt(v);
	return SendToBig(opcode, false);
}

// ������ݿ�
void Game::CheckDB()
{
	if (!m_pSqlite->TableIsExists("items")) {
		m_pSqlite->Exec("create table items("\
			"id integer primary key autoincrement, "\
			"account varchar(32),"\
			"name varchar(32),"\
			"created_at integer"\
			")");
		m_pSqlite->Exec("create index IF NOT EXISTS idx_name on items(name);");
	}
	if (!m_pSqlite->TableIsExists("item_count")) {
		m_pSqlite->Exec("create table item_count("\
			"id integer primary key autoincrement, "\
			"name varchar(32),"\
			"num integer,"\
			"created_at integer,"\
			"updated_at integer)");
	}
	if (!m_pSqlite->TableIsExists("fb_count")) {
		m_pSqlite->Exec("create table fb_count("\
			"id integer primary key autoincrement, "\
			"num integer)");
	}
}

// ������Ʒ��Ϣ
void Game::UpdateDBItem(const char* account, const char* item_name)
{
	int now_time = time(nullptr);

	char sql[255];
	sprintf_s(sql, "INSERT INTO items(account,name,created_at) VALUES ('%s','%s',%d)",
		account, item_name, now_time);
	m_pSqlite->Exec(sql);

	sprintf_s(sql, "SELECT num FROM item_count WHERE name='%s'", item_name);
	if (m_pSqlite->GetRowCount(sql) == 0) { // û��
		printf("����\n");
		sprintf_s(sql, "INSERT INTO item_count(name,num,created_at,updated_at) VALUES ('%s',%d,%d,%d)", 
			item_name, 1, now_time, now_time);
		m_pSqlite->Exec(sql);
	}
	else { // ����
		printf("����\n");
		sprintf_s(sql, "UPDATE item_count SET num=num+1,updated_at=%d WHERE name='%s'",
			now_time, item_name);
		m_pSqlite->Exec(sql);
	}
}

// ����ˢ��������
void Game::UpdateDBFB(int count)
{
	if (m_pSqlite->GetRowCount("SELECT num FROM fb_count WHERE id=1") == 0) { // û��
		printf("����\n");
		m_pSqlite->Exec("INSERT INTO fb_count(id,num) VALUES (1,1)");
	}
	else { // ����
		printf("����\n");
		m_pSqlite->Exec("UPDATE fb_count SET num=num+1 WHERE id=1");
	}
}

// ����ˢ���������ı�
void Game::UpdateFBCountText(int lx, bool add)
{
	if (add) {
		UpdateDBFB();
	}
	
	char num[16] = { '0', 0 };
	m_pSqlite->GetOneCol("SELECT num FROM fb_count WHERE id=1", num);

	char text[32];
	sprintf_s(text, "%d/%s", lx, num);

	my_msg2* msg = new my_msg2;
	ZeroMemory(msg, sizeof(my_msg2));
	strcpy(msg->id, "fb_count");
	strcpy(msg->text, text);

	PostMessage(m_hUIWnd, WM_USER + 100, (WPARAM)msg, 1);
}

// ���µ��߸�������ı�
void Game::UpdateOffLineAllText(int offline, int reborn)
{
	char text[32];
	sprintf_s(text, "%d/%d", offline, reborn);
	
	my_msg2* msg = new my_msg2;
	ZeroMemory(msg, sizeof(my_msg2));
	strcpy(msg->id, "offline_count");
	strcpy(msg->text, text);

	PostMessage(m_hUIWnd, WM_USER + 100, (WPARAM)msg, 1);
}

// д���ռ�
void Game::WriteLog(const char* log)
{
	//m_LogFile << log << endl;
}

// ��ȡ�����ļ�
DWORD Game::ReadConf()
{
	char path[255], logfile[255];

	strcpy(path, m_chPath);
	strcpy(logfile, m_chPath);
	strcat(path, "\\�ʺ�.txt");
	strcat(logfile, "\\�ռ�.txt");

	//m_LogFile.open(logfile);
	WriteLog("��һ��");

	printf("�ʺ��ļ�:%s\n", path);
	OpenTextFile file;
	if (!file.Open(path)) {
		printf("�Ҳ���'�ʺ�.txt'�ļ�������");
		return false;
	}

	int i = 0, index = 0;
	int length = 0;
	char data[128];
	while ((length = file.GetLine(data, 128)) > -1) {
		//printf("length:%d\n", length);
		if (length == 0) {
			continue;
		}

		trim(data);
		if (strstr(data, "::") == nullptr && strstr(data, "=")) { // ��������
			if (!ReadAccountSetting(data)) {
				ReadSetting(data);
			}
			
			continue;
		}

		// �ʺ��б�
		Explode explode("::", data);
		if (explode.GetCount() < 2)
			continue;

		Account* p = GetAccount(explode[0]);
		if (!p) { // ������
			p = new Account;
			strcpy(p->Name, explode[0]);
			strcpy(p->Password, explode[1]);
			strcpy(p->Role,    "--");
			ZeroMemory(p->SerBig, sizeof(p->SerBig));
			ZeroMemory(p->SerSmall, sizeof(p->SerSmall));
			p->RoleNo = explode.GetValue2Int(2);
			p->IsGetXL = 0;
			p->IsReady = 0;
			p->IsBig = explode.GetCount() > 3 && strcmp(explode[3], "���") == 0;
			p->OfflineLogin = 1;
			p->Status = ACCSTA_INIT;
			GetStatusStr(p);
			p->Socket = 0;
			p->LoginTime = 0;
			p->PlayTime = 0;
			p->AtFBTime = 0;
			p->LastTime = 0;
			p->Index = m_AccountList.size();
			m_AccountList.push_back(p);

			//printf("explode[2]:%s\n", explode[2]);
			AddAccount(p);
		}
	}
	m_pAccoutCtrl->FillRow();
	return 0;
}

// ��ȡ�ʺ�����
bool Game::ReadAccountSetting(const char* data)
{
	Explode explode("=", data);
	Account* p = GetAccount(explode[0]);
	if (!p)
		return false;

	Explode arr("|", explode[1]);
	if (arr.GetCount() == 2) {
		strcpy(p->SerBig, arr[0]);
		strcpy(p->SerSmall, arr[1]);
		LOGVARN2(64, "cb", "����.%s: %s|%s", explode[0], arr[0], arr[1]);
	}

	return true;
}

// ��ȡ��������
void Game::ReadSetting(const char* data)
{
	char tmp[16] = { 0 };
	Explode explode("=", data);
	if (strcmp(explode[0], "���ʹ��") == 0) {
		if (strcmp(explode[1], "������+����") == 0)
			m_pHome->SetFree(true);
	}
	if (strcmp(explode[0], "��Ϸ·��") == 0) {
		strcpy(m_Setting.GamePath, explode[1]);
	}
	if (strcmp(explode[0], "��Ϸ�ļ�") == 0) {
		strcpy(m_Setting.GameFile, explode[1]);
	}
	if (strcmp(explode[0], "��Ϸ����") == 0) {
		strcpy(m_Setting.SerBig, explode[1]);
	}
	if (strcmp(explode[0], "��ϷС��") == 0) {
		strcpy(m_Setting.SerSmall, explode[1]);
	}
	if (strcmp(explode[0], "��¼��ʱ") == 0) {
		m_Setting.LoginTimeOut = explode.GetValue2Int(1);
		strcpy(tmp, "��");
		m_pJsCall->SetSetting("login_timeout", m_Setting.LoginTimeOut);
	}
	if (strcmp(explode[0], "��Ϸ��ʱ") == 0) {
		m_Setting.TimeOut = explode.GetValue2Int(1);
		strcpy(tmp, "��");
	}
	if (strcmp(explode[0], "��������") == 0) {
		m_Setting.ReConnect = strcmp("��", explode[1]) == 0;
	}
	if (strcmp(explode[0], "�Զ��л��ʺ�") == 0) {
		m_Setting.AutoLoginNext = strcmp("��", explode[1]) == 0;
	}
	if (strcmp(explode[0], "ˢ��ػ�") == 0) {
		m_Setting.ShutDownNoXL = strcmp("��", explode[1]) == 0;
		//m_pJsCall->SetSetting("shuawan_shutdown", m_Setting.ShutDownNoXL);
	}
	if (strcmp(explode[0], "��ʱ�ػ�") == 0) {
		Explode t("-", explode[1]);
		if (t[0]) {
			Explode s(":", t[0]);
			m_Setting.ShutDown_SH = s.GetValue2Int(0);
			m_Setting.ShutDown_SM = s.GetValue2Int(1);
		}
		if (t[1]) {
			Explode e(":", t[1]);
			m_Setting.ShutDown_EH = e.GetValue2Int(0);
			m_Setting.ShutDown_EM = e.GetValue2Int(1);
		}

		m_pJsCall->SetSetting("shutdown_sh", m_Setting.ShutDown_SH);
		m_pJsCall->SetSetting("shutdown_sm", m_Setting.ShutDown_SM);
		m_pJsCall->SetSetting("shutdown_eh", m_Setting.ShutDown_EH);
		m_pJsCall->SetSetting("shutdown_em", m_Setting.ShutDown_EM);
	}
	if (strcmp(explode[0], "��ʱ����") == 0) {
		Explode t("-", explode[1]);
		if (t[0]) {
			Explode s(":", t[0]);
			m_Setting.OffLine_SH = s.GetValue2Int(0);
			m_Setting.OffLine_SM = s.GetValue2Int(1);
		}
		if (t[1]) {
			Explode e(":", t[1]);
			m_Setting.OffLine_EH = e.GetValue2Int(0);
			m_Setting.OffLine_EM = e.GetValue2Int(1);
		}

		m_pJsCall->SetSetting("offline_sh", m_Setting.OffLine_SH);
		m_pJsCall->SetSetting("offline_sm", m_Setting.OffLine_SM);
		m_pJsCall->SetSetting("offline_eh", m_Setting.OffLine_EH);
		m_pJsCall->SetSetting("offline_em", m_Setting.OffLine_EM);
	}
	if (strcmp(explode[0], "��ʱ��¼") == 0) {
		Explode t("-", explode[1]);
		if (t[0]) {
			Explode s(":", t[0]);
			m_Setting.AutoLogin_SH = s.GetValue2Int(0);
			m_Setting.AutoLogin_SM = s.GetValue2Int(1);
		}
		if (t[1]) {
			Explode e(":", t[1]);
			m_Setting.AutoLogin_EH = e.GetValue2Int(0);
			m_Setting.AutoLogin_EM = e.GetValue2Int(1);
		}

		m_pJsCall->SetSetting("autologin_sh", m_Setting.AutoLogin_SH);
		m_pJsCall->SetSetting("autologin_sm", m_Setting.AutoLogin_SM);
		m_pJsCall->SetSetting("autologin_eh", m_Setting.AutoLogin_EH);
		m_pJsCall->SetSetting("autologin_em", m_Setting.AutoLogin_EM);
	}

	LOGVARN2(64, "cb", "����.%s: %s%s", explode[0], explode[1], tmp);
}

// �Զ��ػ�
void Game::AutoShutDown()
{
	if (m_Setting.ShutDownNoXL)
		ShutDown();
}

// ��ʱ�ػ�
bool Game::ClockShutDown(int flag)
{
	if (IsInTime(m_Setting.ShutDown_SH, m_Setting.ShutDown_SM, m_Setting.ShutDown_EH, m_Setting.ShutDown_EM)) {
		ShutDown();
		return true;
	}
	return false;
}

// �ػ�
void Game::ShutDown()
{
	system("shutdown -s -t 10");
}

// ��ǰʱ���Ƿ��ڴ�ʱ��
bool Game::IsInTime(int s_hour, int s_minute, int e_hour, int e_minute)
{
	SYSTEMTIME stLocal;
	::GetLocalTime(&stLocal); // ��õ�ǰʱ��
	if (s_hour == e_hour) { // ʱ����ͬһСʱ
		return stLocal.wHour == s_hour
			&& stLocal.wMinute >= s_minute && stLocal.wMinute <= e_minute;
	}
	if (s_hour < e_hour) { // ����ͬһСʱ
		if (stLocal.wHour == s_hour) // �ڿ�ʼСʱʱ��
			return stLocal.wMinute >= s_minute;
		if (stLocal.wHour == e_hour) // �ڽ���Сʱʱ��
			return stLocal.wMinute <= e_minute;
		if (stLocal.wHour > s_hour && stLocal.wHour < e_hour) // ���м�
			return true;
	}
	return false;
}

// ����
void Game::PutSetting(wchar_t* name, int v)
{
	if (wcscmp(name, L"login_timeout") == 0) {
		m_Setting.LoginTimeOut = v;
		LOGVARN2(64, "cb", "�޸�.��¼��ʱ:%d��.", v);
	}
	else if (wcscmp(name, L"qiazhu") == 0) {
		SendQiaZhuS(v);
		LOGVARN2(64, "cb", "�޸�.��ס����:%d��.", v);
	}
	else if (wcscmp(name, L"shuawan_shutdown") == 0) {
		m_Setting.ShutDownNoXL = v;
		LOGVARN2(64, "cb", "�޸�.ˢ��ػ�:%s.", v ? "��" : "��");
	}
	/******** ��ʱ�ػ� ********/
	else if (wcscmp(name, L"shutdown_sh") == 0) {
		m_Setting.ShutDown_SH = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ�ػ�(��ʼ):%d��.", v);
	}
	else if (wcscmp(name, L"shutdown_sm") == 0) {
		m_Setting.ShutDown_SM = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ�ػ�(��ʼ):%d����.", v);
	}
	else if (wcscmp(name, L"shutdown_eh") == 0) {
		m_Setting.ShutDown_EH = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ�ػ�(����):%d��.", v);
	}
	else if (wcscmp(name, L"shutdown_em") == 0) {
		m_Setting.ShutDown_EM = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ�ػ�(����):%d����.", v);
	}
	/******** ��ʱ���� ********/
	else if (wcscmp(name, L"offline_sh") == 0) {
		m_Setting.OffLine_SH = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ����(��ʼ):%d��.", v);
	}
	else if (wcscmp(name, L"offline_sm") == 0) {
		m_Setting.OffLine_SM = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ����(��ʼ):%d����.", v);
	}
	else if (wcscmp(name, L"offline_eh") == 0) {
		m_Setting.OffLine_EH = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ����(����):%d��.", v);
	}
	else if (wcscmp(name, L"offline_em") == 0) {
		m_Setting.OffLine_EM = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ����(����):%d����.", v);
	}
	/******** ��ʱ��¼ ********/
	else if (wcscmp(name, L"autologin_sh") == 0) {
		m_Setting.AutoLogin_SH = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ��¼(��ʼ):%d��.", v);
	}
	else if (wcscmp(name, L"autologin_sm") == 0) {
		m_Setting.AutoLogin_SM = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ��¼(��ʼ):%d����.", v);
	}
	else if (wcscmp(name, L"autologin_eh") == 0) {
		m_Setting.AutoLogin_EH = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ��¼(����):%d��.", v);
	}
	else if (wcscmp(name, L"autologin_em") == 0) {
		m_Setting.AutoLogin_EM = v;
		LOGVARN2(64, "cb", "�޸�.��ʱ��¼(����):%d����.", v);
	}

	else if (wcscmp(name, L"no_auto_select") == 0) {
		m_Setting.NoAutoSelect = v;
		LOGVARN2(64, "cb", "�޸�.�ֶ�ѡ��:%s.", v ? "��" : "��");
	}
	else if (wcscmp(name, L"is_debug") == 0) {
		m_Setting.IsDebug = v;
		LOGVARN2(64, "cb", "�޸�.������Ϣ:%s.", v ? "��ʾ" : "����ʾ");
	}
	else if (wcscmp(name, L"talk_open") == 0) {
		m_Setting.TalkOpen = v;
		LOGVARN2(64, "cb", "�޸�.��������:%s.", v ? "��" : "��");
	}
}

// ����
void Game::PutSetting(wchar_t* name, wchar_t* v)
{
	if (!v || wcslen(v) == 0)
		return;

	char* value = wchar2char(v);
	printf("����:%s\n", value);
	delete value;
}

// ����Ϸ
void Game::OpenGame(int index)
{
	if (GetOnLineCount() >= MAX_ACCOUNT_LOGIN) {
		::MessageBox(NULL, L"Ŀǰֻ֧��5���˺ţ������ڴ�������", L"��Ϣ", MB_OK);
		LOGVARN2(64, "red", "Ŀǰֻ֧��5���˺ţ������ڴ�������");
		return;
	}

	AutoPlay(index, false);
}

// ����
// ����Ϸ
void Game::Talk(wchar_t* text, int type)
{
	char* value = wchar2char(text);
	//printf("����:%s %d\n", value, type);

	m_pServer->m_Server.ClearSendString();
	m_pServer->m_Server.SetInt(type);
	m_pServer->m_Server.SetInt(strlen(value));
	m_pServer->m_Server.SetContent((void*)value, strlen(value));
	m_pServer->SendToOther(0, SCK_TALK, false);

	delete value;
}

// ע��DLL
void Game::InstallDll()
{
	if (!m_pHome->IsValid() && !m_pDriver->m_bIsInstallDll) {
		m_pJsCall->ShowMsg("�޷�����, ���ȼ���.", "��ʾ", 2);
		return;
	}
	m_pJsCall->SetBtnDisabled("start_btn", 1);
	m_pDriver->InstallDll();
	m_pJsCall->SetText("start_btn", m_pDriver->m_bIsInstallDll ? "ֹͣ" : "����");
	m_pJsCall->SetBtnDisabled("start_btn", 0);

	if (!m_pDriver->m_bIsInstallDll) {
		BOOL bWow64 = TRUE;
		IsWow64Process(GetCurrentProcess(), &bWow64);
		if (!bWow64) {
			m_pJsCall->ShowMsg("������֧��32λϵͳ, �������ɳ�64λϵͳ.", "ϵͳ�汾Ϊ32λ", 2);
		}
	}
}

// �Զ��Ǻ�
void Game::AutoPlay(int index, bool stop)
{
	m_pJsCall->SetBtnDisabled("start_btn", 1);
	if (stop) {
		SetLoginFlag(-2);
		m_pJsCall->SetText("auto_play_btn", "�Զ��Ǻ�");
		m_pJsCall->SetBtnDisabled("start_btn", 0);
		return;
	}
	if (!m_pDriver->m_bIsInstallDll)
		InstallDll();
	if (m_pDriver->m_bIsInstallDll) {
		m_pJsCall->SetText("auto_play_btn", "ֹͣ�Ǻ�");
		SetLoginFlag(index);
		if (!AutoLogin())
			LOGVARN2(64, "blue", "��ȫ��������Ϸ");
	}
	else {
		m_pJsCall->SetBtnDisabled("start_btn", 0);
	}
} 

// ����ʺ�
void Game::AddAccount(Account * account)
{
	std::string name = "<b class='c3'>";
	name += account->Name;
	if (account->IsBig) {
		name += "[���]";
	}
	name += "</b>";

	m_pAccoutCtrl->AddRow(account->Index + 1, "--");
	m_pAccoutCtrl->SetText(account->Index, 1, (char*)name.c_str());
	m_pAccoutCtrl->SetText(account->Index, 2, account->StatusStr);
	m_pAccoutCtrl->SetClass(account->Index, -1, "c6", 1);
}

// ��֤����
void Game::VerifyCard(wchar_t * card)
{
	char* value = wchar2char(card);
	printf("����:%s\n", value);

	if (m_pHome->Recharge(value)) {
		m_pJsCall->SetText("card_date", (char*)m_pHome->GetExpireTime_S().c_str());
		m_pJsCall->UpdateStatusText((char*)m_pHome->GetMsgStr().c_str(), 2);
		::MessageBoxA(NULL, m_pHome->GetMsgStr().c_str(), "��Ϣ", MB_OK);
	}
	else {
		m_pJsCall->UpdateStatusText((char*)m_pHome->GetMsgStr().c_str(), 3);
		::MessageBoxA(NULL, m_pHome->GetMsgStr().c_str(), "��Ϣ", MB_OK);
	}
	delete value;
}

// ���³���汾
void Game::UpdateVer()
{
	CreateThread(NULL, NULL, m_funcUpdateVer, NULL, NULL, NULL);
}

// �����ʺ�״̬
void Game::UpdateAccountStatus(Account * account)
{
	UpdateText(account->Index, 2, account->StatusStr);
	if (account->LastTime) {
		char time_str[64];
		time2str(time_str, account->LastTime, 8);
		UpdateText(account->Index, 4, time_str);
	}
}

// �����ʺ�����ʱ��
void Game::UpdateAccountPlayTime(Account * account)
{
	if (!account->PlayTime)
		return;

	char cm[64];
	int c = time(nullptr) - account->PlayTime;
	if (c > 86400) {
		sprintf_s(cm, "%d��%02dСʱ%02d��", c / 86400, (c % 86400) / 3600, (c % 3600) / 60);
	}
	else {
		sprintf_s(cm, "%dСʱ%02d��%02d��", (c % 86400) / 3600, (c % 3600) / 60, c % 60);
	}
	UpdateText(account->Index, 3, cm);
}

// �����ռǵ�UI����
void Game::AddUILog(char text[], char cla[], bool ui)
{
	if (ui) {
		m_pLogCtrl->AddLog(text, cla);
		return;
	}

	my_msg2* msg = new my_msg2;

	ZeroMemory(msg, sizeof(my_msg2));
	strcpy(msg->id, m_pLogCtrl->m_Id);
	strcpy(msg->text, text);
	if (cla)
		strcpy(msg->cla, cla);

	PostMessage(m_hUIWnd, WM_USER + 101, (WPARAM)msg, 1);
}

// ����
void Game::UpdateText(int row, int col, char text[])
{
	my_msg2* msg = new my_msg2;

	ZeroMemory(msg, sizeof(m_Msg));
	strcpy(msg->text, text);
	msg->value = row;
	msg->value2 = col;
	PostMessage(m_hUIWnd, WM_USER + 103, (WPARAM)msg, 1);
}
