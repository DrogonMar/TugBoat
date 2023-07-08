#include <TugBoat/Core/App.h>

class TestApp : public TugBoat::App{
public:
    TestApp(){
        m_Log << Info << "Lol";
    }
    
private:
    LOG("TestApp");
};

TugBoat::App* TugBoat::CreateApp(){
    return new TestApp();
}
