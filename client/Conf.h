#pragma once

#include "../utils/json11/json11.hpp"

std::string GetLoggingLocation();
void SetLoggingLocation(std::string s);

void SetSettingsFile(std::string s);
bool SaveSettings(json11::Json::object& settings);
json11::Json::object GetSettings();
