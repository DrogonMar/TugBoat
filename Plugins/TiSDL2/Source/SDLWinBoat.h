#pragma once
#include <TugBoat/Boats/WindowBoat.h>
#include <TugBoat/Core/Log.h>
#include <SDL2/SDL.h>
#include <unordered_map>

using namespace TugBoat;

class SDLWinBoat : public WindowBoat{
public:
	const char* GetName() const override { return "SDLWinBoat"; }

	static SDL_WindowFlags ConvertWindowFlags(const WindowFlags flags){
		Uint32 sdl_flags = 0;
		if(flags & WindowFlags_Resizable){
			sdl_flags |= SDL_WINDOW_RESIZABLE;
		}
		if(flags & WindowFlags_Minimizable){
			sdl_flags |= SDL_WINDOW_MINIMIZED;
		}
		if(flags & WindowFlags_Maximizable){
			sdl_flags |= SDL_WINDOW_MAXIMIZED;
		}
		if(flags & WindowFlags_Closable){
			sdl_flags |= SDL_WINDOW_BORDERLESS;
		}
		if(flags & WindowFlags_Shown){
			sdl_flags |= SDL_WINDOW_SHOWN;
		}

		return (SDL_WindowFlags)sdl_flags;
	}

	BID CreateID(){
		//find an unused id
		BID id = 0;
		while(m_Windows.find(id) != m_Windows.end()){
			id++;
		}

		return id;
	}

	BID CreateWindow(const std::string& title, UIVec size, WindowFlags flags) override;

	void DestroyWindow(BID window) override;

	void ProcessEvents() override;

	bool InternalWindowEvent(SDL_WindowEvent& event);

	SDLWinBoat();

	~SDLWinBoat();
private:
	std::unordered_map<BID, SDL_Window*> m_Windows;

	LOG("SDLWinBoat");
};
