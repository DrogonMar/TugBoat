#pragma once
#include "TiX11.h"
#include "TugBoat/Boats/WindowBoat.h"
#include <TugBoat/Core.h>
#include <optional>

struct XWindowData{
	//Holds per window data
	Window window;
};

class XWindBoat : public TugBoat::WindowBoat
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

	BID CreateWindow(const std::string& title, UIVec size, TugBoat::WindowFlags flags) override;
	UIVec GetWindowSize(BID window) override;
	void SetWindowTitle(BID window, const std::string& title) override;

	void DestroyWindow(BID window) override;

	void ProcessEvents() override;

	VkSurfaceKHR GetSurface(BID window) override;
	TugBoat::NativeWindowHandle GetNativeWindowHandle(BID window) override;

    std::optional<BID> GetBIDFromX11Window(const Window& window) const;
    
	XWindBoat();

	~XWindBoat();
private:
	LOG("XWindBoat");
	std::unordered_map<BID, TugBoat::Ref<XWindowData>> m_Windows;
    Atom m_atomWmDelete;
};
