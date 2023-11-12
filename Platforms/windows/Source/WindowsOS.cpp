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

ulong WindowsOS::GetTicksUsec(){
	LARGE_INTEGER ticksPerSecond;
	QueryPerformanceFrequency(&ticksPerSecond);
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);
	return (ulong)((tick.QuadPart * 1000000) / ticksPerSecond.QuadPart);
}

void WindowsOS::SleepUsec(ulong usec){
	Sleep((DWORD)(usec / 1000));
}

void *WindowsOS::LoadSharedLibrary(const std::string &path)
{
	HMODULE sharedObj = LoadLibrary(path.c_str());
	if (!sharedObj) {
		m_Log << Error << "Failed to load shared library: " << (int)GetLastError();
		return nullptr;
	}

	return sharedObj;
}

void WindowsOS::UnloadSharedLibrary(void *handle)
{
	FreeLibrary((HMODULE)handle);
}

void *WindowsOS::GetFunctionPointer(void *handle, const std::string &functionName)
{
	auto ptr = GetProcAddress((HMODULE)handle, functionName.c_str());
	return (void*)ptr;
}

int WindowsOS::AskSelection(const std::string &title,
							const std::string &bodyMessage,
							const std::vector<std::string> &options)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	const LPCSTR CLASS_NAME = "AskSelectionWindow";
	WNDCLASS wc = {};
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW - 1);
	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(0,
							   CLASS_NAME,
							   title.data(),
							   WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
							   300, 300, NULL, NULL, hInstance, NULL);

	if (hwnd == NULL) {
		m_Log << Error << "Failed to create window: " << (int)GetLastError();
		return -1;
	}

	// List box
	HWND hListBox = CreateWindowEx(0, "LISTBOX", NULL,
								   WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_STANDARD,
								   10, 10, 260, 200, hwnd, NULL, hInstance, NULL);

	// Add items
	for (const auto & option : options) {
		SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)option.c_str());
	}

	//Create ok button
	HWND button = CreateWindowEx(0, "BUTTON", "OK",
				   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				   10, 220, 260, 30, hwnd, (HMENU)1, hInstance, NULL);

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	int selectedItemIndex = -1;

	MSG msg;
	BOOL bRet;
	while ((bRet = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) != 0 || IsWindow(hwnd))
	{
		if (bRet == -1)
		{
			// Error occurred, handle it if needed
			return -1;
		}

		if (msg.message == WM_QUIT)
			break;

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		// Add your additional checks here, for example:
		if (msg.hwnd == button && msg.message == WM_LBUTTONUP)
		{
			// OK button is clicked, close the window
			selectedItemIndex = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
			DestroyWindow(hwnd);
		}
	}

	return selectedItemIndex;
}

WindowsOS::WindowsOS()
{
}

WindowsOS::~WindowsOS()
{
}

void WindowsOS::ShowMessageBox(MessageBoxType type, const std::string &title, const std::string &message)
{
	OS::ShowMessageBox(type, title, message);
	HWND hwnd = GetActiveWindow();
	UINT uType = 0;
	switch (type) {
		case MessageBox_Info:
			uType = MB_OK | MB_ICONINFORMATION;
			break;
		case MessageBox_Warning:
			uType = MB_OK | MB_ICONWARNING;
			break;
		case MessageBox_Error:
			uType = MB_OK | MB_ICONERROR;
			break;
	}

	MessageBox(hwnd, message.c_str(), title.c_str(), uType);
}

Optional<std::string> WindowsOS::GetEnvVar(const std::string &name)
{
	char buffer[1024];
	DWORD size = GetEnvironmentVariable(name.c_str(), buffer, 1024);
	if (size == 0) {
		return {};
	}
	return std::string(buffer);
}
