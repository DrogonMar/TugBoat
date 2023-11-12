#include "TiX11.h"
#include "XWindBoat.h"
#include <TugBoat/Boats/WindowBoat.h>
using namespace TugBoat;

TiX11* TiX11::m_Instance = nullptr;

PLUGIN_IMPL(1, TiX11)

void TiX11::Init()
{
	m_Instance = this;
	m_Log << Info << "TiX11 Initialized";

	m_Display = XOpenDisplay(nullptr);
	if(!m_Display){
		m_Log << Error << "Failed to open display";
		return;
	}

	m_Log << Info << "Display opened";
	m_Screen = DefaultScreen(m_Display);
	m_Log << Info << "Screen: " << m_Screen;

    
	WindowBoat::Register("X11", new XWindBoat());
}

void TiX11::Shutdown()
{
	m_Log << Info << "TiX11 Shutdown";
	WindowBoat::Unregister("X11");
	XCloseDisplay(m_Display);
}
