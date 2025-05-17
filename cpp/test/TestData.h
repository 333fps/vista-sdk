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
			static nlohmann::json data;
			static bool loaded = false;
			if ( !loaded )
			{
				std::string jsonFilePath = testDataPath;

				std::ifstream jsonFile( jsonFilePath );
				if ( !jsonFile.is_open() )
				{
					throw std::runtime_error( "Failed to open global test data file: " + jsonFilePath );
				}
				try
				{
					jsonFile >> data;
					loaded = true;
				}
				catch ( [[maybe_unused]] const nlohmann::json::parse_error& ex )
				{
					std::string errMsg = "JSON parse error in '" + jsonFilePath +
										 "'. Type: " + std::to_string( ex.id ) +
										 ", Byte: " + std::to_string( ex.byte ) +
										 ". Original what() likely too long.";
					throw std::runtime_error( errMsg );
				}
			}

			return data;
		}
	}
}
