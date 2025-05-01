#
# VISTA-SDK-CPP
#

# Include headers and sources
set(public_headers
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ChdDictionary.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ChdDictionary.hpp
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
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ILocalId.hpp
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ILocalIdBuilder.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ILocalIdBuilder.hpp
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/IUniversalIdBuilder.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/IUniversalId.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ImoNumber.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ISO19848Dtos.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/LocalId.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/LocalIdBuilder.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/LocalIdItems.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/Locations.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/LocationsDto.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/MetadataTag.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/ParsingErrors.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/UniversalIdBuilder.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/UniversalId.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/VIS.h
	${VISTA_SDK_INCLUDE_DIR}/dnv/vista/sdk/VisVersion.h
)

set(private_sources
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
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/ILocalId.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/ILocalIdBuilder.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/IUniversalIdBuilder.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/IUniversalId.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/ImoNumber.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/ISO19848Dtos.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/LocalId.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/LocalIdBuilder.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/LocalIdItems.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/Locations.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/LocationsDto.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/MetadataTag.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/ParsingErrors.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/UniversalIdBuilder.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/UniversalId.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/VIS.cpp
	${VISTA_SDK_SOURCE_DIR}/dnv/vista/sdk/VisVersion.cpp
)

add_library(${PROJECT_NAME} SHARED ${public_headers} ${private_sources})

target_precompile_headers(${PROJECT_NAME} PRIVATE ../src/dnv/vista/sdk/pch.h)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
		$<INSTALL_INTERFACE:include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/../src
)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

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
