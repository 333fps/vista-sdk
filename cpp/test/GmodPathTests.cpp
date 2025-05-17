/**
 * @file GmodPathTests.cpp
 * @brief Unit tests for the GmodPath class.
 */

#include "pch.h"

#include "TestData.h"

#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	namespace
	{
		constexpr const char* TEST_DATA_PATH = "testdata/GmodPaths.json";
	}

	namespace GmodTestsFixture
	{
	}

	namespace GmodPathTestsParametrized
	{
		//=====================================================================
		// Tests
		//=====================================================================

		//----------------------------------------------
		// Test_GmodPath_Parse
		//----------------------------------------------

		struct GmodPathParseValidParam
		{
			std::string visVersionString;
			std::string pathString;
		};

		class GmodPathParseValidTest : public ::testing::TestWithParam<GmodPathParseValidParam>
		{
		protected:
			GmodPathParseValidTest() : m_vis( VIS::instance() )
			{
			}

			VIS& m_vis;
		};

		static std::vector<GmodPathParseValidParam> loadValidGmodPathData()
		{
			std::vector<GmodPathParseValidParam> params;
			const nlohmann::json& jsonData = testData( TEST_DATA_PATH );
			const std::string dataKey = "Valid";

			if ( jsonData.contains( dataKey ) && jsonData[dataKey].is_array() )
			{
				for ( const auto& item : jsonData[dataKey] )
				{
					if ( item.is_object() && item.contains( "visVersion" ) && item["visVersion"].is_string() &&
						 item.contains( "path" ) && item["path"].is_string() )
					{
						std::string visVersionStr = item["visVersion"].get<std::string>();
						std::string pathStr = item["path"].get<std::string>();
						params.push_back( { visVersionStr, pathStr } );
					}
				}
			}
			return params;
		}

		TEST_P( GmodPathParseValidTest, Test_GmodPath_Parse )
		{
			const auto& param = GetParam();

			VisVersion visVersion = VisVersionExtensions::parse( param.visVersionString );

			std::optional<GmodPath> parsedGmodPathOptional;
			bool parsed = GmodPath::tryParse( param.pathString, visVersion, parsedGmodPathOptional );

			ASSERT_TRUE( parsed );
			ASSERT_TRUE( parsedGmodPathOptional.has_value() );
			ASSERT_EQ( param.pathString, parsedGmodPathOptional.value().toString() );
		}

		INSTANTIATE_TEST_SUITE_P(
			GmodPathParseValidSuite,
			GmodPathParseValidTest,
			::testing::ValuesIn( loadValidGmodPathData() ) );

		//----------------------------------------------
		// Test_GmodPath_Parse_Invalid
		//----------------------------------------------

		struct GmodPathParseInvalidParam
		{
			std::string visVersionString;
			std::string pathString;
		};

		class GmodPathParseInvalidTest : public ::testing::TestWithParam<GmodPathParseInvalidParam>
		{
		protected:
			GmodPathParseInvalidTest() : m_vis( VIS::instance() )
			{
			}

			VIS& m_vis;
		};

		static std::vector<GmodPathParseInvalidParam> loadInvalidGmodPathData()
		{
			std::vector<GmodPathParseInvalidParam> params;
			const nlohmann::json& jsonData = testData( TEST_DATA_PATH );
			const std::string dataKey = "Invalid";

			if ( jsonData.contains( dataKey ) && jsonData[dataKey].is_array() )
			{
				for ( const auto& item : jsonData[dataKey] )
				{
					if ( item.is_object() && item.contains( "visVersion" ) && item["visVersion"].is_string() &&
						 item.contains( "path" ) && item["path"].is_string() )
					{
						std::string visVersionStr = item["visVersion"].get<std::string>();
						std::string pathStr = item["path"].get<std::string>();
						params.push_back( { visVersionStr, pathStr } );
					}
				}
			}
			return params;
		}

		/*
		TEST_P( GmodPathParseInvalidTest, Test_GmodPath_Parse_Invalid )
		{
			const auto& param = GetParam();

			VisVersion visVersion = VisVersionExtensions::parse( param.visVersionString );

			std::optional<GmodPath> parsedGmodPathOptional;
			bool parsed = GmodPath::tryParse( param.pathString, visVersion, parsedGmodPathOptional );

			ASSERT_FALSE( parsed );
			ASSERT_FALSE( parsedGmodPathOptional.has_value() );
		}
		*/

		/*
		INSTANTIATE_TEST_SUITE_P(
			GmodPathParseInvalidSuite,
			GmodPathParseInvalidTest,
			::testing::ValuesIn( loadInvalidGmodPathData() ) );
		*/

		//----------------------------------------------
		// Test_GmodPath_Parse_Invalid
		//----------------------------------------------

	}
}
