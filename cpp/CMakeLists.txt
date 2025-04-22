#
# VISTA-SDK-CPP
#

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC-specific settings
if(MSVC)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
	set(BUILD_SHARED_LIBS TRUE)
endif()

# Define directories
set(VISTA_SDK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VISTA_SDK_RESOURCE_DIR ${VISTA_SDK_SOURCE_DIR}/resources)
set(VISTA_SDK_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin)

# Define build options
option(VISTA_SDK_BUILD_TESTS	"Build tests"						ON	)
option(VISTA_SDK_COPY_RESOURCES "Copy resources to build directory"	ON	)
option(VISTA_SDK_BUILD_SAMPLES	"Build vista-sdk-cpp samples"		ON	)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VISTA_SDK_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VISTA_SDK_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${VISTA_SDK_OUTPUT_DIR})

# Copy resources
if(EXISTS ${VISTA_SDK_RESOURCES_DIR})
	add_custom_target(CopyResources ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory ${VISTA_SDK_OUTPUT_DIR}/resources
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			${VISTA_SDK_RESOURCES_DIR} ${VISTA_SDK_OUTPUT_DIR}/resources
		COMMENT "Copying VISTA SDK resources to build directory"
	)
	message(STATUS "Resources will be copied from ${VISTA_SDK_RESOURCES_DIR} to ${VISTA_SDK_OUTPUT_DIR}/resources")
else()
	message(WARNING "Resources directory not found: ${VISTA_SDK_RESOURCES_DIR}. Skipping resource copy.")
endif()

include(CheckCXXSourceCompiles)

# Compiler tuning
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

	add_compile_options(/wd4061) # switch not handled
	add_compile_options(/wd4100) # unreferenced formal parameter
	add_compile_options(/wd4189) # local variable is initialized but not referenced
	add_compile_options(/wd4267) # conversion from 'size_t' to 'type', possible loss of data
	add_compile_options(/wd4365) # conversion signed/unsigned mismatch
	add_compile_options(/wd4388) # signed/unsigned mismatch
	add_compile_options(/wd4456) # declaration hides previous local declaration
	add_compile_options(/wd4625) # copy constructor was implicitly defined as deleted
	add_compile_options(/wd4626) # assignement operator was implicitly defined as deleted
	add_compile_options(/wd4820) # padding
	add_compile_options(/wd5026) # move constructor was implicitly defined as deleted
	add_compile_options(/wd5027) # move assignement operator was implicitly defined as deleted
	add_compile_options(/wd5038) # will be initialized after
	add_compile_options(/wd5045) # Qspectre
	add_compile_options(/wd5267) # definition of implicit copy constructor/assignment operator is deprecated because it has a user-provided assignment operator/copy constructor
	add_compile_options(/wd6246) # local declaration of 'variable' hides declaration of same name in outer scope
	add_compile_options(/wd6326) # potential comparison of a constant with another constant
	add_compile_options(/wd6387) # 'argument' may be 'value': this does not adhere to the specification for the function
endif()

# Enable SSE4.2 instructions in the compiler
if(MSVC)
	add_compile_options(/arch:AVX)
	add_compile_options(/arch:AVX2)
else()
	add_compile_options(-msse4.2)
endif()

# Add subdirectories
add_subdirectory(cmake)

# Enable and add tests if enabled
if(VISTA_SDK_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test_gtest)
	#add_subdirectory(test_catch2)
endif()

# Print configuration summary
message(STATUS "VISTA SDK Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build tests: ${VISTA_SDK_BUILD_TESTS}")
message(STATUS "  Build examples: ${VISTA_SDK_BUILD_EXAMPLES}")
message(STATUS "  Copy resources: ${VISTA_SDK_COPY_RESOURCES}")
message(STATUS "  Resources source: ${VISTA_SDK_RESOURCE_DIR}")
message(STATUS "  Resources destination: ${VISTA_SDK_OUTPUT_DIR}/resources")
message(STATUS "  Output directory: ${VISTA_SDK_OUTPUT_DIR}")
