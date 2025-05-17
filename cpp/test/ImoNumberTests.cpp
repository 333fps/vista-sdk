/**
 * @file ImoNumberTests.cpp
 * @brief Unit tests for the ImoNumber class.
 */

#include "pch.h"

#include "dnv/vista/sdk/ImoNumber.h"

namespace dnv::vista::sdk
{
	class ImoNumberTests : public ::testing::Test
	{
	protected:
		struct TestDataItem
		{
			std::string value;
			bool success;
			std::optional<std::string> output;
		};

		std::vector<TestDataItem> testData;

		virtual void SetUp() override
		{
			std::vector<std::string> possiblePaths = {
				"testdata/ImoNumbers.json",
				"../testdata/ImoNumbers.json",
				"../../testdata/ImoNumbers.json",
				"../../../testdata/ImoNumbers.json",
				"./ImoNumbers.json" };

			std::ifstream file;
			std::string attemptedPaths;
			bool fileOpened = false;

			for ( const auto& path : possiblePaths )
			{
				file.open( path );
				if ( file.is_open() )
				{
					SPDLOG_INFO( "Found test data at path: {}", path );
					fileOpened = true;
					break;
				}

				attemptedPaths += path + ", ";
				file.clear();
				SPDLOG_DEBUG( "Failed to open test data file: {}", path );
			}

			if ( !fileOpened )
			{
				SPDLOG_ERROR( "Failed to open test data file. Attempted paths: {}", attemptedPaths );
				ASSERT_TRUE( false ) << "Failed to open ImoNumbers.json. Attempted paths: " << attemptedPaths;
				return;
			}

			nlohmann::json data;
			try
			{
				data = nlohmann::json::parse( file );
			}
			catch ( [[maybe_unused]] const nlohmann::json::parse_error& ex )
			{
				SPDLOG_ERROR( "JSON parse error: {}", ex.what() );
				ASSERT_TRUE( false ) << "Failed to parse ImoNumbers.json: " << ex.what();
				return;
			}

			ASSERT_TRUE( data.is_object() || data.is_array() ) << "JSON data is not a valid object or array";

			const nlohmann::json& testCases = data.is_array() ? data : ( data.contains( "imoNumbers" ) ? data.at( "imoNumbers" ) : data );

			ASSERT_TRUE( testCases.is_array() ) << "Test cases must be an array";

			for ( const auto& item : testCases )
			{
				ASSERT_TRUE( item.contains( "value" ) && item.at( "value" ).is_string() ) << "Item missing 'value' field or not a string";
				ASSERT_TRUE( item.contains( "success" ) && item.at( "success" ).is_boolean() ) << "Item missing 'success' field or not a boolean";

				std::optional<std::string> output;
				if ( item.contains( "output" ) && item.at( "output" ).is_string() )
				{
					output = item.at( "output" ).get<std::string>();
				}

				testData.push_back( { item.at( "value" ).get<std::string>(), item.at( "success" ).get<bool>(), output } );
			}
		}
	};

	TEST_F( ImoNumberTests, Test_Validation )
	{
		for ( const auto& item : testData )
		{
			auto parsedImo = ImoNumber::tryParse( item.value );
			bool parsedOk = parsedImo.has_value();

			if ( item.success )
			{
				EXPECT_TRUE( parsedOk );
			}
			else
			{
				EXPECT_FALSE( parsedOk );
			}

			if ( item.output.has_value() && parsedOk )
			{
				EXPECT_EQ( parsedImo->toString(), item.output.value() );
			}
		}
	}
}
