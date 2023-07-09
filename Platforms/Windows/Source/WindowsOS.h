#pragma once
#include <TugBoat/Core/OS.h>

using namespace TugBoat;

class WindowsOS : OS {
public:
	std::string GetExePath() override;
	std::vector<std::string> GetArgs() override;
private:
	LOG("WindowsOS");
};
