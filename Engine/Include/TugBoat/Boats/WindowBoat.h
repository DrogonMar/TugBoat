#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Boat.h>
#include <TugBoat/NativeWindow.h>
#include <string>
#include <vector>
#include "TugBoat/Vectors.h"

#include <TugBoat/Core/RemoveShortTypes.h>
#include <vulkan/vulkan.hpp>
#include <TugBoat/Core/AddShortTypes.h>

namespace TugBoat{
enum WindowFlags{
	WindowFlags_None = 0,
	WindowFlags_Resizable = 1 << 0,
	WindowFlags_Minimizable = 1 << 1,
	WindowFlags_Maximizable = 1 << 2,
	WindowFlags_Closable = 1 << 3,
	WindowFlags_Shown = 1 << 4,
};

class TB_API WindowBoat  {
public:
	//Create a window
	virtual BID CreateWindow(const std::string& title, UIVec size, WindowFlags flags){
		return INVALID_BID;
	}
	virtual void DestroyWindow(BID window){}
	virtual UIVec GetWindowSize(BID window){ return {}; }
	virtual VkSurfaceKHR GetSurface(BID window){ return {}; }

	virtual void ProcessEvents(){}

	[[nodiscard]] virtual std::vector<std::string> GetInstanceExtensions() const;
	[[nodiscard]] virtual NativeWindowHandle GetNativeWindowHandle(BID window) { return {}; }

BOAT(WindowBoat);
};
}