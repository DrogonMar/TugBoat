#include "LinuxOS.h"
#include <unistd.h>
#include <climits>
#include <string>
#include <filesystem>
#include <dlfcn.h>
#include <algorithm>

LinuxOS::LinuxOS(int argc, char **argv) : OS()
{
	this->m_Argc = argc;
	this->m_Argv = argv;
}

std::string LinuxOS::GetExePath(){
	char buffer[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
	//this will include the exe name, we need to remove it
	if (len != -1) {
		buffer[len] = '\0';
		std::string path(buffer);
		auto pos = path.find_last_of('/');
		if (pos != std::string::npos) {
			return path.substr(0, pos);
		}
	}
	return {};
}

std::vector<std::string> LinuxOS::GetArgs(){
	std::vector<std::string> args;
	for (int i = 1; i < this->m_Argc; i++) {
		args.emplace_back(this->m_Argv[i]);
	}
	return args;
}

void *LinuxOS::LoadSharedLibrary(const std::string &path)
{
	void* sharedObj = dlopen(path.c_str(), RTLD_NOW);
	if (!sharedObj) {
		m_Log << Error << "Failed to load shared library: " << dlerror();
		return nullptr;
	}

	return sharedObj;
}

void LinuxOS::UnloadSharedLibrary(void *handle)
{
	int result = dlclose(handle);
	if (result != 0) {
		m_Log << Error << "Failed to unload shared library: " << dlerror();
	}
}

void *LinuxOS::GetFunctionPointer(void *handle, const std::string &functionName)
{
	dlerror();
	void* function = dlsym(handle, functionName.c_str());
	const char* error = dlerror();
	if (error) {
		m_Log << Error << "Failed to get function pointer: " << error;
		return nullptr;
	}
	return function;
}

ulong LinuxOS::GetTicksUsec()
{
	timespec ts{};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

void LinuxOS::SleepUsec(ulong usec)
{
	usleep(usec);
}

int LinuxOS::ZenAskSelection(const std::string &title,
						   const std::string &bodyMessage,
						   const std::vector<std::string> &options){
	//Were gonna use zenity for this
	//We just want a list of the strings where only one can be selected
	//zenity wont return the index but the string itself, so we need to find the index
	/*
	 * zenity --list 1 "Apples" 2 "Peaches" 3 "Pumpkin" 4 "Pie" --column="id" \
--column="Select your choice" --hide-column=1 --print-column=1
	 */

	std::string command = "zenity --list ";
	for (int i = 0; i < options.size(); i++) {
		command += std::to_string(i) + " \"" + options[i] + "\" ";
	}
	command += R"(--column="id" --column="Select your choice" --hide-column=1 --print-column=1)";


	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe) {
		m_Log << Error << "Failed to open pipe to zenity";
		return -1;
	}

	char buffer[128];
	std::string result;
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != nullptr) {
			result += buffer;
		}
	}

	int exitCode = pclose(pipe);
	if (exitCode != 0) {
		m_Log << Error << "Failed to close pipe to zenity";
		return -1;
	}

	//zenity returns the index of the selected item
	//remove the newline
	result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
	//convert to int
	int index = std::stoi(result);
	return index;
}

int LinuxOS::KDEAskSelection(const std::string &title,
							 const std::string &bodyMessage,
							 const std::vector<std::string> &options){
	std::stringstream ss;
	ss << "kdialog --title \"" << title << "\" --menu \"" << bodyMessage << "\" ";

	for (int i = 0; i < options.size(); ++i) {
		ss << i << " \"" << options[i] << "\" ";
	}

	FILE* pipe = popen(ss.str().c_str(), "r");
	if (!pipe) {
		m_Log << Error << "Failed to open pipe to kdialog";
		return -1;
	}

	char buffer[16];
	std::string result;
	while (!feof(pipe)) {
		if (fgets(buffer, 16, pipe) != nullptr) {
			result += buffer;
		}
	}

	int exitCode = pclose(pipe);
	if (exitCode != 0) {
		return -1;
	}

	result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
	int index = std::stoi(result);
	return index;

}

int LinuxOS::AskSelection(const std::string &title,
						 const std::string &bodyMessage,
						 const std::vector<std::string> &options)
{
	if(UseKde)
		return KDEAskSelection(title, bodyMessage, options);
	else
		return ZenAskSelection(title, bodyMessage, options);
}

void LinuxOS::ZenShowMessageBox(TugBoat::MessageBoxType type, const std::string &title, const std::string &message)
{
	OS::ShowMessageBox(type, title, message);
	std::string command = "zenity --";
	switch (type) {
		case MessageBox_Info:
			command += "info";
			break;
		case MessageBox_Warning:
			command += "warning";
			break;
		case MessageBox_Error:
			command += "error";
			break;
	}

	command += " --title=\"" + title + "\" --text=\"" + message + "\"";

	system(command.c_str());
}

void LinuxOS::KDEShowMessageBox(TugBoat::MessageBoxType type, const std::string &title, const std::string &message){
	std::stringstream ss;
	ss << "kdialog --title \"" << title << "\" ";
	switch (type) {
		case MessageBox_Info:
			ss << "--msgbox \"";
			break;
		case MessageBox_Warning:
			ss << "--sorry \"";
			break;
		case MessageBox_Error:
			ss << "--error \"";
			break;
	}

	ss << message << "\"";

	system(ss.str().c_str());
}

void LinuxOS::ShowMessageBox(MessageBoxType type, const std::string &title, const std::string &message)
{
	if(UseKde)
		KDEShowMessageBox(type, title, message);
	else
		ZenShowMessageBox(type, title, message);
}

Optional<std::string> LinuxOS::GetEnvVar(const std::string &name)
{
	char* value = getenv(name.c_str());
	if (value == nullptr) {
		return {};
	}
	return std::string(value);
}

void LinuxOS::SleepMsec(ulong msec)
{
	usleep(msec * 1000);
}

void LinuxOS::DetectKDE()
{
	auto envar = GetEnvVar("XDG_CURRENT_DESKTOP");
	if(envar.hasValue && envar.value == "KDE"){
		UseKde = true;
	}
}
