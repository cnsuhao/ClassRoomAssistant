#include "SettingView.h"
#include "../CjrCurl/IMyCurl.h"
#include "../Login/LoginWnd.h"

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
			if (IDOK == TipMsg::ShowMsgWindow(*this, _T("确定要保存吗？"), _T("提示")))
			{
				SaveModify();
			}
			Close();
		}
		else if (msg.pSender->GetName() == _T("btn_upload"))
		{
			if (!local_fileName.empty())
			{
				ICjrCurl *curl = ICjrCurl::GetInstance();
				string upload_url = "http://" + login_ip + "/upload.cgi?type=uploadpicture&token=" + LoginWnd::getToken(login_ip);
				curl->Upload(upload_url,local_fileName,"");
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
				CreateDirectoryA(login_ip.c_str(), NULL);
				msg.pSender->SetBkImage(strFile);
				local_fileName = string(strFile);
				char fname[MAX_PATH];
				strcpy(fname, strFile);
				PathStripPath(fname);
				string path = login_ip +"/"+ string(fname);
				cfg->addValue("icopath", path);
				cfg->save();
				CopyFileA(strFile, path.c_str(), FALSE);
			}
		}
	}
	else if (msg.sType == DUI_MSGTYPE_RETURN || msg.sType == DUI_MSGTYPE_KILLFOCUS)
	{
		/*if (msg.pSender->GetName() == _T("edit_remote1"))
		{
			cfg->addValue("ip1", msg.pSender->GetText().GetData(),"remote");
		}
		else if (msg.pSender->GetName() == _T("edit_remote2"))
		{
			cfg->addValue("ip2", msg.pSender->GetText().GetData(), "remote");
		}
		else if (msg.pSender->GetName() == _T("edit_remote3"))
		{
			cfg->addValue("ip3", msg.pSender->GetText().GetData(), "remote");
		}
		else if (msg.pSender->GetName() == _T("edit_remote4"))
		{
			cfg->addValue("ip4", msg.pSender->GetText().GetData(), "remote");
		}
		else if (msg.pSender->GetName() == _T("edit_lubo_IP"))
		{
			cfg->addValue("luboip", msg.pSender->GetText().GetData(), "remote");
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
			bnameUpdate = true;
		}
	}

}

void SettingView::Init()
{
	cfg = new ConfigFile(CLOUD_IP_FILE);
	remoteIP[0] = cfg->getValue("ip1", "remote");
	remoteIP[1] = cfg->getValue("ip2", "remote");
	remoteIP[2] = cfg->getValue("ip3", "remote");
	remoteIP[3] = cfg->getValue("ip4", "remote");
	remoteIP[4] = cfg->getValue("luboip", "remote");
	remoteIP[5] = cfg->getValue("cloudip", "remote");

	localIP = cfg->getValue("localip", "local");
	name = cfg->getValue("name", "local");

	CEditUI* edit_name = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_name")));
	edit_name->SetText(name.c_str());
	string path = cfg->getValue("icopath");
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
	}

	remote_edit[4] = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_lubo_IP")));
	remote_edit[4]->SetText(remoteIP[4].c_str());
	remote_edit[5] = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_cloud_IP")));
	remote_edit[5]->SetText(remoteIP[5].c_str());

}

void SettingView::SaveModify()
{
	CEditUI* edit_ip1 = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_remote1")));
	CEditUI* edit_ip2 = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_remote2")));
	CEditUI* edit_ip3 = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_remote3")));
	CEditUI* edit_ip4 = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_remote4")));
	CEditUI* edit_lubo = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_lubo_IP")));
	CEditUI* edit_cloud = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_cloud_IP")));
	CEditUI *edit_name = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edit_name")));
	if (bnameUpdate)
	{
		cfg->addValue("name", edit_name->GetText().GetData());
		::PostMessageA(::GetParent(*this), WM_UPDATE_DEVNAME, NULL, NULL);
		dev_name = cfg->getValue("name");
		bnameUpdate = false;
	}

	cfg->addValue("ip1", edit_ip1->GetText().GetData(), "remote");
	cfg->addValue("ip2", edit_ip2->GetText().GetData(), "remote");
	cfg->addValue("ip3", edit_ip3->GetText().GetData(), "remote");
	cfg->addValue("ip4", edit_ip4->GetText().GetData(), "remote");
	cfg->addValue("cloudip", edit_cloud->GetText().GetData(), "remote");
	cfg->addValue("luboip",edit_lubo->GetText().GetData(),"remote");
	cfg->save();
}