#include <time.h>
#include "NotifyWnd.h"

#define  TIMER_ID_LOGIN	1000




NotifyWnd::NotifyWnd(std::string msg, std::string title) :_msg(msg), _title(title)
{

}


NotifyWnd::~NotifyWnd()
{
}

LPCTSTR NotifyWnd::GetWindowClassName()const
{
	return _T("NotifyWnd");
}


CDuiString NotifyWnd::GetSkinFile()
{
	return _T("NotifyView.xml");
}

void NotifyWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender->GetName() == _T("btn_ok"))
		{
			this->Close(IDOK);
		}
		else if (msg.pSender->GetName() == _T("btn_cancel"))
		{
			this->Close(IDCANCEL);
		}
	}
}


CDuiString NotifyWnd::GetSkinFolder()
{
	return _T("Skin");
}

void NotifyWnd::Init()
{
	CLabelUI *labtip = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_tip")));
	labtip->SetText(_msg.c_str());
	CLabelUI *labtitle = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_title")));
	labtitle->SetText(_title.c_str());
	CButtonUI*btn_ok = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_ok")));
	CButtonUI*btn_cancel = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_cancel")));
	lab_time = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("time_out")));
}

LRESULT NotifyWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER)
		return OnTimer(uMsg, wParam, lParam);
	else
		return __super::HandleMessage(uMsg, wParam, lParam);
}

void NotifyWnd::timeClose(unsigned int utime)
{
	current_second = utime/1000;
	if (!lab_time->IsVisible())
	{
		char msg[20];
		sprintf(msg,"%d S",current_second);
		lab_time->SetText(msg);
		lab_time->SetVisible(true);
	}
	SetTimer(*this, ID_TIME_TIMEOUT, utime, NULL);
	SetTimer(*this, ID_TIME_ACCOUT_SECOND, 998, NULL);
}

LRESULT NotifyWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER)
	{
		if (wParam == ID_TIME_TIMEOUT)
		{
			KillTimer(*this,ID_TIME_TIMEOUT);
			Close(IDCANCEL);
		}
		else if (wParam == ID_TIME_ACCOUT_SECOND)
		{
			--current_second;
			if (current_second >= 0)
			{
				char msg[20];
				sprintf(msg, "%d S", current_second);
				lab_time->SetText(msg);
				lab_time->Invalidate();
			}
			else
			{
				KillTimer(*this, ID_TIME_ACCOUT_SECOND);
			}
		}
	}
	return 0;
}

int TipMsg::ShowMsgWindow(HWND parentHwnd, std::string msg, std::string title)
{
	NotifyWnd* pwnd = new NotifyWnd(msg,title);
	pwnd->Create(parentHwnd, _T("Tip"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	pwnd->CenterWindow();
	return pwnd->ShowModal();
}

int TipMsg::ShowMsgWindowTime(HWND parentHwnd, unsigned int utime, std::string msg, std::string title)
{
	NotifyWnd* pwnd = new NotifyWnd(msg, title);
	pwnd->Create(parentHwnd, _T("Tip"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	pwnd->timeClose(utime);
	pwnd->CenterWindow();
	return pwnd->ShowModal();
}