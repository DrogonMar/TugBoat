#include "SDLWinBoat.h"
#include "TiSDL2.h"
#include "TugBoat/Dock.h"
#include "TugBoat/Core/PluginManager.h"

void SDLWinBoat::DestroyWindow(BID window)
{
	if(m_Windows.find(window) == m_Windows.end()){
		m_Log << Error << "Failed to destroy window: " << window << " does not exist";
		return;
	}

	SDL_DestroyWindow(m_Windows[window]);
	m_Windows.erase(window);
}

BID SDLWinBoat::CreateWindow(const std::string &title, UIVec size, WindowFlags flags)
{
	SDL_Window* window = SDL_CreateWindow(title.c_str(),
										  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
										  size.x, size.y,
										  ConvertWindowFlags(flags));
	if(window == nullptr){
		m_Log << Error << "Failed to create window";
		return INVALID_BID;
	}

	BID id = CreateID();
	m_Windows[id] = window;
	return id;
}

bool SDLWinBoat::InternalWindowEvent(SDL_WindowEvent &event)
{
	BID event_bid = INVALID_BID;

	for(auto& window : m_Windows) {
		auto id = SDL_GetWindowID(window.second);
		if(id == event.windowID){
			//cool
			event_bid = window.first;
			break;
		}
	}
	if (event_bid == INVALID_BID) {
		//Maybe normal, could've been an old event.
		//oh well just say we didn't handle it and let someone else handle it
		return false;
	}

	switch(event.event) {
		case SDL_WINDOWEVENT_CLOSE:
			//send out a close event
			//for now log
			m_Log << Info << "Window " << event_bid << " closed";

			return true;
	}

	return false;
}

void SDLWinBoat::ProcessEvents()
{
	//Let the plugin handle events
	TiSDL2::ProcessEvents();
}

SDLWinBoat::SDLWinBoat()
{
	TiSDL2::AddEventHandler("WindowBoat", [this](SDL_Event& event){
		if(event.type != SDL_WINDOWEVENT)
			return false;
		return InternalWindowEvent(event.window);
	});
}

SDLWinBoat::~SDLWinBoat()
{
	TiSDL2::RemoveEventHandler("WindowBoat");
}

