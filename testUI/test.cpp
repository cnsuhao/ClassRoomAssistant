#include <time.h>
#include "test.h"

#define  TIMER_ID_LOGIN	1000




testWnd::testWnd() 
{

}


testWnd::~testWnd()
{

}

LPCTSTR testWnd::GetWindowClassName()const
{
	return _T("testWnd");
}


CDuiString testWnd::GetSkinFile()
{
	return _T("login.xml");
}

void testWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender->GetName() == _T("btn_login"))
		{
			CAnimationTabLayoutUI* anitab = static_cast<CAnimationTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_lay")));
			anitab->SelectItem(1);
		}
		OnSetting(msg);
		OnOk(msg);
		OnCancel(msg);
	}
}


CDuiString testWnd::GetSkinFolder()
{
	return _T("Skin");
}

void testWnd::Init()
{
	m_PBtn_Ok = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_ok")));
	m_pBtn_Cannel = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_cancel")));
	m_PBtn_Setting = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_setting")));
	m_pTab = static_cast<CAnimationTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_lay")));
	m_PEdit_User = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_user")));
	m_PEdit_Passwd = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_passwd")));
	m_PEdit_Client = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_client")));
	m_PEdit_Server = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_server")));

}

LRESULT testWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
		return __super::HandleMessage(uMsg, wParam, lParam);
}


void testWnd::OnSetting(TNotifyUI& msg)
{
	if (msg.pSender == m_PBtn_Setting)
	{
		if (m_PBtn_Setting && m_pTab)
		{
			m_pTab->SelectItem(3);
		}
	}
}

void testWnd::OnOk(TNotifyUI& msg)
{
	if (msg.pSender == m_PBtn_Ok)
	{
		if (m_PBtn_Ok && m_pTab)
		{
			m_pTab->SelectItem(0);
		}
	}
}

void testWnd::OnCancel(TNotifyUI& msg)
{
	if (msg.pSender == m_pBtn_Cannel)
	{
		if (m_pBtn_Cannel && m_pTab)
		{
			m_pTab->SelectItem(0);
		}
	}
}