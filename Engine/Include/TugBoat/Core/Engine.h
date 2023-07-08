#pragma once
#include <TugBoat/Core/Core.h>
#include <TugBoat/Core/Log.h>

namespace TugBoat {
class TB_API Engine {
public:
	static Engine* GetInstance();

	Engine();
	~Engine();

private:
	LOG("Engine");

	static Engine* s_Instance;
};
}