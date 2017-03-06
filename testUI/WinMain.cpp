#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include "../Duilib/UIlib.h"
#include <mmsystem.h>
#pragma comment(lib, "WINMM.LIB")
#ifdef _DEBUG
#pragma  comment(lib,"../lib/DuiLib_d.lib")
#else
#pragma  comment(lib,"../lib/DuiLib.lib")
#endif
using namespace DuiLib;
using namespace std;
#include "test.h"


#include <vld.h>
int  _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
	testWnd *login = new testWnd;
	login->Create(NULL, _T("µÇÂ¼"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	login->CenterWindow();
	UINT lres=login->ShowModal();
	
	return EXIT_SUCCESS;

}