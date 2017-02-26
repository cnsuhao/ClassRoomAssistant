#include "MainView.h"
#include "../Video/Video.h"
#include "../SettingView/SettingView.h"
#include "../NotifyView/NotifyWnd.h"
#include "../Login/LoginWnd.h"
#include "../xml/tinyxml.h"
#include "../xml/tinystr.h"
#include "../CMyCharConver.h"
#include "../CjrCurl/IMyCurl.h"
#define  TIME_ID_UPDATE_TIME	1001
#define  TIME_ID_NOTIFY			1002


#define WM_CLIENT_ADDED		WM_USER+100



MainView::MainView() :lab_date(NULL), lab_time(NULL), updateMenberJoin_thread(NULL), updatePicture_thread(NULL),
switch_view_thread(NULL), current_view(-1), current_index(-1), client(NULL)
{
	if (!client)
	{
		client = ITCPClient::getInstance();
		client->open(login_ip.c_str(), 4507);
		client->setListener(this);
	}
	local_thread = CreateThread(NULL, 0, initLocalUrl, this, NULL, NULL);
}
MainView::~MainView()
{
	if (client)
	{
		client->close();
		ITCPClient::releaseInstance(client);
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
			string iplist = res["ip"];
			int pos = -1;
			while ((pos = iplist.find_first_of(';')) != -1)
			{
				ip_table.push_back(iplist.substr(0, pos));
				iplist.erase(0, pos + 1);
			}
			while (!ip_table.empty())
			{
				just_join = ip_table.back();
				if (!updateMenberJoin_thread)
				{
					updateMenberJoin_thread = CreateThread(NULL, 0, updateJoin_Proc, this, NULL, NULL);
				}
				else
				{
					WaitForSingleObject(updateMenberJoin_thread, 3000);
					CloseHandle(updateMenberJoin_thread);
					updateMenberJoin_thread = CreateThread(NULL, 0, updateJoin_Proc, this, NULL, NULL);
				}
				ip_table.pop_back();
				Sleep(200);
			}


			
		}
		else if (res["type"] == "UpdateDevName")
		{
			//更新设备名称
			class_list[res["ip"]].dev_name = res["name"];
			class_list[res["ip"]].class_ctrl.lab_title->SetText(res["name"].c_str());
			class_list[res["ip"]].class_ctrl.lab_title->Invalidate();
		}
		else if (res["type"] == "UpdatePicture")
		{
			if (class_list.find(res["ip"]) == class_list.end())
				return;
			just_update_pic = res["ip"];
			if (!updatePicture_thread)
			{
				updatePicture_thread = CreateThread(NULL, 0, updatePicture_Proc, this, NULL, NULL);
			}
			else
			{
				WaitForSingleObject(updatePicture_thread, 3000);
				CloseHandle(updatePicture_thread);
				updatePicture_thread = CreateThread(NULL, 0, updatePicture_Proc, this, NULL, NULL);
			}
		}
		else if (res["type"] == "JoinMeeting")
		{
			//讲课端无此消息
		}
		else if (res["type"] == "QuitMeeting")
		{
			//收到听课端退出的消息
			if(class_list.find(res["ip"])!=class_list.end())
			{
				delete_classUI(class_list[res["ip"]], class_list.size());
				TipMsg::ShowMsgWindowTime(*this, 1000,class_list[res["ip"]].dev_name+"退出听课" ,"提示");
				class_list[res["ip"]].class_video.video->stop();
				class_list[res["ip"]].class_video.video->flushBk();
				class_list.erase(class_list.find(res["ip"]));

			}
			if (class_list.size() == 0)
			{
				CLabelUI* lab_ico = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status")));
				lab_ico->SetVisible(true);

				CLabelUI* lab_icoon = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status_on")));
				lab_icoon->SetVisible(false);
			}

		}
		else if (res["type"] == "SelChat")
		{
			//讲课端 不需要
		}
		else if (res["type"] == "RequestJoin")
		{
			just_join = res["ip"];
			if (!updateMenberJoin_thread)
			{
				updateMenberJoin_thread = CreateThread(NULL, 0, updateJoin_Proc, this, NULL, NULL);
			}
			else
			{
				WaitForSingleObject(updateMenberJoin_thread,3000);
				CloseHandle(updateMenberJoin_thread);
				updateMenberJoin_thread = CreateThread(NULL, 0, updateJoin_Proc, this, NULL, NULL);
			}
		}
		else if (res["type"] == "RequestSpeak")
		{
			just_speak_ip = res["ip"];
			::PostMessageA(*this, WM_SELECT_SPEAK, NULL, NULL);
		}
	}
}

