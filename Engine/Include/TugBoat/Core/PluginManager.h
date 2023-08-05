#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <TugBoat/Core/PluginHandle.h>
#include <map>

namespace TugBoat {
typedef std::map<std::string, PluginHandle> PluginMap;

class TB_API PluginManager {
public:
	TB_API PluginManager();
	TB_API ~PluginManager();

	void TB_API LoadPlugins();
	void TB_API UnloadPlugins();

private:
	PluginMap m_LoadedPlugins;

	LOG("PluginManager");
};
}
