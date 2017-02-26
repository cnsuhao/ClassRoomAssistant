#define ICJR_CURL __declspec(dllexport) 

#include "MyCurl.h"


///////////////////////////////////////
//
CjrCurl oMyCurl;
ICjrCurl *ICjrCurl::GetInstance()
{
	return (ICjrCurl *)&oMyCurl;
}


///////////////////////////////////////
//
static size_t OnPost(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	size_t nRet = -1;
	
	std::string* pStr = (std::string *)lpVoid;
	if(pStr && buffer)
	{
		pStr->append((char*)buffer, size*nmemb);
		nRet = nmemb;
	}
	
	return nRet;
}

static size_t OnGet(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	size_t nRet = -1;

	std::string* pStr = (std::string *)lpVoid;
	if(pStr && buffer)
	{
		pStr->append((char*)buffer, size*nmemb);
		nRet = nmemb;
	}

	return nRet;
}

static size_t OnHeader(void *buffer, size_t size, size_t nmemb, void *data)
{
	return (size_t)(size * nmemb);
}

struct stDownloadInfo
{
	FILE *pFile;
	DWORD dwHadWrite;
	DWORD dwBlockSize;
	DWORD dwLocalSize;
	DWORD dwTotalSize;
	int  nPercent;
	std::string strID;
	ICjrCurlNotify *pListener;
};

static size_t OnDownload(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	size_t nRet = -1;

	stDownloadInfo *pstInfo = (stDownloadInfo *)lpVoid;
	if(pstInfo && buffer)
	{
		FILE *pFile = (FILE *)pstInfo->pFile;
		if(pFile)
		{
			nRet = fwrite(buffer, size, nmemb, pFile);
			if(nRet>0)
			{
				pstInfo->dwHadWrite += nRet;
				pstInfo->dwLocalSize += nRet;

				if(pstInfo->dwBlockSize>0 && pstInfo->dwHadWrite>=pstInfo->dwBlockSize)
				{
					pstInfo->dwHadWrite = 0;
					int nPercent = pstInfo->dwLocalSize / pstInfo->dwBlockSize;
					if(nPercent > pstInfo->nPercent)
					{
						pstInfo->nPercent = nPercent;
						if(pstInfo->pListener)
							pstInfo->pListener->IMyCurlNotifyDownload(pstInfo->strID, pstInfo->nPercent);
					}

					printf("OnDownload -->percent=%d \n", nPercent);
				}
			}
		}
	}
	
	return nRet;
}

static int OnDownloadProgress(const char* flag, double dtotal, double dnow, double ultotal, double ulnow)
{
	printf("--%.2lf%%\n--",dnow/dtotal);
	return 0;
}

struct stUploadInfo
{
	DWORD dwHadWrite;
	DWORD dwBlockSize;
	DWORD dwLocalSize;
	DWORD dwTotalSize;
	int  nPercent;
	std::string strID;
	ICjrCurlNotify *pListener;
};

static size_t OnUpload(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	size_t nRet = size*nmemb;

	stUploadInfo *pstInfo = (stUploadInfo *)lpVoid;
	if(pstInfo && buffer)
	{
		pstInfo->dwHadWrite += nRet;
		pstInfo->dwLocalSize += nRet;

		if(pstInfo->dwBlockSize>0 && pstInfo->dwHadWrite>=pstInfo->dwBlockSize)
		{
			pstInfo->dwHadWrite = 0;
			int nPercent = pstInfo->dwLocalSize / pstInfo->dwBlockSize;
			if(nPercent > pstInfo->nPercent)
			{
				pstInfo->nPercent = nPercent;
				if(pstInfo->pListener)
					pstInfo->pListener->IMyCurlNotifyUpload(pstInfo->strID, pstInfo->nPercent);

				printf("OnUpload -->percent=%d \n", nPercent);
			}
		}
	}

	return nRet;
}

static int OnUploadProgress(const char* flag, double dtotal, double dnow, double ultotal, double ulnow)
{
	stUploadInfo *pstInfo = (stUploadInfo *)flag;
	if(pstInfo)
	{
		int nPercent = (100* ulnow)/ultotal;
		if(nPercent > pstInfo->nPercent)
		{
			pstInfo->nPercent = nPercent;
			if(pstInfo->pListener)
				pstInfo->pListener->IMyCurlNotifyUpload(pstInfo->strID, pstInfo->nPercent);

			printf("OnUpload -->percent=%d \n", nPercent);
		}
	}
	return 0;
}

//////////////////////////////////////
//

CjrCurl::CjrCurl():
	m_pListener(NULL)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

CjrCurl::~CjrCurl()
{
	curl_global_cleanup();
}

void CjrCurl::SetListener(ICjrCurlNotify *pListener)
{
	m_pListener = pListener;
}

bool CjrCurl::Post(std::string strUrl, std::string strPost, std::string & strResponse)
{
	bool bRet = false;
	
	CURL* pCurl = curl_easy_init();
	if(pCurl)
	{
		curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());
		curl_easy_setopt(pCurl, CURLOPT_POST, 1);
		curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strPost.c_str());
		curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, NULL);
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, OnPost);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)&strResponse);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 3);
		//curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 3);
		if(CURLE_OK == curl_easy_perform(pCurl))
		{
			bRet = true;
		}
		curl_easy_cleanup(pCurl);
	}

	return bRet;
}

