#include "WindowsOS.h"
#include <TugBoat/Core/Engine.h>
#include <TugBoat/Core/App.h>
#include <string>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, int nCmdShow)
{
	WindowsOS os;

	auto log = Log("WindowsMain");
	log << Info << "Starting TugBoat";

	log << Info << "Exe Location: " << os.GetExePath();
	auto args = os.GetArgs();
	log << Info << "Args num: " << std::to_string(args.size());
	//loop through args
	for (auto& arg : args) {
		log << Info << "Arg: " << arg;
	}


	//wait for input
	system("pause");
    return 0;
}
