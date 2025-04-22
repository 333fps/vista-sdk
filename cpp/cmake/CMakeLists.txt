#
# VISTA-SDK-CPP
#

# Include headers and sources
set(VISTA_SDK_INCLUDES
	../include/dnv/vista/sdk/ChdDictionary.h
	../include/dnv/vista/sdk/ChdDictionary.hpp
	../include/dnv/vista/sdk/Codebook.h
	../include/dnv/vista/sdk/Codebooks.h
	../include/dnv/vista/sdk/CodebookName.h
	../include/dnv/vista/sdk/CodebooksDto.h
	../include/dnv/vista/sdk/EmbeddedResource.h
	../include/dnv/vista/sdk/Gmod.h
	../include/dnv/vista/sdk/GmodDto.h
	../include/dnv/vista/sdk/GmodNode.h
	../include/dnv/vista/sdk/GmodPath.h
	../include/dnv/vista/sdk/GmodVersioning.h
	../include/dnv/vista/sdk/GmodVersioningDto.h
	../include/dnv/vista/sdk/ILocalId.h
	../include/dnv/vista/sdk/ILocalIdBuilder.h
	../include/dnv/vista/sdk/ILocalIdBuilder.hpp
	../include/dnv/vista/sdk/IUniversalIdBuilder.h
	../include/dnv/vista/sdk/IUniversalId.h
	../include/dnv/vista/sdk/ImoNumber.h
	../include/dnv/vista/sdk/ISO19848Dtos.h
	../include/dnv/vista/sdk/LocalId.h
	../include/dnv/vista/sdk/LocalIdBuilder.h
	../include/dnv/vista/sdk/LocalIdItems.h
	../include/dnv/vista/sdk/Locations.h
	../include/dnv/vista/sdk/LocationsDto.h
	../include/dnv/vista/sdk/MetadataTag.h
	../include/dnv/vista/sdk/ParsingErrors.h
	../include/dnv/vista/sdk/UniversalIdBuilder.h
	../include/dnv/vista/sdk/UniversalId.h
	../include/dnv/vista/sdk/VIS.h
	../include/dnv/vista/sdk/VisVersion.h
)

set(VISTA_SDK_SOURCES
	../src/dnv/vista/sdk/ChdDictionary.cpp
	../src/dnv/vista/sdk/Codebook.cpp
	../src/dnv/vista/sdk/Codebooks.cpp
	../src/dnv/vista/sdk/CodebookName.cpp
	../src/dnv/vista/sdk/CodebooksDto.cpp
	../src/dnv/vista/sdk/EmbeddedResource.cpp
	../src/dnv/vista/sdk/Gmod.cpp
	../src/dnv/vista/sdk/GmodDto.cpp
	../src/dnv/vista/sdk/GmodNode.cpp
	../src/dnv/vista/sdk/GmodPath.cpp
	../src/dnv/vista/sdk/GmodVersioning.cpp
	../src/dnv/vista/sdk/GmodVersioningDto.cpp
	../src/dnv/vista/sdk/ILocalId.cpp
	../src/dnv/vista/sdk/ILocalIdBuilder.cpp
	../src/dnv/vista/sdk/IUniversalIdBuilder.cpp
	../src/dnv/vista/sdk/IUniversalId.cpp
	../src/dnv/vista/sdk/ImoNumber.cpp
	../src/dnv/vista/sdk/ISO19848Dtos.cpp
	../src/dnv/vista/sdk/LocalId.cpp
	../src/dnv/vista/sdk/LocalIdBuilder.cpp
	../src/dnv/vista/sdk/LocalIdItems.cpp
	../src/dnv/vista/sdk/Locations.cpp
	../src/dnv/vista/sdk/LocationsDto.cpp
	../src/dnv/vista/sdk/MetadataTag.cpp
	../src/dnv/vista/sdk/ParsingErrors.cpp
	../src/dnv/vista/sdk/UniversalIdBuilder.cpp
	../src/dnv/vista/sdk/UniversalId.cpp
	../src/dnv/vista/sdk/VIS.cpp
	../src/dnv/vista/sdk/VisVersion.cpp
)

add_library(${PROJECT_NAME} SHARED ${VISTA_SDK_INCLUDES} ${VISTA_SDK_SOURCES})

target_precompile_headers(${PROJECT_NAME} PRIVATE ../src/dnv/vista/sdk/pch.h)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/../src
)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)

find_package(RapidJSON CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE RapidJSON rapidjson)

find_package(ZLIB REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

find_package(cpuid CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cpuid::cpuid)

find_package(async_mqtt_iface CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE async_mqtt_iface::async_mqtt_iface)

if(VISTA_SDK_BUILD_SAMPLES)
	#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../examples ${CMAKE_CURRENT_BINARY_DIR}/examples)
endif()
