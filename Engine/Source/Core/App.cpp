#include <TugBoat/Core/App.h>

using namespace TugBoat;

App::App(){
    m_Log << Info << "App Created";
}

App::~App(){
	m_Log << Info << "App Destroyed";
}