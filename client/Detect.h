#pragma once

#include "../utils/json11/json11.hpp"
#include "../utils/json11/json11help.h"
#include "../utils/Logging.h"

class Detect
{
public:
	static Detect* getInstance();
	void release();
    
	virtual json11::Json::object getDeviceInfo() = 0;
	virtual json11::Json::object getVersionInfo() = 0;
	virtual json11::Json::array getAntivirus() = 0;
	virtual json11::Json::array getFirewall() = 0;
	virtual json11::Json::array getHardDiskEncryption() = 0;
	virtual json11::Json::array getOsUpdateInfo() = 0;
	virtual json11::Json::array getVirtualMachine() = 0;
        
	virtual json11::Json::array getAntiphishing() = 0;
	virtual json11::Json::array getBackupClient() = 0;
	virtual json11::Json::array getPatchManagement() = 0;
	virtual json11::Json::array getPublicFileSharing() = 0;
	virtual json11::Json::array getBrowser() = 0;
	virtual json11::Json::array getInstantMessenger() = 0;
	virtual json11::Json::array getDataLossPrevention() = 0;
	virtual json11::Json::array getVpnClient() = 0;

	json11::Json::object getFullReport()
	{
		json11::Json::array applications;
		JsonAppend(applications, getAntivirus());
		JsonAppend(applications, getFirewall());
		JsonAppend(applications, getOsUpdateInfo());
		JsonAppend(applications, getVirtualMachine());
		JsonAppend(applications, getAntiphishing());
		JsonAppend(applications, getBackupClient());
		JsonAppend(applications, getPatchManagement());
		JsonAppend(applications, getPublicFileSharing());
		JsonAppend(applications, getBrowser());
		JsonAppend(applications, getInstantMessenger());
		JsonAppend(applications, getDataLossPrevention());
		JsonAppend(applications, getVpnClient());
                        
                json11::Json::object deviceInfo = getDeviceInfo();
                
#if __linux__
                // special modification to linux soh to move "os" section into root and add section "system"
                JsonReplace(deviceInfo, deviceInfo["os"].object_items());
                deviceInfo["os"] = nullptr;
                JsonCleanNull(deviceInfo);
                deviceInfo["system"] = json11::Json::object {};
#endif
		json11::Json::object soh{ {
			"soh", json11::Json::object{
                { "os_info", json11::Json::array{ deviceInfo } },
                { "applications", applications }
			}
		} };
        
        auto version = getVersionInfo();
        soh.insert(version.begin(), version.end());
	
		logWrite(LogLevel::info, 0, "Completed detection", nullptr);
		
		return soh;
	}
	
        
protected:
	Detect() {};
	virtual ~Detect() {};
	static Detect* m_Instance;
};
