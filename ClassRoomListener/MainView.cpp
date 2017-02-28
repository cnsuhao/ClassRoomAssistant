#include "MainView.h"
#include "../Src/LoginWnd.h"
#include "../Src/ConfigFile.h"
#include "../Src/NotifyWnd.h"
#include "../ClassRoomListener/SettingView.h"
#include <fstream>
#define  TIME_ID_UPDATE_TIME	1001
#define  TIME_ID_NOTIFY			1002

MainView::MainView() :
client(NULL),
init_thread(NULL), 
update_thread(NULL),
m_pData(NULL), 
m_pTime(NULL),
m_pNotify(NULL),
m_pNotifyLay(NULL),
m_pCalssLay(NULL),
current_update_state(-1),
m_pConnect(NULL),
m_pHudong(NULL),
m_pServerIP(NULL),
m_pServerName(NULL),
m_pServerPic(NULL),
m_pStateOn(NULL),
m_pStateOff(NULL),
m_pVideo(NULL)
{
	/* send TCP message to server*/
	if(!client)
	{
		client = ITCPClient::getInstance();		
		client->open(user_list::server_ip.c_str(), _tstoi(user_list::server_port.c_str()));
		client->setListener(this);
	}
	char data[] = "type=GetIpList";
	client->sendData(data, strlen(data));
}
MainView::~MainView()
{
	if (client)
	{
		client->close();
		ITCPClient::releaseInstance(client);
	}

	if(init_thread)
	{
		CloseHandle(init_thread);
	}

	if(update_thread)
	{
		CloseHandle(update_thread);
	}
	
}

LPCSTR  MainView::GetWindowClassName() const
{
	return _T("MainView");
}

CDuiString MainView::GetSkinFolder()
{
	return _T("Skin");
}

CDuiString MainView::GetSkinFile()
{
	return _T("MainView.xml");
}

void MainView::Recv(SOCKET sock, const char* ip, const int port, char* data, int dataLength)
{
	if (dataLength > 0)
	{
		map<string, string> res = paraseCommand(data);
		if (res["type"] == "GetIpList")
		{
			string res_list=res["ip"];
			chat_ip = res["ChatIp"];
			int pos = -1;
			while ((pos = res_list.find_first_of(';')) != -1)
			{
				ip_list.insert(res_list.substr(0, pos));
				res_list.erase(0, pos + 1);
			}
			if(!ip_list.empty())
			{
				update_connect_state(true);
			}
			if (!init_thread)
			{
				init_thread = CreateThread(NULL, 0, initProc, this, NULL, NULL);
			}
			
		}
		else if (res["type"] == "UpdateDevName")
		{
			current_update_state = UPDATE_NAME;
			if (update_thread)
			{
				WaitForSingleObject(update_thread,3000);
				CloseHandle(update_thread);
			}
			just_join_update_name = res["ip"];
			class_list[just_join_update_name].name = res["name"];
			ConfigFile cf(CFG_FILE);
			cf.addValue("name", res["name"], just_join_update_name);
			update_thread = CreateThread(NULL, 0, updateProc, (void*)this, NULL, 0);
		}
		else if (res["type"] == "UpdatePicture")
		{
			current_update_state = UPDATE_PIC;
			if (update_thread)
			{
				WaitForSingleObject(update_thread, 3000);
				CloseHandle(update_thread);
			}
			just_join_update_pic = res["ip"];
			update_thread = CreateThread(NULL, 0, updateProc, (void*)this, NULL, 0);
		}
		else if (res["type"] == "JoinMeeting")
		{
			current_update_state = UPDATE_MEM;
			update_connect_state(true);
			if (update_thread)
			{
				WaitForSingleObject(update_thread, 3000);
				CloseHandle(update_thread);
			}
			just_join_member = res["ip"];
			class_list[ip].ip = just_join_member;
			update_thread = CreateThread(NULL, 0, updateProc, (void*)this, NULL, 0);
		}
		else if (res["type"] == "QuitMeeting")
		{
			ManagerItem::Remove(m_pCalssLay, res["ip"].c_str());
			msg_coming(class_list[res["ip"]].name+"退出了");
			if (!class_list[user_list::ip].url.empty())
			{
				m_pConnect->SetText(_T("请求连接"));
				if (m_pVideo->is_playing())
					m_pVideo->stop();
				m_pVideo->play(class_list[user_list::ip].url);
			}
			class_list.erase(class_list.find(res["ip"]));
			if (class_list.find(user_list::ip) == class_list.end())
			{
				update_connect_state(false);
			}
		}
		else if (res["type"] == "SelChat")
		{
			just_join_speak = res["ip"];
			selected_speak(just_join_speak);
		}
		else if (res["type"] == "RequestJoin")
		{
			//none
		}
		else if (res["type"] == "RequestSpeak")
		{
			//none
		}
	}
}

