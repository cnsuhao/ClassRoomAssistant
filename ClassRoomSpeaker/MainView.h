#pragma once
#include "../expresshead.h"
#include "../Video/Video.h"
#include "../HttpRequst/HttpRequest.h"
#include "../ConfigFile/ConfigFile.h"
#include "../TCP/TCPSocket.h"
#include <map>
#include <queue>
#include <vector>

struct _SmallVideoCtrl
{
	CLabelUI*	lab_title;
	CButtonUI*	btn_sound;
	CButtonUI*  btn_sound_off;
	CVideoUI*	video;
	union 
	{
		CButtonUI* btn_teacher;
		CButtonUI* btn_ppt;
	};
	CButtonUI* btn_student;
};



class CICOControlUI : public CLabelUI
{
public:
	CICOControlUI(){}
	~CICOControlUI(){}
	void PaintBkImage(HDC hDC)
	{
		__super::PaintBkImage(hDC);
		/*if (m_sBkImage.IsEmpty()) return;
		if (!DrawImage(hDC, (LPCTSTR)m_sBkImage)) m_sBkImage.Empty();*/
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
	CICOControlUI* m_pICO;		//ͷ��
	CLabelUI*		m_pTitle;
	CLabelUI*		m_pIP;
	CButtonUI*		m_pcnt;		//���Ӱ�ť
protected:
	void initItem();

};

struct _classUnit
{
	CICOControlUI* btn_ico;
	CLabelUI* lab_title;
	CLabelUI* lab_ip;
	CButtonUI* btn_connect;
	CVerticalLayoutUI *lay;
};

struct _classinfo
{
	std::string dev_name;//�豸��
	std::string play_url;//���ŵ�ַ(������)
	std::string picture_path;//ͷ��·��
	_classUnit  class_ctrl;//���ɵĿؼ�
	_SmallVideoCtrl class_video;
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
	void Recv(SOCKET sock, const char* ip, const int port, char* data, int dataLength);
	map<std::string, std::string> paraseCommand(const char* cmd);
private:
	void loadAllClassRoom();
	void loadClassRoom(std::string ip);
protected:
	void Notify(TNotifyUI& msg);
private:
	CLabelUI *lab_date, *lab_time,*lab_notice;;
	_SmallVideoCtrl subVideo[6];
	_classUnit classRoom[4];

	//TCP�ͻ���
	ITCPClient* client;
	void Init();
	//��ʼ���·���Ƶ�ؼ�
	void initSubVideo();
	//��ʼ���Ҳ�����б�ؼ�
	void initClassRoom();
	void update_classroom();
	void DisplayDateTime();
	//��������
	void updateConfigure();

	bool is_init;
	//���½����б�����һ��������
	void delete_classUI(_classinfo info,int current_mount);
	void add_classUI(_classinfo info);
	std::vector<std::string>ip_table;
private:
	std::string just_join;//�ոռ����IP
	std::string just_update_pic;
	std::string just_speak_ip;//ѡ���Ե�IP
	std::map<std::string, _classinfo>class_list;

	queue<_classinfo> class_queue;
	vector<_classinfo> class_vec;
	//��ʼ���б����Ƶ�ؼ�
	void init_vec();
	//�������γ�Ա�����߳�
	HANDLE updateMenberJoin_thread;
	//����ͷ���߳�
	HANDLE updatePicture_thread;
	//�л����߳�
	HANDLE switch_view_thread;
	//���ζ˵Ĳ��ŵ�ַ
	HANDLE local_thread;
	
	friend DWORD WINAPI updateJoin_Proc(_In_ LPVOID paramer);
	friend DWORD WINAPI updatePicture_Proc(_In_ LPVOID paramer);
	friend DWORD WINAPI LiveViewSwitch(_In_ LPVOID paramer);
	friend DWORD WINAPI initLocalUrl(_In_ LPVOID paramer);

private:
	//ѧ��-��ʦ �����л�
	int current_index;
	int current_view;
	std::string local_url;
	std::string local_durl;
	std::string vga_url;
	void cut_view(int index, int view);

};