bool CjrCurl::Get(std::string strUrl, std::string& strResponse)
{
	bool bRet = false;
	
	CURL* pCurl = curl_easy_init();
	if(pCurl)
	{
		curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str()); 
		curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, NULL);
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, OnGet);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)&strResponse);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 3);
		//curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 3);
		if(CURLE_OK == curl_easy_perform(pCurl))
		{
			bRet = true;
		}
		curl_easy_cleanup(pCurl);
	}

	return bRet;
}

bool CjrCurl::Download(std::string strUrl, std::string strDstPath, std::string strID)
{
	bool bRet = false;

	FILE *pFile = fopen(strDstPath.c_str(), "wb");
	if(pFile)
	{
		stDownloadInfo stInfo;
		memset(&stInfo, 0, sizeof(stDownloadInfo));
		stInfo.pListener = m_pListener;
		stInfo.pFile = pFile;
		stInfo.dwTotalSize = GetRemoteFileSize(strUrl.c_str()); 
		stInfo.dwBlockSize = stInfo.dwTotalSize/100;
		stInfo.strID = strID;

		CURL* pCurl = curl_easy_init();
		if(pCurl)
		{
			curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str()); 
			curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, NULL);
			curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, OnDownload);
			curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)&stInfo);
			curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
			curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 3);
			if(CURLE_OK == curl_easy_perform(pCurl))
			{
				bRet = true;
			}

			curl_easy_cleanup(pCurl);
		}
		fclose(pFile);
	}

	if(m_pListener)
		m_pListener->IMyCurlNotifyDownload(strID,bRet?100:-1);

	return bRet;
}

static const char buf[] = "Expect:";

bool CjrCurl::Upload(std::string strUrl, std::string strSrcPath, std::string strID)
{
	bool bRet = false;

	std::string strName;
	if(GetFileName(strSrcPath, strName))
	{
		stUploadInfo stInfo;
		memset(&stInfo, 0, sizeof(stUploadInfo));
		stInfo.pListener = m_pListener;
		stInfo.dwTotalSize = GetLocalFileSize(strSrcPath.c_str()); 
		stInfo.dwBlockSize = stInfo.dwTotalSize/100;
		stInfo.strID = strID;

		CURL* pCurl = curl_easy_init();
		if(pCurl)
		{
			curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());  

			struct curl_httppost *pFormPost = NULL, *pLastPtr = NULL;
			curl_formadd(&pFormPost, &pLastPtr, CURLFORM_COPYNAME, "file1", CURLFORM_FILE, strSrcPath.c_str(), CURLFORM_CONTENTTYPE, "*/*", CURLFORM_END);
			curl_formadd(&pFormPost, &pLastPtr, CURLFORM_COPYNAME, "filename", CURLFORM_COPYCONTENTS, strName.c_str(), CURLFORM_END);
			curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, pFormPost);

			//curl_easy_setopt(pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)stInfo.dwTotalSize);
			//curl_easy_setopt(pCurl, CURLOPT_MAX_SEND_SPEED_LARGE, 10*1000);

			curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
			curl_easy_setopt(pCurl, CURLOPT_PROGRESSFUNCTION, OnUploadProgress);
			curl_easy_setopt(pCurl, CURLOPT_PROGRESSDATA, (char *)&stInfo); 


			curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
			curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 3);

			if(CURLE_OK == curl_easy_perform(pCurl))
			{
				bRet = true;
			}

			if(pFormPost)
				curl_formfree(pFormPost);

			curl_easy_cleanup(pCurl);
		}
	}

	if(m_pListener)
		m_pListener->IMyCurlNotifyUpload(strID,bRet?100:-1);

	return bRet;
}

DWORD CjrCurl::GetRemoteFileSize(std::string strUrl)
{
	double nFileSize = 0.0;

	CURL* pCurl = curl_easy_init();
	if(pCurl)
	{
		curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());
		curl_easy_setopt(pCurl, CURLOPT_HEADER, 1);
		curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 3);
		//curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 3);
		if(CURLE_OK == curl_easy_perform(pCurl))
		{
			if(CURLE_OK != curl_easy_getinfo(pCurl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &nFileSize))
				nFileSize = 0;
		}
		curl_easy_cleanup(pCurl);
	}

	return nFileSize;
}

DWORD CjrCurl::GetLocalFileSize(std::string strPath)
{
	DWORD dwFileSize = 0;

	WIN32_FIND_DATA fileInfo; 
	HANDLE hFind = FindFirstFile(strPath.c_str(), &fileInfo); 
	if(hFind != INVALID_HANDLE_VALUE) 
	{
		dwFileSize = fileInfo.nFileSizeLow; 

		FindClose(hFind); 
	}

	return dwFileSize;
}

bool CjrCurl::GetFileName(std::string strPath, std::string& strName)
{
	bool bRet = false;

	std::size_t nIndex = strPath.rfind('\\');
	if(nIndex != strPath.npos)
	{
		strName = strPath.substr(nIndex+1);
		bRet = true;
	}

	return bRet;
}