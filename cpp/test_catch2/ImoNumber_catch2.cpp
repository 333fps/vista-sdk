#include "pch.h"

#include "dnv/vista/sdk/ImoNumber.h"

namespace dnv::vista::sdk
{
	class ImoNumberTests
	{
	protected:
		struct TestDataItem
		{
			std::string value;
			bool success;
			std::optional<std::string> output;
		};

		std::vector<TestDataItem> testData;

	public:
		ImoNumberTests()
		{
			std::ifstream file;
			file.open( "testdata/ImoNumbers.json" );
			if ( !file.is_open() )
				FAIL( "Failed to open testdata/ImoNumbers.json" );

			rapidjson::IStreamWrapper isw( file );
			rapidjson::Document data;
			data.ParseStream( isw );

			if ( !data.IsObject() && !data.IsArray() )
				FAIL( "JSON data is not a valid object or array" );

			const rapidjson::Value& testCases = data.IsArray() ? data : ( data.HasMember( "imoNumbers" ) ? data["imoNumbers"] : data );

			if ( !testCases.IsArray() )
				FAIL( "Test cases must be an array" );

			for ( const auto& item : testCases.GetArray() )
			{
				if ( !( item.HasMember( "value" ) && item["value"].IsString() ) )
					FAIL( "Item missing value field" );

				if ( !( item.HasMember( "success" ) && item["success"].IsBool() ) )
					FAIL( "Item missing success field" );

				std::optional<std::string> output;
				if ( item.HasMember( "output" ) && item["output"].IsString() )
					output = item["output"].GetString();

				testData.push_back( { item["value"].GetString(),
					item["success"].GetBool(),
					output } );
			}
		}
	};

	TEST_CASE_METHOD( ImoNumberTests, "Test IMO number validation", "[imonumber]" )
	{
		for ( const auto& item : testData )
		{
			INFO( "Testing value: " << item.value );
			std::optional<ImoNumber> parsedImo = ImoNumber::TryParse( item.value );

			if ( item.success )
			{
				REQUIRE( parsedImo.has_value() );
				CHECK_FALSE( parsedImo->ToString().empty() );
			}
			else
			{
				REQUIRE_FALSE( parsedImo.has_value() );
			}

			if ( item.output.has_value() && parsedImo.has_value() )
			{
				CHECK( parsedImo->ToString() == item.output.value() );
			}
		}
	}

	TEST_CASE( "IMO number constructor with valid integer", "[imonumber]" )
	{
		REQUIRE_NOTHROW( ImoNumber( 9074729 ) );

		ImoNumber imo( 9785811 );
		CHECK( imo.ToString() == "IMO9785811" );
		CHECK( static_cast<int>( imo ) == 9785811 );
	}

	TEST_CASE( "IMO number constructor with invalid integer", "[imonumber]" )
	{
		REQUIRE_THROWS_AS( ImoNumber( 123 ), std::invalid_argument );
		REQUIRE_THROWS_AS( ImoNumber( 12345678 ), std::invalid_argument );
		REQUIRE_THROWS_AS( ImoNumber( 1234568 ), std::invalid_argument );
	}

	TEST_CASE( "IMO number constructor with valid string", "[imonumber]" )
	{
		REQUIRE_NOTHROW( ImoNumber( "9074729" ) );
		REQUIRE_NOTHROW( ImoNumber( "IMO9074729" ) );

		ImoNumber imo( "IMO9785811" );
		CHECK( static_cast<int>( imo ) == 9785811 );
	}

	TEST_CASE( "IMO number constructor with invalid string", "[imonumber]" )
	{
		REQUIRE_THROWS_AS( ImoNumber( "abc" ), std::invalid_argument );
		REQUIRE_THROWS_AS( ImoNumber( "IMO123" ), std::invalid_argument );
		REQUIRE_THROWS_AS( ImoNumber( "1234568" ), std::invalid_argument );
	}

	TEST_CASE( "IMO number static Parse method", "[imonumber]" )
	{
		ImoNumber imo1 = ImoNumber::Parse( "9074729" );
		CHECK( imo1.ToString() == "IMO9074729" );

		ImoNumber imo2 = ImoNumber::Parse( "IMO9785811" );
		CHECK( imo2.ToString() == "IMO9785811" );

		REQUIRE_THROWS_AS( ImoNumber::Parse( "invalid" ), std::invalid_argument );
	}

	TEST_CASE( "IMO number IsValid method", "[imonumber]" )
	{
		CHECK( ImoNumber::IsValid( 9074729 ) );
		CHECK( ImoNumber::IsValid( 9785811 ) );
		CHECK_FALSE( ImoNumber::IsValid( 123 ) );
		CHECK_FALSE( ImoNumber::IsValid( 12345678 ) );
		CHECK_FALSE( ImoNumber::IsValid( 1234568 ) );
	}

	TEST_CASE( "IMO number equality comparison", "[imonumber]" )
	{
		ImoNumber imo1( 9074729 );
		ImoNumber imo2( 9074729 );
		ImoNumber imo3( 9785811 );

		CHECK( imo1 == imo2 );
		CHECK_FALSE( imo1 == imo3 );
	}

	TEST_CASE( "IMO number prefix handling", "[imonumber]" )
	{
		ImoNumber imo1 = ImoNumber::Parse( "9074729" );
		ImoNumber imo2 = ImoNumber::Parse( "IMO9074729" );
		ImoNumber imo3 = ImoNumber::Parse( "imo9074729" );

		CHECK( imo1 == imo2 );
		CHECK( imo1 == imo3 );
		CHECK( imo1.ToString() == "IMO9074729" );
		CHECK( imo2.ToString() == "IMO9074729" );
		CHECK( imo3.ToString() == "IMO9074729" );
	}

	TEST_CASE( "IMO number edge cases", "[imonumber]" )
	{
		CHECK( ImoNumber::TryParse( "IMO1000019" ).has_value() );

		CHECK_FALSE( ImoNumber::TryParse( "IMO 9074729" ).has_value() );
		CHECK_FALSE( ImoNumber::TryParse( " IMO9074729" ).has_value() );
		CHECK_FALSE( ImoNumber::TryParse( "IMO9074729 " ).has_value() );
		CHECK_FALSE( ImoNumber::TryParse( "" ).has_value() );

		REQUIRE_THROWS_AS( ImoNumber::Parse( nullptr ), std::invalid_argument );
	}

	TEST_CASE( "IMO number hash function", "[imonumber]" )
	{
		ImoNumber imo1( 9074729 );
		ImoNumber imo2( 9074729 );
		ImoNumber imo3( 9785811 );

		ImoNumber::Hash hasher;

		CHECK( hasher( imo1 ) == hasher( imo2 ) );
		CHECK( hasher( imo1 ) != hasher( imo3 ) );

		std::unordered_map<ImoNumber, std::string, ImoNumber::Hash> imoMap;
		imoMap[imo1] = "Vessel 1";
		imoMap[imo3] = "Vessel 2";

		CHECK( imoMap[imo1] == "Vessel 1" );
		CHECK( imoMap[imo2] == "Vessel 1" );
		CHECK( imoMap[imo3] == "Vessel 2" );
	}
}
