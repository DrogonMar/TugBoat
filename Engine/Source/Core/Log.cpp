//
// Created by drogonmar on 07/08/23.
//
#include <TugBoat/Core/Log.h>

using namespace TugBoat;
const std::string &LevelToString(Level level)
{
	static const std::string DebugStr = "Debug";
	static const std::string InfoStr = "Info";
	static const std::string WarningStr = "Warning";
	static const std::string ErrorStr = "Error";
	static const std::string FatalStr = "Fatal";
	static const std::string UnknownStr = "Unknown";

	switch (level) {
		case Debug:
			return DebugStr;
		case Info:
			return InfoStr;
		case Warning:
			return WarningStr;
		case Error:
			return ErrorStr;
		case Fatal:
			return FatalStr;
	}
	return UnknownStr;
}