DWORD WINAPI updateJoin_Proc(_In_ LPVOID paramer)
{
	MainView *p = (MainView*)paramer;
	p->class_list[p->just_join];
	CreateDirectoryA(p->just_join.c_str(), NULL);
	std::string logincode, namecode, icocode, mmsg, token, pmsg;
	// 登陆获得token
	std::string requestUrl = "http://" + p->just_join + "/" + login_cgi + "type=login&userName=" + admin_user + "&password=" + admin_passwd;
	std::string res = HttpRequest::request(requestUrl);
	TiXmlDocument xml;
	xml.Parse(res.c_str());
	TiXmlNode *root = xml.RootElement();
	for (TiXmlNode *i = root->FirstChildElement(); i; i = i->NextSiblingElement())
	{
		if (strcmp(i->Value(), "code") == 0)
		{
			logincode = string(i->FirstChild()->Value());
		}
		else if (strcmp(i->Value(), "msg") == 0)
		{
			token = string(i->FirstChild()->Value());
		}
	}
	if (logincode == "1")
	{
		//获取名称
		requestUrl = "http://" + p->just_join + "/" + login_cgi + "type=getdevname&token=" + token;
		res = HttpRequest::request(requestUrl);
		TiXmlDocument xmll;
		xmll.Parse(res.c_str());
		TiXmlNode *root = xmll.RootElement();
		for (TiXmlNode *i = root->FirstChildElement(); i; i = i->NextSiblingElement())
		{
			if (strcmp(i->Value(), "code") == 0)
			{
				namecode = string(i->FirstChild()->Value());
			}
			else if (strcmp(i->Value(), "msg") == 0 && namecode == "1")
			{
				mmsg = string(i->FirstChild()->Value());
				p->class_list[p->just_join].dev_name = CMyCharConver::UTF8ToANSI(mmsg);
			}
		}
		//获取播放地址
		{
			string rcode, rmsg, rtmp_url;
			string rurl = "http://" + p->just_join + "/" + login_cgi + "type=queryurl&name=sublurl&token=" + token;
			string rres = HttpRequest::request(rurl);
			TiXmlDocument xmlrr;
			xmlrr.Parse(rres.c_str());
			TiXmlNode *rootr = xmlrr.RootElement();
			for (TiXmlNode *ir = rootr->FirstChildElement(); ir; ir = ir->NextSiblingElement())
			{
				if (strcmp(ir->Value(), "code") == 0)
				{
					rcode = string(ir->FirstChild()->Value());
				}
				else if (strcmp(ir->Value(), "msg") == 0)
				{
					if (ir->FirstChild())
						rmsg = string(ir->FirstChild()->Value());
				}
				else if (rmsg == "successed" && strcmp(ir->Value(), "data") == 0)
				{
					ir = ir->FirstChildElement()->FirstChildElement();
					rtmp_url = string(ir->FirstChild()->Value());
				}
			}
			if (!rtmp_url.empty())
			{
				p->class_list[p->just_join].play_url = rtmp_url.replace(7, 9, p->just_join);
				//p->class_list[p->just_join].class_video.video->MediaPlayer->Load(p->class_list[p->just_join].play_url);
			}
		}

		//获取头像
		requestUrl = "http://" + p->just_join + "/" + login_cgi + "type=getpicture&token=" + token;
		string pres = HttpRequest::request(requestUrl);
		TiXmlDocument xmlp;
		xmlp.Parse(pres.c_str());
		TiXmlNode *rootp = xmlp.RootElement();
		for (TiXmlNode *i = rootp->FirstChildElement(); i; i = i->NextSiblingElement())
		{
			if (strcmp(i->Value(), "code") == 0)
			{
				icocode = string(i->FirstChild()->Value());
			}
			else if (strcmp(i->Value(), "msg") == 0 && icocode == "1")
			{
				pmsg = string(i->FirstChild()->Value());
				p->class_list[p->just_join].picture_path = CMyCharConver::UTF8ToANSI(pmsg);
			}
		}
		//download picture
		if (!p->class_list[p->just_join].picture_path.empty())
		{
			ICjrCurl *cjrcurl = ICjrCurl::GetInstance();
			cjrcurl->Download("http://" + p->just_join + "/" + p->class_list[p->just_join].picture_path, p->just_join + "/" + p->class_list[p->just_join].picture_path, "");

		}
	}
	//控件关联
	p->class_list[p->just_join].class_ctrl.btn_connect = p->classRoom[p->class_list.size() - 1].btn_connect;
	p->class_list[p->just_join].class_ctrl.btn_ico = p->classRoom[p->class_list.size() - 1].btn_ico;
	p->class_list[p->just_join].class_ctrl.lab_ip = p->classRoom[p->class_list.size() - 1].lab_ip;
	p->class_list[p->just_join].class_ctrl.lab_title = p->classRoom[p->class_list.size() - 1].lab_title;
	p->class_list[p->just_join].class_ctrl.lay = p->classRoom[p->class_list.size() - 1].lay;

	p->class_list[p->just_join].class_video.btn_sound_off = p->subVideo[p->class_list.size() - 1].btn_sound_off;
	p->class_list[p->just_join].class_video.btn_sound = p->subVideo[p->class_list.size() - 1].btn_sound;
	p->class_list[p->just_join].class_video.btn_student = p->subVideo[p->class_list.size() - 1].btn_student;
	p->class_list[p->just_join].class_video.btn_teacher = p->subVideo[p->class_list.size() - 1].btn_teacher;
	p->class_list[p->just_join].class_video.lab_title = p->subVideo[p->class_list.size() - 1].lab_title;
	p->class_list[p->just_join].class_video.video = p->subVideo[p->class_list.size() - 1].video;
	p->PostMessageA(WM_CLIENT_ADDED, 0, 0);

	return 0;
}
DWORD WINAPI updatePicture_Proc(_In_ LPVOID paramer)
{
	MainView *p = (MainView*)paramer;

	string icocode, pmsg;
	string requestUrl = "http://" + p->just_update_pic + "/" + login_cgi + "type=getpicture&token=" + LoginWnd::getToken(p->just_update_pic);
	string pres = HttpRequest::request(requestUrl);
	TiXmlDocument xmlp;
	xmlp.Parse(pres.c_str());
	TiXmlNode *rootp = xmlp.RootElement();
	for (TiXmlNode *i = rootp->FirstChildElement(); i; i = i->NextSiblingElement())
	{
		if (strcmp(i->Value(), "code") == 0)
		{
			icocode = string(i->FirstChild()->Value());
		}
		else if (strcmp(i->Value(), "msg") == 0 && icocode == "1")
		{
			pmsg = string(i->FirstChild()->Value());
			p->class_list[p->just_update_pic].picture_path = CMyCharConver::UTF8ToANSI(pmsg);
		}
	}
	//download picture
	if (!p->class_list[p->just_update_pic].picture_path.empty())
	{
		ICjrCurl *cjrcurl = ICjrCurl::GetInstance();
		cjrcurl->Download("http://" + p->just_update_pic + "/" + p->class_list[p->just_update_pic].picture_path, p->just_update_pic + "/" + p->class_list[p->just_update_pic].picture_path, "");
		p->class_list[p->just_update_pic].class_ctrl.btn_ico->SetBkImage((p->just_update_pic +"/"+ p->class_list[p->just_update_pic].picture_path).c_str());
		p->class_list[p->just_update_pic].class_ctrl.btn_ico->Invalidate();

	}
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
	
	//获取本地播放地址
	string rcode, rmsg, rtmp_url;
	string rurl = "http://" + login_ip + "/" + login_cgi + "type=queryurl&name=sublurl&token=" + login_token;
	string rres = HttpRequest::request(rurl);
	TiXmlDocument xmlrr;
	xmlrr.Parse(rres.c_str());
	TiXmlNode *rootr = xmlrr.RootElement();
	for (TiXmlNode *ir = rootr->FirstChildElement(); ir; ir = ir->NextSiblingElement())
	{
		if (strcmp(ir->Value(), "code") == 0)
		{
			rcode = string(ir->FirstChild()->Value());
		}
		else if (strcmp(ir->Value(), "msg") == 0)
		{
			if (ir->FirstChild())
				rmsg = string(ir->FirstChild()->Value());
		}
		else if (rmsg == "successed" && strcmp(ir->Value(), "data") == 0)
		{
			ir = ir->FirstChildElement()->FirstChildElement();
			rtmp_url = string(ir->FirstChild()->Value());
		}
	}
	if (!rtmp_url.empty())
		p->local_url = rtmp_url.replace(7, 9, login_ip);
	//获取本地导播流
	string dcode, dmsg, durl;
	string d_request_url = "http://" + login_ip + "/" + login_cgi + "type=queryurl&name=durl&token=" + login_token;
	string d_res = HttpRequest::request(d_request_url);
	TiXmlDocument d_xml;
	d_xml.Parse(d_res.c_str());
	TiXmlNode *vga_root = d_xml.RootElement();
	for (TiXmlNode *ir = vga_root->FirstChildElement(); ir; ir = ir->NextSiblingElement())
	{
		if (strcmp(ir->Value(), "code") == 0)
		{
			dcode = string(ir->FirstChild()->Value());
		}
		else if (strcmp(ir->Value(), "msg") == 0)
		{
			if (ir->FirstChild())
				dmsg = string(ir->FirstChild()->Value());
		}
		else if (dmsg == "successed" && strcmp(ir->Value(), "data") == 0)
		{
			ir = ir->FirstChildElement()->FirstChildElement();
			durl = string(ir->FirstChild()->Value());
		}
	}
	if (!durl.empty())
		p->local_durl = durl.replace(7, 9, login_ip);
	//获取VGA流
	string vgacode, vgamsg, vga_url;
	string vga_request_url = "http://" + login_ip + "/" + login_cgi + "type=queryurl&name=vgaurl&token=" + login_token;
	string vga_res = HttpRequest::request(vga_request_url);
	TiXmlDocument vag_xml;
	vag_xml.Parse(vga_res.c_str());
	TiXmlNode *d_root = vag_xml.RootElement();
	for (TiXmlNode *ir = d_root->FirstChildElement(); ir; ir = ir->NextSiblingElement())
	{
		if (strcmp(ir->Value(), "code") == 0)
		{
			vgacode = string(ir->FirstChild()->Value());
		}
		else if (strcmp(ir->Value(), "msg") == 0)
		{
			if (ir->FirstChild())
				vgamsg = string(ir->FirstChild()->Value());
		}
		else if (vgamsg == "successed" && strcmp(ir->Value(), "data") == 0)
		{
			ir = ir->FirstChildElement()->FirstChildElement();
			vga_url = string(ir->FirstChild()->Value());
		}
	}
	if (!vga_url.empty())
		p->vga_url = vga_url.replace(7, 9, login_ip);
	Sleep(300);
	CVideoUI *video = static_cast<CVideoUI*>(p->m_PaintManager.FindControl(_T("mainVideo")));
		video->MediaPlayer->Load(p->local_url);
		p->subVideo[4].video->MediaPlayer->Load(p->local_durl);
		p->subVideo[4].lab_title->SetText(_T("本地"));
		p->subVideo[5].video->MediaPlayer->Load(p->vga_url);
		p->subVideo[5].lab_title->SetText(_T("PPT"));
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
		
		
		//点击老师时切换到view1
		char teacher_name[30];
		for (int i = 1; i <= 5; i++)
		{
			//switch live stream to director teacher
			sprintf(teacher_name,"teacher%d",i);
			if (msg.pSender->GetName() == teacher_name)
			{
				cut_view(i, 0);
				break;
			}
		}
		//点击学生时
		char student_name[30];
		for (int i = 1; i <= 5; i++)
		{
			//switch live stream to play student
			sprintf(student_name, "student%d", i);
			if (msg.pSender->GetName() == student_name)
			{
				cut_view(i, 1);
				break;
			}
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

void MainView::delete_classUI(_classinfo info, int current_mount)
{
	if(current_mount==0)
		return;
	CDuiString ver_name = info.class_ctrl.lay->GetName();
	int index = -1;
	if (_tcscmp(ver_name, _T("vv1")))
	{
		index = 1;
	}
	else if (_tcscmp(ver_name, _T("vv2")))
	{
		index = 2;
	}
	else if (_tcscmp(ver_name, _T("vv3")))
	{
		index = 3;
	}
	else if (_tcscmp(ver_name, _T("vv4")))
	{
		index = 4;
	}

	for (int i = index; i != -1 && i < current_mount; i++)
	{
		classRoom[i].btn_connect->SetText(classRoom[i+1].btn_connect->GetText());
		classRoom[i].btn_ico->SetBkImage(classRoom[i+1].btn_ico->GetBkImage());
		classRoom[i].lab_ip->SetText(classRoom[i+1].lab_ip->GetText());
		classRoom[i].lab_title->SetText(classRoom[i+1].lab_title->GetText());
	}
	char hide_verName[20];
	sprintf(hide_verName, "vv%d", current_mount);
	CVerticalLayoutUI* hide_ver = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(hide_verName));
	hide_ver->SetVisible(false);
}
void MainView::add_classUI(_classinfo info)
{
	info.class_ctrl.lay->SetVisible(true);
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
	else if (uMsg == WM_CLIENT_ADDED)
	{
		if (is_init)
		{
			class_list[just_join].class_ctrl.lay->SetVisible(true);
			class_list[just_join].class_ctrl.btn_ico->SetBkImage((just_join + "/" + class_list[just_join].picture_path).c_str());
			class_list[just_join].class_ctrl.lab_ip->SetText(just_join.c_str());
			class_list[just_join].class_ctrl.lab_title->SetText(class_list[just_join].dev_name.c_str());
			class_list[just_join].class_video.lab_title->SetText(class_list[just_join].dev_name.c_str());
			class_list[just_join].class_video.video->MediaPlayer->Load(class_list[just_join].play_url);
			lab_notice->SetText((class_list[just_join].dev_name + "上线啦").c_str());
			m_PaintManager.SendNotify(lab_notice, _T("online"));
			string ss = "type=JoinMeeting&ip=" + just_join + "&name=" + class_list[just_join].dev_name;
			char data[50];
			strcpy(data, ss.c_str());
			client->sendData(data, strlen(data));

			CLabelUI* lab_ico = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status")));
			lab_ico->SetVisible(false);

			CLabelUI* lab_icoon = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status_on")));
			lab_icoon->SetVisible(true);

			if (ip_table.empty())
				is_init = false;
		}

		if (IDOK == TipMsg::ShowMsgWindow(*this, (class_list[just_join].dev_name+"申请加入听课").c_str(), _T("提示")))
		{
			class_list[just_join].class_ctrl.lay->SetVisible(true);
			class_list[just_join].class_ctrl.btn_ico->SetBkImage((just_join+"/"+class_list[just_join].picture_path).c_str());
			class_list[just_join].class_ctrl.lab_ip->SetText(just_join.c_str());
			class_list[just_join].class_ctrl.lab_title->SetText(class_list[just_join].dev_name.c_str());
			class_list[just_join].class_video.lab_title->SetText(class_list[just_join].dev_name.c_str());
			class_list[just_join].class_video.video->MediaPlayer->Load(class_list[just_join].play_url);
			lab_notice->SetText((class_list[just_join].dev_name+"上线啦").c_str());
			m_PaintManager.SendNotify(lab_notice, _T("online"));
			string ss = "type=JoinMeeting&ip=" + just_join + "&name=" + class_list[just_join].dev_name;
			char data[50];
			strcpy(data, ss.c_str());
			client->sendData(data, strlen(data));

			CLabelUI* lab_ico = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status")));
			lab_ico->SetVisible(false);

			CLabelUI* lab_icoon = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_class_status_on")));
			lab_icoon->SetVisible(true);
		}
	}
	else if (uMsg==WM_UPDATE_DEVNAME)
	{
		string s = "type=UpdateDevName&ip="+login_ip+"&name="+dev_name;
		char data[50];
		strcpy(data, s.c_str());
		if (!client)
		{
			client = ITCPClient::getInstance();
			client->open(login_ip.c_str(), 4507);
			client->setListener(this);
		}
		client->sendData(data,strlen(data));
	}
	else if (uMsg == WM_SELECT_SPEAK)
	{
		if (class_list.find(just_speak_ip) != class_list.end())
		{
			if (IDOK == TipMsg::ShowMsgWindowTime(*this, 5000, class_list[just_speak_ip].dev_name + "请求进行互动", "提示"))
			{
				//选择当前发言
				class_list[just_speak_ip].class_video.btn_sound_off->SetVisible(true);
				class_list[just_speak_ip].class_video.btn_sound->SetVisible(false);
				for (map<string, _classinfo>::iterator itor = class_list.begin(); itor != class_list.end(); itor++)
				{
					if (itor->first != just_speak_ip)
					{
						itor->second.class_video.btn_sound_off->SetVisible(false);
						itor->second.class_video.btn_sound->SetInternVisible(true);
					}
				}
				string ss = "type=SelChat&ip=" + just_speak_ip;
				char data[50];
				strcpy(data,ss.c_str());
				client->sendData(data,strlen(data));
				//选择当前播音
			}
		}
	}
	else if (uMsg == WM_UPDATE_ICO)
	{
		string s = "type=UpdatePicture&ip=" + login_ip;
		char data[50];
		strcpy(data, s.c_str());
		if (!client)
		{
			client = ITCPClient::getInstance();
			client->open(login_ip.c_str(), 4507);
			client->setListener(this);
		}
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
	
	lab_notice = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_notify")));
	initSubVideo();
	initClassRoom();
	lab_date = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_date")));
	lab_time = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_time")));
	SetTimer(*this, TIME_ID_UPDATE_TIME, 999, NULL);
	SetTimer(*this, 1111, 500, NULL);
	// hide notify
	CHorizontalLayoutUI* hor_notify = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("NotifyLay")));
	hor_notify->SetVisible(false);

	char data[50]="type=GetIpList";
	if (!client)
	{
		client = ITCPClient::getInstance();
		client->open(login_ip.c_str(), 4507);
		client->setListener(this);
	}
	client->sendData(data, strlen(data));

}
void MainView::DisplayDateTime()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	static char cdate[250];
	static char ctime[250];
	sprintf(cdate, "%d-%d-%d", st.wYear, st.wMonth, st.wDay);
	if (lab_date)
		lab_date->SetText(cdate);

	sprintf(ctime, "%d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);

	if (lab_time)
		lab_time->SetText(ctime);

}

