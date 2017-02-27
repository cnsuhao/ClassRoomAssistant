#include "MainView.h"
#include "../Src/Video.h"
#include "SettingView.h"
#include "../Src/NotifyWnd.h"
#include "../Src/LoginWnd.h"
#include "../Src/tinyxml.h"
#include "../Src/tinystr.h"
#include "../Src/CMyCharConver.h"
#include "../Src/IMyCurl.h"
#define  TIME_ID_UPDATE_TIME	1001
#define  TIME_ID_NOTIFY			1002


#define WM_CLIENT_ADDED		WM_USER+100



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
m_pStateOn(NULL),
m_pStateOff(NULL),
m_pVideo(NULL)
{
	if (!client)
	{
		client = ITCPClient::getInstance();
		client->open(user_list::ip.c_str(), _tstoi(user_list::server_port.c_str()));
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

	if (init_thread)
	{
		CloseHandle(init_thread);
	}

	if (update_thread)
	{
		CloseHandle(update_thread);
	}
	/* make all-user logout*/
	release_thread = CreateThread(NULL, 0, releaseProc, this, NULL, NULL);
	WaitForSingleObject(release_thread, 5000);
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
map<std::string, std::string> MainView::paraseCommand(const char* cmd)
{
	map<std::string, std::string> res_map;
	string scmd = string(cmd);
	int pos = -1;
	while ((pos = scmd.find_first_of('&')) != -1 || scmd.length() >= 1)
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

void MainView::Recv(SOCKET sock, const char* ip, const int port, char* data, int dataLength)
{
	if (dataLength > 0)
	{
		map<string, string> res = paraseCommand(data);
		if (res["type"] == "GetIpList")
		{
			string res_list = res["ip"];
			selected_speak(res["ChatIp"]);
			int pos = -1;
			while ((pos = res_list.find_first_of(';')) != -1)
			{
				ip_list.insert(res_list.substr(0, pos));
				res_list.erase(0, pos + 1);
			}
			if (!ip_list.empty())
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
				WaitForSingleObject(update_thread, 3000);
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
			// none
		}
		else if (res["type"] == "QuitMeeting")
		{
			ManagerItem::Remove(m_pCalssLay, res["ip"].c_str());
			msg_coming(class_list[res["ip"]].name + "退出了");
			if (!class_list[user_list::ip].url.empty())
			{
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
			// none
		}
		else if (res["type"] == "RequestJoin")
		{
			current_update_state = UPDATE_MEM;
			update_connect_state(true);
			if (update_thread)
			{
				WaitForSingleObject(update_thread, 3000);
				CloseHandle(update_thread);
			}
			just_join_member = res["ip"];
			if (just_join_member == user_list::ip)
			{
				if (!class_list[user_list::ip].url.empty())
				{
					m_pVideo->stop();
					m_pVideo->play(class_list[user_list::server_ip].url);
				}
				return;
			}
			update_thread = CreateThread(NULL, 0, updateProc, (void*)this, NULL, 0);
		}
		else if (res["type"] == "RequestSpeak")
		{
			just_join_speak = res["ip"];
			::PostMessage(*this, WM_REQUEST_SPEAK, NULL, NULL);
		}
	}
}

DWORD WINAPI updateJoin_Proc(_In_ LPVOID paramer)
{
	MainView *p = (MainView*)paramer;
	//p->class_list[p->just_join];
	//CreateDirectoryA(p->just_join.c_str(), NULL);
	//std::string logincode, namecode, icocode, mmsg, token, pmsg;
	//// 登陆获得token
	//std::string requestUrl = "http://" + p->just_join + "/" + login_cgi + "type=login&userName=" + admin_user + "&password=" + admin_passwd;
	//std::string res = HttpRequest::request(requestUrl);
	//TiXmlDocument xml;
	//xml.Parse(res.c_str());
	//TiXmlNode *root = xml.RootElement();
	//for (TiXmlNode *i = root->FirstChildElement(); i; i = i->NextSiblingElement())
	//{
	//	if (strcmp(i->Value(), "code") == 0)
	//	{
	//		logincode = string(i->FirstChild()->Value());
	//	}
	//	else if (strcmp(i->Value(), "msg") == 0)
	//	{
	//		token = string(i->FirstChild()->Value());
	//	}
	//}
	//if (logincode == "1")
	//{
	//	//获取名称
	//	requestUrl = "http://" + p->just_join + "/" + login_cgi + "type=getdevname&token=" + token;
	//	res = HttpRequest::request(requestUrl);
	//	TiXmlDocument xmll;
	//	xmll.Parse(res.c_str());
	//	TiXmlNode *root = xmll.RootElement();
	//	for (TiXmlNode *i = root->FirstChildElement(); i; i = i->NextSiblingElement())
	//	{
	//		if (strcmp(i->Value(), "code") == 0)
	//		{
	//			namecode = string(i->FirstChild()->Value());
	//		}
	//		else if (strcmp(i->Value(), "msg") == 0 && namecode == "1")
	//		{
	//			mmsg = string(i->FirstChild()->Value());
	//			p->class_list[p->just_join].dev_name = CMyCharConver::UTF8ToANSI(mmsg);
	//		}
	//	}
	//	//获取播放地址
	//	{
	//		string rcode, rmsg, rtmp_url;
	//		string rurl = "http://" + p->just_join + "/" + login_cgi + "type=queryurl&name=sublurl&token=" + token;
	//		string rres = HttpRequest::request(rurl);
	//		TiXmlDocument xmlrr;
	//		xmlrr.Parse(rres.c_str());
	//		TiXmlNode *rootr = xmlrr.RootElement();
	//		for (TiXmlNode *ir = rootr->FirstChildElement(); ir; ir = ir->NextSiblingElement())
	//		{
	//			if (strcmp(ir->Value(), "code") == 0)
	//			{
	//				rcode = string(ir->FirstChild()->Value());
	//			}
	//			else if (strcmp(ir->Value(), "msg") == 0)
	//			{
	//				if (ir->FirstChild())
	//					rmsg = string(ir->FirstChild()->Value());
	//			}
	//			else if (rmsg == "successed" && strcmp(ir->Value(), "data") == 0)
	//			{
	//				ir = ir->FirstChildElement()->FirstChildElement();
	//				rtmp_url = string(ir->FirstChild()->Value());
	//			}
	//		}
	//		if (!rtmp_url.empty())
	//		{
	//			p->class_list[p->just_join].play_url = rtmp_url.replace(7, 9, p->just_join);
	//			//p->class_list[p->just_join].class_video.video->MediaPlayer->Load(p->class_list[p->just_join].play_url);
	//		}
	//	}

	//	//获取头像
	//	requestUrl = "http://" + p->just_join + "/" + login_cgi + "type=getpicture&token=" + token;
	//	string pres = HttpRequest::request(requestUrl);
	//	TiXmlDocument xmlp;
	//	xmlp.Parse(pres.c_str());
	//	TiXmlNode *rootp = xmlp.RootElement();
	//	for (TiXmlNode *i = rootp->FirstChildElement(); i; i = i->NextSiblingElement())
	//	{
	//		if (strcmp(i->Value(), "code") == 0)
	//		{
	//			icocode = string(i->FirstChild()->Value());
	//		}
	//		else if (strcmp(i->Value(), "msg") == 0 && icocode == "1")
	//		{
	//			pmsg = string(i->FirstChild()->Value());
	//			p->class_list[p->just_join].picture_path = CMyCharConver::UTF8ToANSI(pmsg);
	//		}
	//	}
	//	//download picture
	//	if (!p->class_list[p->just_join].picture_path.empty())
	//	{
	//		ICjrCurl *cjrcurl = ICjrCurl::GetInstance();
	//		cjrcurl->Download("http://" + p->just_join + "/" + p->class_list[p->just_join].picture_path, p->just_join + "/" + p->class_list[p->just_join].picture_path, "");

	//	}
	//}
	////控件关联
	//p->class_list[p->just_join].class_ctrl.btn_connect = p->classRoom[p->class_list.size() - 1].btn_connect;
	//p->class_list[p->just_join].class_ctrl.btn_ico = p->classRoom[p->class_list.size() - 1].btn_ico;
	//p->class_list[p->just_join].class_ctrl.lab_ip = p->classRoom[p->class_list.size() - 1].lab_ip;
	//p->class_list[p->just_join].class_ctrl.lab_title = p->classRoom[p->class_list.size() - 1].lab_title;
	//p->class_list[p->just_join].class_ctrl.lay = p->classRoom[p->class_list.size() - 1].lay;

	//p->class_list[p->just_join].class_video.btn_sound_off = p->subVideo[p->class_list.size() - 1].btn_sound_off;
	//p->class_list[p->just_join].class_video.btn_sound = p->subVideo[p->class_list.size() - 1].btn_sound;
	//p->class_list[p->just_join].class_video.btn_student = p->subVideo[p->class_list.size() - 1].btn_student;
	//p->class_list[p->just_join].class_video.btn_teacher = p->subVideo[p->class_list.size() - 1].btn_teacher;
	//p->class_list[p->just_join].class_video.lab_title = p->subVideo[p->class_list.size() - 1].lab_title;
	//p->class_list[p->just_join].class_video.video = p->subVideo[p->class_list.size() - 1].video;
	//p->PostMessageA(WM_CLIENT_ADDED, 0, 0);

	return 0;
}
DWORD WINAPI updatePicture_Proc(_In_ LPVOID paramer)
{
	MainView *p = (MainView*)paramer;

	//string icocode, pmsg;
	//string requestUrl = "http://" + p->just_update_pic + "/" + login_cgi + "type=getpicture&token=" + LoginWnd::getToken(p->just_update_pic);
	//string pres = HttpRequest::request(requestUrl);
	//TiXmlDocument xmlp;
	//xmlp.Parse(pres.c_str());
	//TiXmlNode *rootp = xmlp.RootElement();
	//for (TiXmlNode *i = rootp->FirstChildElement(); i; i = i->NextSiblingElement())
	//{
	//	if (strcmp(i->Value(), "code") == 0)
	//	{
	//		icocode = string(i->FirstChild()->Value());
	//	}
	//	else if (strcmp(i->Value(), "msg") == 0 && icocode == "1")
	//	{
	//		pmsg = string(i->FirstChild()->Value());
	//		p->class_list[p->just_update_pic].picture_path = CMyCharConver::UTF8ToANSI(pmsg);
	//	}
	//}
	////download picture
	//if (!p->class_list[p->just_update_pic].picture_path.empty())
	//{
	//	ICjrCurl *cjrcurl = ICjrCurl::GetInstance();
	//	cjrcurl->Download("http://" + p->just_update_pic + "/" + p->class_list[p->just_update_pic].picture_path, p->just_update_pic + "/" + p->class_list[p->just_update_pic].picture_path, "");
	//	p->class_list[p->just_update_pic].class_ctrl.btn_ico->SetBkImage((p->just_update_pic +"/"+ p->class_list[p->just_update_pic].picture_path).c_str());
	//	p->class_list[p->just_update_pic].class_ctrl.btn_ico->Invalidate();

	//}
	return 0;
}
DWORD WINAPI LiveViewSwitch(_In_ LPVOID paramer)
{
	MainView *p = (MainView*)paramer;
	//std::string logincode, namecode, icocode, mmsg, token, pmsg;
	//// 登陆获得token
	//std::string requestUrl = "http://" + p->just_join + "/" + login_cgi + "type=login&userName=" + admin_user + "&password=" + admin_passwd;
	//std::string res = HttpRequest::request(requestUrl);
	//TiXmlDocument xml;
	//xml.Parse(res.c_str());
	//TiXmlNode *root = xml.RootElement();
	//for (TiXmlNode *i = root->FirstChildElement(); i; i = i->NextSiblingElement())
	//{
	//	if (strcmp(i->Value(), "code") == 0)
	//	{
	//		logincode = string(i->FirstChild()->Value());
	//	}
	//	else if (strcmp(i->Value(), "msg") == 0)
	//	{
	//		token = string(i->FirstChild()->Value());
	//	}
	//}
	//if (logincode == "1")
	//{
	//	//获取名称
	//	requestUrl = "http://" + p->just_join + "/" + login_cgi + "type=getdevname&token=" + token;
	//	res = HttpRequest::request(requestUrl);
	//	TiXmlDocument xmll;
	//	xmll.Parse(res.c_str());
	//	TiXmlNode *root = xmll.RootElement();
	//	for (TiXmlNode *i = root->FirstChildElement(); i; i = i->NextSiblingElement())
	//	{
	//		if (strcmp(i->Value(), "code") == 0)
	//		{
	//			namecode = string(i->FirstChild()->Value());
	//		}
	//		else if (strcmp(i->Value(), "msg") == 0 && namecode == "1")
	//		{
	//			mmsg = string(i->FirstChild()->Value());
	//			p->class_list[p->just_join].dev_name = CMyCharConver::UTF8ToANSI(mmsg);
	//		}
	//	}
	//}
	return 0;
}

DWORD WINAPI initList(_In_ LPVOID paramer)
{

	return 0;
}


DWORD WINAPI initLocalUrl(_In_ LPVOID paramer)
{
	MainView* p = (MainView*)paramer;
	
	////获取本地播放地址
	//string rcode, rmsg, rtmp_url;
	//string rurl = "http://" + login_ip + "/" + login_cgi + "type=queryurl&name=sublurl&token=" + login_token;
	//string rres = HttpRequest::request(rurl);
	//TiXmlDocument xmlrr;
	//xmlrr.Parse(rres.c_str());
	//TiXmlNode *rootr = xmlrr.RootElement();
	//for (TiXmlNode *ir = rootr->FirstChildElement(); ir; ir = ir->NextSiblingElement())
	//{
	//	if (strcmp(ir->Value(), "code") == 0)
	//	{
	//		rcode = string(ir->FirstChild()->Value());
	//	}
	//	else if (strcmp(ir->Value(), "msg") == 0)
	//	{
	//		if (ir->FirstChild())
	//			rmsg = string(ir->FirstChild()->Value());
	//	}
	//	else if (rmsg == "successed" && strcmp(ir->Value(), "data") == 0)
	//	{
	//		ir = ir->FirstChildElement()->FirstChildElement();
	//		rtmp_url = string(ir->FirstChild()->Value());
	//	}
	//}
	//if (!rtmp_url.empty())
	//	p->local_url = rtmp_url.replace(7, 9, login_ip);
	////获取本地导播流
	//string dcode, dmsg, durl;
	//string d_request_url = "http://" + login_ip + "/" + login_cgi + "type=queryurl&name=durl&token=" + login_token;
	//string d_res = HttpRequest::request(d_request_url);
	//TiXmlDocument d_xml;
	//d_xml.Parse(d_res.c_str());
	//TiXmlNode *vga_root = d_xml.RootElement();
	//for (TiXmlNode *ir = vga_root->FirstChildElement(); ir; ir = ir->NextSiblingElement())
	//{
	//	if (strcmp(ir->Value(), "code") == 0)
	//	{
	//		dcode = string(ir->FirstChild()->Value());
	//	}
	//	else if (strcmp(ir->Value(), "msg") == 0)
	//	{
	//		if (ir->FirstChild())
	//			dmsg = string(ir->FirstChild()->Value());
	//	}
	//	else if (dmsg == "successed" && strcmp(ir->Value(), "data") == 0)
	//	{
	//		ir = ir->FirstChildElement()->FirstChildElement();
	//		durl = string(ir->FirstChild()->Value());
	//	}
	//}
	//if (!durl.empty())
	//	p->local_durl = durl.replace(7, 9, login_ip);
	////获取VGA流
	//string vgacode, vgamsg, vga_url;
	//string vga_request_url = "http://" + login_ip + "/" + login_cgi + "type=queryurl&name=vgaurl&token=" + login_token;
	//string vga_res = HttpRequest::request(vga_request_url);
	//TiXmlDocument vag_xml;
	//vag_xml.Parse(vga_res.c_str());
	//TiXmlNode *d_root = vag_xml.RootElement();
	//for (TiXmlNode *ir = d_root->FirstChildElement(); ir; ir = ir->NextSiblingElement())
	//{
	//	if (strcmp(ir->Value(), "code") == 0)
	//	{
	//		vgacode = string(ir->FirstChild()->Value());
	//	}
	//	else if (strcmp(ir->Value(), "msg") == 0)
	//	{
	//		if (ir->FirstChild())
	//			vgamsg = string(ir->FirstChild()->Value());
	//	}
	//	else if (vgamsg == "successed" && strcmp(ir->Value(), "data") == 0)
	//	{
	//		ir = ir->FirstChildElement()->FirstChildElement();
	//		vga_url = string(ir->FirstChild()->Value());
	//	}
	//}
	//if (!vga_url.empty())
	//	p->vga_url = vga_url.replace(7, 9, login_ip);
	//Sleep(300);
	//CVideoUI *video = static_cast<CVideoUI*>(p->m_PaintManager.FindControl(_T("mainVideo")));
	//	video->MediaPlayer->Load(p->local_url);
	//	p->subVideo[4].video->MediaPlayer->Load(p->local_durl);
	//	p->subVideo[4].lab_title->SetText(_T("本地"));
	//	p->subVideo[5].video->MediaPlayer->Load(p->vga_url);
	//	p->subVideo[5].lab_title->SetText(_T("PPT"));
return 0;
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
			if (IDOK == TipMsg::ShowMsgWindow(*this, _T("确实要退出？"), _T("提示")))
			{
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
		
		


	}
	if (msg.sType == DUI_MSGTYPE_DBCLICK)
	{
		if (msg.pSender->GetName() == _T("mainVideo"))
		{
			CHorizontalLayoutUI* hor = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("SubVideoLay")));
			hor->SetVisible(!hor->IsVisible());
			for (int i = 1; i <= 6; i++)
			{
				char name[123] = "video";
				sprintf(name,"video%d",i);
				CVideoUI * video = static_cast<CVideoUI*>(m_PaintManager.FindControl(name));
				video->SetVisible(hor->IsVisible());
			}	
		}
		/*
		else if (msg.pSender->GetName() == _T("video1"))
		{
			static bool is_play = false;
			if (!is_play)
				subVideo[0].video->play("rtmp://192.168.8.236:1935/live/slive");
			else
				subVideo[0].video->stop();
			is_play=!is_play;
		}
		else if (msg.pSender->GetName() == _T("video2"))
		{
			static bool is_play = false;
			if (!is_play)
				subVideo[1].video->play("rtmp://192.168.8.236:1935/live/slive");
			else
				subVideo[1].video->stop();
			is_play=!is_play;
		}
		else if (msg.pSender->GetName() == _T("video3"))
		{
			static bool is_play = false;
			if (!is_play)
				subVideo[2].video->play("rtmp://192.168.8.236:1935/live/slive");
			else
				subVideo[2].video->stop();
			is_play=!is_play;
		}
		else if (msg.pSender->GetName() == _T("video4"))
		{
			static bool is_play = false;
			if (!is_play)
				subVideo[3].video->play("rtmp://192.168.8.236:1935/live/slive");
			else
				subVideo[3].video->stop();
			is_play=!is_play;
		}
		else if (msg.pSender->GetName() == _T("video5"))
		{
			static bool is_play = false;
			if (!is_play)
				subVideo[4].video->play("rtmp://192.168.8.236:1935/live/slive");
			else
				subVideo[4].video->stop();
			is_play=!is_play;
		}*/
	}
	else if (msg.sType==_T("online"))
	{
		CHorizontalLayoutUI* hor_notify = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("NotifyLay")));
		hor_notify->SetVisible(true);
		PlaySound(_T("mgg.wav"), NULL, SND_ASYNC);
		SetTimer(*this, TIME_ID_NOTIFY, 400, NULL);
	}
}



