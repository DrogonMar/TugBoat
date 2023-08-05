#pragma once
#include "TugBoat/Core.h"
#include <string>

#define PLUGIN_IMPL(engineVersion, pluginClass) \
extern "C" { \
int GetEngineVersion() { \
	return engineVersion; \
} \
TugBoat::Plugin* RegisterPlugin() { \
	return new pluginClass(); \
} \
}

namespace TugBoat {
class TB_API Plugin {
public:
	TB_API Plugin();
	virtual TB_API ~Plugin();

	virtual TB_API void Init();
	virtual TB_API void Shutdown();

	std::string Name;
	std::string Version;
};
}