void MainView::initSubVideo()
{
	for (int i = 0; i < 6; i++)
	{
		char title_name[20];
		sprintf(title_name,"lab_title%d",i+1);
		subVideo[i].lab_title = static_cast<CLabelUI*>(m_PaintManager.FindControl(title_name));

		char snd_name[20];
		sprintf(snd_name, "btn_sound%d", i + 1);
		subVideo[i].btn_sound = static_cast<CButtonUI*>(m_PaintManager.FindControl(snd_name));

		char sndoff_name[20];
		sprintf(sndoff_name, "btn_soundoff%d", i + 1);
		subVideo[i].btn_sound_off = static_cast<CButtonUI*>(m_PaintManager.FindControl(sndoff_name));

		char video_name[20];
		sprintf(video_name, "video%d", i + 1);
		subVideo[i].video = static_cast<CVideoUI*>(m_PaintManager.FindControl(video_name));
		char teacher_name[20];
		sprintf(teacher_name,"teacher%d",i+1);
		subVideo[i].btn_teacher = static_cast<CButtonUI*>(m_PaintManager.FindControl(teacher_name));
		if (i != 5)
		{
			char student_name[20];
			sprintf(student_name, "student%d", i + 1);
			subVideo[i].btn_student = static_cast<CButtonUI*>(m_PaintManager.FindControl(student_name));
		}
		else
		{
			subVideo[i].btn_student = NULL;
		}

	}
}

