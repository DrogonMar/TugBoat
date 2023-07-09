#include "WindowsOS.h"
#include <windows.h>

std::string WindowsOS::GetExePath(){
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

std::vector<std::string> WindowsOS::GetArgs(){
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::vector<std::string> args;
	for (int i = 1; i < argc; i++) {
		//convert to string
		std::wstring ws(argv[i]);
		std::string str(ws.begin(), ws.end());
		args.push_back(str);
	}
	return args;
}
