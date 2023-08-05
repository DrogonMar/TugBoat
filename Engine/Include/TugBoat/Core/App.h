#pragma once
#include "TugBoat/Core.h"
#include <TugBoat/Core/Log.h>

namespace TugBoat {
    class TB_API App{
    public:
        App();
        ~App();

        void Run();
    private:
        LOG("App");
    };
    
    extern App* CreateApp();
}