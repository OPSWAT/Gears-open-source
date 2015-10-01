#pragma once

#include "StringUtils.h"
#include "json11/json11.hpp"

#include <iostream>
#include <thread>
#include <future>

enum class LogLevel {
	trace,
	debug,
	info,
	warn,
	error,
	fatal,
	none
};

template<typename List>
struct LogData;

#ifndef NO_ENABLE_TRACE
#define traceJson(msg, json) (LogImpl( __FILE__, __LINE__, LogData<None>() << msg, LogLevel::trace, 0, json))
#define traceInfo(msg, num) (LogImpl( __FILE__, __LINE__, LogData<None>() << msg, LogLevel::trace, num))
#else
#define traceMessage(msg) void()
#define traceInfo(msg, num) void()
#endif

#define logMessage(msg) (LogImpl( __FILE__, __LINE__, LogData<None>() << msg, LogLevel::debug, 0))
#define logWrite(level, num, msg, json) (LogImpl( __FILE__, __LINE__, LogData<None>() << msg, level, num, json))
#define logError(msg, num) (LogImpl( __FILE__, __LINE__, LogData<None>() << msg, LogLevel::error, num, nullptr))

#ifdef _MSC_VER
#define CONSTEXPR
#define NOEXCEPT
#define NOINLINE _declspec(noinline)
#else
#define CONSTEXPR constexpr
#define NOEXCEPT noexcept
#define NOINLINE __attribute__ ((noinline))
#endif

typedef std::map<LogLevel, std::string> LogLevelStringType;

static LogLevelStringType LogLevelString( {
	{	LogLevel::trace,	"trace" },
	{ 	LogLevel::debug, 	"debug" },
	{ 	LogLevel::info, 	"info" },
	{ 	LogLevel::warn, 	"warn" },
	{	LogLevel::error,	"error" },
	{	LogLevel::fatal,	"fatal" },
	{	LogLevel::none, 	"none" },
});

static LogLevelStringType LogLevelFormatted( {
	{	LogLevel::trace,	R"("trace")" },
	{ 	LogLevel::debug,	R"("debug")" },
	{ 	LogLevel::info, 	R"("info" )" },
	{ 	LogLevel::warn, 	R"("warn" )" },
	{	LogLevel::error,	R"("error")" },
	{	LogLevel::fatal,	R"("fatal")" },
	{	LogLevel::none, 	R"("none" )" },
});

inline LogLevel GetSetLogLevel(LogLevel level = LogLevel::none)
{
	static LogLevel storedLevel = LogLevel::info;	// set default level
	if (level != LogLevel::none)
		storedLevel = level;
	return storedLevel;
}

inline void SetLogLevel(LogLevel level)
{
	GetSetLogLevel(level);
}

inline void SetLogLevel(std::string level)
{
	auto it = find_if(LogLevelString.begin(), LogLevelString.end(), [level](const LogLevelStringType::value_type & p) {
		return p.second == level;
	});
	
	if (it != LogLevelString.end())
		SetLogLevel(it->first);
}

inline LogLevel GetLogLevel()
{
	return GetSetLogLevel();
}

template<typename List>
NOINLINE void LogImpl(std::string file, int line, LogData<List>&& data, LogLevel level, int64_t num = 0, json11::Json json = nullptr) 
{
	if (GetLogLevel() > level)
		return;
	
	std::string::size_type n = file.find_last_of("\\/");
	if (n != std::string::npos)
		file = file.substr(n + 1);

	std::string time = iso_time();
	std::stringstream ss;
	output(ss, std::move(data.list));
	std::string formatted = simple_format((R"({"date":"%","level":%,"thread":%,"msg":%,"file":"%","line":%)"), 
			time, LogLevelFormatted[level], static_cast<uint16_t>(std::hash<std::thread::id>()(std::this_thread::get_id())), json11::Json(ss.str()).dump(), file, line);
	
	if (!json.is_null())
	{
		std::string s = json.dump();
		formatted += simple_format(R"(,"json":%)", s);
	}
	
	if (num != 0)
		formatted += simple_format(R"(,"num":%)", num);

	formatted += "},";

	std::cout << formatted;
	std::cout << std::endl;
}

struct None { };

template<typename List>
struct LogData {
	List list;
};

template<typename Begin, typename Value>
CONSTEXPR LogData<std::pair<Begin&&, Value&&>> operator<<(LogData<Begin>&& begin,
	Value&& value) NOEXCEPT
{
	return{ { std::forward<Begin>(begin.list), std::forward<Value>(value) } };
}

template<typename Begin, size_t n>
CONSTEXPR LogData<std::pair<Begin&&, const char*>> operator<<(LogData<Begin>&& begin,
	const char(&value)[n]) NOEXCEPT
{
	return{ { std::forward<Begin>(begin.list), value } };
}

typedef std::ostream& (*fnOstream)(std::ostream&);

template<typename Begin>
CONSTEXPR LogData<std::pair<Begin&&, fnOstream>> operator<<(LogData<Begin>&& begin,
	fnOstream value) NOEXCEPT
{
	return{ { std::forward<Begin>(begin.list), value } };
}

template <typename Begin, typename Last>
void output(std::ostream& os, std::pair<Begin, Last>&& data)
{
	output(os, std::move(data.first));
	os << data.second;
}

inline void output(std::ostream&, None)
{
}
