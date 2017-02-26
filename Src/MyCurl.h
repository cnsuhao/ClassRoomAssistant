#ifndef __MY_CURL_H__
#define __MY_CURL_H__

#include "IMyCurl.h"
#include "../Src/curl/include/curl.h"
#include "../Src/curl/include/easy.h"
#include "../Src/curl/include/types.h"
#pragma comment(lib,"../lib/libcurl.lib")


#include "Windows.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"wldap32.lib")



class CjrCurl : public ICjrCurl
{
public:
	CjrCurl();
	~CjrCurl();

	void SetListener(ICjrCurlNotify *pListener);
	bool Post(std::string strUrl, std::string strPost, std::string& strResponse);
	bool Get(std::string strUrl, std::string& strResponse);
	bool Download(std::string strUrl, std::string strDstPath, std::string strID);
	bool Upload(std::string strUrl, std::string strSrcPath, std::string strID);

private:
	DWORD GetRemoteFileSize(std::string strUrl);
	DWORD GetLocalFileSize(std::string strPath);
	bool  GetFileName(std::string strPath, std::string& strName);

private:
	ICjrCurlNotify *m_pListener;
};

#endif
