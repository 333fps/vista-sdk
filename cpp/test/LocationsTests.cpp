#include "pch.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/ParsingErrors.h"

namespace dnv::vista::sdk::tests
{
	static std::vector<VisVersion> visVersions()
	{
		return {
			VisVersion::v3_4a,
			VisVersion::v3_5a,
			VisVersion::v3_6a,
			VisVersion::v3_7a,
			VisVersion::v3_8a };
	}

	struct LocationTestCase
	{
		std::string value;
		bool success;
		std::string output;
		std::vector<std::string> expectedErrorMessages;
	};

	class LocationsTests : public ::testing::Test
	{
	protected:
		virtual void SetUp() override
		{
			m_vis = &VIS::instance();
		}

		VIS* m_vis = nullptr;
	};

	class LocationsVersionedTests : public LocationsTests,
									public ::testing::WithParamInterface<VisVersion>
	{
	};

	TEST_P( LocationsVersionedTests, Test_Locations_Loads )
	{
		VisVersion visVersion = GetParam();

		auto locations = m_vis->locations( visVersion );

		auto groups = locations.groups();
		ASSERT_NE( groups.size(), 0 );

		Location testLocation;
		ParsingErrors errors;

		bool success = locations.tryParse( std::string_view( "11" ), testLocation, errors );
		ASSERT_TRUE( success );
		ASSERT_FALSE( errors.hasErrors() );

		ASSERT_EQ( "11", testLocation.toString() );
	}

	INSTANTIATE_TEST_SUITE_P(
		ByVisVersion,
		LocationsVersionedTests,
		::testing::ValuesIn( visVersions() ) );

	TEST_F( LocationsTests, Test_LocationGroups_Properties )
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
		for ( size_t i = 0; i < values.size() - 1; i++ )
		{
			ASSERT_EQ( values[i] + 1, values[i + 1] );
		}
	}

	class LocationsParamTest : public LocationsTests,
							   public ::testing::WithParamInterface<LocationTestCase>
	{
	};

	void verifyParsing( bool succeeded, const ParsingErrors& errors, const Location& parsedLocation, const LocationTestCase& expected )
	{
		if ( !expected.success )
		{
			ASSERT_FALSE( succeeded );
			ASSERT_EQ( parsedLocation, Location() );

			if ( !expected.expectedErrorMessages.empty() )
			{
				ASSERT_TRUE( errors.hasErrors() );

				std::vector<std::string> actualErrors;
				for ( const auto& error : errors )
				{
					auto const& [type, message] = error;
					actualErrors.push_back( message );
				}

				ASSERT_EQ( actualErrors.size(), expected.expectedErrorMessages.size() );
				for ( const auto& expectedError : expected.expectedErrorMessages )
				{
					bool found = false;
					for ( const auto& actualError : actualErrors )
					{
						if ( actualError == expectedError )
						{
							found = true;
							break;
						}
					}
					ASSERT_TRUE( found ) << "Expected error not found: " << expectedError;
				}
			}
		}
		else
		{
			ASSERT_TRUE( succeeded );
			ASSERT_FALSE( errors.hasErrors() );
			ASSERT_NE( parsedLocation, Location() );
			ASSERT_EQ( expected.output, parsedLocation.toString() );
		}
	}

	TEST_P( LocationsParamTest, Test_Locations )
	{
		auto param = GetParam();
		auto locations = m_vis->locations( VisVersion::v3_4a );

		Location stringParsedLocation;
		ParsingErrors stringErrorBuilder;
		bool stringSuccess = locations.tryParse( std::string_view( param.value ), stringParsedLocation, stringErrorBuilder );

		Location spanParsedLocation;
		ParsingErrors spanErrorBuilder;
		bool spanSuccess = locations.tryParse( std::string_view( param.value ), spanParsedLocation, spanErrorBuilder );

		verifyParsing( stringSuccess, stringErrorBuilder, stringParsedLocation, param );
		verifyParsing( spanSuccess, spanErrorBuilder, spanParsedLocation, param );
	}

	std::vector<LocationTestCase> locationsData()
	{
		return {
			{ "11FIPU", true, "11FIPU", {} },
			{ "11FI", true, "11FI", {} },
			{ "11F", true, "11F", {} },
			{ "11", true, "11", {} },
			{ "F", true, "F", {} },
			{ "FIPU", true, "FIPU", {} },
			{ "XYZ", false, "", { "Invalid location code: 'XYZ' with invalid location code(s): 'X','Y','Z'" } },
		};
	}

	INSTANTIATE_TEST_SUITE_P(
		LocationParseData,
		LocationsParamTest,
		::testing::ValuesIn( locationsData() ) );

	TEST_F( LocationsTests, Test_Location_Parse_Throwing )
	{
		auto locations = m_vis->locations( VisVersion::v3_4a );

		ASSERT_THROW( { (void)locations.parse( std::string_view() ); }, std::invalid_argument );
		ASSERT_THROW( { (void)locations.parse( std::string_view( "" ) ); }, std::invalid_argument );
		ASSERT_THROW( { (void)locations.parse( std::string_view( nullptr, 0 ) ); }, std::invalid_argument );
	}

	TEST_F( LocationsTests, Test_Location_Builder )
	{
		// TODO Implement this test when LocationBuilder is available
	}

	TEST_F( LocationsTests, Test_Locations_Equality )
	{
		const auto& gmod = m_vis->gmod( VisVersion::v3_4a );

		auto node1 = gmod["C101.663"].withLocation( "FIPU" );
		auto node2 = gmod["C101.663"].withLocation( "FIPU" );

		ASSERT_EQ( node1, node2 );
		ASSERT_NE( &node1, &node2 );
	}
}
