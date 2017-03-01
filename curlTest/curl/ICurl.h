#ifndef __IMY_CURL_H__
#define __IMY_CURL_H__

#include <string>


class ICurlNotify
{
public:
	virtual void NotifyDownload(std::string strID, int nPercent) = 0;
	virtual void NotifyUpload(std::string strID, int nPercent) = 0;
};

class  ICurl
{
public:
	static ICurl *GetInstance();

	virtual void SetListener(ICurlNotify *pListener) = 0;

	virtual bool Post(std::string strUrl, std::string strPost, std::string& strResponse) = 0;
	virtual bool Get(std::string strUrl, std::string& strResponse) = 0;
	virtual bool Download(std::string strUrl, std::string strDstPath, std::string strID) = 0;
	virtual bool Upload(std::string strUrl, std::string strSrcPath, std::string strID) = 0;

protected:
	virtual ~ICurl(){};
};

#endif