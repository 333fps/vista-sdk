cmake_minimum_required(VERSION 3.20)

find_package(spdlog CONFIG REQUIRED)
find_package(GTest REQUIRED)
include(GoogleTest)

set(TESTS
	CodebookTests
	CodebooksTests
	GmodPath_test
	GmodVersioning_test
	Gmod_test
	ISOString_test
	ImoNumber_test
	LocalId_test
)

foreach(test_target ${TESTS})
	if(NOT TARGET ${test_target})
		add_executable(${test_target} ${test_target}.cpp)

		target_precompile_headers(${test_target} PRIVATE pch.h)

		target_link_libraries(${test_target} PRIVATE
			vista-sdk-cpp
			GTest::gtest_main
			GTest::gtest
			spdlog::spdlog_header_only
		)

		target_include_directories(${test_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

		add_test(
			NAME ${test_target}
			COMMAND ${test_target}
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/cpp/cmake/bin/"
		)
	endif()
endforeach()

if(MSVC)
	foreach(test_target ${TESTS})
		target_compile_options(${test_target} PRIVATE
			/wd4625 # copy constructor was implicitly defined as deleted
			/wd4626 # assignement operator was implicitly defined as deleted
			/wd5026 # move constructor was implicitly defined as deleted
			/wd5027 # move assignement operator was implicitly defined as deleted
		)
	endforeach()
endif()

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
