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

// 监听
void Game::Listen(USHORT port)
{
	m_pServer->Listen(port);
}

// 登录
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

// 进入到登录界面
void Game::GoLoginUI(int left, int top)
{
	if (!IsAutoLogin())
		return;

	// [esp + 08] != 0464 && [esp + 08] != 0200 && [esp + 08] != 0201
	// [esp + 08] != 0738 && [esp + 08] != 0282
	// [esp+0x08]!=0x281&&[esp+0x08]!=0x282&&[esp+0x08]!=0x400&&[esp+0x08]!=0x87&&[esp+0x08]!=0x111
	// [esp+0x08]!=0xD6&&[esp+0x08]!=0x36E&&[esp+0x08]!=0xC5&&[esp+0x08]!=0x30&&[esp+0x08]!=0xCF&&[esp+0x08]!=0xB1&&[esp+0x08]!=0xB7&&[esp+0x08]!=0xB0
	LOGVARN(16, "选择区服");
	// 506,550 500,507 515,395 
	Sleep(1500);
	Drv_LeftClick(MAKESPOS(left + 506), MAKESPOS(top + 550));  // 进入正式版
	printf("点击:%d.%d\n", left + 506, top + 550);
	Sleep(1500);
	Drv_LeftClick(MAKESPOS(left + 500), MAKESPOS(top + 507));  // 选择大区
	printf("点击:%d.%d\n", left + 500, top + 507);
	Sleep(1500);
	Drv_LeftClick(MAKESPOS(left + 515), MAKESPOS(top + 355));  // 选择小区
	printf("点击:%d.%d\n", left + 515, top + 355);
	
}

// 输入帐号密码登录
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
	LOGVARP(log, "输入帐号: %s", p->Name);
	Sleep(1500);
	Drv_LeftClick(MAKESPOS(p->Rect.left + 300), MAKESPOS(p->Rect.top + 265));  // 帐号输入框
	Sleep(1000);
	Drv_Input(del,sizeof(del)); // 清空帐号
	Sleep(100);
	Drv_Input(p->Name, strlen(p->Name)); // 输入帐号
#endif

	LOGVARP(log, "输入密码");
	Sleep(1000);
	Drv_LeftClick(MAKESPOS(p->Rect.left + 300), MAKESPOS(p->Rect.top + 305));  // 密码输入框
	Sleep(1000);
	Drv_Input(p->Password, strlen(p->Password)); // 输入密码
	Sleep(500);

	LOGVARP(log, "进入游戏");
	Drv_LeftClick(MAKESPOS(p->Rect.left + 265), MAKESPOS(p->Rect.top + 430));  // 进入
	//Drv_Input(VK_RETURN);
}

// 自动登号
bool Game::AutoLogin()
{
	char log[64];
	if (!IsAutoLogin())
		return false;

	if (m_iLoginCount == 0) { // 没有要登录的帐号
		LOGVARP(log, "没有要登录的帐号\n");
		return false;
	}
		
	if (m_iLoginFlag >= 0 && m_iLoginIndex != m_iLoginFlag) { // 只登一个账号
		LOGVARP(log, "只登一个账号\n");
		return false;
	}

	int login_count = GetOnLineCount();
	if (login_count >= MAX_ACCOUNT_LOGIN) {
		printf("已登录游戏账号数量:%d\n", GetOnLineCount());
		LOGVARP(log, "已登录游戏账号数量:%d\n", GetOnLineCount());
		return false;
	}	
	
	Account* p = GetAccount(m_iLoginIndex);
	if (!p) // 不存在
		return false;
	if (IsLogin(p) || (p->IsBig && m_Setting.LogoutByGetXL)) { // 已在登录或已登录 或大号不领项链
		m_iLoginIndex++;
		return AutoLogin(); // 登录下一个
	}

	p->LoginTime = time(nullptr);
	SetStatus(p, ACCSTA_READY);
	UpdateAccountStatus(p);
	m_iLoginIndex++;
	LOGVARN(64, "启动游戏, 准备登录%s[%d/%d]", p->Name, login_count + 1, m_iLoginCount);

	printf("%s%s\n", m_Setting.GamePath, m_Setting.GameFile);
	ShellExecuteA(NULL, "open", m_Setting.GameFile, NULL, m_Setting.GamePath, SW_SHOWNORMAL);
	return true;
}

