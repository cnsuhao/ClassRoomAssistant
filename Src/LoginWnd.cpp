#include <time.h>
#include <io.h>
#include "LoginWnd.h"
#include "HttpRequest.h"
#include "ConfigFile.h"
#include "db.h"
#include "tinystr.h"
#include "tinyxml.h"
#include "CMyCharConver.h"
#include "IMyCurl.h"
#define  TIMER_ID_LOGIN	1000


std::string global_user;
std::string globale_passwd;
bool		global_rem=false;
bool		global_auto=false;

std::string private_key = "70a66d36f086fb9036264c7cb67e8e12";//[MD5 of logansoft]

static std::string XorEncrypt(std::string src, std::string key)
{
	std::string tmp = src;
	for (int i = 0; i < src.length(); i++)
	{
		src[i] ^= key[i%key.length()];
	}
	return src;
}


DUI_BEGIN_MESSAGE_MAP(LoginWnd, WindowImplBase)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,OnSelectedChange)
DUI_ON_MSGTYPE(DUI_MSGTYPE_RETURN,OnReturn)
DUI_ON_MSGTYPE(DUI_MSGTYPE_KILLFOCUS,OnKillFocus)
DUI_END_MESSAGE_MAP()

LoginWnd::LoginWnd() : login_error(false),
m_pOptionRem(NULL), m_pOptionAuto(NULL), m_pEditIP(NULL), m_pEditUser(NULL), m_pEditPasswd(NULL),
m_pLabImage(NULL), m_pthread(NULL)
{

}



LoginWnd::~LoginWnd()
{
	if (m_pthread)
	{
		CloseHandle(m_pthread);
		m_pthread = NULL;
	}
}

LPCTSTR LoginWnd::GetWindowClassName()const
{
	return _T("LoginWnd");
}


int callForDB(void* data, int argc, char** argv, char** colname)
{
	if (argc == 5)
	{
		global_user = string(argv[0]);
		globale_passwd = string(argv[1]);
		globale_passwd = XorEncrypt(globale_passwd, private_key);
		global_rem = !strcmp(argv[2], "1");
		global_auto = !strcmp(argv[3], "1");
	}
	return 0;
}
void LoginWnd::Init()
{
	init_self();
	db::Open(DB_FILE);
	db::Exec("create table user (username TEXT  PRIMARY KEY NOT NULL ,password TEXT NOT NULL, rem BOOLEAN,auto BOOLEAN,recentlogin DATETIME)", NULL);
	db::Exec("select * from user where recentlogin in (select max(recentlogin) from user)",callForDB);
	db::Close();
	LoadLocalData();
	if (m_pOptionAuto->IsSelected())
	{
		StartLogin();
	}
}

void LoginWnd::init_self()
{
	m_pOptionRem = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_rem")));
	m_pOptionAuto = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("opt_auto")));
	m_pEditIP = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("ip")));
	m_pEditUser = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("user")));
	m_pEditPasswd = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("passwd")));
	m_pTabSwitch = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("TabLay")));
	m_pLabTip = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_tip")));
	m_pLabImage = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lab_ico")));
}
CDuiString LoginWnd::GetSkinFile()
{
	return _T("LoginWnd.xml");
}
void LoginWnd::OnClick(TNotifyUI& msg)
{
	CDuiString _ctrName = msg.pSender->GetName();
	if (_ctrName == _T("btn_close"))
	{
		this->Close(0);
	}
	else if (_ctrName == _T("btn_login"))
	{
		StartLogin();
	}
	else if (_ctrName == _T("btn_cancel"))
	{
		m_pTabSwitch->SelectItem(0);
		KillTimer(*this, TIMER_ID_LOGIN);
	}
	else if (_ctrName == _T("btn_ok"))
	{
		m_pTabSwitch->SelectItem(0);
		if (m_pthread)
		{
			CloseHandle(m_pthread);
			m_pthread = NULL;
		}
		KillTimer(*this, TIMER_ID_LOGIN);
	}
	else if (_ctrName == _T("btn_rem"))
	{
		m_pOptionRem->Selected(!m_pOptionRem->IsSelected());
		if (!m_pOptionRem->IsSelected())
		{
			m_pOptionAuto->Selected(false);
		}
	}
	else if (_ctrName == _T("btn_auto"))
	{
		m_pOptionAuto->Selected(!m_pOptionAuto->IsSelected());
		if (m_pOptionAuto->IsSelected())
		{
			m_pOptionRem->Selected(true);
		}
	}
}
void LoginWnd::OnReturn(TNotifyUI& msg)
{
	CDuiString _ctrName = msg.pSender->GetName();
	if (_ctrName == _T("user") || _ctrName == _T("passwd"))
	{
		StartLogin();
	}
	else if (_ctrName == _T("ip"))
	{
		ConfigFile cfg(CFG_FILE);
		cfg.addValue("login_ip", m_pEditIP->GetText().GetData(), "accout");
		cfg.save();
		LoadLocalData();
	}
}

