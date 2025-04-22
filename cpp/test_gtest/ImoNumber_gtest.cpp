#include "pch.h"

#include "dnv/vista/sdk/ImoNumber.h"

using namespace dnv::vista::sdk;

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
		std::ifstream file;
		file.open( "testdata/ImoNumbers.json" );
		ASSERT_TRUE( file.is_open() ) << "Failed to open testdata/ImoNumbers.json";

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
				output = item["output"].GetString();

			testData.push_back( { item["value"].GetString(),
				item["success"].GetBool(),
				output } );
		}
	}
};

TEST_F( ImoNumberTests, TestValidation )
{
	for ( const auto& item : testData )
	{
		std::optional<ImoNumber> parsedImo = ImoNumber::tryParse( item.value );

		if ( item.success )
		{
			ASSERT_TRUE( parsedImo.has_value() ) << "Expected parsing to succeed for value: " << item.value;
			EXPECT_NE( parsedImo->toString(), "" ) << "Parsed IMO number should not be empty for value: " << item.value;
		}
		else
		{
			ASSERT_FALSE( parsedImo.has_value() ) << "Expected parsing to fail for value: " << item.value;
		}

		if ( item.output.has_value() )
		{
			EXPECT_EQ( parsedImo->toString(), item.output.value() ) << "Expected output mismatch for value: " << item.value;
		}
	}
}

TEST_F( ImoNumberTests, TestConstructorWithValidInteger )
{
	EXPECT_NO_THROW( {
		ImoNumber imo( 9074729 );
	} );

	ImoNumber imo( 9785811 );
	EXPECT_EQ( imo.toString(), "IMO9785811" );
	EXPECT_EQ( static_cast<int>( imo ), 9785811 );
}

TEST_F( ImoNumberTests, TestConstructorWithInvalidInteger )
{
	EXPECT_THROW( { ImoNumber imo( 123 ); }, std::invalid_argument );

	EXPECT_THROW( { ImoNumber imo( 12345678 ); }, std::invalid_argument );

	EXPECT_THROW( { ImoNumber imo( 1234568 ); }, std::invalid_argument );
}

TEST_F( ImoNumberTests, TestConstructorWithValidString )
{
	EXPECT_NO_THROW( {
		ImoNumber imo( "9074729" );
	} );

	EXPECT_NO_THROW( {
		ImoNumber imo( "IMO9074729" );
	} );

	ImoNumber imo( "IMO9785811" );
	EXPECT_EQ( static_cast<int>( imo ), 9785811 );
}

TEST_F( ImoNumberTests, TestConstructorWithInvalidString )
{
	EXPECT_THROW( { ImoNumber imo( "abc" ); }, std::invalid_argument );

	EXPECT_THROW( { ImoNumber imo( "IMO123" ); }, std::invalid_argument );

	EXPECT_THROW( { ImoNumber imo( "1234568" ); }, std::invalid_argument );
}

TEST_F( ImoNumberTests, TestStaticParseMethod )
{
	ImoNumber imo1 = ImoNumber::parse( "9074729" );
	EXPECT_EQ( imo1.toString(), "IMO9074729" );

	ImoNumber imo2 = ImoNumber::parse( "IMO9785811" );
	EXPECT_EQ( imo2.toString(), "IMO9785811" );

	EXPECT_THROW( { ImoNumber::parse( "invalid" ); }, std::invalid_argument );
}

TEST_F( ImoNumberTests, TestIsValidMethod )
{
	EXPECT_TRUE( ImoNumber::isValid( 9074729 ) );
	EXPECT_TRUE( ImoNumber::isValid( 9785811 ) );
	EXPECT_FALSE( ImoNumber::isValid( 123 ) );
	EXPECT_FALSE( ImoNumber::isValid( 12345678 ) );
	EXPECT_FALSE( ImoNumber::isValid( 1234568 ) );
}

TEST_F( ImoNumberTests, TestEqualityComparison )
{
	ImoNumber imo1( 9074729 );
	ImoNumber imo2( 9074729 );
	ImoNumber imo3( 9785811 );

	EXPECT_TRUE( imo1 == imo2 );
	EXPECT_FALSE( imo1 == imo3 );
}

TEST_F( ImoNumberTests, TestPrefixHandling )
{
	ImoNumber imo1 = ImoNumber::parse( "9074729" );
	ImoNumber imo2 = ImoNumber::parse( "IMO9074729" );
	ImoNumber imo3 = ImoNumber::parse( "imo9074729" );

	EXPECT_TRUE( imo1 == imo2 );
	EXPECT_TRUE( imo1 == imo3 );
	EXPECT_EQ( imo1.toString(), "IMO9074729" );
	EXPECT_EQ( imo2.toString(), "IMO9074729" );
	EXPECT_EQ( imo3.toString(), "IMO9074729" );
}

TEST_F( ImoNumberTests, TestEdgeCases )
{
	EXPECT_TRUE( ImoNumber::tryParse( "IMO1000019" ).has_value() );

	EXPECT_FALSE( ImoNumber::tryParse( "IMO 9074729" ).has_value() );
	EXPECT_FALSE( ImoNumber::tryParse( " IMO9074729" ).has_value() );
	EXPECT_FALSE( ImoNumber::tryParse( "IMO9074729 " ).has_value() );

	EXPECT_FALSE( ImoNumber::tryParse( "" ).has_value() );

	EXPECT_THROW( { ImoNumber::parse( nullptr ); }, std::invalid_argument );
}

TEST_F( ImoNumberTests, TestHashFunction )
{
	ImoNumber imo1( 9074729 );
	ImoNumber imo2( 9074729 );
	ImoNumber imo3( 9785811 );

	ImoNumber::Hash hasher;

	EXPECT_EQ( hasher( imo1 ), hasher( imo2 ) );
	EXPECT_NE( hasher( imo1 ), hasher( imo3 ) );

	std::unordered_map<ImoNumber, std::string, ImoNumber::Hash> imoMap;
	imoMap[imo1] = "Vessel 1";
	imoMap[imo3] = "Vessel 2";

	EXPECT_EQ( imoMap[imo1], "Vessel 1" );
	EXPECT_EQ( imoMap[imo2], "Vessel 1" );
	EXPECT_EQ( imoMap[imo3], "Vessel 2" );
}