// 设置登号类型
void Game::SetLoginFlag(int flag)
{
	m_iLoginFlag = flag;
	m_iLoginIndex = flag >= 0 ? flag : 0;
	m_iLoginCount = GetLoginCount();
	if (flag != -2) {
		LOGVARN(32, "需要登录帐号数量:%d\n", m_iLoginCount);
	}
}

// 获取需要登录的数量
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
			if (max == count) // 登录数量达到最大
				break;
			count++;
		}
			
	}
	return count;
}

// 检查登录超时的帐号
int Game::CheckLoginTimeOut()
{
	// 定时关机
	if (ClockShutDown(1))
		return 0;
	// 定时下线
	if (IsInTime(m_Setting.OffLine_SH, m_Setting.OffLine_SM, m_Setting.OffLine_EH, m_Setting.OffLine_EM)) {
		if (GetOnLineCount() > 0) {
			m_pServer->SendToOther(0, SCK_CLOSE, true);
			return 0;
		}
	}
	// 定时登录
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
				LOGVARN(64, "%s 登录超时", p->Name);
				m_pServer->Send(p->Socket, SCK_CLOSE, true);
				count++;
			}
		}
		if (p->PlayTime && p->Status != ACCSTA_COMPLETED) {
			UpdateAccountPlayTime(p);
			Sleep(500);
		}
		if (0 && IsOnline(p) && p->LastTime) { // 在线
			m_pServer->Send(p->Socket, SCK_PING, true);
			if (m_Setting.TimeOut && (now_time - p->LastTime) > m_Setting.TimeOut) {
				LOGVARN(64, "%s 已经%d秒没有响应", p->Name, now_time - p->LastTime);
				m_pServer->Send(p->Socket, SCK_CLOSE);
			}
		}
	}
	return count;
}

// 设置状态-全部
void Game::SetAllStatus(int status)
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		Account* p = m_AccountList[i];
		p->Status = status;
		GetStatusStr(p);
		UpdateAccountStatus(p);
	}
}

// 设置准备
void Game::SetReady(Account * p, int v)
{
	p->IsReady = v;
}

// 设置状态
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

// 设置SOKCET
void Game::SetSocket(Account * p, SOCKET s)
{
	p->Socket = s;
}

// 设置Flag
void Game::SetFlag(Account * p, int flag)
{
	p->Flag = flag;
}

// 设置已完成
void Game::SetCompleted(Account* p)
{
	LOGVARN2(64, "green", "%s[%s] 已经完成", p->Name, p->Role);

	SetStatus(p, ACCSTA_COMPLETED);
	UpdateAccountStatus(p);
}

// 获取帐号总数
int Game::GetAccountCount()
{
	return m_AccountList.size();
}

// 获取正在刷副本帐号数量
int Game::GetAtFBCount()
{
	int count = 0;
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->Status == ACCSTA_ATFB)
			count++;
	}
	return count;
}

// 获取所有在线账号数量
int Game::GetOnLineCount()
{
	int count = 0;
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (IsOnline(m_AccountList[i]))
			count++;
	}
	return count;
}

// 帐号是否已在列表
Account* Game::GetAccount(const char* name)
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (strcmp(name, m_AccountList[i]->Name) == 0)
			return m_AccountList[i];
	}
	return nullptr;
}

// 获取帐号
Account * Game::GetAccount(int index)
{
	return index < m_AccountList.size() ? m_AccountList[index] : nullptr;
}

