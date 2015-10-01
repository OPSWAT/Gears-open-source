#include <precomp.h>

#include "../utils/Logging.h"
#include "../utils/json11/json11help.h"
#include "Conf.h"

#include <fstream>
#include <cstring>
#include <thread>

using namespace std;
using namespace json11;

static Json::object jsonDefault = JsonParse(R"(
{
	"Ping-Freq": 60,
	"Report-Freq": 3600
}
)").object_items();


static std::mutex mutexSettingsFile;
static string settingsFile;
static Json::object lastSettings;

void SetSettingsFile(string s)
{
	std::lock_guard<std::mutex> guard(mutexSettingsFile);
	settingsFile = s;
}

bool SaveSettings(Json::object& settings)
{
	std::lock_guard<std::mutex> guard(mutexSettingsFile);

	if (settings.size() <= 0)
		return false;	// don't wipe out file on {} object

	if (lastSettings == settings)
		return true;	// being the same is not an error
	
	std::ofstream file(settingsFile);
	if (file)
	{
		file << Json(settings).dump();
		file.close();
		logWrite(LogLevel::info, 0, "Save settings", lastSettings = settings);	// store new settings into lastSettings
		return true;
	}

	logError(string("Failed to open: ") << settingsFile << " msg: " << strerror(errno), errno);

	return false;
}

Json::object GetSettings()
{
	std::lock_guard<std::mutex> guard(mutexSettingsFile);

	std::ifstream file(settingsFile);

	if (file)
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();
		string err;
		Json::object json = Json::parse(buffer.str().c_str(), err).object_items();

		if (err.length() <= 0)
		{
			json.insert(jsonDefault.begin(), jsonDefault.end());
			if (lastSettings != json)
				logWrite(LogLevel::info, 0, "Settings", lastSettings = json);	// store new settings into lastSettings
			return lastSettings;
		}
		else
		{
			logError("Failed to open: " << settingsFile << " json error: " << err, 0);
		}
		
	}
	else
		logError("Failed to open: " << settingsFile << " msg: " << strerror(errno), errno);

	// on error wait a default ping timeout for error resolution otherwise no other wait timeouts can be loaded
	std::this_thread::sleep_for(std::chrono::milliseconds(jsonDefault["Ping-Freq"].int_value() * 1000));
	return Json().object_items();
}


static string logDir;
string GetLoggingLocation()
{
	return logDir;
}

void SetLoggingLocation(string s)
{
	logDir = s;
}
