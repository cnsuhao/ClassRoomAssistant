#pragma once
#include "../Src/expresshead.h"
#include "../Src/Video.h"
#include "../Src/HttpRequest.h"
#include "../Src/ConfigFile.h"
#include "../Src/TCPSocket.h"
#include <map>
#include <set>
#include <queue>
#include <stack>

#define UPDATE_PIC	1
#define UPDATE_NAME 2
#define UPDATE_MEM	3
#define UPDATE_NONE -1

#define TIME_ID_UPDATE_RECORD_TIME 4848

#define  WM_REQUEST_JOIN	WM_USER+1000
#define  WM_REQUEST_SPEAK	WM_USER+1001
/*
*  Main Window for ClassRoomAssistant-server(Speaker)
*
*	Date:	2017-2-12 --- 2017-3-1
*
*/

class CICOControlUI : public CLabelUI
{
public:
	CICOControlUI(){}
	~CICOControlUI(){}
	void PaintBkImage(HDC hDC)
	{
		__super::PaintBkImage(hDC);
		if (m_stranImage.IsEmpty()) return;
		if (!DrawImage(hDC, (LPCTSTR)m_stranImage)) m_stranImage.Empty();
	}
	void SetTranImage(LPCTSTR pStrImage)
	{
		if (pStrImage == m_stranImage)
			return;
		else
			m_stranImage = pStrImage;
		Invalidate();
	}
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("TranImage")) == 0)
			SetTranImage(pstrValue);
		else
			__super::SetAttribute(pstrName, pstrValue);
	}
private:
	CDuiString m_stranImage;

};


class classItemUI : public CVerticalLayoutUI
{
public:
	classItemUI();
	void setTitle(LPCTSTR pstr_title);
	LPCTSTR getTitle()const;
	void setIp(LPCTSTR pstr_ip);
	LPCTSTR getIP()const;
	void setImage(LPCTSTR pstr_image);
	LPCTSTR getImage()const;
	void SetText(LPCTSTR pstrText);
	LPCTSTR getText()const;
	void DoEvent(TEventUI& event);
protected:
	CICOControlUI* m_pICO;		//头像
	CLabelUI*		m_pTitle;
	CLabelUI*		m_pIP;
	CButtonUI*		m_pcnt;		//连接按钮
	CDuiString		m_strIP;
protected:
	void initItem();
	CDuiString pstrIP;

};

class sepUI :public CHorizontalLayoutUI
{
public:
	sepUI(int height = 5)
	{
		this->SetFixedHeight(height);
	}
};

class ManagerItem
{
public:
	struct UI
	{
		classItemUI* item;
		sepUI*	sep;
	};
	static void Add(CContainerUI* pContain, classItemUI* item);
	static void Remove(CContainerUI* pContain, CDuiString ip);
	static classItemUI* getItem(CDuiString ip);
private:
	static std::map<CDuiString, UI>mgr;
};

struct small_video
{
	small_video():title(NULL),snd_on(NULL),snd_off(NULL),
					video(NULL),teacher(NULL),student(NULL){}
	CLabelUI* title;
	CButtonUI* snd_on;
	CButtonUI* snd_off;
	CVideoUI *video;
	union 
	{
		CButtonUI * teacher;
	};
	CButtonUI *student;
};

/* per classRoom data unit*/
struct ItemData
{
	std::string ip;
	std::string url;
	std::string path;
	std::string name;
	small_video media;
};

class MainView :public WindowImplBase, public IListener
{
public:
	MainView();
	~MainView();
public:
	LPCTSTR GetWindowClassName()const;
	CDuiString GetSkinFile();
	CDuiString GetSkinFolder();
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam);
	CControlUI* CreateControl(LPCTSTR pstrClass);
protected:
	void Recv(SOCKET sock, const char* ip, const int port, char* data, int dataLength);

	void Notify(TNotifyUI& msg);

	void Init();
	map<std::string, std::string> paraseCommand(const char* cmd);
private:
	CLabelUI *m_pData, *m_pTime, *m_pNotify;
	CHorizontalLayoutUI *m_pNotifyLay;
	CVerticalLayoutUI	*m_pCalssLay;
	CLabelUI *m_pStateOn, *m_pStateOff;
	CVideoUI *m_pVideo;
	CButtonUI	*m_pRecord, *m_pPause, *m_pStop;
	CLabelUI	*m_pRecordTime;
	small_video video_list[6];
	bool need_sync;
private:
	ITCPClient *client;
	std::stack<small_video> free_stack;// free stack to save free video list
	/* save class-data in hash-table */
	std::map<std::string, ItemData>class_list;

	/* select speak data*/
	std::stack<ItemData>current_speak;

	/* exception msg*/
	std::string error_msg;

	/* online IP-list from TCP server*/
	std::set<std::string>ip_list;

	/* just join ip*/
	std::string just_join_speak;
	std::string just_join_update_pic;
	std::string just_join_update_name;
	std::string just_join_member;

	/* VGA  & director stream*/
	std::string vga_url;
	std::string durl;
	/* update state [1:picture] [2:name] [3:member] [-1:none]*/
	int current_update_state;

	void load_local(const std::string ip);
	void update_pic(const std::string ip);
	void update_name(const std::string ip);
	void get_url(const std::string ip);
	void get_vgaurl(const std::string ip);
	void get_durl(const std::string ip);
	void selected_speak(const std::string ip);

	void msg_coming(const std::string tip);

	void DisplayDateTime();

	void SoundStateOn(int index,bool is_on);

	void update_video_state();

	void init_videoList();

	void update_connect_state(bool is_connect);
private:
	HANDLE init_thread;
	HANDLE update_thread;
	HANDLE release_thread;
	HANDLE sync_thread;
	friend DWORD WINAPI initProc(_In_ LPVOID paramer);
	friend DWORD WINAPI updateProc(_In_ LPVOID paramer);
	friend DWORD WINAPI releaseProc(_In_ LPVOID paramer);
	friend DWORD WINAPI syncProc(_In_ LPVOID paramer);

};


