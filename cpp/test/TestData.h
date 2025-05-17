/**
 * @file TestData.h
 * @brief Helper utilities for loading test data from JSON files.
 */

#pragma once

namespace dnv::vista::sdk
{
	namespace
	{
		static const nlohmann::json& testData( const char* testDataPath )
		{
			static std::map<std::string, nlohmann::json> cache;
			std::string pathStr = testDataPath;

			auto it = cache.find( pathStr );
			if ( it == cache.end() )
			{
				std::ifstream jsonFile( pathStr );
				if ( !jsonFile.is_open() )
				{
					throw std::runtime_error( "Failed to open test data file: " + pathStr );
				}
				try
				{
					nlohmann::json data;
					jsonFile >> data;
					auto [inserted_it, success] = cache.emplace( pathStr, std::move( data ) );
					it = inserted_it;
				}
				catch ( const nlohmann::json::parse_error& ex )
				{
					std::string errMsg = "JSON parse error in '" + pathStr +
										 "'. Type: " + std::to_string( ex.id ) +
										 ", Byte: " + std::to_string( ex.byte ) +
										 ". Original what() likely too long.";
					throw std::runtime_error( errMsg );
				}
			}
			return it->second;
		}
	}
}