// 获取帐号[根据状态]
Account * Game::GetAccountByStatus(int status)
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->Status & status) {
			LOGVARN(64, "获取帐号:%s[%08X|%08X]", m_AccountList[i]->Name, m_AccountList[i]->Status, status);
			return m_AccountList[i];
		}
	}
	return nullptr;
}

// 获取最多项链帐号 last=最后比较的帐号
Account* Game::GetMaxXLAccount(Account** last)
{
	if (last) *last = nullptr;

	Account* p = nullptr;
	int value = 0;
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (!IsLogin(m_AccountList[i])) // 没有在线
			continue;

		if (i != m_iOpenFBIndex && m_AccountList[i]->XL > value) { // 上次开启的优先级排在后面
			p = m_AccountList[i];
			value = p->XL;
			printf("%s有项链%d\n", p->Name, p->XL);
		}
		if (last && !m_AccountList[i]->IsBig) // 保存最后比较帐号
			*last = m_AccountList[i];
	}
	if (!p && m_iOpenFBIndex > -1) { // 没有找到, 尝试上一个的
		p = m_AccountList[m_iOpenFBIndex];
		if (p && p->XL == 0) // 上一个的没有项链了
			p = nullptr;
	}
	if (p) {
		m_iOpenFBIndex = p->Index; // 保存开启副本的帐号索引
	}
	return p;
}

// 获取正在登录的帐号
Account* Game::GetReadyAccount()
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->Status == ACCSTA_READY)
			return m_AccountList[i];
	}
	return nullptr;
}

// 获取下一个要登录的帐号
Account* Game::GetNextLoginAccount()
{
	if (!m_Setting.AutoLoginNext)
		return nullptr;

	printf("GetAccountByStatus(ACCSTA_INIT | ACCSTA_OFFLINE)\n");
	Account* p = GetAccountByStatus(ACCSTA_INIT | ACCSTA_OFFLINE);
	printf("GetAccountByStatus(ACCSTA_INIT | ACCSTA_OFFLINE) 完成\n");
	if (!p) {
		printf("for (int i = 0; i < m_AccountList.size(); i++) \n");
		for (int i = 0; i < m_AccountList.size(); i++) {
			if (CheckStatus(m_AccountList[i], ACCSTA_COMPLETED)) { // 完成的
				tm t;
				time_t play_time = m_AccountList[i]->PlayTime;
				gmtime_s(&t, &play_time);

				tm t2;
				time_t now_time = time(nullptr);
				gmtime_s(&t2, &now_time);

				if (t.tm_mday != t2.tm_mday) { // 帐号是昨天或更早登录的, 可以再次登录
					printf(" 更新帐号状态%d %d %d,%d\n", t.tm_mday, t2.tm_mday, (int)play_time, (int)now_time);
					p = m_AccountList[i];
					SetStatus(p, ACCSTA_INIT); // 更新帐号状态
					break;
				}
			}
		}
		printf("for (int i = 0; i < m_AccountList.size(); i++)  完成\n");
	}

	return p;
}

// 获取帐号
Account* Game::GetAccountBySocket(SOCKET s)
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->Socket == s)
			return m_AccountList[i];
	}
	return nullptr;
}

// 获取大号帐号
Account* Game::GetBigAccount()
{
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->IsBig)
			return m_AccountList[i];
	}
	return nullptr;
}

// 获取大号SOCKET
SOCKET Game::GetBigSocket()
{
	Account* p = GetBigAccount();
	return p ? p->Socket : 0;
}

// 设置已经入队
void Game::SetInTeam(int index)
{
	Account* p = GetAccount(index);
	if (p) {
		m_pServer->CanInFB(p, nullptr, 0);
	}
}

// 关闭游戏
void Game::CloseGame(int index)
{
	Account* p = GetAccount(index);
	if (p) {
		printf("发送关闭:%s\n", p->Name);
		p->OfflineLogin = 0;
		m_pServer->Send(p->Socket, SCK_CLOSE, true);
	}
}

