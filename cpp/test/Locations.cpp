/**
 * @file LocationsTests.cpp
 * @brief Unit tests for the Locations and Location classes.
 */

#include "pch.h"

#include "TestDataLoader.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/LocationBuilder.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VISVersion.h"

namespace dnv::vista::sdk::tests
{
	namespace
	{
		constexpr const char* LOCATIONS_TEST_DATA_PATH = "testdata/Locations.json";
	}

	//=====================================================================
	// Test data
	//=====================================================================

	//----------------------------------------------
	// Valid
	//----------------------------------------------

	struct LocationParseValidParam
	{
		std::string value;
		std::string expectedOutput;
	};

	static std::vector<LocationParseValidParam> loadValidLocationData()
	{
		std::vector<LocationParseValidParam> params;
		const nlohmann::json& jsonData = loadTestData( LOCATIONS_TEST_DATA_PATH );
		const std::string dataKey = "locations";

		if ( jsonData.contains( dataKey ) && jsonData[dataKey].is_array() )
		{
			for ( const auto& item : jsonData[dataKey] )
			{
				if ( item.is_object() &&
					 item.contains( "success" ) && item["success"].is_boolean() &&
					 item["success"].get<bool>() == true &&
					 item.contains( "value" ) && item["value"].is_string() &&
					 item.contains( "output" ) && item["output"].is_string() )
				{
					std::string value = item["value"].get<std::string>();
					std::string output = item["output"].get<std::string>();
					params.push_back( { value, output } );
				}
			}
		}
		return params;
	}

	//----------------------------------------------
	// Invalid
	//----------------------------------------------

	struct LocationParseInvalidParam
	{
		std::string value;
		std::vector<std::string> expectedErrorMessages;
	};

	static std::vector<LocationParseInvalidParam> loadInvalidLocationData()
	{
		std::vector<LocationParseInvalidParam> params;
		const nlohmann::json& jsonData = loadTestData( LOCATIONS_TEST_DATA_PATH );
		const std::string dataKey = "locations";

		if ( jsonData.contains( dataKey ) && jsonData[dataKey].is_array() )
		{
			for ( const auto& item : jsonData[dataKey] )
			{
				if ( item.is_object() &&
					 item.contains( "success" ) && item["success"].is_boolean() &&
					 item["success"].get<bool>() == false &&
					 item.contains( "value" ) )
				{
					std::string value;
					if ( item["value"].is_string() )
					{
						value = item["value"].get<std::string>();
					}
					else if ( item["value"].is_null() )
					{
						continue;
					}

					std::vector<std::string> errorMessages;
					if ( item.contains( "expectedErrorMessages" ) && item["expectedErrorMessages"].is_array() )
					{
						for ( const auto& errorMsg : item["expectedErrorMessages"] )
						{
							if ( errorMsg.is_string() )
							{
								errorMessages.push_back( errorMsg.get<std::string>() );
							}
						}
					}

					params.push_back( { value, errorMessages } );
				}
			}
		}
		return params;
	}

	//=====================================================================
	// Tests
	//=====================================================================

	//----------------------------------------------
	// Test_Locations_Loads
	//----------------------------------------------

	struct VisVersionParam
	{
		VisVersion visVersion;
	};

	static std::vector<VisVersionParam> getTestVisVersions()
	{
		return {
			{ VisVersion::v3_4a },
			{ VisVersion::v3_5a },
			{ VisVersion::v3_6a },
			{ VisVersion::v3_7a },
			{ VisVersion::v3_8a } };
	}

	class LocationsLoadsTests : public ::testing::TestWithParam<VisVersionParam>
	{
	protected:
		void SetUp() override
		{
			vis = &VIS::instance();
		}

		VIS* vis;
	};

	TEST_P( LocationsLoadsTests, Test_Locations_Loads )
	{
		auto param = GetParam();
		VisVersion visVersion = param.visVersion;

		const auto& locations = vis->locations( visVersion );

		ASSERT_NE( nullptr, &locations );

		const auto& groups = locations.groups();
		ASSERT_NE( nullptr, &groups );
	}

	INSTANTIATE_TEST_SUITE_P(
		AllVisVersions,
		LocationsLoadsTests,
		::testing::ValuesIn( getTestVisVersions() ) );

	//----------------------------------------------
	// Test_LocationGroups_Properties
	//----------------------------------------------

	TEST( LocationsTests, Test_LocationGroups_Properties )
	{
		std::vector<int> values = {
			static_cast<int>( LocationGroup::Number ),
			static_cast<int>( LocationGroup::Side ),
			static_cast<int>( LocationGroup::Vertical ),
			static_cast<int>( LocationGroup::Transverse ),
			static_cast<int>( LocationGroup::Longitudinal ) };

		std::set<int> uniqueValues( values.begin(), values.end() );
		ASSERT_EQ( values.size(), uniqueValues.size() );

		ASSERT_EQ( 5, values.size() );

		ASSERT_EQ( 0, static_cast<int>( LocationGroup::Number ) );

		std::sort( values.begin(), values.end() );
		for ( size_t i = 0; i < values.size() - 1; ++i )
		{
			ASSERT_EQ( i, values[i + 1] - 1 );
		}
	}

	//----------------------------------------------
	// Test_Locations
	//----------------------------------------------

	struct LocationTestParam
	{
		std::string value;
		bool success;
		std::string output;
		std::vector<std::string> expectedErrorMessages;
	};

	static std::vector<LocationTestParam> getLocationTestData()
	{
		std::vector<LocationTestParam> params;
		auto validData = loadValidLocationData();
		auto invalidData = loadInvalidLocationData();

		for ( const auto& item : validData )
		{
			params.push_back( { item.value, true, item.expectedOutput, {} } );
		}

		for ( const auto& item : invalidData )
		{
			params.push_back( { item.value, false, "", item.expectedErrorMessages } );
		}

		return params;
	}

