#include <TugBoat/Gfx/Surface.h>
#include "TugBoat/Boats/WindowBoat.h"
#include "TugBoat/NativeWindow.h"
#include "TugBoat/Gfx/RHI.h"
#include <TugBoat/Gfx/Gpu.h>

using namespace TugBoat;

Surface::Surface(Gpu* gpu, BID window)
{
	m_Window = window;
	m_Surface = WindowBoat::Get()->GetSurface(window);

	if (m_Surface == VK_NULL_HANDLE) {
		m_Log << Error << "Failed to create surface";
		return;
	}

	if(!gpu->CanSupportSurface(m_Surface)){
		m_Log << Error << "GPU does not support surface";
		m_Surface = VK_NULL_HANDLE;
		vkDestroySurfaceKHR(RHI::GetInstance()->GetVkInstance(), m_Surface, nullptr);
		return;
	}

}
Surface::~Surface()
{
	m_Log << Info << "Destroying Surface";
	vkDestroySurfaceKHR(RHI::GetInstance()->GetVkInstance(), m_Surface, nullptr);
}
