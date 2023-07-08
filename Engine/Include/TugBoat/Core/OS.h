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

	virtual std::string GetExePath() = 0;
	virtual std::vector<std::string> GetArgs() = 0;
private:
	LOG("OS");

	static OS* s_Instance;
};
}