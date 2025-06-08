/**
 * @file ImoNumberTests.cpp
 * @brief Unit tests for the ImoNumber class.
 */

#include "pch.h"

#include "TestDataLoader.h"

#include "dnv/vista/sdk/ImoNumber.h"

namespace dnv::vista::sdk
{
	namespace
	{
		constexpr const char* IMONUMBERS_TEST_DATA_PATH = "testdata/Codebook.json";
	}

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
			const auto& jsonData = loadTestData( IMONUMBERS_TEST_DATA_PATH );
			const auto& jsonObject = jsonData.get_object();

			auto imoNumbersResult = jsonObject["imoNumbers"];
			if ( !imoNumbersResult.error() && imoNumbersResult.value().is_array() )
			{
				const auto& imoNumbersArray = imoNumbersResult.value().get_array();

				for ( const auto& item : imoNumbersArray )
				{
					if ( item.is_object() )
					{
						const auto& itemObject = item.get_object();

						auto valueResult = itemObject["value"];
						auto successResult = itemObject["success"];
						auto outputResult = itemObject["output"];

						if ( !valueResult.error() && valueResult.value().is_string() &&
							 !successResult.error() && successResult.value().is_bool() &&
							 !outputResult.error() )
						{
							std::string_view valueStr = valueResult.value().get_string().value();
							bool success = successResult.value().get_bool();

							std::optional<std::string> output;
							if ( outputResult.value().is_string() )
							{
								std::string_view outputStr = outputResult.value().get_string().value();
								output = std::string( outputStr );
							}

							testData.push_back( { std::string( valueStr ), success, output } );
						}
					}
				}
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