// 设置已准备好帐号数量
void Game::SetReadyCount(int v)
{
	m_iReadyCount = v;
}

// 设置已准备好帐号数量
int Game::AddReadyCount(int add)
{
	m_iReadyCount += add;
	return m_iReadyCount;
}

// 对比帐号状态
bool Game::CheckStatus(Account* p, int status)
{
	return p ? (p->Status & status) : false;
}

// 是否自动登录
bool Game::IsAutoLogin()
{
	return m_iLoginFlag != -2;
}

// 是否已全部准备好
bool Game::IsAllReady()
{
	int count = 0;
	for (int i = 0; i < m_AccountList.size(); i++) {
		if (m_AccountList[i]->IsReady)
			count++;
	}

	return count >= 5 || count >= m_AccountList.size();
}

// 帐号是否在登录
bool Game::IsLogin(Account * p)
{
	return CheckStatus(p, 0xff);
}

// 帐号是否在线
bool Game::IsOnline(Account * p)
{
	return CheckStatus(p, 0xff);
}

// 获取状态字符
char* Game::GetStatusStr(Account* p)
{
	if (p->Status == ACCSTA_INIT)
		return strcpy(p->StatusStr, "未登录");
	if (p->Status == ACCSTA_READY)
		return strcpy(p->StatusStr, "正在打开游戏...");
	if (p->Status == ACCSTA_LOGIN)
		return strcpy(p->StatusStr, "正在登录...");
	if (p->Status == ACCSTA_ONLINE)
		return strcpy(p->StatusStr, "在线");
	if (p->Status == ACCSTA_OPENFB)
		return strcpy(p->StatusStr, "正在开启副本...");
	if (p->Status == ACCSTA_ATFB)
		return strcpy(p->StatusStr, "正在刷副本...");
	if (p->Status == ACCSTA_COMPLETED)
		return strcpy(p->StatusStr, "已完成");
	if (p->Status == ACCSTA_OFFLINE)
		return strcpy(p->StatusStr, "离线");
	return p->Name;
}

// 发送卡住重启时间
int Game::SendQiaZhuS(int second)
{
	if (second > 0) {
		return SendToBig(SCK_QIAZHUS, second, true);
	}
	return 0;
}

// 发送给大号
int Game::SendToBig(SOCKET_OPCODE opcode, bool clear)
{
	return m_pServer->Send(GetBigSocket(), opcode, clear);
}

// 发送给大号
int Game::SendToBig(SOCKET_OPCODE opcode, int v, bool clear)
{
	if (clear)
		m_pServer->m_Server.ClearSendString();

	m_pServer->m_Server.SetInt(v);
	return SendToBig(opcode, false);
}

// 检查数据库
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

// 更新物品信息
void Game::UpdateDBItem(const char* account, const char* item_name)
{
	int now_time = time(nullptr);

	char sql[255];
	sprintf_s(sql, "INSERT INTO items(account,name,created_at) VALUES ('%s','%s',%d)",
		account, item_name, now_time);
	m_pSqlite->Exec(sql);

	sprintf_s(sql, "SELECT num FROM item_count WHERE name='%s'", item_name);
	if (m_pSqlite->GetRowCount(sql) == 0) { // 没有
		printf("增加\n");
		sprintf_s(sql, "INSERT INTO item_count(name,num,created_at,updated_at) VALUES ('%s',%d,%d,%d)", 
			item_name, 1, now_time, now_time);
		m_pSqlite->Exec(sql);
	}
	else { // 更新
		printf("更新\n");
		sprintf_s(sql, "UPDATE item_count SET num=num+1,updated_at=%d WHERE name='%s'",
			now_time, item_name);
		m_pSqlite->Exec(sql);
	}
}

