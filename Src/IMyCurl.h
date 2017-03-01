#ifndef __IMY_CURL_H__
#define __IMY_CURL_H__

#ifndef ICJR_CURL
#define ICJR_CURL  
#endif

#include <string>


class ICjrCurlNotify
{
public:
	virtual void IMyCurlNotifyDownload(std::string strID, int nPercent) = 0;
	virtual void IMyCurlNotifyUpload(std::string strID, int nPercent) = 0;
};

class ICJR_CURL ICjrCurl
{
public:
	static ICjrCurl *GetInstance();

	virtual void SetListener(ICjrCurlNotify *pListener) = 0;

	virtual bool Post(std::string strUrl, std::string strPost, std::string& strResponse) = 0;
	virtual bool Get(std::string strUrl, std::string& strResponse) = 0;
	virtual bool Download(std::string strUrl, std::string strDstPath, std::string strID) = 0;
	virtual bool Upload(std::string strUrl, std::string strSrcPath, std::string strID) = 0;

protected:
	virtual ~ICjrCurl(){};
};

#endif