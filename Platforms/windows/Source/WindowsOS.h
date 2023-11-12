#pragma once
#include <TugBoat/Core.h>
#include <TugBoat/Core/OS.h>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace TugBoat;

class WindowsOS : OS {
public:
	WindowsOS();
	~WindowsOS();

	std::string GetExePath() override;
	std::vector<std::string> GetArgs() override;

	ulong GetTicksUsec() override;
	void SleepUsec(ulong usec) override;

	void* LoadSharedLibrary(const std::string& path) override;
	void UnloadSharedLibrary(void* handle) override;
	void* GetFunctionPointer(void* handle, const std::string& functionName) override;

	std::string GetSharedLibraryExtension() override { return ".dll"; }

	Optional<std::string> GetEnvVar(const std::string& name) override;

	int AskSelection(const std::string& title, const std::string& bodyMessage, const std::vector<std::string>& options) override;
	void ShowMessageBox(MessageBoxType type, const std::string& title, const std::string& message) override;
private:

	LOG("WindowsOS");
};
