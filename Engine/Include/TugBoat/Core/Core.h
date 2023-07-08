#pragma once

#include <memory>
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

#if defined(TB_PLATFORM_LINUX) or defined(TB_PLATFORM_MACOS)
#if TB_DYNAMIC_LINK
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
#elif defined(TB_PLATFORM_LINUX) || defined(TB_PLATFORM_MACOS)
#include <signal.h>
#define BREAK() raise(SIGTRAP)
#else
#error "Break not supported on platform."
#endif
#define TB_ENABLE_ASSERTS
#else
#define BREAK()
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

#include "AddShortTypes.h"

namespace TugBoat {
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
}
