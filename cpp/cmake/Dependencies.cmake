# ==============================================================================
# VISTA-SDK-CPP - C++ Library Dependencies
# ==============================================================================

# --- FetchContent dependencies ---
include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.15.3
  GIT_SHALLOW    TRUE
  CMAKE_ARGS
	-DSPDLOG_BUILD_SHARED=OFF
	-DSPDLOG_ENABLE_PCH=ON
	-DSPDLOG_BUILD_EXAMPLE=OFF
	-DSPDLOG_BUILD_EXAMPLE_HO=OFF
	-DSPDLOG_BUILD_TESTS=OFF
	-DSPDLOG_BUILD_EXAMPLE_HO=OFF
	-DSPDLOG_BUILD_BENCH=OFF
	-DSPDLOG_BUILD_WARNINGS=OFF
	-DSPDLOG_FMT_EXTERNAL=ON
	-DSPDLOG_NO_EXCEPTIONS=ON
	-DSPDLOG_PREVENT_CHILD_FD=OFF
)

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        v1.3.1
  GIT_SHALLOW    TRUE
  CMAKE_ARGS
	-DZLIB_BUILD_SHARED=OFF
	-DZLIB_BUILD_TESTING=OFF
	-DZLIB_BUILD_STATIC=ON
	-DZLIB_BUILD_MINIZIP=OFF
	-DZLIB_PREFIX=OFF
	-DZLIB_INSTALL=OFF
)

FetchContent_Declare(
  cpuid
  GIT_REPOSITORY https://github.com/anrieff/libcpuid.git
  GIT_TAG        v0.8.0
  GIT_SHALLOW    TRUE
  CMAKE_ARGS
	-DBUILD_SHARED_LIBS=OFF
	-DLIBCPUID_ENABLE_TESTS=OFF
	-DLIBCPUID_BUILD_DRIVERS=OFF
	-DLIBCPUID_ENABLE_DOCS=OFF
)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        11.2.0
  GIT_SHALLOW    TRUE
  CMAKE_ARGS
	-DBUILD_SHARED_LIBS=OFF
	-DFMT_FUZZ=OFF
	-DFMT_TEST=OFF
	-DFMT_DOC=OFF
	-DFMT_INSTALL=OFF
	-DFMT_HEADER_ONLY=ON
	-DFMT_OS=OFF
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
  GIT_SHALLOW    TRUE
  CMAKE_ARGS
	-DBUILD_SHARED_LIBS=OFF
	-DBUILD_GMOCK=OFF
	-DINSTALL_GTEST=OFF
	-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>DLL
)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.4
  GIT_SHALLOW    TRUE
  CMAKE_ARGS
	-DBUILD_SHARED_LIBS=OFF
	-DBENCHMARK_ENABLE_INSTALL=OFF
	-DBENCHMARK_ENABLE_TESTING=OFF
	-DBENCHMARK_ENABLE_GTEST_TESTS=OFF
	-DBENCHMARK_USE_BUNDLED_GTEST =OFF
	-DBENCHMARK_DOWNLOAD_DEPENDENCIES=OFF
)

FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG        v3.13.0
  GIT_SHALLOW    TRUE
  CMAKE_ARGS
	-DBUILD_SHARED_LIBS=OFF
	-DSIMDJSON_ENABLE_THREADS=ON
	-DSIMDJSON_SANITIZE=OFF
	-DSIMDJSON_BUILD_STATIC_LIB=ON
	-DSIMDJSON_DISABLE_DEPRECATED_API=ON
	-DSIMDJSON_DEVELOPMENT_CHECKS=OFF
)

FetchContent_MakeAvailable(fmt spdlog zlib cpuid simdjson googletest googlebenchmark)
