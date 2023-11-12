#pragma once
typedef int BID;
#define INVALID_BID -1

#ifdef _WIN32
#ifdef _WIN64
#define TB_PLATFORM_WINDOWS
#else
#error "x86 not supported."
#endif
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
#error "IOS Sim not supported."
#elif TARGET_OS_IPHONE == 1
#error "IOS not supported."
#elif TARGET_OS_MAC == 1
#define TB_PLATFORM_MACOS
#else
#error "Unknown apple platform."
#endif

#elif defined(__ANDROID__)
#define TB_PLATFORM_ANDROID
#error "Android not supported. :["
#elif defined(__linux__)
#define TB_PLATFORM_LINUX
#elif defined(__WIIU__)
#define TB_PLATFORM_WIIU
#error "Wii U not supported. :["
#elif defined(__EMSCRIPTEN__)
#define TB_PLATFORM_WEB
#elif defined(__3DS__)
#define TB_PLATFORM_3DS
#else
#error "Unknown platform."
#endif

#ifdef TB_PLATFORM_WINDOWS
#ifdef TB_BUILD_DLL
#define TB_API
#else
#define TB_API
#endif
#endif

#if defined(TB_PLATFORM_LINUX) || defined(TB_PLATFORM_MACOS) || defined(TB_PLATFORM_ANDROID) || defined(TB_PLATFORM_WEB) || defined(TB_PLATFORM_3DS)
#ifdef TB_DYNAMIC_LINK
#ifdef TB_BUILD_DLL
#define TB_API __attribute__((visibility("default")))
#else
#define TB_API
#endif
#else
#define TB_API
#endif
#endif

#ifdef TB_DEBUG
#if defined(TB_PLATFORM_WINDOWS)
#include <windows.h>
#undef CreateWindow
#define BREAK() DebugBreak()
#elif defined(TB_PLATFORM_LINUX) || defined(TB_PLATFORM_MACOS) || defined(TB_PLATFORM_ANDROID) || defined(TB_PLATFORM_WEB) || defined(TB_PLATFORM_3DS)
#include <signal.h>
#define BREAK() raise(SIGTRAP)
#else
#error "Break not supported on platform."
#endif
#define TB_ENABLE_ASSERTS
#else
#define BREAK()
#endif

#ifdef TB_DEBUG
#define TB_DEBUG_BOOL true
#else
#define TB_DEBUG_BOOL false
#endif

#ifdef TB_ENABLE_ASSERTS
#define TB_ASSERT(x, ...)                                   \
    {                                                       \
        if (!(x)) {                                         \
            TB_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
            BREAK();                                        \
        }                                                   \
    }
#define TB_CORE_ASSERT(x, ...)                                   \
    {                                                            \
        if (!(x)) {                                              \
            TB_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
            BREAK();                                             \
        }                                                        \
    }
#else
#define TB_ASSERT(x, ...)
#define TB_CORE_ASSERT(x, ...)
#endif

//Windows hacky bits
#ifdef TB_PLATFORM_WINDOWS
//Windows doesnt have ulong for some reason?
#define ulong unsigned long
#endif

#include "Core/AddShortTypes.h"
#include <memory>

//A define that helps with user friendly error messages that need to be shown to the user
/*
 * m_Log << Error << "Failed to create swapchain";
		OS::GetInstance()->ShowMessageBox(MessageBox_Error, "Failed to create swapchain", "Failed to create swapchain");

 */

#define TB_SHOW_ERROR(message) OS::GetInstance()->ShowMessageBox(MessageBox_Error, "An error has occured", message)
#define TB_SHOW_FATAL_ERROR(message) OS::GetInstance()->ShowMessageBox(MessageBox_Error, "A fatal error has occured", message); m_Log << Fatal << message

namespace TugBoat {

struct Version {
	uint8_t major;
	uint16_t minor;
	uint16_t patch;

	Version() : major(0), minor(0), patch(0) {}

	Version(uint8_t major, uint16_t minor, uint16_t patch) : major(major), minor(minor), patch(patch) {}

	[[nodiscard]] uint32_t To32() const{
		return (major << 22) | (minor << 12) | patch;
	}
};

#define CLASS_DELETE_COPY(classname) classname (const classname&) = delete;
#define CLASS_DELETE_SETTER(classname) classname& operator= (const classname&) = delete;

template <typename T>
using Scope = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr Scope<T> CreateScope(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

//class / struct called Optional<T>
//has a bool that says if it has a value or not
//has a T value

template <typename T>
struct Optional {
	bool hasValue;
	T value;

	Optional() : hasValue(false), value(T()) {}

	Optional(T value) : hasValue(true), value(value) {}

	Optional& operator=(T value)
	{
		hasValue = true;
		this->value = value;
		return *this;
	}

	explicit operator bool() const
	{
		return hasValue;
	}

	T& operator*()
	{
		return value;
	}

	const T& operator*() const
	{
		return value;
	}

	T* operator->()
	{
		return &value;
	}

	const T* operator->() const
	{
		return &value;
	}
};
}