void MainView::initClassRoom()
{
	for (int i = 0; i < 4; i++)
	{
		char ico_name[MAX_PATH];
		sprintf(ico_name, "school_ico%d", i + 1);
		classRoom[i].btn_ico = static_cast<CICOControlUI*>(m_PaintManager.FindControl(_T(ico_name)));
		char school_name[MAX_PATH];
		sprintf(school_name, "lab_school_name%d", i + 1);
		classRoom[i].lab_title = static_cast<CLabelUI*>(m_PaintManager.FindControl(school_name));
		char school_ip[MAX_PATH];
		sprintf(school_ip,"lab_school_ip%d",i+1);
		classRoom[i].lab_ip = static_cast<CLabelUI*>(m_PaintManager.FindControl(school_ip));
		char school_connect[MAX_PATH];
		sprintf(school_connect, "btn_connect%d", i + 1);
		classRoom[i].btn_connect = static_cast<CButtonUI*>(m_PaintManager.FindControl(school_connect));
		char ver_name[MAX_PATH];
		sprintf(ver_name, "vv%d", i + 1);
		classRoom[i].lay = static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(ver_name));
		classRoom[i].lay->SetVisible(false);

	}
}

/*
index:子视频控件的序号
view:直播画面切换
*/
void MainView::cut_view(int index, int view)
{
	current_index = index;
	current_view = view;
	if (!switch_view_thread)
	{
		switch_view_thread = CreateThread(NULL, 0, LiveViewSwitch, this, NULL, NULL);
	}
	else
	{
		WaitForSingleObject(switch_view_thread, 3000);
		CloseHandle(switch_view_thread);
		switch_view_thread = CreateThread(NULL, 0, LiveViewSwitch, this, NULL, NULL);
	}
}

