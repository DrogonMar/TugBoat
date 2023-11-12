#include "LinuxOS.h"
#include <TugBoat/Core/Engine.h>
#include <TugBoat/Core/App.h>
#include <string>

using namespace TugBoat;

int main(int argc, char** argv)
{
	LinuxOS os(argc, argv);
	os.DetectKDE();

	auto log = Log("LinuxMain");
	log << Info << "Entering engine main";

	auto engine = Engine::GetInstance();
	auto result = engine->Main();
	if (result != 0)
	{
		log << Error << "Engine main returned " << result;
		return result;
	}

	engine->Run();
	engine->Shutdown();

	return result;
}
