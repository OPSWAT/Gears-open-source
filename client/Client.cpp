#include <precomp.h>

#include "Client.h"
#include "Conf.h"
#include "Detect.h"

#include "../utils/EzCurl.h"
#include "../utils/Logging.h"
#include "../utils/Timer.h"

#include "../utils/json11/json11.hpp"
#include "../utils/json11/json11help.h"
#include "../utils/optionparser/JsonOptionParser.h"

using namespace std;
using namespace option;
using namespace json11;
using namespace Curl;

struct HttpRequest
{
	string method;
	string url;
	Json::object headers;
	Json body;
};

struct HttpResponse
{
	uint16_t responseStatus;
	uint32_t errorCode;
	Json::object body;
};

HttpResponse performHttpRequest(const HttpRequest& httpRequest)
{
	HttpResponse httpResponse;
	Curl::headers_t headers = JsonToMap(httpRequest.headers);

	headers["Content-Type"] = "application/json";
	Curl::httpRequest req  = { httpRequest.method, httpRequest.url, { headers, httpRequest.body.is_null()?"":httpRequest.body.dump() } };
	traceInfo("request request: " << httpRequest.method << " " << httpRequest.url, 0);
	traceJson("request headers", Json(httpRequest.headers));
	traceJson("request body", httpRequest.body);
	CurlLib::getInstance()->Request(req);

	if ( (httpResponse.errorCode = req.errorCode) != 0)
	{
		logError("CurlLib Error: " << req.error, req.errorCode);
		httpResponse.body["err"] = req.error;
		return httpResponse;
	}

	string parseErr;
	if (req.response.body.length() > 0)
		httpResponse.body = Json::parse(req.response.body, parseErr).object_items();

	traceJson("response headers", Json(req.response.headers));

	if (parseErr.length() != 0)
	{
		httpResponse.body = Json::object();
		httpResponse.body["err"] = parseErr;
		httpResponse.body["text"] = req.response.body;
	}

	traceJson("Response body", httpResponse.body);

	httpResponse.responseStatus = req.respCode;
	return httpResponse;
}

bool isHttpSuccessful(HttpResponse response, const char* msg)
{

	if (response.errorCode != 0)
	{
		logError("Failed " << msg, response.errorCode);
		return false;
	}

	if (isStatusCode200s(response.responseStatus))
	{
		logWrite(LogLevel::info, response.responseStatus, "Successful response code for " << msg, nullptr);
		return true;
	}

	logError("Failed " << msg << " due to status code", response.responseStatus);
	return false;
}

bool sendKeepAlive()
{
	Json::object settings = GetSettings();
	Json::object headers;

	CopySelectedObject(headers, settings, { "Gears-Access-Token", "Gears-Access-Key" });

	string url = settings["Gears-Server"].string_value() + "/api/v2/accounts/" + settings["license_key"].string_value() + "/devices/"
					+ settings["device_id"].string_value() + "/ping";

	logMessage("Sending KeepAlive message");
	isHttpSuccessful(performHttpRequest( { "POST", url, headers, Json::object{} } ), "send KeepAlive to server");

	return true;	// return true so timer will resent report 
}

struct AutoDetect
{
	Detect* getInstance() { return Detect::getInstance(); }
	~AutoDetect() { getInstance()->release(); }
};

bool reportData()
{
	Json::object request = AutoDetect().getInstance()->getFullReport();
	Json::object settings = GetSettings();
	Json::object headers;

	CopySelectedObject(headers, settings, { "Gears-Access-Token", "Gears-Access-Key" });
	CopySelectedObject(request, settings, { "device_name", "agent_version" });

	string url = settings["Gears-Server"].string_value() + "/api/v2/accounts/" + settings["license_key"].string_value() + "/devices/"
					+ settings["device_id"].string_value() + "/report/health";

	logMessage("Sending soh message");
	isHttpSuccessful(performHttpRequest( { "PUT", url, headers, request } ), "send soh to server");
	
	return true;	// return true so timer will resent report 
}

bool registerClient()
{
	AutoDetect detect;

	Json::object request = detect.getInstance()->getDeviceInfo();
	Json::object info = detect.getInstance()->getVersionInfo();
	Json::object settings = GetSettings();

	string typeMsg = VerifyObjectType(settings, { { "agent_version", Json::STRING }, { "Gears-Auth", Json::STRING } });
	if (!typeMsg.empty())
	{
		logWrite(LogLevel::warn, 0, typeMsg, nullptr);
		return true;	// return true so timer will retry
	}

	Json::object jsonHeaders;
	CopySelectedObject(request, settings, { "device_name", "agent_version" });
	CopySelectedObject(jsonHeaders, settings, { "Gears-Auth" });
	
	if (request["device_name"].string_value().empty() && !info["device_name"].string_value().empty())
		request["device_name"] = info["device_name"];

	string url = settings["Gears-Server"].string_value() + "/api/v2/accounts/devices/registration_code";
	
	logMessage("registerClient");
	HttpResponse httpResponse = performHttpRequest( { "POST", url, jsonHeaders, request } );
	if (!isHttpSuccessful(httpResponse, "send Register with server"))
		return true;

	typeMsg = VerifyObjectType(httpResponse.body, { { "license_key", Json::STRING }, { "device_id", Json::STRING }, { "access_key", Json::STRING }, { "access_token", Json::STRING } });
	if (!typeMsg.empty())
	{
		logError(typeMsg, 0);
		return true;
	}

	CopySelectedObject(settings, httpResponse.body, { "device_id", "license_key"} );
	settings["Gears-Access-Key"] = httpResponse.body["access_key"];
	settings["Gears-Access-Token"] = httpResponse.body["access_token"];

	return !SaveSettings(settings);		// return false to stop timer
}

