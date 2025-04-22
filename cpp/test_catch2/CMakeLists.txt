cmake_minimum_required(VERSION 3.20)

find_package(spdlog CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
include(CTest)
include(Catch)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(TESTS
	ImoNumber_catch2
	ISOString_catch2
	Codebook_catch2
	Codebooks_catch2
	Gmod_catch2
	LocalId_catch2
)

foreach(TEST ${TESTS})
if(NOT TARGET ${TEST})
	add_executable(${TEST} ${TEST}.cpp)

	target_precompile_headers(${TEST} PRIVATE pch.h)

	target_link_libraries(${TEST} PRIVATE
		vista-sdk-cpp
		Catch2::Catch2WithMain
		spdlog::spdlog_header_only
	)

	target_include_directories(${TEST} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

	# Set output directory for each test executable
	set_target_properties(${TEST} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release
	)

	catch_discover_tests(
		${TEST}
		TEST_PREFIX "${TEST}"
		EXTRA_ARGS "--verbosity=2"
	)

	# Add dependency on CopyTestData
	add_dependencies(${TEST} CopyTestData)
endif()
endforeach()

# Copy testdata
if(EXISTS ${VISTA_SDK_TEST_DATA_DIR})
	add_custom_target(CopyTestData ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory ${VISTA_SDK_OUTPUT_DIR}/testdata
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			${VISTA_SDK_TEST_DATA_DIR} ${VISTA_SDK_OUTPUT_DIR}/testdata
		COMMENT "Copying VISTA SDK testdata to build directory"
	)
	message(STATUS "testdata will be copied from ${VISTA_SDK_TEST_DATA_DIR} to ${VISTA_SDK_OUTPUT_DIR}/testdata")
else()
	message(WARNING "testdata directory not found: ${VISTA_SDK_TEST_DATA_DIR}. Skipping testdata copy.")
endif()
