#include "TiSDL2.h"
#include <TugBoat/Dock.h>
#include "SDLWinBoat.h"
#include <SDL2/SDL.h>

using namespace TugBoat;

PLUGIN_IMPL(1, TiSDL2);

//we need a vector of functions that take in an event and return a bool
static std::unordered_map<std::string, std::function<bool(SDL_Event&)>> m_EventHandlers;

void TiSDL2::Init()
{
	//Setup
	m_Log << Info << "TiSDL2 Init";
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
		m_Log << Error << "Failed to init SDL2";
	}

	Dock::GetInstance()->AddBoat<WindowBoat, SDLWinBoat>();
}

void TiSDL2::Shutdown()
{
	//Cleanup
	m_Log << Info << "TiSDL2 Shutdown";
	Dock::GetInstance()->RemoveBoat<SDLWinBoat>();
	SDL_Quit();
}

void TiSDL2::ProcessEvents()
{
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		for(auto& handler : m_EventHandlers){
			auto handled = handler.second(event);
			if(handled){
				break;
			}
		}
	}
}

void TiSDL2::AddEventHandler(const std::string &name, std::function<bool(SDL_Event &)> handler)
{
	m_EventHandlers[name] = std::move(handler);
}

void TiSDL2::RemoveEventHandler(const std::string &name)
{
	m_EventHandlers.erase(name);
}