void LoginWnd::OnKillFocus(TNotifyUI& msg)
{
	if (msg.pSender->GetName() == _T("ip"))
	{
		ConfigFile cfg(CFG_FILE);
		cfg.addValue("login_ip", m_pEditIP->GetText().GetData(), "accout");
		cfg.save();
		LoadLocalData();
	}
}

void LoginWnd::OnSelectedChange(TNotifyUI& msg)
{
	CDuiString _ctrName = msg.pSender->GetName();
	if (_ctrName == _T("opt_auto"))
	{
		if (m_pOptionAuto->IsSelected())
		{
			m_pOptionRem->Selected(true);
		}
	}
	else if (_ctrName == _T("opt_rem"))
	{
		if (!m_pOptionRem->IsSelected())
		{
			m_pOptionAuto->Selected(false);
		}
	}
}


CDuiString LoginWnd::GetSkinFolder()
{
	return _T("Skin");
}

void LoginWnd::LoadLocalData()
{
	ConfigFile cfg(CFG_FILE);

	m_pEditIP->SetText(cfg.getValue("login_ip", "accout").c_str());
	string ico_path = cfg.getValue("path", cfg.getValue("login_ip", "accout").c_str());
	if (!ico_path.empty() && Logan::file_exist(ico_path))
	{
		m_pLabImage->SetBkImage(ico_path.c_str());
	}
	m_pEditUser->SetText(global_user.c_str());
	m_pEditPasswd->SetText(globale_passwd.c_str());
	m_pOptionRem->Selected(global_rem);
	m_pOptionAuto->Selected(global_auto);

}

void LoginWnd::StartLogin()
{
	if (LoginCheck())
	{
		m_pTabSwitch->SelectItem(1);
		SetTimer(*this, TIMER_ID_LOGIN, 3000, NULL);
	}
}

bool LoginWnd::LoginCheck()
{
	if (m_pEditIP->GetText().IsEmpty())
	{
		LoginError("IP地址不能为空");
		return false;
	}
	else if (m_pEditUser->GetText().IsEmpty())
	{
		LoginError("用户名不能为空");
		return false;
	}
	else if (m_pEditPasswd->GetText().IsEmpty())
	{
		LoginError("密码不能为空");
		return false;
	}
	return true;
}

void LoginWnd::LoginError(const char* msg)
{
	m_pLabTip->SetText(msg);
	m_pTabSwitch->SelectItem(2);
}

LRESULT LoginWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_TIMER)
		return OnTimer(uMsg, wParam, lParam);
	else if (uMsg == WM_LOGIN_OK)
	{
		OutputDebugStringA("login-ok");
		Close(1);
	}
	else
		return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT LoginWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	if (wParam == TIMER_ID_LOGIN)
	{
		if (m_pthread)
		{
			CloseHandle(m_pthread);
			m_pthread = NULL;
		}
		m_pthread = CreateThread(NULL, 0, LoginProc, this, NULL, NULL);
	}
	return 0;
}


DWORD WINAPI LoginProc(_In_ LPVOID paramer)
{
	LoginWnd* l = (LoginWnd*)paramer;
	try
	{
		::KillTimer(*l, TIMER_ID_LOGIN);
		Logan::login(l->m_pEditIP->GetText().GetData(), l->m_pEditUser->GetText().GetData(), l->m_pEditPasswd->GetText().GetData());
	}
	catch (std::exception& e)
	{
		::KillTimer(*l, TIMER_ID_LOGIN);
		l->LoginError(e.what());
		WaitForSingleObject(l->m_pthread, 5000);
		CloseHandle(l->m_pthread);
		l->m_pthread = NULL;
		return 0;
	}

	db::Open(DB_FILE);
	char sqlcmd[800];
	time_t timer;
	time(&timer);
	sprintf(sqlcmd, "insert into user values(\"%s\",\"%s\",%d,%d,%lld)",
		l->m_pEditUser->GetText().GetData(),
		l->m_pOptionRem->IsSelected() ?
		XorEncrypt(l->m_pEditPasswd->GetText().GetData(),private_key).c_str () : "",
		l->m_pOptionRem->IsSelected()?1:0,
		l->m_pOptionAuto->IsSelected()?1:0, timer);
	db::Exec(sqlcmd, NULL);
	sprintf(sqlcmd, "update user set password=\"%s\",rem=%d,auto=%d,recentlogin=%lld where username=\"%s\"",
		l->m_pOptionRem->IsSelected() ? 
		XorEncrypt(l->m_pEditPasswd->GetText().GetData(), private_key).c_str() : "",
		l->m_pOptionRem->IsSelected() ? 1 : 0,
		l->m_pOptionAuto->IsSelected() ? 1 : 0, timer, l->m_pEditUser->GetText().GetData());
	db::Exec(sqlcmd, NULL);
	db::Close();
	ConfigFile cfg(CFG_FILE);
	cfg.addValue("login_ip", l->m_pEditIP->GetText().GetData(), "accout");
	cfg.save();
	
	::PostMessage(*l, WM_LOGIN_OK, NULL, NULL);

	return 0;
}

