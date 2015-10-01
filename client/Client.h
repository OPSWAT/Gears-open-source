#pragma once

#ifndef WIN32

#define GEARS_CONFIGURATION_LOGGING_DIR                    "/var/log/gears/"
#define GEARS_CONFIGURATION_FILE_DIR                       "/etc/gears/"
#define GEARS_CONFIGURATION_FILE_NAME                      "gears.json"
#define CURL_CA_BUNDLE NULL

#else

#define GEARS_CONFIGURATION_LOGGING_DIR                    "./"
#define GEARS_CONFIGURATION_FILE_DIR                       "./"
#define GEARS_CONFIGURATION_FILE_NAME                      "gears.json"
#define CURL_CA_BUNDLE "curl-ca-bundle.crt"

#endif

#define GEARS_CONFIGURATION_FILE_PATH						GEARS_CONFIGURATION_FILE_DIR GEARS_CONFIGURATION_FILE_NAME
