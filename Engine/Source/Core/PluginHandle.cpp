#include <TugBoat/Core/PluginHandle.h>
#include <TugBoat/Core/OS.h>

using namespace TugBoat;

PluginHandle::PluginHandle()
{
	// Setup info about plugin here.
}

PluginHandle::PluginHandle(const std::filesystem::path& filename)
	: m_sharedLibHandle(0)
	, m_referenceCount(0)
	, m_GetEngineVersionAddress(0)
	, m_RegisterPluginAddress(0)
{
	auto os = OS::GetInstance();
	m_sharedLibHandle = os->LoadSharedLibrary(filename.string());
	if(m_sharedLibHandle == 0)
	{
		m_Log << Error << "Failed to load shared library: " << filename.string();
		m_Valid = false;
		return;
	}

	auto engineVersionPtr = os->GetFunctionPointer(m_sharedLibHandle, "GetEngineVersion");
	if (engineVersionPtr == 0)
	{
		os->UnloadSharedLibrary(m_sharedLibHandle);
		m_Log << Error << "Failed to get GetEngineVersion function pointer from shared library: " << filename.string();
		return;
	}

	m_GetEngineVersionAddress = reinterpret_cast<GetEngineVersionFunction*>( engineVersionPtr );

	auto registerPluginPtr = os->GetFunctionPointer(m_sharedLibHandle, "RegisterPlugin");
	if (registerPluginPtr == 0)
	{
		os->UnloadSharedLibrary(m_sharedLibHandle);
		m_Log << Error << "Failed to get RegisterPlugin function pointer from shared library: " << filename.string();
		return;
	}

	m_RegisterPluginAddress = reinterpret_cast<RegisterPluginFunction*>( registerPluginPtr );

	this->m_referenceCount = new size_t(1);

	m_Valid = true;
}

PluginHandle::PluginHandle(const PluginHandle& other)
	: m_sharedLibHandle(other.m_sharedLibHandle)
	, m_referenceCount(other.m_referenceCount)
	, m_GetEngineVersionAddress(other.m_GetEngineVersionAddress)
	, m_RegisterPluginAddress(other.m_RegisterPluginAddress)
{
	if (this->m_referenceCount)
		++(*this->m_referenceCount);
}

PluginHandle::~PluginHandle()
{
	if(m_Valid && this->m_referenceCount != nullptr) {
		int remainingReferences = --*(this->m_referenceCount);

		if (remainingReferences == 0) {
			m_Log << Debug << "Unloading shared library";
			delete this->m_referenceCount;
			delete this->m_plugin;
			OS::GetInstance()->UnloadSharedLibrary(this->m_sharedLibHandle);
			m_Valid = false;
		}
	}
}