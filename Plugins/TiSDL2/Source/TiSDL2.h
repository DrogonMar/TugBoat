#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <TugBoat/Core/Plugin.h>
#include <utility>
#include <vector>
#include <functional>
#include <algorithm>
#include "SDL2/SDL.h"

using namespace TugBoat;

class TiSDL2 : public Plugin {
public:
	TiSDL2() : Plugin() {
		Name = "TiSDL2";
		Version = "0.0.1";
	}
	~TiSDL2() override= default;

	void Init() override;
	void Shutdown() override;

	static void AddEventHandler(const std::string& name, std::function<bool(SDL_Event&)> handler);
	static void RemoveEventHandler(const std::string& name);

	static void ProcessEvents();
private:
	LOG("TiSDL2");
};