CControlUI* LoginWnd::CreateControl(LPCTSTR pstrClass)
{
	if (0 == _tcscmp(pstrClass, _T("AnimationTabLayout")))
	{
		return new CAnimationTabLayoutUI;
	}
	return __super::CreateControl(pstrClass);
}

/*****************************************************************************/

std::string Logan::login(std::string ip, std::string user, std::string pwd)
{
	std::string requestUrl = "http://" + ip + "/" + user_list::cgi + "type=login&userName=" + user + "&password=" + pwd;
	std::string res = HttpRequest::request(requestUrl);
	std::string str_res;
	try
	{
		str_res = parse_msg_node(res);
		strToken::tokenTable_type token_table = strToken::getInstance();
		token_table.update_table(ip, str_res);
	}
	catch (std::exception& e)
	{
		throw e;
	}
	return str_res;
}

bool Logan::logout(std::string ip, std::string user)
{
	std::string requestUrl = "http://" + ip + "/" + user_list::cgi + "type=logout&userName=" + user;
	try
	{
		Logan::query_msg_node(requestUrl, ip);
	}
	catch (std::exception& e)
	{
		return false;
	}
	return true;
}
std::string Logan::parse_msg_node(std::string document)
{
	std::string code, msg;
	TiXmlDocument xml;
	xml.Parse(document.c_str());
	TiXmlNode *root = xml.RootElement();
	for (TiXmlNode *node = root->FirstChildElement(); node; node = node->NextSiblingElement())
	{
		if (strcmp(node->Value(), "code") == 0)
		{
			code = string(node->FirstChild()->Value());
		}
		else if (strcmp(node->Value(), "msg") == 0)
		{
			msg = string(node->FirstChild()->Value());
		}
	}
	if (code == "1")
	{
		return CMyCharConver::UTF8ToANSI(msg) ;
	}
	else
	{
		throw std::exception(msg.c_str());
		return "";
	}
}

std::string Logan::parse_url_node(std::string document)
{
	TiXmlDocument xml;
	string code, msg, rtmp_url;
	xml.Parse(document.c_str());
	TiXmlNode *root = xml.RootElement();
	for (TiXmlNode *ir = root->FirstChildElement(); ir; ir = ir->NextSiblingElement())
	{
		if (strcmp(ir->Value(), "code") == 0)
		{
			code = string(ir->FirstChild()->Value());
		}
		else if (strcmp(ir->Value(), "msg") == 0)
		{
			if (ir->FirstChild())
				msg = string(ir->FirstChild()->Value());
			if (code != "1")
			{
				throw std::exception(msg.c_str());
			}
		}
		else if (msg == "successed" && strcmp(ir->Value(), "data") == 0)
		{
			ir = ir->FirstChildElement()->FirstChildElement();
			rtmp_url = string(ir->FirstChild()->Value());
		}
	}
	return rtmp_url;
}


std::string Logan::query_url(std::string requestUrl, std::string ip)
{

	std::string str_ret;
	try
	{
		strToken::tokenTable_type token_table = strToken::getInstance();
		std::string res = HttpRequest::request(requestUrl + "&token=" + token_table.getValue(ip));
		str_ret = parse_url_node(res);
	}
	catch (std::exception& e)
	{
		if (strcmp(e.what(), "must login first") == 0)
		{
			login(ip, user_list::user_name, user_list::passwd);
			query_url(requestUrl, ip);
		}
		else
		{
			throw e;
		}
	}
	return str_ret;

}

std::string Logan::query_msg_node(std::string requestUrl, std::string ip)
{
	static std::string str_ret;
	try
	{
		strToken::tokenTable_type token_table = strToken::getInstance();
		std::string res = HttpRequest::request(requestUrl+"&token="+token_table.getValue(ip));
		str_ret=parse_msg_node(res);
	}
	catch (std::exception& e)
	{
		if (strcmp(e.what(), "must login first") == 0)
		{
			login(ip, user_list::user_name, user_list::passwd);
			query_msg_node(requestUrl, ip);
		}
		else
		{
			throw e;
		}
	}
	return str_ret;
}

bool Logan::upload(std::string upLoadUrl, std::string ip, std::string local_path)
{
	ICjrCurl *curl = ICjrCurl::GetInstance();
	strToken::tokenTable_type token_table = strToken::getInstance();
	std::string requestUrl = upLoadUrl + "&token=" + token_table.getValue(ip);
	return curl->Upload(requestUrl, local_path, "---upload---");
}

bool Logan::download(std::string upLoadUrl, std::string local_path)
{
	ICjrCurl *cjrcurl = ICjrCurl::GetInstance();
	return cjrcurl->Download(upLoadUrl, local_path, "---download---");
}

bool Logan::file_exist(const std::string path)
{
	return (_access(path.c_str(), NULL) != -1);
}