/**
 * @file TestDataLoader.cpp
 * @brief Helper utilities for loading test data from JSON files.
 */

#include "pch.h"

#include "TestDataLoader.h"

namespace dnv::vista::sdk
{
	struct ParsedJsonData
	{
		std::shared_ptr<simdjson::dom::parser> parser;
		simdjson::dom::element root_element;

		ParsedJsonData( std::shared_ptr<simdjson::dom::parser> p, simdjson::dom::element elem )
			: parser( p ), root_element( elem ) {}
	};

	static std::unordered_map<std::string, std::shared_ptr<ParsedJsonData>> g_testDataCache;

	const simdjson::dom::element& loadTestData( const char* testDataPath )
	{
		const std::string filename{ testDataPath };

		auto it = g_testDataCache.find( filename );
		if ( it != g_testDataCache.end() )
		{
			return it->second->root_element;
		}

		std::ifstream jsonFile( testDataPath );
		if ( !jsonFile.is_open() )
		{
			throw std::runtime_error( std::string( "Failed to open test data file: " ) + testDataPath );
		}

		std::string jsonData( ( std::istreambuf_iterator<char>( jsonFile ) ),
			std::istreambuf_iterator<char>() );

		auto parser = std::make_shared<simdjson::dom::parser>();

		auto result = parser->parse( jsonData );

		if ( result.error() )
		{
			throw std::runtime_error( "JSON parse error in '" + std::string( testDataPath ) + "'. Error: " + std::string( simdjson::error_message( result.error() ) ) );
		}

		auto element = result.value();

		auto parsed_data = std::make_shared<ParsedJsonData>( parser, element );
		g_testDataCache.emplace( filename, parsed_data );

		return parsed_data->root_element;
	}
}
