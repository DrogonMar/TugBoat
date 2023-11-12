#pragma once
#include "TugBoat/Core.h"
#include <TugBoat/Core/Log.h>
#include <string>
#include <vector>
//The OS class is something platforms implement

namespace TugBoat {

enum MessageBoxType {
	MessageBox_Info,
	MessageBox_Warning,
	MessageBox_Error
};

class OS{
public:
	static OS* GetInstance();

	OS();
	~OS();

	//This is the path to the executable without the executable name
	virtual std::string GetExePath() = 0;
	//All the arguments passed to the executable not including the executable name
	virtual std::vector<std::string> GetArgs() = 0;

	virtual ulong GetTicksUsec() = 0;
	virtual void SleepUsec(ulong usec) = 0;
	virtual void SleepMsec(ulong msec) = 0;

	virtual void* LoadSharedLibrary(const std::string& path) = 0;
	virtual void UnloadSharedLibrary(void* handle) = 0;
	virtual void* GetFunctionPointer(void* handle, const std::string& functionName) = 0;

	virtual std::string GetSharedLibraryExtension() = 0;

	virtual Optional<std::string> GetEnvVar(const std::string& name) = 0;

	//Helper functions, not required to be implemented
	[[maybe_unused]] virtual int AskSelection(const std::string& title, const std::string& bodyMessage, const std::vector<std::string>& options){
		return -1;
	}

	virtual void ShowMessageBox(MessageBoxType type, const std::string& title, const std::string& message) {
		switch(type){
			case MessageBox_Info:
				m_Log << Info << message;
				break;
			case MessageBox_Warning:
				m_Log << Warning << message;
				break;
			case MessageBox_Error:
				m_Log << Error << message;
				break;
		}
	};

private:
	LOG("OS");

	static OS* s_Instance;
};
}