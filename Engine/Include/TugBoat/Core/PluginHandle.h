#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/Log.h>
#include <TugBoat/Core/Plugin.h>
#include <filesystem>
#include <string>

namespace TugBoat {
// Represents a plugin's shared library
// Plugin class is a friendly way of interacting with the plugin

// -----------------------------------------
// | PluginHandle                          |
// |    - libMyPlugin.so                   |
// |      - MyPluginClass (impls Plugin)   |
// -----------------------------------------
class TB_API PluginHandle {
private:
	typedef int GetEngineVersionFunction();
	typedef Plugin* RegisterPluginFunction();

public:
	TB_API PluginHandle();
	TB_API PluginHandle(const std::filesystem::path& filename);
	TB_API PluginHandle(const PluginHandle& other);
	TB_API ~PluginHandle();

	[[nodiscard]] TB_API int GetEngineVersion() const
	{
		return this->m_GetEngineVersionAddress();
	}

	TB_API void RegisterPlugin()
	{
		m_plugin = this->m_RegisterPluginAddress();
	}

	TB_API Plugin* GetPlugin()
	{
		return m_plugin;
	}

	TB_API bool IsValid(){return m_Valid;}


private:
	bool m_Valid = false;
	Plugin* m_plugin;
	void* m_sharedLibHandle;
	size_t* m_referenceCount;

	GetEngineVersionFunction* m_GetEngineVersionAddress;
	RegisterPluginFunction* m_RegisterPluginAddress;
	LOG("PluginHandle");
};
}
