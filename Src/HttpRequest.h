#pragma once

#ifdef WIN32
#include "curl/include/curl.h"
#include "curl/include/easy.h"
#include "curl/include/types.h"
#pragma comment(lib,"../lib/libcurl.lib")
#else
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/types.h>
#endif 

#include <iostream>
#include <string>
using namespace std;

class HttpRequest
{
	
public:
	typedef enum
	{
		Get = 1,
		Post = 2,
		Put = 3,
		Delete = 4,
		Connect = 5,
		Head = 6,
		Options = 7,
		Trace = 8

	}request_type;
	typedef struct
	{
		char* file_name;
		char* file_path;
	}post_type;
	HttpRequest();
	~HttpRequest();
	static std::string request(const std::string& httpArg, post_type* type=NULL,HttpRequest::request_type request_method = HttpRequest::Get);
private:
	static size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *data);
	static size_t readfileCallback(void *ptr, size_t size, size_t nmemb, void *data);
	static std::string getMethod(const std::string& httpArg)throw(exception);
	static std::string postMethod(const std::string& httpArg, post_type* type)throw(exception);
	static std::string putMethod(const std::string& httpArg, post_type* type)throw(exception);
	static int progressdownloadCallback(const char* flag, double dtotal, double dnow, double ultotal, double ulnow);
	static int progressuploadCallback(const char* flag, double dtotal, double dnow, double ultotal, double ulnow);

};

