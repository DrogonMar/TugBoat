#include <TugBoat/Core/Environment.h>
#include <filesystem>

using namespace TugBoat;

Environment::Environment(int argc, char** argv)
{
    m_Log << Info << "Initializing environment";

    // Iterate through the arguments and store them in m_Args
    for (int i = 0; i < argc; i++) {
        m_Args.push_back(argv[i]);
    }

    // argv[0] is something like "./bin/Debug/Test" if outside of the output directory
    //  or "./Test" if in the same directory

    std::filesystem::path absolutePath;
    m_Log << Info << "Getting absolutePath";

    // Check if m_Args[0] is a absolute path
    if (std::filesystem::path(m_Args[0]).is_absolute()) {
        m_Log << Info << "Path is absolute";
        absolutePath = m_Args[0];
    } else {
        m_Log << Info << "Path isnt absolute, trying to get absolute path";
        std::string argv0 = "";
        if (m_Args[0].substr(0, 2) == "./") {
            // If it does, remove the "./"
            argv0 = m_Args[0].substr(2);
        } else {
            // If it doesn't, it's in the output directory
            argv0 = m_Args[0];
        }
        absolutePath = std::filesystem::absolute(argv0);
    }

    m_ExecutableLocation = absolutePath.parent_path().string();
    m_Log << Info << "Executable Location: " << m_ExecutableLocation;
}

Environment::~Environment()
{
}