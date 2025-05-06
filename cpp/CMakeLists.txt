# ==============================================================================
# VISTA-SDK-CPP - C++ Library CMake Configuration
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

# --- Project Setup ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --- Dependencies ---

# --- nlohmann/json (Header-only, manual download) ---
set(NLOHMANN_JSON_VERSION "v3.12.0")
set(NLOHMANN_JSON_URL "https://github.com/nlohmann/json/releases/download/${NLOHMANN_JSON_VERSION}/json.hpp")
set(NLOHMANN_JSON_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_deps/nlohmann_json-include")
set(NLOHMANN_JSON_HEADER "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/json.hpp")

if(NOT EXISTS ${NLOHMANN_JSON_HEADER})
	message(STATUS "Downloading nlohmann/json header (${NLOHMANN_JSON_VERSION})...")
	file(DOWNLOAD ${NLOHMANN_JSON_URL} ${NLOHMANN_JSON_HEADER}
		 SHOW_PROGRESS
		 STATUS download_status
		 LOG download_log)
	list(GET download_status 0 download_code)
	if(NOT download_code EQUAL 0)
		message(FATAL_ERROR "Failed to download nlohmann/json header. Error: ${download_log}")
	else()
		message(STATUS "Downloaded nlohmann/json header to ${NLOHMANN_JSON_HEADER}")
	endif()
else()
	message(STATUS "Found existing nlohmann/json header: ${NLOHMANN_JSON_HEADER}")
endif()

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${NLOHMANN_JSON_INCLUDE_DIR})
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)

# --- FetchContent Dependencies ---
include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.15.2
  GIT_SHALLOW    TRUE
  CMAKE_ARGS     "-DSPDLOG_BUILD_EXAMPLE=OFF;-DSPDLOG_BUILD_TESTING=OFF;-DSPDLOG_BUILD_SHARED=OFF"
)

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        v1.3.1
  GIT_SHALLOW    TRUE
  CMAKE_ARGS     "-DZLIB_BUILD_TESTING=OFF;-DZLIB_BUILD_SHARED=OFF;-DZLIB_BUILD_STATIC=ON;-DZLIB_BUILD_MINIZIP=OFF;-DZLIB_INSTALL=OFF"
)

FetchContent_Declare(
  cpuid
  GIT_REPOSITORY https://github.com/anrieff/libcpuid.git
  GIT_TAG        v0.7.0
  GIT_SHALLOW    TRUE
  CMAKE_ARGS     "-DLIBCPUID_ENABLE_TESTS=OFF;-DBUILD_SHARED_LIBS=OFF;-DLIBCPUID_BUILD_DRIVERS=OFF;-DLIBCPUID_ENABLE_DOCS=OFF"
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.16.0
  GIT_SHALLOW    TRUE
  CMAKE_ARGS     "-DBUILD_GMOCK=OFF;-DINSTALL_GTEST=OFF;-DBUILD_SHARED_LIBS=OFF"
)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        11.0.2
  GIT_SHALLOW    TRUE
  CMAKE_ARGS     "-DFMT_TEST=OFF;-DBUILD_SHARED_LIBS=OFF"
)

FetchContent_MakeAvailable(fmt spdlog zlib cpuid googletest)

# --- Build Options ---
option(VISTA_SDK_BUILD_TESTS		"Build tests"							ON	)
option(VISTA_SDK_COPY_RESOURCES		"Copy resources to build directory" 	ON	)
option(VISTA_SDK_BUILD_SAMPLES		"Build vista-sdk-cpp samples"			ON	)

# --- Platform Specific Settings ---
if(MSVC)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
	set(BUILD_SHARED_LIBS TRUE)
endif()

# --- Ccache Setup ---
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
	message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
	set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
	set(ENV{CCACHE_SLOPPINESS} "pch_defines,time_macros")
	set(ENV{CCACHE_MAXSIZE} "5G")
	if(MSVC)
		set(ENV{CCACHE_CPP2} "yes")
		set(ENV{CCACHE_BASEDIR} "${CMAKE_SOURCE_DIR}")
	endif()
	message(STATUS "ccache enabled - compilation will be accelerated")
else()
	message(STATUS "ccache not found - normal compilation will be used")
endif()

# --- Output Directories ---
set(VISTA_SDK_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VISTA_SDK_BIN_DIR}/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VISTA_SDK_BIN_DIR}/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${VISTA_SDK_BIN_DIR}/$<CONFIG>)

# --- Compiler Settings ---
include(CheckCXXSourceCompiles)

