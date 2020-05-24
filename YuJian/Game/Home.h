#pragma once

#include "HttpClient.h"
#include <string>

#define HOME_HOST      L"fz.myhostcpp.com"
#define HOME_GAME_FLAG "2"

class Home
{
public:
	Home();
	// �����Ƿ����
	void SetFree(bool v);
	// �Ƿ���Ч
	bool IsValid();
	// ��ֵ
	bool Recharge(const char* card);
	// ��֤
	bool Verify();
	// ��ô˻�����Чʱ�䣨�룩
	int  GetExpire();
	// �������ؽ��
	void Parse();
	// ��÷��ؽ���е�ֵ
	bool GetValue(char* key, char value[], int length);
	// ��÷��ؽ���е�ֵ
	int  GetValue(char* key, int length, int default_value=0);
	// ��÷��ؽ���е�ֵ
	int  GetValue(const char* key, int length, int default_value = 0);
	// ������֤ʱ��
	int  SetVerifyTime();
	// ���õ���ʱ��
	int  SetEndTime();
	// ���õ���ʱ���ַ���
	void SetExpireTime_S();
	// ��õ���ʱ���ַ���
	std::string& GetExpireTime_S();
	void SetError(int code, const char* str);
	void SetError(int code, wchar_t* str);
	void SetError(int code, CString str);
	void SetError(int code, const char* str, DWORD status);
	void SetError(int code, wchar_t* str, DWORD status);
	void SetErrorCode(int code);
	void SetMsgStr(wchar_t* str);
	void SetMsgStr(CString str);
	void SetMsgStr(wchar_t* str, DWORD status);
	void SetMsgStr(const char* str, DWORD status);
	std::string& GetMsgStr();
protected:
	// ���ؽ��ָ��
	char* m_pRepsone;
	// ��֤ʱ�䣨�룩
	int   m_iVerifyTime;
	// ����ʱ��
	int m_iExpire;
	// ����ʱ��
	int   m_iEndTime;

	char m_MachineId[33];
	int  m_Status;
	int  m_Error;

	// �Ƿ����
	bool m_bFree = false;

	// ��վ��������
	std::string m_sResult;
	// ��ʾ����
	std::string m_MsgStr;
	// ����ʱ��
	std::string m_sExpireTime;
};