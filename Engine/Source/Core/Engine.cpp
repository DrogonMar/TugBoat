#include "TugBoat/Core/Log.h"
#include <TugBoat/Core/Engine.h>

using namespace TugBoat;

Engine* Engine::s_Instance = nullptr;

Engine* Engine::GetInstance()
{
	if (s_Instance == nullptr)
	{
		s_Instance = new Engine();
	}
	return s_Instance;
}

Engine::Engine(){
	m_Log << Info << "Engine Created";
}

Engine::~Engine(){
	m_Log << Info << "Engine Destroyed";
}