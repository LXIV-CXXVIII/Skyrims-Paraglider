#pragma once
#define NOMINMAX
//_CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "xbyak/xbyak.h"
//_CRT_SECURE_NO_WARNINGS;

//#include "C:/dev/steamworks_sdk_152/sdk/public/steam/steam_api.h"
//#pragma comment(lib, "steam_api64.lib");
//#define NOMINMAX
//#include <Windows.h>

#ifdef NDEBUG
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif

#ifdef SKSE_SUPPORT_XBYAK
[[nodiscard]] void* allocate(Xbyak::CodeGenerator& a_code);
#endif

using namespace std::literals;

namespace logger = SKSE::log;

#define DLLEXPORT __declspec(dllexport)

namespace stl
{
    using nonstd::span;
    using SKSE::stl::report_and_fail;
}