map<std::string, std::string> MainView::paraseCommand(const char* cmd)
{
	map<std::string, std::string> res_map;
	string scmd = string(cmd);
	int pos = -1;
	while ((pos = scmd.find_first_of('&')) != -1 || scmd.length()>=1)
	{
		if (pos == -1)
			pos = scmd.length();
		string percmd = scmd.substr(0, pos);
		string key = percmd.substr(0, percmd.find_first_of('='));
		string values = percmd.substr(percmd.find_first_of('=') + 1, percmd.length());
		res_map[key] = values;
		scmd.erase(0, pos + 1);
	}
	return res_map;
}

void MainView::Notify(TNotifyUI& msg)
{
	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender->GetName() == _T("btn_min"))
		{
			::ShowWindow(*this, SW_MINIMIZE);
		}
		else if (msg.pSender->GetName() == _T("btn_max"))
		{
			static bool record = false;
			if (!record)
			{
				::ShowWindow(*this, SW_MAXIMIZE);
				record = !record;
			}
			else
			{
				::ShowWindow(*this, SW_RESTORE);
				record = !record;
			}
		}
		else if (msg.pSender->GetName() == _T("btn_close"))
		{
			if (IDOK == TipMsg::ShowMsgWindow(*this, _T("是否退出"), _T("提示")))
			{
			string str = "type=QuitMeeting&ip=" + user_list::ip;
			char data[50];
			strcpy(data, str.c_str());
			client->sendData(data,strlen(data));
			/* make all-user logout*/
			release_thread = CreateThread(NULL, 0, releaseProc, this, NULL, NULL);
			WaitForSingleObject(release_thread, 5000);
			Close();
			}
		}
		else if (msg.pSender->GetName() == _T("btn_expend"))
		{
			CVerticalLayoutUI *ver = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("ClassRoomLay")));
			ver->SetVisible(!ver->IsVisible());
			msg.pSender->SetVisible(false);
			CButtonUI *btn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_unexpend")));
			btn->SetVisible(true);
		}
		else if (msg.pSender->GetName() == _T("btn_unexpend"))
		{
			CVerticalLayoutUI *ver = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("ClassRoomLay")));
			ver->SetVisible(!ver->IsVisible());
			msg.pSender->SetVisible(false);
			CButtonUI *btn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_expend")));
			btn->SetVisible(true);
		}
		else if (msg.pSender->GetName()==_T("btn_setting"))
		{
			SettingView * setview = new SettingView;
			setview->Create(*this, _T("Setting"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
			setview->CenterWindow();
			setview->ShowModal();			
		}
		else if (msg.pSender == m_pConnect)
		{
			if (msg.pSender->GetText() == _T("请求连接"))
			{
				string str = "type=RequestJoin&ip=" + user_list::ip;
				char data[50];
				strcpy(data, str.c_str());
				client->sendData(data, strlen(data));
			}
			else
			{
				string str = "type=QuitMeeting&ip=" + user_list::ip;
				char data[50];
				strcpy(data, str.c_str());
				client->sendData(data, strlen(data));
				ManagerItem::Remove(m_pCalssLay, user_list::ip.c_str());
				msg.pSender->SetText(_T("请求连接"));
			}
			
		}
		else if (msg.pSender==m_pHudong)
		{
			string str = "type=RequestSpeak&ip=" + user_list::ip;
			char data[50];
			strcpy(data, str.c_str());
			client->sendData(data, strlen(data));
		}
	}
	else if(msg.sType==DUI_MSGTYPE_DBCLICK)
	{
		if(msg.pSender->GetName ()==_T("mainVideo"))
		{
			CVideoUI *video = static_cast<CVideoUI*>(m_PaintManager.FindControl(_T("mainVideo")));
			video->fullSrc();
		}
	}
}

