#include <precomp.h>

#include "EzCurl.h"
#include "Logging.h"
#include "StringUtils.h"

using namespace std;

namespace Curl
{

#ifdef _MSC_VER
#include <curl.h>
#else
#include <curl/curl.h>
#endif

template<class T>
CURLcode setopt(CURL *curl, CURLoption option, T v)
{
	return curl_easy_setopt(curl, option, static_cast<T>(v));
}

static size_t header_callback(char * buffer, size_t size, size_t nitems, void * outstream)
{
	if (outstream != NULL)
	{
		string header = buffer;
		size_t sp = header.find(':');
		string name;
		if (sp != string::npos)
			name = header.substr(0, sp);

		string value = header.substr(sp != string::npos ? sp : 0);

		if (sp == string::npos)
		{
			size_t skip = value.find(' ') + 1;
			value = value.substr(skip);
			skip = value.find(' ') + 1;
			value = value.substr(skip);
		}
		else
			value = value.substr(1);

		value = trim(value);

		if (!name.empty() || !value.empty())
		{
			headers_t& headers = *reinterpret_cast<headers_t*>(outstream);
			headers[name] = value;
		}
	}
	return size * nitems;
}

static size_t write_callback(char * buffer, size_t size, size_t nitems, void * outstream)
{
	if (outstream != NULL)
	{
		std::string* pStrUserData = reinterpret_cast<std::string*>(outstream);
		*pStrUserData += std::string(buffer).substr(0, size * nitems);	// will truncate if ptr is binary with null char (don't execpt binary reponse)
	}
	return size * nitems;
}

void CurlLib::Init()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

/*static*/ CurlLib* CurlLib::getInstance()
{
	if (m_Instance == NULL)
	{
		m_Instance = new CurlLib();
		m_Instance->Init();
	}
	return m_Instance;
}

/*static*/ void CurlLib::destroyInstance()
{
	if (m_Instance)
	{
		m_Instance->Deinit();
		delete m_Instance;
		m_Instance = NULL;
	}
}

void CurlLib::Request(httpRequest& req)
{
	if (m_Instance == NULL)
		return;
	
	if (sizeof(req.error) < CURL_ERROR_SIZE)
	{
		string msg = "httpRequest::error smaller than CURL_ERROR_SIZE";
		std::copy(msg.begin(), msg.end(), req.error);
		req.errorCode = -3;
		return;
	}

	CURL* curl_handle = curl_easy_init();
	if (curl_handle == NULL)
	{
		string msg = "curl_easy_init failed";
		std::copy(msg.begin(), msg.end(), req.error);
		req.errorCode = -2;
		return;
	}
	
	setopt<const char*>(curl_handle, CURLOPT_URL, req.url.c_str());

	if (req.request.body.length() > 0 && req.method != "GET" && req.method != "HEAD")
	{
		setopt<uint32_t>(curl_handle, CURLOPT_POST, 1);
		setopt<const char*>(curl_handle, CURLOPT_POSTFIELDS, req.request.body.c_str());
		setopt(curl_handle, CURLOPT_POSTFIELDSIZE, (curl_off_t)req.request.body.length());
	}

	setopt<const char*>(curl_handle, CURLOPT_CUSTOMREQUEST, req.method.c_str());

	//Set Custom Headers, Required for Non URL Encoded POST - ie. Content-Type: text/html
	curl_slist * header_list = NULL;
	if (!req.request.headers.empty()) {
		for (auto i : req.request.headers)	{
			header_list = curl_slist_append(header_list, (i.first + ": " + i.second).c_str());
		}
		setopt<curl_slist *>(curl_handle, CURLOPT_HTTPHEADER, header_list);
	}

	// Https certificate verification
	setopt<uint32_t>(curl_handle, CURLOPT_SSL_VERIFYPEER, m_bVerifyPeer);
	setopt<uint32_t>(curl_handle, CURLOPT_SSL_VERIFYHOST, m_bVerifyPeer?2:0);	// should be default but make sure

	if (m_certDir != NULL)
		setopt<const char*>(curl_handle, CURLOPT_CAINFO, m_certDir);

	setopt<curl_write_callback>(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
	setopt<std::string*>(curl_handle, CURLOPT_WRITEDATA, &req.response.body);
	setopt<headers_t*>(curl_handle, CURLOPT_HEADERDATA, &req.response.headers);
	setopt<curl_write_callback>(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);

	setopt<char*>(curl_handle, CURLOPT_ERRORBUFFER, req.error);
	setopt<uint32_t>(curl_handle, CURLOPT_TIMEOUT, req.totalTimeoutSec);	// Set the timeout of the whole operation, (default is 0, which means no timeout)
	setopt<uint32_t>(curl_handle, CURLOPT_CONNECTTIMEOUT, req.connectTimeoutSec); // Set the timeout for connecting to the server. (default is 0, which uses 300 seconds)
	req.errorCode = curl_easy_perform(curl_handle);

	if (req.errorCode == 0)
		curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &req.respCode);

	if (header_list != NULL)
		curl_slist_free_all(header_list);
	
	curl_easy_cleanup(curl_handle);
}

/*static*/ CurlLib* CurlLib::m_Instance = NULL;

}	//namespace Curl


