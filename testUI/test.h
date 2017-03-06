#pragma once
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include "../Duilib/UIlib.h"
#include <mmsystem.h>
#pragma comment(lib, "WINMM.LIB")
#ifdef _DEBUG
#pragma  comment(lib,"../lib/DuiLib_d.lib")
#else
#pragma  comment(lib,"../lib/DuiLib.lib")
#endif
using namespace DuiLib;
using namespace std;


#define ID_TIME_TIMEOUT	1314

#define ID_TIME_ACCOUT_SECOND	1315

class CRoundPhotoUI :public  CLabelUI
{
public:
	CRoundPhotoUI(){  }
	void PaintBkImage(HDC hDC)
	{
		__super::PaintBkImage(hDC);
		Gdiplus::Graphics graphics(hDC);
		Pen myPen(m_dwBackColor, 2);
		graphics.DrawRectangle(&myPen, m_rcItem.left, m_rcItem.top, GetWidth(), GetHeight());
		int a = (m_rcItem.right - m_rcItem.left) / 2;
		int b = (m_rcItem.bottom - m_rcItem.top) / 2;
		for (int x = 0; x <= a; x++)
		{
			int y = b - b*sqrt(1.0 - (x*x*1.0) / (a*a*1.0));
			graphics.DrawLine(&myPen, m_rcItem.left, y + m_rcItem.top, m_rcItem.left + a - x, y + m_rcItem.top);
			graphics.DrawLine(&myPen, m_rcItem.left + a + x, y + m_rcItem.top, m_rcItem.right, y + m_rcItem.top);
			graphics.DrawLine(&myPen, m_rcItem.left, m_rcItem.bottom - y, m_rcItem.left + a - x, m_rcItem.bottom - y);
			graphics.DrawLine(&myPen, m_rcItem.left + a + x, m_rcItem.bottom - y, m_rcItem.right, m_rcItem.bottom - y);

		}
		CRenderEngine::DrawEllipse(hDC, m_rcItem, 5, m_dwBackColor);
	}
};

class testWnd :public WindowImplBase
{
public:
	testWnd();
	~testWnd();
public:
	LPCTSTR GetWindowClassName()const;
	CDuiString GetSkinFile();
	CDuiString GetSkinFolder();
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	CControlUI* CreateControl(LPCTSTR pstrClass)
	{
		if (_tcscmp(pstrClass, _T("RoundPhoto")) == 0)
			return new CRoundPhotoUI;
		else if (_tcscmp(pstrClass, _T("AnimationTabLayout")) == 0)
		{
			return new CAnimationTabLayoutUI;
		}
	}
protected:
	void Init();
	void Notify(TNotifyUI& msg);
	CButtonUI *m_PBtn_Setting, *m_PBtn_Ok, *m_pBtn_Cannel;
	CAnimationTabLayoutUI* m_pTab;
	CEditUI	*m_PEdit_User, *m_PEdit_Passwd, *m_PEdit_Client, *m_PEdit_Server;

	void OnSetting(TNotifyUI& msg);
	void OnOk(TNotifyUI& msg);
	void OnCancel(TNotifyUI& msg);
};
