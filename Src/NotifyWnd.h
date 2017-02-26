#pragma once
#include "../Src/expresshead.h"

#define ID_TIME_TIMEOUT	1314

#define ID_TIME_ACCOUT_SECOND	1315

class NotifyWnd :public WindowImplBase
{
public:
	NotifyWnd(std::string msg,std::string title);
	~NotifyWnd();
public:
	LPCTSTR GetWindowClassName()const;
	CDuiString GetSkinFile();
	CDuiString GetSkinFolder();
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void timeClose(unsigned int utime);
protected:
	void Init();
	std::string _msg, _title;
	CLabelUI *lab_time;
	void Notify(TNotifyUI& msg);
	int current_second;
};

class TipMsg
{
public:
	static int ShowMsgWindow(HWND parentHwnd,std::string msg="提示信息",std::string title="提示");
	static int ShowMsgWindowTime(HWND parentHwnd, unsigned int utime=1000, std::string msg = "提示信息", std::string title = "提示");
};