if(MSVC)
	add_compile_options(/std:c++20)
	add_compile_options(/MP)
	add_compile_options(/W4)
	add_compile_options(/Wall)
	add_compile_options(/WX-)
	add_compile_options(/permissive-)
	add_compile_options(/fp:precise)
	add_compile_options(/Zc:__cplusplus)
	add_compile_options(/Zc:inline)
	add_compile_options(/Zc:preprocessor)
	add_compile_options(/external:anglebrackets)
	add_compile_options(/external:W0)
	#add_compile_options(/analyze:WX-)
	#add_compile_options(/analyze:external-)
	#add_compile_options(/analyze)

	#add_compile_options(/wd4061) # switch not handled
	#add_compile_options(/wd4100) # unreferenced formal parameter
	#add_compile_options(/wd4189) # local variable is initialized but not referenced
	#add_compile_options(/wd4244) # 'conversion' conversion from 'type1' to 'type2', possible loss of data
	#add_compile_options(/wd4267) # conversion from 'size_t' to 'type', possible loss of data
	#add_compile_options(/wd4365) # conversion signed/unsigned mismatch
	#add_compile_options(/wd4388) # signed/unsigned mismatch
	#add_compile_options(/wd4456) # declaration hides previous local declaration
	#add_compile_options(/wd4514) # 'function' : unreferenced inline function has been removed
	#add_compile_options(/wd4625) # copy constructor was implicitly defined as deleted
	#add_compile_options(/wd4626) # assignement operator was implicitly defined as deleted
	add_compile_options(/wd4710) # 'function' : function not inlined
	add_compile_options(/wd4711) # function 'function' selected for inline expansion
	add_compile_options(/wd4820) # padding
	#add_compile_options(/wd4868) # compiler may not enforce left-to-right evaluation order in braced initializer list
	#add_compile_options(/wd5026) # move constructor was implicitly defined as deleted
	#add_compile_options(/wd5027) # move assignement operator was implicitly defined as deleted
	#add_compile_options(/wd5038) # will be initialized after
	add_compile_options(/wd5045) # Qspectre
	#add_compile_options(/wd5267) # definition of implicit copy constructor/assignment operator is deprecated because it has a user-provided assignment operator/copy constructor
	#add_compile_options(/wd6246) # local declaration of 'variable' hides declaration of same name in outer scope
	#add_compile_options(/wd6326) # potential comparison of a constant with another constant
	#add_compile_options(/wd6387) # 'argument' may be 'value': this does not adhere to the specification for the function

	add_compile_options(/arch:AVX)
	add_compile_options(/arch:AVX2)
else() # GCC / Clang
	add_compile_options(-Wall -Wextra -Wpedantic)

	add_compile_options(-msse4.2)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-fdiagnostics-color=always)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		add_compile_options(-fcolor-diagnostics)
	endif()
endif()

# --- Copy Resources (Build Time) ---
if(VISTA_SDK_COPY_RESOURCES)
	if(EXISTS ${VISTA_SDK_RESOURCE_DIR})
		set(VISTA_SDK_RESOURCE_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources)
		add_custom_target(CopyResources ALL
			COMMAND ${CMAKE_COMMAND} -E make_directory ${VISTA_SDK_RESOURCE_OUTPUT_DIR}
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				${VISTA_SDK_RESOURCE_DIR} ${VISTA_SDK_RESOURCE_OUTPUT_DIR}
			COMMENT "Copying VISTA SDK resources to build directory ($<CONFIG>)"
			VERBATIM
		)
		message(STATUS "Resources will be copied from ${VISTA_SDK_RESOURCE_DIR} to ${VISTA_SDK_RESOURCE_OUTPUT_DIR} at build time")
	else()
		message(WARNING "Resources directory not found: ${VISTA_SDK_RESOURCE_DIR}. Skipping resource copy.")
	endif()
else()
	message(STATUS "Skipping resource copy based on VISTA_SDK_COPY_RESOURCES option.")
endif()

# --- Source Files ---

set(PUBLIC_HEADERS
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ChdDictionary.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ChdDictionary.inl
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/Codebook.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/Codebooks.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/CodebookName.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/CodebooksDto.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/EmbeddedResource.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/Gmod.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/GmodDto.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/GmodNode.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/GmodPath.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/GmodVersioning.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/GmodVersioningDto.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ILocalId.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ILocalId.inl
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ILocalIdBuilder.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ILocalIdBuilder.inl
	# ${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/IUniversalIdBuilder.h
	# ${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/IUniversalId.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ImoNumber.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ISO19848Dtos.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/LocalId.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/LocalIdBuilder.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/LocalIdItems.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/LocalIdParsingErrorBuilder.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/Locations.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/LocationsDto.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/MetadataTag.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ParsingErrors.h
	# ${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/UniversalIdBuilder.h
	# ${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/UniversalId.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/VIS.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/VisVersion.h
)

