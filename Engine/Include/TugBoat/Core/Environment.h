#pragma once
#include <TugBoat/Core/Core.h>
#include <TugBoat/Core/Log.h>

#include <vector>

namespace TugBoat {
class TB_API Environment {
public:
    Environment(int argc, char** argv);
    ~Environment();

    const std::string GetExecutableLocation()
    {
        return m_ExecutableLocation;
    }

    const std::vector<std::string> GetArgs()
    {
        return m_Args;
    }

private:
    std::vector<std::string> m_Args;
    std::string m_ExecutableLocation;

    LOG("Environment");
};
}