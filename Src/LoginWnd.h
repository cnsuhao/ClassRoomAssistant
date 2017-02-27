#pragma once
#include "expresshead.h"
#include <exception>

#define  WM_LOGIN_OK WM_USER+100

class Logan
{
public:
	static std::string parse_msg_node(std::string document)throw(std::exception);
	static std::string parse_url_node(std::string document)throw(std::exception);
	static std::string login(std::string ip, std::string user, std::string pwd)throw(std::exception);
	static bool	logout(std::string ip, std::string user);
	static std::string query_url(std::string requestUrl, std::string ip);
	static std::string query_msg_node(std::string requestUrl, std::string ip);
	static bool upload(std::string upLoadUrl, std::string ip, std::string local_path);
	static bool download(std::string upLoadUrl, std::string local_path);
private:
};


class LoginWnd :public WindowImplBase
{
	DUI_DECLARE_MESSAGE_MAP()
public:
	LoginWnd();
	~LoginWnd();
public:
	LPCTSTR GetWindowClassName()const;
	CDuiString GetSkinFile();
	CDuiString GetSkinFolder();
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam);
	CControlUI* CreateControl(LPCTSTR pstrClass);
private:
	bool login_error;
protected:
	void OnClick(TNotifyUI& msg);
	void OnReturn(TNotifyUI& msg);
	void OnKillFocus(TNotifyUI& msg);
	void OnSelectedChange(TNotifyUI& msg);
private:
	void Init();
	void LoadLocalData();
	void StartLogin();
	void LoginError(const char* msg);
	void LoginSuccess();
	bool LoginCheck();
private:
	COptionUI *m_pOptionRem,*m_pOptionAuto;
	CEditUI	  *m_pEditIP, *m_pEditUser, *m_pEditPasswd;
	CTabLayoutUI *m_pTabSwitch;
	CLabelUI	*m_pLabTip,*m_pLabImage;
	void init_self();

	HANDLE m_pthread;
	friend DWORD WINAPI LoginProc(_In_ LPVOID paramer);
};


