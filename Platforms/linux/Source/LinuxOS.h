#pragma once
#include <TugBoat/Core/OS.h>

using namespace TugBoat;

class LinuxOS : OS {
public:
	LinuxOS(int argc, char** argv);

	std::string GetExePath() override;
	std::vector<std::string> GetArgs() override;

	ulong GetTicksUsec() override;
	void SleepUsec(ulong usec) override;

	void* LoadSharedLibrary(const std::string& path) override;
	void UnloadSharedLibrary(void* handle) override;
	void* GetFunctionPointer(void* handle, const std::string& functionName) override;

	std::string GetSharedLibraryExtension() override { return ".so"; }

	int AskSelection(const std::string& title, const std::string& bodyMessage, const std::vector<std::string>& options) override;

	void ShowMessageBox(MessageBoxType type, const std::string& title, const std::string& message) override;
private:
	LOG("LinuxOS");
	int m_Argc;
	char **m_Argv;
};