// 更新刷副本次数
void Game::UpdateDBFB(int count)
{
	if (m_pSqlite->GetRowCount("SELECT num FROM fb_count WHERE id=1") == 0) { // 没有
		printf("增加\n");
		m_pSqlite->Exec("INSERT INTO fb_count(id,num) VALUES (1,1)");
	}
	else { // 更新
		printf("更新\n");
		m_pSqlite->Exec("UPDATE fb_count SET num=num+1 WHERE id=1");
	}
}

// 更新刷副本次数文本
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

// 更新掉线复活次数文本
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

// 写入日记
void Game::WriteLog(const char* log)
{
	//m_LogFile << log << endl;
}

// 读取配置文件
DWORD Game::ReadConf()
{
	char path[255], logfile[255];

	strcpy(path, m_chPath);
	strcpy(logfile, m_chPath);
	strcat(path, "\\帐号.txt");
	strcat(logfile, "\\日记.txt");

	//m_LogFile.open(logfile);
	WriteLog("第一行");

	printf("帐号文件:%s\n", path);
	OpenTextFile file;
	if (!file.Open(path)) {
		printf("找不到'帐号.txt'文件！！！");
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
		if (strstr(data, "::") == nullptr && strstr(data, "=")) { // 其它配置
			if (!ReadAccountSetting(data)) {
				ReadSetting(data);
			}
			
			continue;
		}

		// 帐号列表
		Explode explode("::", data);
		if (explode.GetCount() < 2)
			continue;

		Account* p = GetAccount(explode[0]);
		if (!p) { // 不存在
			p = new Account;
			strcpy(p->Name, explode[0]);
			strcpy(p->Password, explode[1]);
			strcpy(p->Role,    "--");
			ZeroMemory(p->SerBig, sizeof(p->SerBig));
			ZeroMemory(p->SerSmall, sizeof(p->SerSmall));
			p->RoleNo = explode.GetValue2Int(2);
			p->IsGetXL = 0;
			p->IsReady = 0;
			p->IsBig = explode.GetCount() > 3 && strcmp(explode[3], "大号") == 0;
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

// 读取帐号设置
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
		LOGVARN2(64, "cb", "区服.%s: %s|%s", explode[0], arr[0], arr[1]);
	}

	return true;
}

// 读取其它设置
void Game::ReadSetting(const char* data)
{
	char tmp[16] = { 0 };
	Explode explode("=", data);
	if (strcmp(explode[0], "免费使用") == 0) {
		if (strcmp(explode[1], "无限期+永久") == 0)
			m_pHome->SetFree(true);
	}
	if (strcmp(explode[0], "游戏路径") == 0) {
		strcpy(m_Setting.GamePath, explode[1]);
	}
	if (strcmp(explode[0], "游戏文件") == 0) {
		strcpy(m_Setting.GameFile, explode[1]);
	}
	if (strcmp(explode[0], "游戏大区") == 0) {
		strcpy(m_Setting.SerBig, explode[1]);
	}
	if (strcmp(explode[0], "游戏小区") == 0) {
		strcpy(m_Setting.SerSmall, explode[1]);
	}
	if (strcmp(explode[0], "登录超时") == 0) {
		m_Setting.LoginTimeOut = explode.GetValue2Int(1);
		strcpy(tmp, "秒");
		m_pJsCall->SetSetting("login_timeout", m_Setting.LoginTimeOut);
	}
	if (strcmp(explode[0], "游戏超时") == 0) {
		m_Setting.TimeOut = explode.GetValue2Int(1);
		strcpy(tmp, "秒");
	}
	if (strcmp(explode[0], "断线重连") == 0) {
		m_Setting.ReConnect = strcmp("是", explode[1]) == 0;
	}
	if (strcmp(explode[0], "自动切换帐号") == 0) {
		m_Setting.AutoLoginNext = strcmp("是", explode[1]) == 0;
	}
	if (strcmp(explode[0], "刷完关机") == 0) {
		m_Setting.ShutDownNoXL = strcmp("是", explode[1]) == 0;
		//m_pJsCall->SetSetting("shuawan_shutdown", m_Setting.ShutDownNoXL);
	}
	if (strcmp(explode[0], "定时关机") == 0) {
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
	if (strcmp(explode[0], "定时下线") == 0) {
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
	if (strcmp(explode[0], "定时登录") == 0) {
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

	LOGVARN2(64, "cb", "配置.%s: %s%s", explode[0], explode[1], tmp);
}

// 自动关机
void Game::AutoShutDown()
{
	if (m_Setting.ShutDownNoXL)
		ShutDown();
}

// 定时关机
bool Game::ClockShutDown(int flag)
{
	if (IsInTime(m_Setting.ShutDown_SH, m_Setting.ShutDown_SM, m_Setting.ShutDown_EH, m_Setting.ShutDown_EM)) {
		ShutDown();
		return true;
	}
	return false;
}

// 关机
void Game::ShutDown()
{
	system("shutdown -s -t 10");
}

// 当前时间是否在此时间
bool Game::IsInTime(int s_hour, int s_minute, int e_hour, int e_minute)
{
	SYSTEMTIME stLocal;
	::GetLocalTime(&stLocal); // 获得当前时间
	if (s_hour == e_hour) { // 时间是同一小时
		return stLocal.wHour == s_hour
			&& stLocal.wMinute >= s_minute && stLocal.wMinute <= e_minute;
	}
	if (s_hour < e_hour) { // 不是同一小时
		if (stLocal.wHour == s_hour) // 在开始小时时间
			return stLocal.wMinute >= s_minute;
		if (stLocal.wHour == e_hour) // 在结束小时时间
			return stLocal.wMinute <= e_minute;
		if (stLocal.wHour > s_hour && stLocal.wHour < e_hour) // 在中间
			return true;
	}
	return false;
}

// 设置
void Game::PutSetting(wchar_t* name, int v)
{
	if (wcscmp(name, L"login_timeout") == 0) {
		m_Setting.LoginTimeOut = v;
		LOGVARN2(64, "cb", "修改.登录超时:%d秒.", v);
	}
	else if (wcscmp(name, L"qiazhu") == 0) {
		SendQiaZhuS(v);
		LOGVARN2(64, "cb", "修改.卡住重启:%d秒.", v);
	}
	else if (wcscmp(name, L"shuawan_shutdown") == 0) {
		m_Setting.ShutDownNoXL = v;
		LOGVARN2(64, "cb", "修改.刷完关机:%s.", v ? "是" : "否");
	}
	/******** 定时关机 ********/
	else if (wcscmp(name, L"shutdown_sh") == 0) {
		m_Setting.ShutDown_SH = v;
		LOGVARN2(64, "cb", "修改.定时关机(开始):%d点.", v);
	}
	else if (wcscmp(name, L"shutdown_sm") == 0) {
		m_Setting.ShutDown_SM = v;
		LOGVARN2(64, "cb", "修改.定时关机(开始):%d分钟.", v);
	}
	else if (wcscmp(name, L"shutdown_eh") == 0) {
		m_Setting.ShutDown_EH = v;
		LOGVARN2(64, "cb", "修改.定时关机(结束):%d点.", v);
	}
	else if (wcscmp(name, L"shutdown_em") == 0) {
		m_Setting.ShutDown_EM = v;
		LOGVARN2(64, "cb", "修改.定时关机(结束):%d分钟.", v);
	}
	/******** 定时下线 ********/
	else if (wcscmp(name, L"offline_sh") == 0) {
		m_Setting.OffLine_SH = v;
		LOGVARN2(64, "cb", "修改.定时下线(开始):%d点.", v);
	}
	else if (wcscmp(name, L"offline_sm") == 0) {
		m_Setting.OffLine_SM = v;
		LOGVARN2(64, "cb", "修改.定时下线(开始):%d分钟.", v);
	}
	else if (wcscmp(name, L"offline_eh") == 0) {
		m_Setting.OffLine_EH = v;
		LOGVARN2(64, "cb", "修改.定时下线(结束):%d点.", v);
	}
	else if (wcscmp(name, L"offline_em") == 0) {
		m_Setting.OffLine_EM = v;
		LOGVARN2(64, "cb", "修改.定时下线(结束):%d分钟.", v);
	}
	/******** 定时登录 ********/
	else if (wcscmp(name, L"autologin_sh") == 0) {
		m_Setting.AutoLogin_SH = v;
		LOGVARN2(64, "cb", "修改.定时登录(开始):%d点.", v);
	}
	else if (wcscmp(name, L"autologin_sm") == 0) {
		m_Setting.AutoLogin_SM = v;
		LOGVARN2(64, "cb", "修改.定时登录(开始):%d分钟.", v);
	}
	else if (wcscmp(name, L"autologin_eh") == 0) {
		m_Setting.AutoLogin_EH = v;
		LOGVARN2(64, "cb", "修改.定时登录(结束):%d点.", v);
	}
	else if (wcscmp(name, L"autologin_em") == 0) {
		m_Setting.AutoLogin_EM = v;
		LOGVARN2(64, "cb", "修改.定时登录(结束):%d分钟.", v);
	}

	else if (wcscmp(name, L"no_auto_select") == 0) {
		m_Setting.NoAutoSelect = v;
		LOGVARN2(64, "cb", "修改.手动选区:%s.", v ? "是" : "否");
	}
	else if (wcscmp(name, L"is_debug") == 0) {
		m_Setting.IsDebug = v;
		LOGVARN2(64, "cb", "修改.调试信息:%s.", v ? "显示" : "不显示");
	}
	else if (wcscmp(name, L"talk_open") == 0) {
		m_Setting.TalkOpen = v;
		LOGVARN2(64, "cb", "修改.喊话开启:%s.", v ? "是" : "否");
	}
}

// 设置
void Game::PutSetting(wchar_t* name, wchar_t* v)
{
	if (!v || wcslen(v) == 0)
		return;

	char* value = wchar2char(v);
	printf("喊话:%s\n", value);
	delete value;
}

// 打开游戏
void Game::OpenGame(int index)
{
	if (GetOnLineCount() >= MAX_ACCOUNT_LOGIN) {
		::MessageBox(NULL, L"目前只支持5个账号，敬请期待！！！", L"信息", MB_OK);
		LOGVARN2(64, "red", "目前只支持5个账号，敬请期待！！！");
		return;
	}

	AutoPlay(index, false);
}

// 喊话
// 打开游戏
void Game::Talk(wchar_t* text, int type)
{
	char* value = wchar2char(text);
	//printf("喊话:%s %d\n", value, type);

	m_pServer->m_Server.ClearSendString();
	m_pServer->m_Server.SetInt(type);
	m_pServer->m_Server.SetInt(strlen(value));
	m_pServer->m_Server.SetContent((void*)value, strlen(value));
	m_pServer->SendToOther(0, SCK_TALK, false);

	delete value;
}

// 注入DLL
void Game::InstallDll()
{
	if (!m_pHome->IsValid() && !m_pDriver->m_bIsInstallDll) {
		m_pJsCall->ShowMsg("无法启动, 请先激活.", "提示", 2);
		return;
	}
	m_pJsCall->SetBtnDisabled("start_btn", 1);
	m_pDriver->InstallDll();
	m_pJsCall->SetText("start_btn", m_pDriver->m_bIsInstallDll ? "停止" : "启动");
	m_pJsCall->SetBtnDisabled("start_btn", 0);

	if (!m_pDriver->m_bIsInstallDll) {
		BOOL bWow64 = TRUE;
		IsWow64Process(GetCurrentProcess(), &bWow64);
		if (!bWow64) {
			m_pJsCall->ShowMsg("本程序不支持32位系统, 请升级成成64位系统.", "系统版本为32位", 2);
		}
	}
}

// 自动登号
void Game::AutoPlay(int index, bool stop)
{
	m_pJsCall->SetBtnDisabled("start_btn", 1);
	if (stop) {
		SetLoginFlag(-2);
		m_pJsCall->SetText("auto_play_btn", "自动登号");
		m_pJsCall->SetBtnDisabled("start_btn", 0);
		return;
	}
	if (!m_pDriver->m_bIsInstallDll)
		InstallDll();
	if (m_pDriver->m_bIsInstallDll) {
		m_pJsCall->SetText("auto_play_btn", "停止登号");
		SetLoginFlag(index);
		if (!AutoLogin())
			LOGVARN2(64, "blue", "已全部登入游戏");
	}
	else {
		m_pJsCall->SetBtnDisabled("start_btn", 0);
	}
} 

// 添加帐号
void Game::AddAccount(Account * account)
{
	std::string name = "<b class='c3'>";
	name += account->Name;
	if (account->IsBig) {
		name += "[大号]";
	}
	name += "</b>";

	m_pAccoutCtrl->AddRow(account->Index + 1, "--");
	m_pAccoutCtrl->SetText(account->Index, 1, (char*)name.c_str());
	m_pAccoutCtrl->SetText(account->Index, 2, account->StatusStr);
	m_pAccoutCtrl->SetClass(account->Index, -1, "c6", 1);
}

// 验证卡号
void Game::VerifyCard(wchar_t * card)
{
	char* value = wchar2char(card);
	printf("卡号:%s\n", value);

	if (m_pHome->Recharge(value)) {
		m_pJsCall->SetText("card_date", (char*)m_pHome->GetExpireTime_S().c_str());
		m_pJsCall->UpdateStatusText((char*)m_pHome->GetMsgStr().c_str(), 2);
		::MessageBoxA(NULL, m_pHome->GetMsgStr().c_str(), "信息", MB_OK);
	}
	else {
		m_pJsCall->UpdateStatusText((char*)m_pHome->GetMsgStr().c_str(), 3);
		::MessageBoxA(NULL, m_pHome->GetMsgStr().c_str(), "信息", MB_OK);
	}
	delete value;
}

// 更新程序版本
void Game::UpdateVer()
{
	CreateThread(NULL, NULL, m_funcUpdateVer, NULL, NULL, NULL);
}

// 更新帐号状态
void Game::UpdateAccountStatus(Account * account)
{
	UpdateText(account->Index, 2, account->StatusStr);
	if (account->LastTime) {
		char time_str[64];
		time2str(time_str, account->LastTime, 8);
		UpdateText(account->Index, 4, time_str);
	}
}

// 更新帐号在线时长
void Game::UpdateAccountPlayTime(Account * account)
{
	if (!account->PlayTime)
		return;

	char cm[64];
	int c = time(nullptr) - account->PlayTime;
	if (c > 86400) {
		sprintf_s(cm, "%d天%02d小时%02d分", c / 86400, (c % 86400) / 3600, (c % 3600) / 60);
	}
	else {
		sprintf_s(cm, "%d小时%02d分%02d秒", (c % 86400) / 3600, (c % 3600) / 60, c % 60);
	}
	UpdateText(account->Index, 3, cm);
}

// 输入日记到UI界面
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

// 更新
void Game::UpdateText(int row, int col, char text[])
{
	my_msg2* msg = new my_msg2;

	ZeroMemory(msg, sizeof(m_Msg));
	strcpy(msg->text, text);
	msg->value = row;
	msg->value2 = col;
	PostMessage(m_hUIWnd, WM_USER + 103, (WPARAM)msg, 1);
}
