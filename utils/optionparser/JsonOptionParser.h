#include "optionparser.h"

#include "../json11/json11.hpp"
#include "../json11/json11help.h"
#include <iostream>
#include <sstream>

/*
 * JsonOptionParser extends optionparse.h (The Lean Mean C++ Option Parser). by placing command line 
 * formation into json11 object. optionparse.h uses follows getopt() and getopt_long() conventions 
 * and should refer to its doumentation setting up option::Descriptor.
 * 
 * Parameters that have argements are indicated by JsonArg::Required or JsonArg::Options otherwise
 * ArgJson::none is needed.
 *
 * Parameters are all stored a json object ParseCmdline["parameters"]. Parameters with no arguments
 * are set by name to 'true', Also properly formatted Json on the commanded is parsed into json. 
 * 
 * For example:
 * 
 * Commandline													Json
 * --------------------											-------------
 * my_program.exe --run											{ "parameters": {"run": true} } 
 * 
 * my_program.exe --run fast									{ "parameters": {"run": "fast"} }
 * 
 * my_program.exe --run --user bob								{ "parameters": {"run": true, "user":"bob" } }
 * 
 * Proper Json is stored as json object:
 * my_program.exe --user "{\"name\":\"bob\",\"admin\":true}"		{ "parameters": {"user": { "name": "bob". "admin": true } }
 * 
 * Incorrectly formatted Json treated like any other string with quotes escaped:
 * my_program.exe --user "{\"name\":bob\",\"admin\":true}"		{ "parameters": {"user": "{\"name\":bob\",\"admin\":true}" } }
 * 
 */

namespace option
{
using namespace json11;

enum optionIndex { UNKNOWN = 0, HELP, FIRST };

struct JsonArg : public Arg
{
//	static ArgStatus Json(const Option& option, bool msg)	// Eventually require proper json and return parsing errors
	
	static ArgStatus Required(const Option& option, bool msg)
	{
		if (option.arg != 0)
			return ARG_OK;

		// not sure how to supply stream instead of cerr
		//if (msg)
		//	std::cerr << "Option " << option.name << "' requires an argument\n";

		return ARG_ILLEGAL;
	}
};


// ParseCmdline["usage"] is populated with usage info
// ParseCmdline["parameters"] hold detected parameters and arguments
// ParseCmdline["error"] is present when error detected
inline Json ParseCmdline(const Descriptor usage[], int argc = 0, char* argv[] = NULL)
{
	Json::object ret;
	std::stringstream ss;
	
	argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
	Stats stats(usage, argc, argv);
	unique_ptr<Option[]> options(new Option[stats.options_max]);
	unique_ptr<Option[]> buffer(new Option[stats.buffer_max]);
	Parser parse(usage, argc, argv, options.get(), buffer.get());

	printUsage(ss, usage);
	ret["usage"] = ss.str();
	ss.str("");		// 'empty' stream

	do 
	{
		if (options[HELP] || argc <= 0)
			break;

		if (parse.error())
		{
			ss << "Illegal Argument";
			break;
		}

		for (Option* opt = options[UNKNOWN]; opt != NULL; opt = opt->next())
			ss << "Unknown option: " << opt->name << "\n";

		if (options[UNKNOWN] != NULL)
			break;

		for (int i = 0; i < parse.nonOptionsCount(); i++)
			ss << "Non-option: " << parse.nonOption(i) << "\n";

		if (parse.nonOptionsCount() > 0)
			break;
		
		Json::object parameters;
		for (uint32_t i = 0; i < stats.options_max; i++)
		{
			const char* parameterName = options[i].desc != NULL ? options[i].desc->longopt : NULL;
			if (parameterName == NULL)
				continue;
			
			if (options[i].arg != NULL)		// option has arguments
			{
				std::string err;
				Json settings = Json::parse(options[i].arg, err);
				if (!settings.is_null())
					parameters.insert(std::make_pair(parameterName, settings));
				else
					parameters.insert(std::make_pair(parameterName, Json(options[i].arg)));		// json parse failed store argument as string
			}
			else if (options[i].type())		// option present with no arguments
				parameters[parameterName] = true;
		}
		
		ret["parameters"] = parameters;
	}
	while(0);

	if (ss.str().length())
		ret["error"] = ss.str();
	
	return ret;
}

#ifdef NO_INVOKE_YET

int JsonRun(const Json& parameters);

inline int InvokeJsonRun(const Descriptor usage[], int argc, char* argv[])
{
	Json cmdline = ParseCmdline(argc, argv, usage);
	
	if (!cmdline["err"].is_null())
	{
		std::cout << cmdline["err"].string_value();
		return -1;
	}
	return ::JsonRun(cmdline["parameters"]);
}

#endif

}