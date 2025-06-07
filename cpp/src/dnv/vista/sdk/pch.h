#pragma once

#include <immintrin.h>

/* STL */
#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <regex>
#include <set>
#include <shared_mutex>
#include <span>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

/* Libs */
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <zlib.h>
#if defined( _MSC_VER )
#	include <libcpuid.h>
#elif defined( __GNUC__ )
#	include <cpuid.h>
#endif
