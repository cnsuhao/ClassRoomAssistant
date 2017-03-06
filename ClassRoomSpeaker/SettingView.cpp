#include "SettingView.h"
#include "../Src/IMyCurl.h"
#include "../Src/LoginWnd.h"
#include "../Src/CMyCharConver.h"
SettingView::SettingView() :cfg(NULL), bnameUpdate(false)
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

void SettingView::Notify(TNotifyUI& msg)
{
	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender->GetName() == _T("btn_close"))
		{
			if (bnameUpdate)
			{
				if (IDOK == TipMsg::ShowMsgWindow(*this, _T("确定要保存吗？"), _T("提示")))
				{
					SaveModify();
				}
			}
			Close();
		}
		else if (msg.pSender->GetName() == _T("btn_upload"))
		{
			if (!local_fileName.empty())
			{
				string upload_url = "http://" + user_list::ip + "/upload.cgi?type=uploadpicture";
				Logan::upload(upload_url, user_list::ip, local_fileName);
				::PostMessageA(::GetParent(*this), WM_UPDATE_ICO, NULL, NULL);
			}
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
				cfg->addValue("path",path, user_list::ip);
				cfg->save();
				CopyFileA(strFile, path.c_str(), FALSE);
			}
		}
	}
		else if (msg.sType == DUI_MSGTYPE_TEXTCHANGED)
		{
			if (msg.pSender->GetName() == _T("edit_name"))
			{
				bnameUpdate = true;
			}
		}

}

void SettingView::Init()
{
	cfg = new ConfigFile(CFG_FILE);
	remoteIP[0] = cfg->getValue("ip1", "remote");
	remoteIP[1] = cfg->getValue("ip2", "remote");
	remoteIP[2] = cfg->getValue("ip3", "remote");
	remoteIP[3] = cfg->getValue("ip4", "remote");
	remoteIP[4] = cfg->getValue("login_ip", "accout");
	remoteIP[5] = cfg->getValue("cloud_ip", "server");

	localIP = cfg->getValue("login_ip", "accout");
	name = cfg->getValue("name", user_list::ip);

	CEditUI* edit_name = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_name")));
	edit_name->SetText(name.c_str());
	string path = cfg->getValue("path",user_list::ip);
	if (!path.empty())
	{
		CButtonUI* btn_ico = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_ico")));
		btn_ico->SetBkImage(path.c_str());
	}

	for (int i = 0; i < 4; i++)
	{
		char ctrl_name[200];
		sprintf(ctrl_name, "edit_remote%d", i + 1);
		remote_edit[i] = static_cast<CEditUI*>(m_PaintManager.FindControl(ctrl_name));
		if (!remoteIP[i].empty())
			remote_edit[i]->SetText(remoteIP[i].c_str());
		remote_edit[i]->SetEnabled(false);
	}

	remote_edit[4] = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_lubo_IP")));
	remote_edit[4]->SetEnabled(false);
	remote_edit[4]->SetText(remoteIP[4].c_str());
	remote_edit[5] = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_cloud_IP")));
	remote_edit[5]->SetEnabled(false);
	remote_edit[5]->SetText(remoteIP[5].c_str());

}

void SettingView::SaveModify()
{
	CEditUI *edit_name = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_name")));
	if (bnameUpdate)
	{
		cfg->addValue("name", edit_name->GetText().GetData(), user_list::ip);
		string dev_name = cfg->getValue("name", user_list::ip);
		OnUpdate_name(dev_name);
		bnameUpdate = false;
	}
}
void  SettingView::OnUpdate_name(std::string new_name)
{
	std::string requestUrl = "http://" + user_list::ip + "/" + user_list::cgi + "type=setdevname&name=" + CMyCharConver::ANSIToUTF8(new_name);
	Logan::query_msg_node(requestUrl, user_list::ip);
	::PostMessageA(::GetParent(*this), WM_UPDATE_DEVNAME, NULL, NULL);
}