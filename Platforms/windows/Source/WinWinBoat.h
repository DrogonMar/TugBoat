#pragma once
#include <TugBoat/Boats/WindowBoat.h>
#include <TugBoat/Core/Log.h>
#include <unordered_map>
#include <windows.h>

using namespace TugBoat;

struct WindowData{
	HWND window;
};

class WinWinBoat : public WindowBoat
{
public:
	BID CreateID(){
		//find an unused id
		BID id = 0;
		while(m_Windows.find(id) != m_Windows.end()){
			id++;
		}

		return id;
	}
	std::vector<std::string> GetInstanceExtensions() const override;

	BID CreateWindow(const std::string& title, UIVec size, WindowFlags flags) override;
	void DestroyWindow(BID window) override;
	void SetWindowTitle(BID window, const std::string& title) override;

	void ProcessEvents() override;

	NativeWindowHandle GetNativeWindowHandle(BID window) override;
	VkSurfaceKHR GetSurface(BID window) override;

	WinWinBoat();

	~WinWinBoat();
private:
	std::unordered_map<BID, Ref<WindowData>> m_Windows;
	LOG("WinWinBoat");
};
