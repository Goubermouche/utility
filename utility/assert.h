#pragma once
#include "streams/console.h"
#include "types.h"

#ifdef DEBUG
#define DEBUG_MODE
#else
#define RELEASE_MODE
#endif

#ifdef _WIN32
#include <intrin.h>

#define DEBUG_BREAK() __debugbreak()
#define SYSTEM_WINDOWS
#elif __linux__
#include <signal.h>

#define DEBUG_BREAK() raise(SIGTRAP)
#define SYSTEM_LINUX
#else
#error "Unsupported platform!"
#endif

#ifdef DEBUG_MODE
#define ASSERT(__condition, __message, ...)                \
  do {                                                     \
    if(!(__condition)) {                                   \
      utility::console::print_err(__message, __VA_ARGS__); \
			utility::console::flush();                           \
      DEBUG_BREAK();                                       \
    }                                                      \
  } while(false)
#else
#define ASSERT(__condition, __message)
#endif
