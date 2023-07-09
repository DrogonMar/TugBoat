#pragma once
#include <TugBoat/Core/Core.h>
#include <TugBoat/Core/Log.h>
#include <string>
#include <vector>
//The OS class is something platforms implement

namespace TugBoat {
class OS{
public:

	static OS* GetInstance();

	OS();
	~OS();

	//This is the path to the executable without the executable name
	virtual std::string GetExePath() = 0;
	//All the arguments passed to the executable not including the executable name
	virtual std::vector<std::string> GetArgs() = 0;
private:
	LOG("OS");

	static OS* s_Instance;
};
}