int runAgent(const Json & parameters)
{
	if (GetSettings()["Gears-Access-Key"].string_value().empty())			// if no key then need register
		Timer RegisterTimer(GetSettings()["Ping-Freq"].int_value() * 1000, false, &registerClient);

	Timer PingTimer(GetSettings()["Ping-Freq"].int_value() * 1000, true, &sendKeepAlive);
	Timer Report(GetSettings()["Report-Freq"].int_value() * 1000, false, &reportData);
	return 0;
}	

int retireClient(const Json & parameters)
{
	Json::object settings = GetSettings();
	Json::object headers;

	CopySelectedObject(headers, settings, { "Gears-Access-Token", "Gears-Access-Key" });
	
	string url = settings["Gears-Server"].string_value() + "/api/v2/accounts/" + settings["license_key"].string_value() + "/devices/"
					+ settings["device_id"].string_value();

	if (isHttpSuccessful(performHttpRequest( { "DELETE", url, headers, nullptr } ), "retiring client"))
	{
		// remove old access key
		if (settings.find("Gears-Access-Key") != settings.end())
			settings.erase(settings.find("Gears-Access-Key"));
		SaveSettings(settings);
	}

	return 0;
}

int replaceSettings(const Json & parameters)
{
	Json::object settings = parameters["setting-replace"].object_items();
	SaveSettings(settings);
	return 0;
}

int updateSettings(const Json & parameters)
{
	Json::object json = GetSettings();
	JsonReplace(json, parameters["setting"].object_items());
	JsonCleanNull(json);
	SaveSettings(json);
	return 0;
}

template <typename T>
int runParamters(T runFunction, const Json & parameters)
{
	logWrite(LogLevel::info, 0, "Startup parameters", parameters);
	return runFunction(parameters);
}

enum clientOptionIndex { RETIRE = option::optionIndex::FIRST, UPDATE, REPLACE, DAEMON, CONF, LOG };

const option::Descriptor usage[] =
{
	{UNKNOWN, 0, "" , "", JsonArg::None,					"USAGE: gearsd [options]\n\n"
															"Options:" },
	{HELP, 0, "" , "help", JsonArg::None,					"  --help  \tPrint usage and exit." },
	{RETIRE, 1, "", "retire", JsonArg::None,				"  --retire  \tRetires gearsd from cloud." },
	{UPDATE, 0, "", "setting", JsonArg::Required,			"  --setting {} \tAdds new json settings on startup." },
	{DAEMON, 1, "", "run", JsonArg::None,					"  --run \tRun gearsd continuously." },
	{CONF, 1, "", "setting-file", JsonArg::Required,		"  --setting-file \tSettings file." },
	{0,0,0,0,0,0}
};

int Run(int argc, char* argv[])
{
	Json cmdline = option::ParseCmdline(usage, argc, argv);
	
	if (!cmdline["error"].is_null())
	{
		cout << cmdline["usage"].string_value();
		cout << cmdline["error"].string_value();
		return 1;
	}

	if (cmdline["parameters"]["setting-file"].is_string())
		SetSettingsFile(cmdline["parameters"]["setting-file"].string_value());
	else
		SetSettingsFile(GEARS_CONFIGURATION_FILE_PATH);
	
	SetLoggingLocation(GEARS_CONFIGURATION_LOGGING_DIR);
	SetLogLevel(GetSettings()["Log-Level"].string_value());
	
	int ret = 1;
	if (cmdline["parameters"]["setting"].is_object())
	{
		runParamters<>(updateSettings, cmdline["parameters"]);
		ret  = 0;
	}
	
	if (cmdline["parameters"]["run"].bool_value())
		return runParamters<>(runAgent, cmdline["parameters"]);
	else if (cmdline["parameters"]["retire"].bool_value())
		return runParamters<>(retireClient, cmdline["parameters"]);
	
	if (ret != 0)
	{
		cout << cmdline["usage"].string_value();
		return ret;
	}
	
	return 0;
}

int main(int argc, char* argv[])
{
	return Run(argc, argv);
}
