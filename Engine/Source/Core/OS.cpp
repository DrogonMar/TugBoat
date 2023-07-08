#include <TugBoat/Core/OS.h>

using namespace TugBoat;

OS* OS::s_Instance = nullptr;

OS* OS::GetInstance()
{
	return s_Instance;
}


OS::OS(){
	m_Log << Info << "OS Created";
	s_Instance = this;
}

OS::~OS(){
	m_Log << Info << "OS Destroyed";
	s_Instance = nullptr;
}