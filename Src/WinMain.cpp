#include "../src/expresshead.h"
#include "../Src/LoginWnd.h"
#ifdef CLIENT_LISTENER
#include "../ClassRoomListener/MainView.h"
#include "../ClassRoomListener/resource.h"
#else
#include "../ClassRoomSpeaker/MainView.h"
#include "../ClassRoomSpeaker/resource.h"
#endif


#include <vld.h>
int  _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	//进程互斥
#ifdef CLIENT_LISTENER
	HANDLE Hmutex = CreateMutex(NULL, TRUE, _T("LmainWindows"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND h = ::FindWindow(NULL, _T("LmainWindows"));
		if (h)
		{
			::SetForegroundWindow(h);
		}
		CloseHandle(Hmutex);
		::MessageBox(0, _T("有课堂助手程序在运行中..."), _T("提示"), MB_OK);
		return EXIT_FAILURE;
}

	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr))
		return 0;
#else
	HANDLE Hmutex = CreateMutex(NULL, TRUE, _T("SmainWindows"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND h = ::FindWindow(NULL, _T("SmainWindows"));
		if (h)
		{
			::SetForegroundWindow(h);
		}
		CloseHandle(Hmutex);
		::MessageBox(0, _T("有课堂助手程序在运行中..."), _T("提示"), MB_OK);
		return EXIT_FAILURE;
	}

	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr))
		return 0;

#endif 


	user_list::init(CFG_FILE);
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
	//CPaintManagerUI::SetResourceZip("skin.zip");
	LoginWnd *login = new LoginWnd();
	login->Create(NULL, _T("登录"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	login->SetIcon(IDI_ICON1);
	login->CenterWindow();
	UINT lres=login->ShowModal();
	if (lres == 1)
	{
		MainView *mainview = new MainView();
#ifdef CLIENT_LISTENER
		mainview->Create(NULL, _T("LmainWindows"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
#else
		mainview->Create(NULL, _T("SmainWindows"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
#endif 	
		mainview->SetIcon(IDI_ICON1);
		mainview->CenterWindow();
		mainview->ShowModal();
	}
	::CoUninitialize();
	return EXIT_SUCCESS;

}