	class LocationsTests : public ::testing::TestWithParam<LocationTestParam>
	{
	};

	TEST_P( LocationsTests, Test_Locations )
	{
		auto param = GetParam();
		std::string value = param.value;
		bool success = param.success;
		std::string output = param.output;
		std::vector<std::string> expectedErrorMessages = param.expectedErrorMessages;

		auto& vis = VIS::instance();
		const auto& locations = vis.locations( VisVersion::v3_4a );

		Location stringParsedLocation;
		ParsingErrors stringErrorBuilder;
		bool stringSuccess = locations.tryParse( std::string_view( param.value ), stringParsedLocation, stringErrorBuilder );

		Location spanParsedLocation;
		ParsingErrors spanErrorBuilder;
		bool spanSuccess = locations.tryParse( std::string_view( value ), spanParsedLocation, spanErrorBuilder );

		auto verify = [&]( bool succeeded, const ParsingErrors& errors, const Location& parsedLocation ) {
			if ( !success )
			{
				ASSERT_FALSE( succeeded );
				ASSERT_EQ( Location{}, parsedLocation );

				if ( !expectedErrorMessages.empty() )
				{
					ASSERT_TRUE( errors.hasErrors() );
					std::vector<std::string> actualErrors;

					auto enumerator = errors.enumerator();
					while ( enumerator.next() )
					{
						const auto& errorEntry = enumerator.current();
						actualErrors.push_back( errorEntry.message );
					}

					ASSERT_EQ( expectedErrorMessages, actualErrors );
				}
			}
			else
			{
				ASSERT_TRUE( succeeded );
				ASSERT_FALSE( errors.hasErrors() );
				ASSERT_NE( Location{}, parsedLocation );
				ASSERT_EQ( output, parsedLocation.toString() );
			}
		};

		verify( stringSuccess, stringErrorBuilder, stringParsedLocation );
		verify( spanSuccess, spanErrorBuilder, spanParsedLocation );
	}

	INSTANTIATE_TEST_SUITE_P(
		AllLocations,
		LocationsTests,
		::testing::ValuesIn( getLocationTestData() ) );

	//----------------------------------------------
	// Test_Location_Parse_Throwing
	//----------------------------------------------

	TEST( LocationsTests, Test_Location_Parse_Throwing )
	{
		auto& vis = VIS::instance();
		const auto& locations = vis.locations( VisVersion::v3_4a );

		ASSERT_THROW( locations.parse( "" ), std::invalid_argument );
		ASSERT_THROW( locations.parse( std::string_view{} ), std::invalid_argument );
	}

	//----------------------------------------------
	// Test_Location_Builder
	//----------------------------------------------

	TEST( LocationsTests, Test_Location_Builder )
	{
		auto& vis = VIS::instance();
		const auto& locations = vis.locations( VisVersion::v3_4a );

		std::string locationStr = "11FIPU";
		auto location = locations.parse( locationStr );

		auto builder = LocationBuilder::create( locations );

		builder = builder.withNumber( 11 ).withSide( 'P' ).withTransverse( 'I' ).withLongitudinal( 'F' ).withValue( 'U' );

		ASSERT_EQ( "11FIPU", builder.toString() );
		ASSERT_EQ( 11, builder.number().value() );
		ASSERT_EQ( 'P', builder.side().value() );
		ASSERT_EQ( 'U', builder.vertical().value() );
		ASSERT_EQ( 'I', builder.transverse().value() );
		ASSERT_EQ( 'F', builder.longitudinal().value() );

		ASSERT_THROW( builder = builder.withValue( 'X' ), std::invalid_argument );
		ASSERT_THROW( builder = builder.withNumber( -1 ), std::invalid_argument );
		ASSERT_THROW( builder = builder.withNumber( 0 ), std::invalid_argument );
		ASSERT_THROW( builder = builder.withSide( 'A' ), std::invalid_argument );
		ASSERT_THROW( builder = builder.withValue( 'a' ), std::invalid_argument );

		ASSERT_EQ( location, builder.build() );

		builder = LocationBuilder::create( locations ).withLocation( builder.build() );

		ASSERT_EQ( "11FIPU", builder.toString() );
		ASSERT_EQ( 11, builder.number().value() );
		ASSERT_EQ( 'P', builder.side().value() );
		ASSERT_EQ( 'U', builder.vertical().value() );
		ASSERT_EQ( 'I', builder.transverse().value() );
		ASSERT_EQ( 'F', builder.longitudinal().value() );

		builder = builder.withValue( 'S' ).withValue( 2 );

		ASSERT_EQ( "2FISU", builder.toString() );
		ASSERT_EQ( 2, builder.number().value() );
		ASSERT_EQ( 'S', builder.side().value() );
		ASSERT_EQ( 'U', builder.vertical().value() );
		ASSERT_EQ( 'I', builder.transverse().value() );
		ASSERT_EQ( 'F', builder.longitudinal().value() );
	}

	//----------------------------------------------
	// Test_Locations_Equality
	//----------------------------------------------

	TEST( LocationsTests, Test_Locations_Equality )
	{
		auto& vis = VIS::instance();

		const auto& gmod = vis.gmod( VisVersion::v3_4a );

		auto node1 = gmod["C101.663"].withLocation( "FIPU" );

		auto node2 = gmod["C101.663"].withLocation( "FIPU" );

		ASSERT_EQ( node1, node2 );
		ASSERT_NE( &node1, &node2 );
	}
}