set(PRIVATE_SOURCES
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/ChdDictionary.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/Codebook.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/Codebooks.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/CodebookName.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/CodebooksDto.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/EmbeddedResource.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/Gmod.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/GmodDto.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/GmodNode.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/GmodPath.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/GmodVersioning.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/GmodVersioningDto.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/ImoNumber.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/ISO19848Dtos.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/LocalId.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/LocalIdBuilder.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/LocalIdItems.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/LocalIdParsingErrorBuilder.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/Locations.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/LocationsDto.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/MetadataTag.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/ParsingErrors.cpp
	# ${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/UniversalIdBuilder.cpp
	# ${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/UniversalId.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/VIS.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/VisVersion.cpp
)

# --- Library Definition ---
add_library(${PROJECT_NAME} SHARED)

# Add sources to the target.
target_sources(${PROJECT_NAME}
	PUBLIC
		${PUBLIC_HEADERS}
	PRIVATE
		${PRIVATE_SOURCES}
)

# --- Precompiled Headers ---
target_precompile_headers(${PROJECT_NAME} PRIVATE ${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/pch.h)

# --- Include Directories ---
target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${VISTA_SDK_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${VISTA_SDK_SOURCE_DIR}
		${zlib_SOURCE_DIR}
)

# --- Link Libraries ---
target_link_libraries(${PROJECT_NAME} PRIVATE
	spdlog::spdlog
	nlohmann_json::nlohmann_json
	zlibstatic
	cpuid
	fmt::fmt
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	$<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF>
)

# --- Samples ---
# if(VISTA_SDK_BUILD_SAMPLES)
#     add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../examples ${CMAKE_CURRENT_BINARY_DIR}/examples)
# endif()

# --- Tests ---
if(VISTA_SDK_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()

# --- Configuration Summary ---
get_directory_property(COMPILE_OPTIONS COMPILE_OPTIONS)
get_directory_property(COMPILE_DEFINITIONS COMPILE_DEFINITIONS)
get_directory_property(LINK_OPTIONS LINK_OPTIONS)
string(REPLACE ";" " " FORMATTED_COMPILE_OPTIONS "${COMPILE_OPTIONS}")
string(REPLACE ";" " " FORMATTED_COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS}")
string(REPLACE ";" " " FORMATTED_LINK_OPTIONS "${LINK_OPTIONS}")

message(STATUS "========== VISTA SDK C++ Configuration ==========")
message(STATUS "Project:                 ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Description:             ${PROJECT_DESCRIPTION}")
message(STATUS "Top-Level Source Dir:    ${CMAKE_SOURCE_DIR}")
message(STATUS "Build Directory:         ${CMAKE_BINARY_DIR}")
message(STATUS "Generator:               ${CMAKE_GENERATOR}")
message(STATUS "Platform:                ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_PROCESSOR})")
message(STATUS "Build Type:              ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Shared Libs:       ${BUILD_SHARED_LIBS}")
message(STATUS "C++ Compiler:            ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Compiler Path:       ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard:            ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Tests:             ${VISTA_SDK_BUILD_TESTS}")
message(STATUS "Build Samples:           ${VISTA_SDK_BUILD_SAMPLES}")
message(STATUS "Copy Resources:          ${VISTA_SDK_COPY_RESOURCES}")
if(VISTA_SDK_COPY_RESOURCES)
	message(STATUS "Resources Source:        ${VISTA_SDK_RESOURCE_DIR}")
	message(STATUS "Resources Target:        ${VISTA_SDK_BIN_DIR}/<CONFIG>/resources")
endif()
message(STATUS "Output Base Directory:   ${VISTA_SDK_BIN_DIR}")
if(CCACHE_PROGRAM)
	message(STATUS "ccache Enabled:          ${CCACHE_PROGRAM}")
else()
	message(STATUS "ccache Enabled:          No")
endif()
message(STATUS "")
message(STATUS "---------- Compiler Options ----------")
message(STATUS "Compile Options:         ${FORMATTED_COMPILE_OPTIONS}")
message(STATUS "Compile Definitions:     ${FORMATTED_COMPILE_DEFINITIONS}")
message(STATUS "Link Options:            ${FORMATTED_LINK_OPTIONS}")

# --- ccache Statistics ---
if(CCACHE_PROGRAM)
	message(STATUS "")
	message(STATUS "---------- ccache Statistics ----------")
	execute_process(
		COMMAND ${CCACHE_PROGRAM} -s
		OUTPUT_VARIABLE CCACHE_STATS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	string(REPLACE "\n" ";" CCACHE_STATS_LINES "${CCACHE_STATS}")
	foreach(LINE ${CCACHE_STATS_LINES})
		message(STATUS "${LINE}")
	endforeach()
endif()

message(STATUS "========================================")
