#ifndef __EXPRESS_HEARD__
#define __EXPRESS_HEARD__

#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include "../Duilib/UIlib.h"
#include "token.h"
#include <mmsystem.h>
#pragma comment(lib, "WINMM.LIB")
#ifdef _DEBUG
#pragma  comment(lib,"../lib/DuiLib_d.lib")
#else
#pragma  comment(lib,"../lib/DuiLib.lib")
#endif
using namespace DuiLib;
using namespace std;
#endif



#define  CLOUD_IP_FILE	"express.conf"
#define  DB_FILE		"user.db"
#define  WM_UPDATE_DEVNAME	WM_USER+123
#define  WM_UPDATE_ICO		WM_USER+124

#define  WM_SELECT_SPEAK	WM_USER+200

#define  WM_ERROR_TIP		WM_USER+404
#define CFG_FILE	"express.txt"