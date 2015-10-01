#pragma once

#include <map>

namespace Curl
{

typedef std::map<std::string, std::string> headers_t;

struct httpEnity
{
	headers_t headers;
	std::string body;
};

struct httpRequest
{
	std::string method;
	std::string url;
	struct httpEnity request;
	struct httpEnity response;	// response.headers[""] is Reason-Phrase
	char error[512];			// native curl error text
	uint32_t errorCode;			// native curl error code
	uint16_t respCode;			// http response code
	uint16_t connectTimeoutSec;
	uint16_t totalTimeoutSec;
};

class CurlLib
{
	const char* m_certDir = NULL;
	bool m_bVerifyPeer = true;

	CurlLib() {}
	~CurlLib() {}

	static CurlLib* m_Instance;
	void Init();
	void Deinit() {}

public:
	static CurlLib* getInstance();
	static void destroyInstance();

	void Request(httpRequest& req);
};

inline uint8_t getStausCodeClass(uint16_t respCode)
{
	return respCode / 100;
}

inline bool isStatusCode200s(uint16_t respCode)
{
	return getStausCodeClass(respCode) == 2;
}

inline bool isStatusCodeError(uint16_t respCode)
{
	uint8_t statusClass = getStausCodeClass(respCode);
	return  statusClass == 4 || statusClass == 5;
}

}	//namespace Curl
