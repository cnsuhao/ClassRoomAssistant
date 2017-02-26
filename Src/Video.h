#pragma once
#include "../Src/expresshead.h"
#ifdef _DEBUG
#pragma comment(lib,"../lib/LivePlayerD.lib")
#else
#pragma comment(lib,"../lib/LivePlayer.lib")
#endif // DEBUG


class CVideoUI;
class CVideoWnd :public CWindowWnd
{
public:
	CVideoWnd();
	LPCTSTR GetWindowClassName() const;
	void Init(CVideoUI* pOwner);
	void PainBk();
	RECT CalPos();
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	CVideoUI*	    m_pOwner;
	int click_tick;
	clock_t pre_time, cur_time;
};

class CVideoUI :public CContainerUI 
{
	friend class CVideoWnd;
public:
	CVideoUI(void);
	~CVideoUI(void);
	void SetVisible(bool bVisible=true );
	void SetPos(RECT rc);
	HWND getHwnd();
	void Init();
	bool play(std::string url);
	void stop();
	void setVolume(int volume);
	void setMute(bool mute=true);
	void flushBk();
	void fullSrc();
private:
	CVideoWnd *m_pwindows;
	HWND oldParent;
	bool is_full;
	std::string playurl;
public:
	void work();

};
