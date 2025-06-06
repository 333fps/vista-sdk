/**
 * @file TestDataLoader.cpp
 * @brief Helper utilities for loading test data from JSON files.
 */

#include "pch.h"
#include "TestDataLoader.h"

namespace dnv::vista::sdk
{
	static std::unordered_map<std::string, nlohmann::json> g_testDataCache;

	const nlohmann::json& loadTestData( const char* testDataPath )
	{
		const std::string_view pathView{ testDataPath };

		if ( auto it = g_testDataCache.find( std::string{ pathView } ); it != g_testDataCache.end() )
		{
			return it->second;
		}

		std::ifstream jsonFile( testDataPath );
		if ( !jsonFile.is_open() )
		{
			throw std::runtime_error( std::string{ "Failed to open test data file: " } + testDataPath );
		}

		try
		{
			nlohmann::json data;
			jsonFile >> data;

			auto [inserted_it, success] = g_testDataCache.emplace( std::string{ pathView }, std::move( data ) );

			return inserted_it->second;
		}
		catch ( const nlohmann::json::parse_error& ex )
		{
			throw std::runtime_error(
				std::string{ "JSON parse error in '" } + testDataPath +
				"'. Type: " + std::to_string( ex.id ) +
				", Byte: " + std::to_string( ex.byte ) +
				". Original what() likely too long." );
		}
	}
}
