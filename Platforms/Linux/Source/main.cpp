#pragma once
#include "LinuxOS.h"
#include <TugBoat/Core/Engine.h>
#include <TugBoat/Core/App.h>
#include <string>

using namespace TugBoat;

int main(int argc, char** argv)
{
	LinuxOS os;

	auto log = Log("LinuxMain");
	log << Info << "Starting TugBoat";

	log << Info << "Exe Location: " << os.GetExePath();
	log << Info << "Args num: " << std::to_string(os.GetArgs().size());
}
