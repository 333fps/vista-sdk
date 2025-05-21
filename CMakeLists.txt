# ==============================================================================
# VISTA-SDK-CPP - Top-level CMake configuration
# ==============================================================================

# --- Project definition ---
cmake_minimum_required(VERSION 3.20)

project(vista-sdk-cpp
	VERSION 0.1.0
	DESCRIPTION "DNV VISTA SDK C++ implementation"
	LANGUAGES CXX
)

# --- Build configuration ---
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "In-source builds are not allowed. Please use a separate build directory.")
endif()

# --- Testing setup ---
enable_testing()

# --- Define project directories ---
set(VISTA_SDK_CPP_INCLUDE_DIR         "${CMAKE_CURRENT_SOURCE_DIR}/cpp/include"  CACHE PATH "Path to the C++ SDK include directory")
set(VISTA_SDK_CPP_SOURCE_DIR          "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src"      CACHE PATH "Path to the C++ SDK source directory")
set(VISTA_SDK_CPP_TEST_DIR            "${CMAKE_CURRENT_SOURCE_DIR}/cpp/test"     CACHE PATH "Path to the C++ SDK test directory")
set(VISTA_SDK_CPP_SAMPLES_DIR         "${CMAKE_CURRENT_SOURCE_DIR}/cpp/samples"  CACHE PATH "Path to the C++ SDK samples directory")
set(VISTA_SDK_CPP_DOCUMENTATION_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/cpp/doc"      CACHE PATH "Path to the C++ SDK Doxygen documentation directory")
set(VISTA_SDK_RESOURCE_DIR            "${CMAKE_CURRENT_SOURCE_DIR}/resources"    CACHE PATH "Path to the SDK resources directory")
set(VISTA_SDK_TEST_DATA_DIR           "${CMAKE_CURRENT_SOURCE_DIR}/testdata"     CACHE PATH "Path to the SDK test data directory")

# --- Add core C++ library subdirectory ---
add_subdirectory(cpp/cmake)

# --- Top-level configuration summary ---
message(STATUS "--- VISTA SDK Top-Level Configuration Summary ---")
message(STATUS "Build Type               : ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Directory          : ${CMAKE_BINARY_DIR}")
message(STATUS "Source Directory         : ${CMAKE_SOURCE_DIR}")
message(STATUS "Include Dir              : ${VISTA_SDK_CPP_INCLUDE_DIR}")
message(STATUS "Source Dir               : ${VISTA_SDK_CPP_SOURCE_DIR}")
message(STATUS "Test Dir                 : ${VISTA_SDK_CPP_TEST_DIR}")
message(STATUS "Sample Dir               : ${VISTA_SDK_CPP_SAMPLES_DIR}")
message(STATUS "Doxygen docementation Dir: ${VISTA_SDK_CPP_DOCUMENTATION_DIR}")
message(STATUS "Resource Dir             : ${VISTA_SDK_RESOURCE_DIR}")
message(STATUS "Test Data Dir            : ${VISTA_SDK_TEST_DATA_DIR}")
message(STATUS "-------------------------------------------------")
