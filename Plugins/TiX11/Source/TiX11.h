#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <TugBoat/Core/Plugin.h>
#include <X11/Xlib.h>

class TiX11 : public TugBoat::Plugin
{
public:
	TiX11() : Plugin()
	{
		Name = "TiX11";
		Version = "0.0.1";
	}

	~TiX11() override = default;

	void Init() override;
	void Shutdown() override;

	static TiX11* GetInstance() { return m_Instance; }

	Display* m_Display{};
	int m_Screen{};

private:
	LOG("TiX11");
	static TiX11 *m_Instance;
};
