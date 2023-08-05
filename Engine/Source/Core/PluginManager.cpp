#include <TugBoat/Core/PluginManager.h>
#include <filesystem>
#include <algorithm>
#include "TugBoat/Core/OS.h"

using namespace TugBoat;

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
}

// https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
bool hasEnding(std::string const& fullString, std::string const& ending)
{
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

void PluginManager::LoadPlugins()
{
	auto os = OS::GetInstance();
	auto sharedLibExt = os->GetSharedLibraryExtension();
	std::filesystem::path pluginPath = std::filesystem::path(os->GetExePath()) / "Plugins";
	if(!exists(pluginPath)){
		m_Log << Debug << "No plugins directory found";
		return;
	}

	m_Log << Debug << "Loading plugins from: " << pluginPath.string();
	for (const auto& plugin : std::filesystem::directory_iterator(pluginPath)){
		std::string filename = plugin.path().filename().string();
		//if not a .so or .dll file, skip
		{
			std::string lowercaseFilename = filename;
			std::transform(lowercaseFilename.begin(), lowercaseFilename.end(), lowercaseFilename.begin(), ::tolower);
			if (!hasEnding(lowercaseFilename, sharedLibExt)){
				m_Log << Debug << "Skipping " << filename << " because it is not a shared library";
				continue;
			}
		}

		// Don't load already loaded plugins
		if(this->m_LoadedPlugins.find(filename) != this->m_LoadedPlugins.end()){
			m_Log << Debug << "Skipping " << filename << " because it is already loaded";
			continue;
		}

		m_Log << Debug << "Loading plugin: " << filename;

		auto handle = PluginHandle(plugin.path());
		if(!handle.IsValid()){
			m_Log << Info << "Failed to load plugin: " << plugin.path().string();
			return;
		}
		m_LoadedPlugins.insert(
			PluginMap::value_type(
				filename,
				handle
				)
			).first->second.RegisterPlugin();
	}

	// Call init on all plugins
	for (auto& plugin : this->m_LoadedPlugins){
		auto corePlugin = plugin.second.GetPlugin();
		m_Log << Info << "Initializing plugin: " << corePlugin->Name;
		corePlugin->Init();
	}
}

void PluginManager::UnloadPlugins()
{
	m_Log << Debug << "Unloading plugins";

	for (auto& plugin : this->m_LoadedPlugins){
		plugin.second.GetPlugin()->Shutdown();
	}

	m_Log << Info << "Plugins unloaded";
}