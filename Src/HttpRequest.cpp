#include "HttpRequest.h"
#include <fstream>

HttpRequest::HttpRequest()
{
}


HttpRequest::~HttpRequest()
{
}


int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
//	CURL *easy_handle = static_cast<CURL *>(clientp);
//	char timeFormat[9] = "Unknow";
//
//	// Defaults to bytes/second  
//	double speed;
//	string unit = "B";
//	curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD, &speed); // curl_get_info必须在curl_easy_perform之后调用  
//
//	if (speed != 0)
//	{
//		// Time remaining  
//		double leftTime = (Lenfile - dlnow - 0) / speed;
//		int hours = leftTime / 3600;
//		int minutes = (leftTime - hours * 3600) / 60;
//		int seconds = leftTime - hours * 3600 - minutes * 60;
//
//#ifdef _WIN32  
//		sprintf_s(timeFormat, 9, "%02d:%02d:%02d", hours, minutes, seconds);
//#else  
//		sprintf(timeFormat, "%02d:%02d:%02d", hours, minutes, seconds);
//#endif  
//	}
//
//	if (speed > 1024 * 1024 * 1024)
//	{
//		unit = "G";
//		speed /= 1024 * 1024 * 1024;
//	}
//	else if (speed > 1024 * 1024)
//	{
//		unit = "M";
//		speed /= 1024 * 1024;
//	}
//	else if (speed > 1024)
//	{
//		unit = "kB";
//		speed /= 1024;
//	}
//
//	printf("speed:%.2f%s/s", speed, unit.c_str());
//
//	if (dltotal != 0)
//	{
//		double progress = (dlnow + 0) / Lenfile * 100;
//		printf("\t%.2f%%\tRemaing time:%s\n", progress, timeFormat);
//	}
//
	return 0;
}

std::string HttpRequest::request(const std::string& httpArg, post_type* type, HttpRequest::request_type request_method)
{
	std::string retBuffer;
	switch (request_method)
	{
	case HttpRequest::Get:
	{
		retBuffer = getMethod(httpArg);
	}
	break;
	case HttpRequest::Post:
	{
		retBuffer = postMethod(httpArg, type);
	}
	default:
		break;
	}
	return retBuffer;
}

size_t HttpRequest::writeCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t sres = 0;
	std::string *buffer = (std::string*)data;
	if (ptr&&buffer)
	{
	buffer->append((char*)ptr, size*nmemb);
	sres = nmemb;
	}
	return sres;
}

size_t HttpRequest::readfileCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	ifstream *is = (ifstream*)data;
	is->read((char*)ptr, size*nmemb);
	return size*nmemb;
}
int HttpRequest::progressuploadCallback(const char* flag, double dtotal, double dnow, double ultotal, double ulnow)
{
	printf("%0.2lf%%\n", ulnow * 100 / ultotal);
	return 0;
}
int HttpRequest::progressdownloadCallback(const char* flag, double dtotal, double dnow, double ultotal, double ulnow)
{
	printf("%0.2lf%%\n", dnow * 100 / dtotal);
	return 0;
}
std::string HttpRequest::getMethod(const std::string& httpArg)throw(exception)
{
	CURL *curl;
	CURLcode code;
	std::string buffer;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, httpArg.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION,progressdownloadCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
	char s[123];
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, s);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 5);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
	code=curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	if (code != CURLE_OK)
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(code));
		throw std::exception(curl_easy_strerror(code));
	}
	return buffer;
}
std::string HttpRequest::postMethod(const std::string& httpArg, post_type* type)throw(exception)
{
	std::string buffer;
	
	CURL *curl;
	CURLcode code;
	CURLM *multi_handle;
	int still_running;

	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	struct curl_slist *headerlist = NULL;
	static const char buf[] = "Yoohoo:wand_niu@live.com";

	/* Fill in the file upload field. This makes libcurl load data from
	the given file name when curl_easy_perform() is called. */
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "file1",
		CURLFORM_FILE,type->file_path,
		CURLFORM_CONTENTTYPE, "*/*",
		CURLFORM_END);

	/* Fill in the filename field */
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "filename",
		CURLFORM_COPYCONTENTS, type->file_name,
		CURLFORM_END);

	curl = curl_easy_init();
	headerlist = curl_slist_append(headerlist, buf);
	if (curl ) {

		/* what URL that receives this POST */
		//curl_easy_setopt(curl, CURLOPT_HTTPHEADER, "Expect:");
		curl_easy_setopt(curl, CURLOPT_URL,httpArg.c_str());
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "type=uploadpicture&token=54e40cf1-bd26-4e");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressuploadCallback);
		curl_easy_setopt(curl, CURLOPT_MAXFILESIZE_LARGE, 2*1<<30);//2G
		char p[123];
		curl_easy_setopt(curl,CURLOPT_PROGRESSDATA,p);
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&buffer);
		code=curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		curl_formfree(formpost);
		curl_slist_free_all(headerlist);
		if (code != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(code));
			throw std::exception(curl_easy_strerror(code));
		}
	}
	return buffer;
}
std::string HttpRequest::putMethod(const std::string& httpArg, post_type* type)throw(exception)
{
	CURL *curl;
	CURLcode res;
	std::string buffer;
	ifstream is(type->file_path, std::ios::binary | std::ios::in);
	is.seekg(0, std::ios::end);
	unsigned long long file_length = is.tellg();
	is.seekg(0, std::ios::beg);
	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if (curl) {
		/* we want to use our own read function */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, readfileCallback);

		/* enable uploading */
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* HTTP PUT please */
		curl_easy_setopt(curl, CURLOPT_PUT, 1L);

		/* specify target URL, and note that this URL should include a file
		name, not only a directory */
		curl_easy_setopt(curl, CURLOPT_URL, httpArg.c_str());

		/* now specify which file to upload */
		curl_easy_setopt(curl, CURLOPT_READDATA, (void*)&is);

		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writeCallback);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&buffer);

		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,file_length);

		/* Now run off and do what you've been told! */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	is.close();
	curl_global_cleanup();

	if (res != CURLE_OK)
	{
		throw std::exception(curl_easy_strerror(res));
	}
	return buffer;
}
