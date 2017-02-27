#pragma once
#include "../Src/expresshead.h"
#include "../Src/ConfigFile.h"
#include <vector>
#include "../Src/LoginWnd.h"
class SettingView :public WindowImplBase
{
public:
	SettingView();
	~SettingView();
	LPCTSTR GetWindowClassName()const;
	CDuiString GetSkinFile();
	CDuiString GetSkinFolder();
	void setShortIco(std::string filename);
private:
	void Notify(TNotifyUI& msg);
	void Init();
	void OnUpload();
	void SaveModify();
	void OnUpdate_name(std::string new_name);
	std::string remote_info[4];
	// [0]name [1]havaclass [2]lobo [3]cloud
	CEditUI* remote_edit[4];
	ConfigFile *cfg;
	CLabelUI* lab_ico;
	string local_fileName;
	std::string dev_name;
	bool btext_changed;
};
