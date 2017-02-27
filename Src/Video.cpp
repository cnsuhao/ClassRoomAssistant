#include "Video.h"



CVideoWnd::CVideoWnd() :m_pOwner(NULL), click_tick(0)
{
	cur_time = pre_time = 0;
}


CVideoUI::CVideoUI(void) :m_pwindows(NULL),is_full(false)
{
}

CVideoUI::~CVideoUI(void)
{
	delete m_pwindows;
}

LPCTSTR CVideoWnd::GetWindowClassName() const
{
	return _T("CVideoWnd");
}
void CVideoWnd::Init(CVideoUI* pOwner)
{
	m_pOwner = pOwner;
	RECT rcpos=CalPos();
	RECT rcwnd;
	::GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(),&rcwnd);
	rcpos.left+=rcwnd.left;
	rcpos.right+=rcwnd.left;
	rcpos.top+=rcwnd.top;
	rcpos.bottom+=rcwnd.top;
	Create(m_pOwner->GetManager()->GetPaintWindow(),NULL,WS_CHILD|WS_EX_TOPMOST  | WS_VISIBLE,0,rcpos);
	::ShowWindow(m_hWnd,SW_SHOW);
}
LRESULT CVideoWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ERASEBKGND)
	{
		PainBk();
	}
	else if (uMsg == WM_LBUTTONDOWN)
	{
		++click_tick;
		cur_time = clock();
		if (cur_time - pre_time > 200)
		{
			click_tick = 0;
		}
		pre_time = cur_time;
		if (click_tick >= 1)
		{
			m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_DBCLICK);
			click_tick = 0;
		}
	}
	/*else if (uMsg==WM_SIZE)
	{
	}
	else if (uMsg==WM_RBUTTONDOWN)
	{
		
	}*/
	return CWindowWnd::HandleMessage(uMsg,wParam,lParam);
}


RECT CVideoWnd::CalPos()
{
	return m_pOwner->GetPos();
}
void CVideoWnd::PainBk()
{
	CDuiRect rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	PAINTSTRUCT    ps;
	HDC hDC = ::BeginPaint(m_hWnd, &ps);
	HDC hMemDC = ::CreateCompatibleDC(hDC);
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
	::SelectObject(hMemDC, hBitmap);
	CRenderEngine::DrawColor(hMemDC, rcClient, 0x0);
	//CRenderEngine::DrawColor(hMemDC, rcClient, 0xffff0000);

	BitBlt(hDC, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hMemDC, 0, 0, SRCCOPY);
	DeleteDC(hMemDC);
	DeleteObject(hBitmap);
	::EndPaint(m_hWnd, &ps);
}

/**************************************************/
void CVideoUI::SetVisible(bool bVisible )
{
	ShowWindow(getHwnd(), bVisible);
	__super::SetVisible(bVisible);
}
void CVideoUI::SetPos(RECT rc)
{
	__super::SetPos(rc);
	if(m_pwindows)
	{
		RECT rcpos=m_pwindows->CalPos();
		::SetWindowPos(m_pwindows->GetHWND(),NULL,rcpos.left,rcpos.top,rcpos.right-rcpos.left,rcpos.bottom-rcpos.top,SWP_NOZORDER | SWP_NOACTIVATE);
	}
}
HWND CVideoUI::getHwnd()
{
	if(!m_pwindows)
	{
		m_pwindows = new CVideoWnd();
		ASSERT(m_pwindows);
		m_pwindows->Init(this);
	}
	return m_pwindows->GetHWND();
}
void CVideoUI::Init()
{
	if(!m_pwindows)
	{
		m_pwindows = new CVideoWnd();
		ASSERT(m_pwindows);
		m_pwindows->Init(this);
	}	
	media_play = ILivePlayer::GetInstance();
	media_play->SetHWND(getHwnd());
}

void CVideoUI::fullSrc()
{
	if(is_full)
	{
		::SetParent(*m_pwindows, oldParent);
		m_pwindows->RestoreScreen();
	}
	else
	{
		oldParent = ::GetParent(*m_pwindows);
		::SetParent(*m_pwindows, ::GetDesktopWindow());
		m_pwindows->FullScreen();
	}
	is_full = !is_full;
}
bool CVideoUI::play(std::string url)
{
	if (!media_play->Load(url))
	{
		Sleep(300);
		return media_play->Load(url);
	}
	return true;
	
}

void CVideoUI::stop()
{
	media_play->Stop();
}
void CVideoUI::setVolume(int volume)
{
}
void CVideoUI::setMute(bool mute)
{
}

void CVideoUI::flushBk()
{
	if (m_pwindows)
	{
		::ShowWindow(*m_pwindows, SW_SHOW);
		m_pwindows->PainBk();
	}
}