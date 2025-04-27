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

			rapidjson::IStreamWrapper isw( file );
			rapidjson::Document data;
			data.ParseStream( isw );

			ASSERT_TRUE( data.IsObject() || data.IsArray() ) << "JSON data is not a valid object or array";

			const rapidjson::Value& testCases = data.IsArray() ? data : ( data.HasMember( "imoNumbers" ) ? data["imoNumbers"] : data );

			ASSERT_TRUE( testCases.IsArray() ) << "Test cases must be an array";

			for ( const auto& item : testCases.GetArray() )
			{
				ASSERT_TRUE( item.HasMember( "value" ) && item["value"].IsString() ) << "Item missing value field";
				ASSERT_TRUE( item.HasMember( "success" ) && item["success"].IsBool() ) << "Item missing success field";

				std::optional<std::string> output;
				if ( item.HasMember( "output" ) && item["output"].IsString() )
				{
					output = item["output"].GetString();
				}

				testData.push_back( { item["value"].GetString(), item["success"].GetBool(), output } );
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