CControlUI* MainView::CreateControl(LPCTSTR pstrClass)
{
	if (_tcscmp(pstrClass, _T("Video")) == 0)
		return new CVideoUI();
	else if (_tcscmp(pstrClass, _T("ICOImage")) == 0)
		return new CICOControlUI();
	else
		return __super::CreateControl(pstrClass);
}
LRESULT MainView::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER)
		return OnTimer(uMsg, wParam, lParam);
	else if (uMsg == WM_ERROR_TIP)
	{
		TipMsg::ShowMsgWindow(*this, error_msg.c_str());
	}
	else if (uMsg == WM_UPDATE_DEVNAME || uMsg == WM_UPDATE_ICO)
	{
		classItemUI *item = ManagerItem::getItem(user_list::ip.c_str());
		if (uMsg == WM_UPDATE_ICO)
		{
			update_pic(user_list::ip);
			if (item)
				item->setImage(class_list[user_list::ip].path.c_str());
			std::string str = "type=UpdatePicture&ip=" + user_list::ip;
			char data[50];
			strcpy(data, str.c_str());
			client->sendData(data, strlen(data));
		}
		else
		{
			update_name(user_list::ip);
			if (item)
				item->setTitle(class_list[user_list::ip].name.c_str());
			std::string str = "type=UpdateDevName&ip=" + user_list::ip + "&name=" + class_list[user_list::ip].name;
			char data[50];
			strcpy(data, str.c_str());
			client->sendData(data, strlen(data));
		}

	}
	else
		return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT MainView::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == TIME_ID_UPDATE_TIME)
	{
		DisplayDateTime();
	}
	else if (wParam == TIME_ID_NOTIFY)
	{
		LPCTSTR s=m_pNotify->GetText().GetData();
		TCHAR title[200];
		if (m_pNotify->GetText().GetLength() <= 2)
		{
			KillTimer(*this, TIME_ID_NOTIFY);
			m_pNotifyLay->SetVisible(false);
		}
		_tcscpy(title, (s + 2));
		m_pNotify->SetText(title);
	}

	return 0;
}
void MainView::Init()
{
	m_pData = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_date")));
	m_pTime = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_time")));
	m_pNotify = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_notify")));
	m_pNotifyLay = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("NotifyLay")));
	m_pCalssLay = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("CalssRoomListLay")));

	m_pServerIP = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("server_ip")));
	m_pServerName = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("server_name")));
	m_pServerPic = static_cast<CICOControlUI*>(m_PaintManager.FindControl(_T("server_ico")));
	m_pConnect = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_request_connect")));
	m_pHudong = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_request_interact")));

	m_pStateOn = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status_on")));
	m_pStateOff = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status")));

	m_pVideo = static_cast<CVideoUI*>(m_PaintManager.FindControl(_T("mainVideo")));

	

	/* hide notify lay*/
	m_pNotifyLay->SetVisible(false);

	/* load server info*/
	m_pServerIP->SetText(user_list::server_ip.c_str());
	m_pServerName->SetText("");
	/* show real-dataTime */
	SetTimer(*this, TIME_ID_UPDATE_TIME, 999, NULL);
}

void MainView::update_name(const std::string ip)
{
	std::string requestUrl = "http://" + ip + "/" + user_list::cgi + "type=getdevname";
	try
	{
		std::string name = Logan::query_msg_node(requestUrl, ip);
		class_list[ip].name = name;
		ConfigFile cfg(CFG_FILE);
		cfg.addValue("name", name,ip);
		cfg.save();
	}
	catch (std::exception& e)
	{
		throw e;
	}
}

