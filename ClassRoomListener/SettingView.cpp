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
			if (btext_changed)
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
			if (GetOpenFileNameA(&ofn))
			{
				SetCurrentDirectoryA(sbuffer);
				CreateDirectoryA(user_list::ip.c_str(), NULL);
				msg.pSender->SetBkImage(strFile);
				local_fileName = string(strFile);
				char fname[MAX_PATH];
				strcpy(fname, strFile);
				PathStripPath(fname);
				string path = user_list::ip + "/" + string(fname);
				cfg->addValue("path", user_list::ip);
				cfg->save();
				CopyFileA(strFile, path.c_str(), FALSE);
			}
		}
	}
	else if (msg.sType == DUI_MSGTYPE_RETURN || msg.sType == DUI_MSGTYPE_KILLFOCUS)
	{
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
	cfg = new ConfigFile(CFG_FILE);

	remote_info[0] = cfg->getValue("name", user_list::ip);
	remote_info[1] = cfg->getValue("server_ip", "server");
	remote_info[2] = user_list::ip;
	remote_info[3] = cfg->getValue("cloudip", "server");

	string path = cfg->getValue("path", user_list::ip);
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
	edit_classIP->SetEnabled(false);
	CEditUI *edit_loboIP = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_lubo_IP")));
	if (!remote_info[2].empty())
		edit_loboIP->SetText(remote_info[2].c_str());
	edit_loboIP->SetEnabled(false);
	CEditUI *edit_cloudIP = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_cloud_IP")));
	if (!remote_info[3].empty())
		edit_cloudIP->SetText(remote_info[3].c_str());
	edit_cloudIP->SetEnabled(false);
	lab_ico = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_ico")));

}

void SettingView::OnUpload()
{
	std::string uploadUrl = "http://" + user_list::ip + "/upload.cgi?type=uploadpicture";
	if (Logan::file_exist(local_fileName))
	{
		Logan::upload(uploadUrl, user_list::ip, local_fileName);
		::PostMessage(::GetParent(*this), WM_UPDATE_ICO,NULL,NULL);
	}
}

void SettingView::SaveModify()
{
	CEditUI *edit_name = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_name")));
	if (btext_changed)
	{
		cfg->addValue("name", edit_name->GetText().GetData(), user_list::ip);
		dev_name = cfg->getValue("name",user_list::ip);
		OnUpdate_name(dev_name);
		btext_changed = false;
	}
}

void  SettingView::OnUpdate_name(std::string new_name)
{
	std::string requestUrl = "http://" + user_list::ip + "/" + user_list::cgi +"type=setdevname&name=" + CMyCharConver::ANSIToUTF8(new_name);
	Logan::query_msg_node(requestUrl, user_list::ip);
	::PostMessageA(::GetParent(*this),WM_UPDATE_DEVNAME,NULL,NULL);
}