#include "SettingView.h"
#include "../Src/HttpRequest.h"
#include "../Src/tinystr.h"
#include "../Src/tinyxml.h"
#include "../Src/LoginWnd.h"
#include "MainView.h"
#include "../Src/IMyCurl.h"
#include "../Src/NotifyWnd.h"
#include "../Src/CMyCharConver.h"

SettingView::SettingView() :cfg(NULL), lab_ico(NULL),
btext_changed(false)
{
}


SettingView::~SettingView()
{
	if (cfg)
	{
		cfg->save();
		delete cfg;
		cfg = NULL;
	}
}


LPCTSTR SettingView::GetWindowClassName()const
{
	return _T("SettingView");
}

CDuiString SettingView::GetSkinFile()
{
	return _T("SettingView.xml");
}
CDuiString SettingView::GetSkinFolder()
{
	return _T("Skin");
}

void SettingView::setShortIco(std::string filename)
{
	if (!filename.empty())
		lab_ico->SetBkImage(filename.c_str());
	lab_ico->Invalidate();
}

void SettingView::Notify(TNotifyUI& msg)
{
	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender->GetName() == _T("btn_close"))
		{
			if (IDOK == TipMsg::ShowMsgWindow(*this, _T("确定保存？"), _T("提示")))
			{
				SaveModify();
			}
			this->Close();
		}
		else if (msg.pSender->GetName() == _T("btn_upload"))
		{
			OnUpload();
		}
		else if (msg.pSender->GetName() == _T("btn_ico"))
		{
			OPENFILENAMEA ofn;
			char strFile[MAX_PATH];
			memset(&ofn, 0, sizeof(OPENFILENAME));
			memset(strFile, 0, sizeof(char)*MAX_PATH);
			ofn.hwndOwner = *this;
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.lpstrFilter = "图片(*.png;*.bmp;*.jpg)\0*.ipg*;*.bmp;*.png\0jpg(*.jpg)\0png(*.png)\0bmp(*.bmp)\0\0";
			ofn.lpstrFile = strFile;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST;
			char sbuffer[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, sbuffer);
			/*if (GetOpenFileNameA(&ofn))
			{
				SetCurrentDirectoryA(sbuffer);
				CreateDirectoryA(login_ip.c_str(), NULL);
				msg.pSender->SetBkImage(strFile);
				local_fileName = string(strFile);
				char fname[MAX_PATH];
				strcpy(fname, strFile);
				PathStripPath(fname);
				string path = login_ip + "/" + string(fname);
				cfg->addValue("icopath", path);
				cfg->save();
				CopyFileA(strFile, path.c_str(), FALSE);
			}*/
		}
	}
	else if (msg.sType == DUI_MSGTYPE_RETURN || msg.sType == DUI_MSGTYPE_KILLFOCUS)
	{
		/*if (msg.pSender->GetName() == _T("btext_changed"))
		{
			cfg->addValue("name", msg.pSender->GetText().GetData(), "local");
		}
		else if (msg.pSender->GetName() == _T("edit_class_IP"))
		{
			cfg->addValue("classip", msg.pSender->GetText().GetData(), "remote");
		}
		else if (msg.pSender->GetName() == _T("edit_lubo_IP"))
		{
			cfg->addValue("ip", msg.pSender->GetText().GetData(), "remote");
		}
		else if (msg.pSender->GetName() == _T("edit_cloud_IP"))
		{
			cfg->addValue("cloudip", msg.pSender->GetText().GetData(), "remote");
		}*/
	}
	else if (msg.sType == DUI_MSGTYPE_TEXTCHANGED)
	{
		if (msg.pSender->GetName() == _T("edit_name"))
		{
			btext_changed = true;
		}
	}
}

void SettingView::Init()
{
	cfg = new ConfigFile(CLOUD_IP_FILE);

	remote_info[0] = cfg->getValue("name", "local");
	remote_info[1] = cfg->getValue("classip", "remote");
	remote_info[2] = cfg->getValue("ip", "local");
	remote_info[3] = cfg->getValue("cloudip", "remote");

	string path = cfg->getValue("icopath");
	if (!path.empty())
	{
		CButtonUI* btn_ico = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_ico")));
		btn_ico->SetBkImage(path.c_str());
	}

	CEditUI *edit_name = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_name")));
	if (!remote_info[0].empty())
		edit_name->SetText(remote_info[0].c_str());
	CEditUI *edit_classIP = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_class_IP")));
	if (!remote_info[1].empty())
		edit_classIP->SetText(remote_info[1].c_str());
	CEditUI *edit_loboIP = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_lubo_IP")));
	if (!remote_info[2].empty())
		edit_loboIP->SetText(remote_info[2].c_str());
	CEditUI *edit_cloudIP = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_cloud_IP")));
	if (!remote_info[3].empty())
		edit_cloudIP->SetText(remote_info[3].c_str());
	lab_ico = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_ico")));

}

void SettingView::OnUpload()
{
	/*if (!local_fileName.empty())
	{
		ICjrCurl* icurl = ICjrCurl::GetInstance();
		icurl->Upload("http://"+login_ip+"/upload.cgi?type=uploadpicture&token=" + LoginWnd::getToken(login_ip), local_fileName, "");
		::PostMessageA(::GetParent(*this), WM_UPDATE_ICO, NULL, NULL);
	}*/
	
}

void SettingView::SaveModify()
{
	/*CEditUI* edit_class_IP = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_class_IP")));
	CEditUI* edit_lubo = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_lubo_IP")));
	CEditUI* edit_cloud = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_cloud_IP")));
	CEditUI *edit_name = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_name")));
	if (btext_changed)
	{
		cfg->addValue("name", edit_name->GetText().GetData(),"local");
		dev_name = cfg->getValue("name","local");
		OnUpdate_name(dev_name);
		btext_changed = false;
	}

	cfg->addValue("classip", edit_class_IP->GetText().GetData(), "remote");
	cfg->addValue("cloudip", edit_cloud->GetText().GetData(), "remote");
	cfg->addValue("luboip", edit_lubo->GetText().GetData(), "remote");
	cfg->save();*/
}

void  SettingView::OnUpdate_name(std::string new_name)
{
	//std::string  code, msg;
	//std::string requestUrl = "http://" +login_ip + "/" + login_cgi + "type=setdevname&name=" + CMyCharConver::ANSIToUTF8(new_name) + "&token=" + login_token;
	//std::string res=HttpRequest::request(requestUrl);
	//TiXmlDocument xml;
	//xml.Parse(res.c_str());
	//TiXmlNode *root = xml.RootElement();
	//for (TiXmlNode *i = root->FirstChildElement(); i; i = i->NextSiblingElement())
	//{
	//	if (strcmp(i->Value(), "code") == 0)
	//	{
	//		code = string(i->FirstChild()->Value());
	//	}
	//	else if (strcmp(i->Value(), "msg") == 0)
	//	{
	//		msg = string(i->FirstChild()->Value());
	//	}
	//}
	//if (code != "1")
	//{
	//	std::string requestUrl = "http://" + login_ip + "/" + login_cgi + "type=setdevname&name=" + CMyCharConver::ANSIToUTF8(new_name) + "&token=" + LoginWnd::getToken(login_ip);
	//	HttpRequest::request(requestUrl);
	//}
	////发送TCP消息[更新了设备名]
	//::PostMessageA(::GetParent(*this),WM_UPDATE_DEVNAME,NULL,NULL);
}