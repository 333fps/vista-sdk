cmake_minimum_required(VERSION 3.20)

find_package(spdlog CONFIG REQUIRED)
find_package(GTest REQUIRED)
include(GoogleTest)

set(TESTS
	ImoNumber_gtest
	ISOString_gtest
	Codebook_gtest
	Codebooks_gtest
	Gmod_gtest
	LocalId_gtest
	GmodVersioning_gtest
	GmodPath_gtest
)

foreach(TEST ${TESTS})
if(NOT TARGET ${TEST})
	add_executable(${TEST} ${TEST}.cpp)

	target_precompile_headers(${TEST} PRIVATE pch.h)

	target_link_libraries(${TEST} PRIVATE
		vista-sdk-cpp
		GTest::gtest_main
		GTest::gtest
		spdlog::spdlog_header_only
	)

	target_include_directories(${TEST} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

	add_test(
		NAME ${TEST}
		COMMAND ${TEST}
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/cpp/cmake/bin/"
	)
endif()
endforeach()


# Copy testdata
#if(EXISTS ${VISTA_SDK_TEST_DATA_DIR} AND NOT TARGET CopyTestData)
#	add_custom_target(CopyTestData ALL
#		COMMAND ${CMAKE_COMMAND} -E make_directory ${VISTA_SDK_OUTPUT_DIR}/testdata
#		COMMAND ${CMAKE_COMMAND} -E copy_directory
#			${VISTA_SDK_TEST_DATA_DIR} ${VISTA_SDK_OUTPUT_DIR}/testdata
#		COMMENT "Copying VISTA SDK testdata to build directory"
#	)
#	message(STATUS "testdata will be copied from ${VISTA_SDK_TEST_DATA_DIR} to ${VISTA_SDK_OUTPUT_DIR}/testdata")
#else()
#	message(WARNING "testdata directory not found: ${VISTA_SDK_TEST_DATA_DIR}. Skipping testdata copy.")
#endif()
