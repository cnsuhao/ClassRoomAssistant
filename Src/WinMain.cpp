#include "../src/expresshead.h"
#include "../Src/LoginWnd.h"
#ifdef CLIENT_LISTENER
#include "../ClassRoomListener/MainView.h"
#else
#include "../ClassRoomSpeaker/MainView.h"
#endif
#include <vld.h>

int  _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	//进程互斥
	HANDLE Hmutex = CreateMutex(NULL, TRUE, _T("mainWindows"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND h = ::FindWindow(NULL, _T("mainWindows"));
		if (h)
		{
			::SetForegroundWindow(h);
		}
		CloseHandle(Hmutex);
		::MessageBox(0, _T("该程序正在运行"), _T("提示"), MB_OK);
		return EXIT_FAILURE;
	}

	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr))
		return 0;

	user_list::init(CFG_FILE);
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

	LoginWnd *login = new LoginWnd();
	login->Create(NULL, _T("登录"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	//login->SetIcon(IDI_ICON1);
	login->CenterWindow();
	UINT lres=login->ShowModal();
	if (lres == 1)
	{
		MainView *mainview = new MainView();
		mainview->Create(NULL, _T("mainPage"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
		//mainview->SetIcon(IDI_ICON1);
		mainview->CenterWindow();
		mainview->ShowModal();
	}
	::CoUninitialize();
	return EXIT_SUCCESS;

}