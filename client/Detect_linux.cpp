#include <precomp.h>

#include "../utils/json11/json11help.h"
#include "../utils/Logging.h"
#include "../utils/StringUtils.h"

using namespace std;
using namespace json11;

#include "Detect.h"

#include <string.h>

#include <unistd.h>
#include <pwd.h>
#include <shadow.h>
#include <utmpx.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>












#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <netdb.h>

#include <ifaddrs.h>
#include <netdb.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <pwd.h>
#include <shadow.h>
#include <utmpx.h>
#include <sys/statvfs.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>


#ifdef NO_USER_INFO

std::string getCurrentUser();
std::string getHostname();

string getCurrentUser()
{
    char* name = NULL;
    struct utmpx* entry = NULL;

    while ((entry = getutxent()) != NULL)
    {
        if (strcmp(entry->ut_line, ":0") == 0) // tty=":0" maybe the first user logged in to system
            name = entry->ut_user;
    }
    
    endutxent();

    return name ? string(name) : "";
}

enum class isUserPasswordSetReturn { unknown, yes, no};
isUserPasswordSetReturn isUserPasswordSet(string username)
{
    struct spwd* shadow_entry;
    
    /* Read the correct hash from the shadow entry */
    shadow_entry = getspnam(username.c_str());
    if (shadow_entry == NULL)
    {
        logMessage("shadow info is not available");
        return isUserPasswordSetReturn::unknown;
    }
    
    if (strcmp(shadow_entry->sp_pwdp, "") == 0)
        return isUserPasswordSetReturn::yes;
    else 
        return isUserPasswordSetReturn::no;
}

#endif

string getHostname()
{
    char name[1024];
    int rc = gethostname(name, 1024);
    if (rc == 0)
        return string(name);
    else
    {
        logError("error getting host name", rc);
        return "";
    }
}


struct networkAdapter {
	std::string mac;
	std::vector<std::string> ipv4;
	std::vector<std::string> maskV4;
	std::vector<std::string> ipv6;
	std::vector<std::string> maskV6;
	bool adapter_enabled;
	std::string media_state;
};

typedef std::map<string, networkAdapter> Adapters;

