#pragma once
#include <TugBoat/Core/OS.h>

using namespace TugBoat;

class LinuxOS : OS {
public:
	std::string GetExePath() override;
	std::vector<std::string> GetArgs() override;
private:
};