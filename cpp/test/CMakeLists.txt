# ==============================================================================
# VISTA-SDK-CPP - Test Suite CMake Configuration
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

project(vista-sdk-cpp-tests
	VERSION 1.0.0
	DESCRIPTION "DNV VISTA SDK C++ Tests"
	LANGUAGES CXX)

# --- GoogleTest Integration ---
include(GoogleTest)

# --- Test Executable Definitions ---
set(TEST_SOURCES
	CodebookTests.cpp
	CodebooksTests.cpp
	GmodPathTests.cpp
	GmodTests.cpp
	GmodVersioningTests.cpp
	ImoNumberTests.cpp
	ISOStringTests.cpp
	LocalIdTests.cpp
	LocationsTests.cpp
)

# --- Configure Each Test Target ---
foreach(test_source ${TEST_SOURCES})
	get_filename_component(test_target ${test_source} NAME_WE)

	if(NOT TARGET ${test_target})
		add_executable(${test_target} ${test_source})

		target_precompile_headers(${test_target} PRIVATE pch.h)

		target_link_libraries(${test_target} PRIVATE
			vista-sdk-cpp
			GTest::gtest_main
			spdlog::spdlog
			nlohmann_json::nlohmann_json
			zlibstatic
		)

		# Add include directories.
		target_include_directories(${test_target} PRIVATE
			${VISTA_SDK_INCLUDE_DIR}
		)

		target_compile_definitions(${test_target} PRIVATE
			 $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF>
		)

		set(TARGET_OUTPUT_DIR "$<TARGET_FILE_DIR:${test_target}>")

		# Discover tests.
		gtest_discover_tests(${test_target}
			WORKING_DIRECTORY ${TARGET_OUTPUT_DIR}
			DISCOVERY_VERBOSE_OUTPUT TRUE
		)

		# Apply MSVC-specific warning suppressions.
		if(MSVC)
			target_compile_options(${test_target} PRIVATE
				/wd4625 # copy constructor was implicitly defined as deleted
				/wd4626 # assignement operator was implicitly defined as deleted
				/wd4702 # unreachable code
				/wd5026 # move constructor was implicitly defined as deleted
				/wd5027 # move assignment operator was implicitly defined as deleted
			)
		endif()

	endif()
endforeach()

# --- Copy Test Data (Build Time) ---
if(EXISTS ${VISTA_SDK_TEST_DATA_DIR})
	set(VISTA_SDK_TESTDATA_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testdata)

	if(NOT TARGET CopyTestData)
		add_custom_target(CopyTestData ALL
			COMMAND ${CMAKE_COMMAND} -E make_directory ${VISTA_SDK_TESTDATA_OUTPUT_DIR}
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				${VISTA_SDK_TEST_DATA_DIR} ${VISTA_SDK_TESTDATA_OUTPUT_DIR}
			COMMENT "Copying VISTA SDK testdata to build directory ($<CONFIG>)"
			VERBATIM
		)
		message(STATUS "testdata will be copied from ${VISTA_SDK_TEST_DATA_DIR} to ${VISTA_SDK_TESTDATA_OUTPUT_DIR} at build time")
	endif()
else()
	message(WARNING "testdata directory not found: ${VISTA_SDK_TEST_DATA_DIR}. Skipping testdata copy.")
endif()
