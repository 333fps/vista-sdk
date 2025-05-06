# ==============================================================================
# VISTA-SDK-CPP - Top-Level CMake Configuration
# ==============================================================================
# DNV VISTA SDK C++ Implementation

# --- Project Definition ---
cmake_minimum_required(VERSION 3.20)

project(vista-sdk-cpp
	VERSION 0.1.0
	DESCRIPTION "DNV VISTA SDK C++ Implementation"
	LANGUAGES CXX
)

# --- Build Configuration ---
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are not allowed. Please use a separate build directory.")
endif()

# --- Define Project Directories ---
set(VISTA_SDK_INCLUDE_DIR	"${CMAKE_CURRENT_SOURCE_DIR}/cpp/include" CACHE PATH "Path to the C++ SDK include directory")
set(VISTA_SDK_SOURCE_DIR 	"${CMAKE_CURRENT_SOURCE_DIR}/cpp/src"     CACHE PATH "Path to the C++ SDK source directory")
set(VISTA_SDK_RESOURCE_DIR	"${CMAKE_CURRENT_SOURCE_DIR}/resources"   CACHE PATH "Path to the SDK resources directory")
set(VISTA_SDK_TEST_DATA_DIR	"${CMAKE_CURRENT_SOURCE_DIR}/testdata"    CACHE PATH "Path to the SDK test data directory")

# --- Add Subdirectories ---
add_subdirectory(cpp)

# --- Configuration Summary ---
message(STATUS "--- VISTA SDK Configuration Summary ---")
message(STATUS "Build Type		: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Directory	: ${CMAKE_BINARY_DIR}")
message(STATUS "Source Directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "Include Dir		: ${VISTA_SDK_INCLUDE_DIR}")
message(STATUS "Resource Dir	: ${VISTA_SDK_RESOURCE_DIR}")
message(STATUS "Test Data Dir	: ${VISTA_SDK_TEST_DATA_DIR}")
message(STATUS "---------------------------------------")