void MainView::init_vec()
{
	for (int i = 0; i < 4; i++)
	{
		_classinfo info;
		info.class_ctrl.btn_connect = classRoom[i].btn_connect;
		info.class_ctrl.btn_ico = classRoom[i].btn_ico;
		info.class_ctrl.lab_ip = classRoom[i].lab_ip;
		info.class_ctrl.lab_title = classRoom[i].lab_title;
		info.class_ctrl.lay = classRoom[i].lay;

		info.class_video.btn_ppt = subVideo[i].btn_ppt;
		info.class_video.btn_sound = subVideo[i].btn_sound;
		info.class_video.btn_sound_off = subVideo[i].btn_sound_off;
		info.class_video.btn_student = subVideo[i].btn_student;
		info.class_video.btn_teacher = subVideo[i].btn_teacher;
		info.class_video.lab_title = subVideo[i].lab_title;
		info.class_video.video = subVideo[i].video;
		class_vec.push_back(info);
	}
}

void MainView::update_classroom()
{
	// from queue to update UI state
	for (int i = 0; i < class_queue.size(); i++)
	{
		class_vec[i].class_ctrl.btn_connect->SetText(class_queue.front().class_ctrl.btn_connect->GetText());
		class_vec[i].class_ctrl.btn_ico->SetBkImage(class_queue.front().picture_path.c_str());
		class_vec[i].class_ctrl.lab_ip->SetText(class_queue.front().class_ctrl.lab_ip->GetText());
		class_vec[i].class_ctrl.lab_title->SetText(class_queue.front().class_ctrl.lab_title->GetText());
		class_vec[i].class_ctrl.lay->SetVisible(class_queue.front().class_ctrl.lay->IsVisible());

		class_vec[i].dev_name = class_queue.front().dev_name;
		class_vec[i].picture_path = class_queue.front().picture_path;
		class_vec[i].play_url = class_queue.front().play_url;

		class_vec[i].class_video.btn_ppt->SetText(class_queue.front().class_video.btn_ppt->GetText());
		class_vec[i].class_video.btn_sound->SetVisible(class_queue.front().class_video.btn_sound->IsVisible());
		class_vec[i].class_video.btn_sound_off->SetVisible(class_queue.front().class_video.btn_sound_off->IsVisible());
	}
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
	//SIZE s{ 5, 5 };
	//this->SetBorderRound(s);

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
	//SIZE s{ 5, 5 };
	//m_pcnt->SetBorderRound(s);
	m_pcnt->SetText(_T("连接"));
	this->Add(m_pcnt);
}

