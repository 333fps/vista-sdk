#pragma once

#ifdef _WIN32
#	define NOMINMAX
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <psapi.h>
#	pragma comment( lib, "psapi.lib" )
#endif

/* STL */
#include <array>
#include <optional>
#include <mutex>
#include <shared_mutex>
#include <unordered_set>

/* Libs */
#include <fmt/format.h>
#include <simdjson.h>
#include <benchmark/benchmark.h>
