#pragma once
#include "../expresshead.h"
#include "../ConfigFile/ConfigFile.h"
#include <vector>
#include "../NotifyView/NotifyWnd.h"
class SettingView :public WindowImplBase
{
public:
	SettingView();
	~SettingView();
	LPCTSTR GetWindowClassName()const;
	CDuiString GetSkinFile();
	CDuiString GetSkinFolder();
private:
	void Notify(TNotifyUI& msg);
	void Init();
	void SaveModify();
	std::string remoteIP[6];
	CEditUI* remote_edit[6];
	std::string  localIP;
	std::string  name;
	ConfigFile *cfg;
	std::string local_fileName;
	bool bnameUpdate;
};