void MainView::load_local(const std::string ip)
{
	/* load pic*/
	ConfigFile cf(CFG_FILE);
	std::string picPath = cf.getValue("path", ip);
	class_list[ip].ip = ip;
	if (!Logan::file_exist(picPath))
	{
		try
		{
			CreateDirectoryA(ip.c_str(), NULL);
			update_pic(ip);
		}
		catch (std::exception& e)
		{
			throw e;
		}
	}
	else
	{
		class_list[ip].path = picPath;
	}

	/* load name*/
	try
	{
		update_name(ip);
	}
	catch (std::exception& e)
	{
		throw e;
	}
}

void MainView::update_pic(const std::string ip)
{
	std::string requestPicNameUrl = "http://" + ip + "/" + user_list::cgi + "type=getpicture";
	try
	{
		std::string picName = Logan::query_msg_node(requestPicNameUrl, ip);
		std::string downloadUrl = "http://" + ip + "/" + picName;
		if (!picName.empty())// if no picture on server
		{
			std::string path = ip + "/" + picName;
			if (!Logan::download(downloadUrl, path))
			{
				throw std::exception("download error");
			}
			ConfigFile cf(CFG_FILE);
			class_list[ip].path = path;
			cf.addValue("path", path, ip);
			cf.save();
		}
	}
	catch (std::exception& e)
	{
		throw e;
	}
}

void MainView::get_url(const std::string ip)
{
	std::string requestUrl = "http://" +ip + "/" + user_list::cgi+"type=queryurl&name=lurl";
	try
	{
		std::string url = Logan::query_url(requestUrl, ip);
		if(url.length () > 9)
		{
			url=url.replace(7, 9, ip);
			class_list[ip].url = url;
		}
	}
	catch (std::exception& e)
	{
		throw e;
	}
}

void MainView::selected_speak(const std::string ip)
{
	if(current_speak.empty ())
	{
		current_speak.push(class_list[ip]);
	}
	else
	{
		ItemData item = current_speak.top();
		current_speak.pop();
		classItemUI *itemUI1 = ManagerItem::getItem(item.ip.c_str ());
		itemUI1->SetText(_T("听课中"));
		current_speak.push(class_list[ip]);
	}
	classItemUI *item = ManagerItem::getItem(ip.c_str());
	if (item)
		item->SetText(_T("发言中"));
}


void MainView::msg_coming(const std::string tip)
{
	m_pNotify->SetText(tip.c_str());
	m_pNotifyLay->SetVisible(true);
	PlaySoundA("msg.wav", NULL, SND_ASYNC | SND_FILENAME);
	SetTimer(*this, TIME_ID_NOTIFY, 400, NULL);
}