Adapters getNetworkAdapters()
{
    Adapters adapters;
	
    struct ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1)
        return adapters;

    for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL
                || 0 != (ifa->ifa_flags & IFF_LOOPBACK)		// filters loopback interfaces (lo0)
                || 0 != (ifa->ifa_flags & IFF_POINTOPOINT)	// filters tunnel software network interface (gif0)
                || ifa->ifa_flags == 0)						// filters 6to4 tunnel interface (stf0))
        {
            continue;
        }

		struct networkAdapter& adapter = adapters[ifa->ifa_name];
        
        if (ifa->ifa_addr->sa_family == AF_PACKET)
        {
            struct sockaddr_ll* sll = (struct sockaddr_ll*) ifa->ifa_addr;
            char mac[18];
            if (sll->sll_halen == 6)
                snprintf(mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x", sll->sll_addr[0], sll->sll_addr[1], sll->sll_addr[2], sll->sll_addr[3], 
                                                sll->sll_addr[4], sll->sll_addr[5]);
            else //if address length is greater than 6, it's a firewire port
                continue;

			adapter.mac = mac;
        }
        else if (ifa->ifa_addr->sa_family == AF_INET)
        {
            char host[NI_MAXHOST];
            int ret = getnameinfo(ifa->ifa_addr, sizeof (struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (ret != 0) {
				logError("getnameinfo() failed: " << gai_strerror(ret), ret);
                continue;
            }
            std::string sIPV4(host);
            adapter.ipv4.push_back(sIPV4);
            
	        struct sockaddr_in* sa = (struct sockaddr_in*) ifa->ifa_netmask;
            adapter.maskV4.push_back(inet_ntoa(sa->sin_addr));
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            char host[NI_MAXHOST];
            int ret = getnameinfo(ifa->ifa_addr, sizeof (struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST | AI_PASSIVE);
            if (ret != 0)
            {
				logError("getnameinfo() failed: " << gai_strerror(ret), ret);
                continue;
            }
            std::string ip(host);
            // strip the zone-id off if exists
            if (ip.find_first_of("%") != std::string::npos)
                ip = ip.substr(0, ip.find_first_of("%"));
            adapter.ipv6.push_back(ip);
			
            //adapter.maskv6; //ignored
        }
		else
			continue;

        // detect adapter_enabled and media_state
        int socId = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (socId < 0)
            continue;

        struct ifreq if_req;
        strncpy(if_req.ifr_name, ifa->ifa_name, sizeof (if_req.ifr_name));
        int rv = ioctl(socId, SIOCGIFFLAGS, &if_req);
        close(socId);

        if (rv != -1)
		{
			adapter.adapter_enabled = (if_req.ifr_flags & IFF_UP) && (if_req.ifr_flags & IFF_RUNNING);
			
			if (adapter.adapter_enabled)
				adapter.media_state = "connected";
			else
				adapter.media_state = "disconnected";
		}
	}

    freeifaddrs(ifaddr);

    return adapters;
}

class DetectLinux : public Detect
{
	virtual Json::object getDeviceInfo() override;
	virtual Json::object getVersionInfo() override;
	virtual Json::array getAntivirus() override { return Json::array(); }
	virtual Json::array getFirewall() override { return Json::array(); }
	virtual Json::array getHardDiskEncryption() override { return Json::array(); }
	virtual Json::array getOsUpdateInfo() override { return Json::array(); }
	virtual Json::array getVirtualMachine() override { return Json::array(); }

	virtual Json::array getAntiphishing() override { return Json::array(); }
	virtual Json::array getBackupClient() override { return Json::array(); }
	virtual Json::array getPatchManagement() override { return Json::array(); }
	virtual Json::array getPublicFileSharing() override { return Json::array(); }
	virtual Json::array getBrowser() override { return Json::array(); }
	virtual Json::array getInstantMessenger() override { return Json::array(); }
	virtual Json::array getDataLossPrevention() override { return Json::array(); }
	virtual Json::array getVpnClient() override { return Json::array(); }
	
public:
	DetectLinux() {}
	~DetectLinux() {}

};

/*static*/ Detect* Detect::getInstance()
{
	if (m_Instance == NULL)
		m_Instance = new DetectLinux();

	return m_Instance;
}

void Detect::release() { }

/*static*/ Detect* Detect::m_Instance = NULL;

Json::array getNetworkAdaptersInfo() 
{
    Json::array aAdapterArrays;
    map<string, networkAdapter> mAdapters;
    auto adapters = getNetworkAdapters();
    if (!adapters.empty())
    {
        for (auto adapter : adapters) 
        {
            Json::object adapterObject;
			
			adapterObject["description"] = adapter.first;
            adapterObject["mac"] = adapter.second.mac;
            adapterObject["ipv4"] = adapter.second.ipv4;
		    adapterObject["ipv6"] = adapter.second.ipv6;
            adapterObject["adapter_enabled"] = adapter.second.adapter_enabled;
            adapterObject["media_state"] = adapter.second.media_state;
			string mask = adapter.second.maskV4.empty() ? "" : adapter.second.maskV4.front();
			adapterObject["subnet_masks"] = Json::array{ Json::object{ { "mask", mask } } };
            
            aAdapterArrays.push_back(adapterObject);
        }
    }

    return aAdapterArrays;
}

Json::object DetectLinux::getDeviceInfo() 
{
	Json::object device;
    Json::object osDetails;


	osDetails["family"] = "linux";
	JsonCheck(osDetails, "version", "");
	JsonCheck(osDetails, "architecture", "");
	JsonCheck(osDetails, "vendor", "");
	JsonCheck(osDetails, "name", "Linux");
	osDetails["language"] = "";

#ifdef NO_USER_INFO
    std::string username = getCurrentUser();
    if (!username.empty())
    {
        auto isPassSet = isUserPasswordSet(username);
        if (isPassSet != isUserPasswordSetReturn::unknown)
            osDetails["user_password_set"] = isPassSet == isUserPasswordSetReturn::yes;
        device["local_user"] = Json::array{ Json::object{ {"name", username} } };
    }
#endif
	
    device["os"] = osDetails;
    device["device_type"] = "desktop";
    device["network_adapter_info"] = getNetworkAdaptersInfo();

		
    return device;
}

Json::object DetectLinux::getVersionInfo()
{
	string version;
	
	return Json::object({
		{"agent_type", "persistent"},
		{"device_name", getHostname()},
		{"hostname", getHostname()},
		{"scan_time", iso_time()},
		{"core_version", version},
	});
}
