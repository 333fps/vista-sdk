# ==============================================================================
# VISTA-SDK-CPP - Test suite CMake configuration
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

project(vista-sdk-cpp-tests
	VERSION 0.1.0
	DESCRIPTION "DNV VISTA SDK C++ Tests"
	LANGUAGES CXX)

# --- GoogleTest integration ---
include(GoogleTest)

# --- Test source files ---
set(TEST_SOURCES
	Codebook.cpp
	Codebooks.cpp
	Gmod.cpp
	GmodPath.cpp
	GmodVersioning.cpp
	ImoNumber.cpp
	ISOString.cpp
	LocalId.cpp
	Locations.cpp
	UniversalId.cpp
)

# --- Set executables prefix ---
set(TEST_PREFIX "TEST_")

if(VISTA_SDK_CPP_BUILD_SMOKE_TESTS)
	list(APPEND TEST_SOURCES GmodVersioningSmoke.cpp)
endif()

# --- Copy test data at build time ---
if(EXISTS ${VISTA_SDK_TEST_DATA_DIR})
	set(VISTA_SDK_CPP_TEST_DATA_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/testdata)

	if(NOT TARGET CopyTestData)
		add_custom_target(CopyTestData ALL
			COMMAND ${CMAKE_COMMAND} -E make_directory ${VISTA_SDK_CPP_TEST_DATA_OUTPUT_DIR}
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				${VISTA_SDK_TEST_DATA_DIR} ${VISTA_SDK_CPP_TEST_DATA_OUTPUT_DIR}
			COMMENT "Copying VISTA SDK testdata to build directory ($<CONFIG>)"
			VERBATIM
		)
		message(STATUS "Test data will be copied from ${VISTA_SDK_TEST_DATA_DIR} to ${VISTA_SDK_CPP_TEST_DATA_OUTPUT_DIR} at build time")
	endif()
else()
	message(WARNING "Test data directory not found: ${VISTA_SDK_TEST_DATA_DIR}. Skipping test data copy.")
endif()

# --- Configure test executables ---
foreach(test_source ${TEST_SOURCES})
	get_filename_component(test_target_name ${test_source} NAME_WE)

	set(test_target_name ${TEST_PREFIX}${test_target_name})


	if(NOT TARGET ${test_target_name})
		add_executable(${test_target_name} TestDataLoader.h TestDataLoader.cpp ${test_source})

		target_precompile_headers(${test_target_name} PRIVATE pch.h)

		target_link_libraries(${test_target_name} PRIVATE
			vista-sdk-cpp
			GTest::gtest_main
			spdlog::spdlog
			fmt::fmt
			nlohmann_json::nlohmann_json
			zlibstatic
		)

		target_compile_definitions(${test_target_name} PRIVATE
			$<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF>
			$<$<AND:$<CONFIG:Debug>,$<BOOL:${CI_FORCE_DEBUG_SPDLOG_WARN}>>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_WARN>
		)

		target_include_directories(${test_target_name} PRIVATE
			${VISTA_SDK_CPP_INCLUDE_DIR}
		)

		if(TARGET CopyTestData)
			add_dependencies(${test_target_name} CopyTestData)
		endif()

		gtest_discover_tests(${test_target_name}
			WORKING_DIRECTORY "$<TARGET_FILE_DIR:${test_target_name}>"
			DISCOVERY_VERBOSE_OUTPUT TRUE
			PROPERTIES
				TIMEOUT 120
				WILL_FAIL FALSE
				OUTPUT_ON_FAILURE TRUE
		)

		set_target_properties(${benchmark_target_name} PROPERTIES
			CXX_STANDARD 20
			CXX_STANDARD_REQUIRED ON
			RUNTIME_OUTPUT_DIRECTORY "${VISTA_SDK_CPP_BIN_DIR}/$<CONFIG>"
			EXCLUDE_FROM_ALL FALSE
		)

		if(MSVC)
			target_compile_options(${test_target_name} PRIVATE
				/wd4625 # copy constructor was implicitly defined as deleted
				/wd4626 # assignement operator was implicitly defined as deleted
				/wd4702 # unreachable code
				/wd4834 # discarding return value of function with [[nodiscard]] attribute
				/wd5026 # move constructor was implicitly defined as deleted
				/wd5027 # move assignment operator was implicitly defined as deleted
			)
		endif()
	endif()
endforeach()