void MainView::DisplayDateTime()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	static char cdate[250];
	static char ctime[250];
	sprintf(cdate, "%d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
	if (m_pData)
		m_pData->SetText(cdate);

	sprintf(ctime, "%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
	if (m_pTime)
		m_pTime->SetText(ctime);
}

void MainView::update_connect_state(bool is_connect)
{
	if (m_pStateOn &&m_pStateOff)
	{
		m_pStateOn->SetVisible(is_connect);
		m_pStateOff->SetVisible(!is_connect);
	}
}

DWORD WINAPI initProc(_In_ LPVOID paramer)
{
	MainView* p = (MainView*)paramer;
	try
	{
		/* first load login-ip info & server info*/
		p->load_local(user_list::ip);
		p->get_url(user_list::ip);
		p->load_local(user_list::server_ip);
		p->get_url(user_list::server_ip);

		/* set name of server*/
		p->m_pServerName->SetText(p->class_list[user_list::server_ip].name.c_str());

		/* set picture of server*/
		if (Logan::file_exist(p->class_list[user_list::server_ip].path))
			p->m_pServerPic->SetBkImage(p->class_list[user_list::server_ip].path.c_str());

		/*play video of login server*/
		if (!p->class_list[user_list::ip].url.empty())
		{
			p->m_pVideo->play(p->class_list[user_list::ip].url);
		}
		/* then get info from network*/
		bool bHad = false;
		for (std::set<std::string>::iterator itor = p->ip_list.begin(); itor != p->ip_list.end();itor++)
		{
			CreateDirectoryA(itor->c_str(), NULL);
			if(*itor!=user_list::ip)
			{
				p->load_local(*itor);
				p->class_list[*itor].ip = *itor;
				classItemUI *item = new classItemUI;
				item->setIp(itor->c_str());
				if (!p->class_list[*itor].path.empty() && Logan::file_exist(p->class_list[*itor].path))
					item->setImage(p->class_list[*itor].path.c_str());
				item->setTitle(p->class_list[*itor].name.c_str());
				ManagerItem::Add(p->m_pCalssLay, item);
			}
			else
			{
				classItemUI *item = new classItemUI;
				item->setIp(itor->c_str());
				if (!p->class_list[*itor].path.empty() && Logan::file_exist(p->class_list[*itor].path))
					item->setImage(p->class_list[*itor].path.c_str());
				item->setTitle(p->class_list[*itor].name.c_str());
				ManagerItem::Add(p->m_pCalssLay, item);
				p->m_pConnect->SetText(_T("断开连接"));
				p->update_connect_state(true);
				if (!p->class_list[*itor].url.empty())
				{
					bHad = true;
					//if (p->m_pVideo->is_playing())
						//p->m_pVideo->stop();
					p->m_pVideo->play(p->class_list[user_list::server_ip].url);
				}
			}
		}

		if (!bHad && !p->class_list[user_list::ip].url.empty())
		{
			p->m_pVideo->play(p->class_list[user_list::ip].url);
		}
	}
	catch (std::exception& e)
	{
		p->error_msg = e.what();
		::PostMessageA(*p, WM_ERROR_TIP, NULL, NULL);
	}

	/* update chat state*/
	if (!p->chat_ip.empty())
		p->selected_speak(p->chat_ip);
	return 0;
}

DWORD WINAPI updateProc(_In_ LPVOID paramer)
{
	MainView *p = (MainView*)paramer;
	
	try
	{
		if (p->current_update_state == UPDATE_PIC)
		{
			p->update_pic(p->just_join_update_pic);
			classItemUI *item = ManagerItem::getItem(p->just_join_update_pic.c_str());
			if (!p->class_list[p->just_join_update_pic].path.empty() && Logan::file_exist(p->class_list[p->just_join_update_pic].path))
				item->setImage(p->class_list[p->just_join_update_pic].path.c_str());
			p->msg_coming(p->class_list[p->just_join_update_pic].name + "更新了头像");
			p->current_update_state = UPDATE_NONE;
		}
		else if (p->current_update_state == UPDATE_NAME)
		{
			p->update_name(p->just_join_update_name);
			classItemUI *item = ManagerItem::getItem(p->just_join_update_name.c_str());
			item->setTitle(p->class_list[p->just_join_update_name].name.c_str());
			p->msg_coming(p->class_list[p->just_join_update_name].name + "更新了名称");
			p->current_update_state = UPDATE_NONE;
		}
		else if (p->current_update_state == UPDATE_MEM)
		{
			p->load_local(p->just_join_member);
			classItemUI *item = new classItemUI;
			item->setIp(p->just_join_member.c_str());
			item->setTitle(p->class_list[p->just_join_member].name.c_str());
			if (!p->class_list[p->just_join_member].path.empty() && Logan::file_exist(p->class_list[p->just_join_member].path))
				item->setImage(p->class_list[p->just_join_member].path.c_str());
			ManagerItem::Add(p->m_pCalssLay, item);
			p->msg_coming(p->class_list[p->just_join_member].name + "上线了");
			if (p->just_join_member == user_list::ip)
			{
				p->get_url(p->just_join_member);
				p->m_pConnect->SetText(_T("断开连接"));
				if (!p->class_list[user_list::ip].url.empty())
				{
					if (p->m_pVideo->is_playing())
					{
						p->m_pVideo->stop();
					}
					p->m_pVideo->play(p->class_list[user_list::server_ip].url);
				}
			}
			p->current_update_state = UPDATE_NONE;
		}			
	}
	catch (std::exception& e)
	{
		p->error_msg = e.what();
		::PostMessageA(*p, WM_ERROR_TIP, NULL, NULL);
	}
	return 0;
}

DWORD WINAPI releaseProc(_In_ LPVOID paramer)
{
	MainView *p = (MainView*)paramer;

	for (std::map<std::string, ItemData>::iterator i = p->class_list.begin(); i != p->class_list.end();i++)
	{
		if(i->first!=user_list::ip)
			Logan::logout(i->first, user_list::user_name);
	}
	return 0;
}

/************************************************************************/

classItemUI::classItemUI() : m_pIP(NULL),
m_pState(NULL), m_pICO(NULL), m_pTitle(NULL)
{
	initItem();
}

void classItemUI::setTitle(LPCTSTR pstr_title)
{
	m_pTitle->SetText(pstr_title);
}

LPCTSTR classItemUI::getTitle()const
{
	return m_pTitle->GetText();
}

void classItemUI::setIp(LPCTSTR pstr_ip)
{
	pstrIP = pstr_ip;
	m_pIP->SetText(pstrIP);
}

LPCTSTR classItemUI::getIP()const
{
	return pstrIP /* m_pIP->GetText()*/;
}

void classItemUI::setImage(LPCTSTR pstr_image)
{
	m_pICO->SetBkImage(pstr_image);
}

LPCTSTR classItemUI::getImage()const
{
	return m_pICO->GetBkImage();
}
void classItemUI::SetText(LPCTSTR pstrText)
{
	m_pState->SetText(pstrText);
}

LPCTSTR classItemUI::getText()const
{
	return m_pState->GetText();
}

void classItemUI::DoEvent(TEventUI& event)
{
	__super::DoEvent(event);
}
void classItemUI::initItem()
{

	this->SetFixedHeight(90);
	this->SetFixedWidth(170);
	this->SetBkColor(0xff383838);
	SIZE s;
	s.cx = 5;
	s.cy = 5;
	this->SetBorderRound(s);

	m_pICO = new CICOControlUI();
	m_pICO->SetFloat(true);
	m_pICO->SetAttribute("pos", "3,0,0,0");
	m_pICO->SetFixedHeight(45);
	m_pICO->SetFixedWidth(45);
	m_pICO->SetBkImage("ico.jpg");
	m_pICO->SetTranImage("tran.png");
	this->Add(m_pICO);

	m_pTitle = new CLabelUI();
	m_pTitle->SetFloat(true);
	m_pTitle->SetAttribute("pos", "48,0,170,30");
	m_pTitle->SetText("未命名");
	m_pTitle->SetAttribute("align", "center");
	m_pTitle->SetTextColor(0xffffffff);
	m_pTitle->SetFixedHeight(30);
	this->Add(m_pTitle);

	m_pIP = new CLabelUI();
	m_pIP->SetFloat(true);
	m_pIP->SetAttribute("pos", "48,30,170,45");
	m_pIP->SetAttribute("align", "center");
	m_pIP->SetTextColor(0xffAEA4A9);
	m_pIP->SetText("255.255.255.255");
	this->Add(m_pIP);

	CLabelUI* line = new CLabelUI();
	line->SetFloat(true);
	line->SetAttribute("pos", "10,46,160,47");
	line->SetBkColor(0xff666666);
	this->Add(line);

	m_pState = new CLabelUI();
	m_pState->SetFloat(true);
	m_pState->SetAttribute("pos", "50,53,120,84");
	m_pState->SetAttribute("align", "center");
	m_pState->SetTextColor(Color::Cyan);
	m_pState->SetText(_T("听课中"));
	this->Add(m_pState);
}



std::map<CDuiString, ManagerItem::UI> ManagerItem::mgr;

void ManagerItem::Add (CContainerUI* pContain, classItemUI* item)
{
	if(mgr.find (item->getIP ())!=mgr.end ())
	{
		Remove(pContain, item->getIP());
	}
	ManagerItem::UI u;
	u.item = item;
	u.sep = new sepUI(5);
	CDuiString strIp(item->getIP());
	mgr[item->getIP()] = u;
	pContain->Add(u.sep);
	pContain->Add(u.item);
}

void ManagerItem::Remove(CContainerUI* pContain, CDuiString ip)
{
	if (mgr.find(ip) != mgr.end())
	{
		pContain->Remove(mgr[ip].sep);
		pContain->Remove(mgr[ip].item);
		mgr.erase(mgr.find(ip));
	}
}

classItemUI* ManagerItem::getItem(CDuiString ip)
{
	return mgr[ip].item;
}