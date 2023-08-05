#include "WindowsOS.h"
#include <TugBoat/Core/Engine.h>
#include <TugBoat/Core/App.h>
#include <string>
#include <windows.h>
#include "WinWinBoat.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, int nCmdShow)
{
	WindowsOS os;

	auto log = Log("WindowsMain");

	WindowBoat::Register("Win32", CreateRef<WinWinBoat>());
	log << Info << "Entering engine main";
	auto engine = Engine::GetInstance();
	auto result = engine->Main();
	if(result != 0){
		log << Error << "Engine main returned " << result;
		return result;
	}

	engine->Run();
	engine->Shutdown();

	//wait for input
	system("pause");
    return result;
}