CControlUI* MainView::CreateControl(LPCTSTR pstrClass)
{
	if (_tcscmp(pstrClass, _T("Video")) == 0)
		return new CVideoUI();
	else  if (_tcscmp(pstrClass, _T("ICOImage")) == 0)
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
	else if (uMsg == WM_REQUEST_JOIN)
	{
		if (IDOK == TipMsg::ShowMsgWindow(*this, class_list[just_join_member].name+"请求加入", ""))
		{
			classItemUI *item = new classItemUI;
			item->setIp(just_join_member.c_str());
			item->setTitle(class_list[just_join_member].name.c_str());
			if (!class_list[just_join_member].path.empty())
				item->setImage(class_list[just_join_member].path.c_str());
			ManagerItem::Add(m_pCalssLay, item);
			msg_coming(class_list[just_join_member].name + "上线了");
			std::string sstr = "type=JoinMeeting&ip=" + just_join_member + "&name=" + class_list[just_join_member].name;
			char data[50];
			strcpy(data, sstr.c_str());
			client->sendData(data,strlen(data));
			// play video
			class_list[just_join_member].media = video_list[class_list.size() - 2];
			class_list[just_join_member].media.video->stop();
			//class_list[just_join_member].media.video->play(class_list[just_join_member].url);
			video_list[1].video->play(class_list[just_join_member].url);
		}
		else
		{
			class_list.erase(class_list.find(just_join_member));
		}
	}
	else if (uMsg == WM_REQUEST_SPEAK)
	{
		if (IDOK == TipMsg::ShowMsgWindowTime(*this, 5000, class_list[just_join_speak].name+"请求发言"))
		{
			selected_speak(just_join_speak);
		}
	}
	else if (uMsg==WM_UPDATE_DEVNAME)
	{
		string s = "type=UpdateDevName&ip="+user_list::ip+"&name="+class_list[user_list::ip].name;
		char data[50];
		strcpy(data, s.c_str());
		client->sendData(data,strlen(data));
	}
	else if (uMsg == WM_UPDATE_ICO)
	{
		string s = "type=UpdatePicture&ip=" + user_list::ip;
		char data[50];
		strcpy(data, s.c_str());
		client->sendData(data, strlen(data));
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
	else if (wParam == 1111)
	{
		CLabelUI*lab = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_notify")));
		lab->SetText(_T("欢迎使用课堂助手讲课端"));
		m_PaintManager.SendNotify(lab, _T("online"));
		KillTimer(*this, 1111);
	}
	else if (wParam == TIME_ID_NOTIFY)
	{
		CLabelUI*lab = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_notify")));
		LPCTSTR s=lab->GetText().GetData();
		TCHAR title[200];
		if (lab->GetText().GetLength() <= 2)
		{
			KillTimer(*this, TIME_ID_NOTIFY);
			CHorizontalLayoutUI* hor_notify = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("NotifyLay")));
			hor_notify->SetVisible(false);

		}
		_tcscpy(title, (s + 2));
		lab->SetText(title);
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
	m_pStateOn = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status_on")));
	m_pStateOff = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status")));
	m_pVideo = static_cast<CVideoUI*>(m_PaintManager.FindControl(_T("mainVideo")));
	init_videoList();
	/* hide notify lay*/
	m_pNotifyLay->SetVisible(false);
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
		ConfigFile cf(CFG_FILE);
		cf.addValue("name", name, ip);
		cf.save();
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
	if (picPath.empty())
	{
		try
		{
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
	std::string name = cf.getValue("name", ip);
	if (name.empty())
	{
		try
		{
			update_name(ip);
		}
		catch (std::exception& e)
		{
			throw e;
		}
	}
	else
	{
		class_list[ip].name = name;
	}
}

void MainView::update_pic(const std::string ip)
{
	std::string requestPicNameUrl = "http://" + ip + "/" + user_list::cgi + "type=getpicture";
	try
	{
		std::string picName = Logan::query_msg_node(requestPicNameUrl, ip);
		std::string downloadUrl = "http://" + ip + "/" + picName;
		CreateDirectoryA(ip.c_str(), NULL);
		if (!picName.empty())// if no picture on server
		{
			std::string path = ip + "/" + picName;
			if (!Logan::download(downloadUrl, path))
			{
				throw std::exception("download error");
			}
			ConfigFile cf(CFG_FILE);
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
	std::string requestUrl = "http://" + ip + "/" + user_list::cgi + "type=queryurl&name=sublurl";
	try
	{
		std::string url = Logan::query_url(requestUrl, ip);
		if (url.length() > 9)
		{
			url = url.replace(7, 9, ip);
			class_list[ip].url = url;
		}
	}
	catch (std::exception& e)
	{
		throw e;
	}
}


void MainView::get_vgaurl(const std::string ip)
{
	std::string requestUrl = "http://" + ip + "/" + user_list::cgi + "type=queryurl&name=vgaurl";
	try
	{
		std::string url = Logan::query_url(requestUrl, ip);
		if (url.length() > 9)
		{
			url = url.replace(7, 9, ip);
			vga_url = url;
		}
	}
	catch (std::exception& e)
	{
		throw e;
	}
}

void MainView::get_durl(const std::string ip)
{
	std::string requestUrl = "http://" + ip + "/" + user_list::cgi + "type=queryurl&name=durl";
	try
	{
		std::string url = Logan::query_url(requestUrl, ip);
		if (url.length() > 9)
		{
			url = url.replace(7, 9, ip);
			durl = url;
		}
	}
	catch (std::exception& e)
	{
		throw e;
	}
}

void MainView::selected_speak(const std::string ip)
{
	if (current_speak.empty())
	{
		current_speak.push(class_list[ip]);
	}
	else
	{
		ItemData item = current_speak.top();
		if (class_list[item.ip].media.snd_on)
		{
			class_list[item.ip].media.snd_on->SetVisible(false);
			class_list[item.ip].media.snd_off->SetVisible(true);
		}
	}
	if (class_list[ip].media.snd_on && !ip.empty())
	{
		class_list[ip].media.snd_on->SetVisible(true);
		class_list[ip].media.snd_off->SetVisible(false);
	}
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

	sprintf(ctime, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
	if (m_pTime)
		m_pTime->SetText(ctime);
}

void MainView::SoundStateOn(int index, bool is_on)
{
	if (index < 0 && index>5)
		return;
	else
	{
		video_list[index].snd_on->SetVisible(is_on);
		video_list[index].snd_off->SetVisible(!is_on);
	}
}

void MainView::init_videoList()
{
	for (int i = 0; i < 6; i++)
	{
		char title_name[20];
		sprintf(title_name,"lab_title%d",i+1);
		video_list[i].title = static_cast<CLabelUI*>(m_PaintManager.FindControl(title_name));

		char snd_name[20];
		sprintf(snd_name, "btn_sound%d", i + 1);
		video_list[i].snd_on = static_cast<CButtonUI*>(m_PaintManager.FindControl(snd_name));

		char sndoff_name[20];
		sprintf(sndoff_name, "btn_soundoff%d", i + 1);
		video_list[i].snd_off = static_cast<CButtonUI*>(m_PaintManager.FindControl(sndoff_name));

		char video_name[20];
		sprintf(video_name, "video%d", i + 1);
		video_list[i].video = static_cast<CVideoUI*>(m_PaintManager.FindControl(video_name));
		char teacher_name[20];
		sprintf(teacher_name,"teacher%d",i+1);
		video_list[i].teacher = static_cast<CButtonUI*>(m_PaintManager.FindControl(teacher_name));
		if (i != 5)
		{
			char student_name[20];
			sprintf(student_name, "student%d", i + 1);
			video_list[i].student = static_cast<CButtonUI*>(m_PaintManager.FindControl(student_name));
		}
		else
		{
			video_list[i].student = NULL;
		}

	}
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
		/* first load login-ip info */
		p->load_local(user_list::ip);
		p->get_url(user_list::ip);
		p->get_vgaurl(user_list::ip);
		p->get_durl(user_list::ip);
		/* play main video*/
		if (!p->class_list[user_list::ip].url.empty())
		{
			p->m_pVideo->stop();
			p->m_pVideo->play(p->class_list[user_list::ip].url);
		}
		/* play vga(PPT) stream*/
		p->video_list[5].title->SetText(_T("PPT"));
		if (!p->vga_url.empty())
		{
			p->video_list[5].video->stop();
			p->video_list[5].video->play(p->vga_url);
		}

		/* play director stream*/
		p->video_list[4].title->SetText(_T("本地导播"));
		if (!p->durl.empty())
		{
			p->video_list[4].video->stop();
			p->video_list[4].video->play(p->durl);
		}
		/* then get info from network*/
		for (std::set<std::string>::iterator itor = p->ip_list.begin(); itor != p->ip_list.end(); itor++)
		{
			p->load_local(*itor);
			p->get_url(*itor);
			classItemUI *item = new classItemUI;
			item->setIp(itor->c_str());
			p->class_list[*itor].ip = *itor;
			if (!p->class_list[*itor].path.empty())
				item->setImage(p->class_list[*itor].path.c_str());
			item->setTitle(p->class_list[*itor].name.c_str());
			ManagerItem::Add(p->m_pCalssLay, item);

			static int index = 0;
			p->class_list[*itor].media = p->video_list[index];
			p->class_list[*itor].media.title->SetText(p->class_list[*itor].name.c_str());
			++index;
			if (!p->class_list[*itor].url.empty())
			{
				p->class_list[*itor].media.video->stop();
				p->class_list[*itor].media.video->play(p->class_list[*itor].url);
			}
		}
	}
	catch (std::exception& e)
	{
		p->error_msg = e.what();
		::PostMessageA(*p, WM_ERROR_TIP, NULL, NULL);
	}
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
			if (!p->class_list[p->just_join_update_pic].path.empty())
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
			::PostMessageA(*p, WM_REQUEST_JOIN, NULL, NULL);
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

	for (std::map<std::string, ItemData>::iterator i = p->class_list.begin(); i != p->class_list.end(); i++)
	{
		if (i->first != user_list::ip)
			Logan::logout(i->first, user_list::user_name);
	}
	return 0;
}


/****************************************************/


classItemUI::classItemUI() : m_pIP(NULL),
m_pcnt(NULL), m_pICO(NULL), m_pTitle(NULL)
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
	m_pIP->SetText(pstr_ip);
}

LPCTSTR classItemUI::getIP()const
{
	return m_pIP->GetText();
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
	m_pcnt->SetText(pstrText);
}

LPCTSTR classItemUI::getText()const
{
	return m_pcnt->GetText();
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

	m_pcnt = new CButtonUI();
	m_pcnt->SetFloat(true);
	m_pcnt->SetAttribute("pos", "50,53,120,84");
	m_pcnt->SetHotBkColor(0xff21a1db);
	m_pcnt->SetPushedBkColor(0xff21a1ff);
	m_pcnt->SetBkColor(0xff171717);
	m_pcnt->SetTextColor(0xffffffff);
	m_pcnt->SetBorderRound(s);
	m_pcnt->SetText(_T("连接"));
	this->Add(m_pcnt);
}

/************************************************************/

std::map<CDuiString, ManagerItem::UI> ManagerItem::mgr;

void ManagerItem::Add(CContainerUI* pContain, classItemUI* item)
{
	if (mgr.find(item->getIP()) != mgr.end())
	{
		Remove(pContain, item->getIP());
	}
	ManagerItem::UI u;
	u.item = item;
	u.sep = new sepUI(5);
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