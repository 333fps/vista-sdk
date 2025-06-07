/**
 * @file UniversalIdTests.cpp
 * @brief Unit tests for LocalId and LocalIdBuilder classes.
 */

#include "pch.h"

#include "TestDataLoader.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/ImoNumber.h"

#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/LocalIdParsingErrorBuilder.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/UniversalId.h"
#include "dnv/vista/sdk/UniversalIdBuilder.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VISVersion.h"

namespace dnv::vista::sdk
{
	const ParsingErrors ParsingErrors::Empty = ParsingErrors();
}

namespace dnv::vista::sdk::tests
{
	//----------------------------------------------
	// Test Data
	//----------------------------------------------

	static const std::vector<std::string> testData = {
		"data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet",
		"data.dnv.com/IMO1234567/dnv-v2/vis-3-7a/612.21/C701.23/C633/meta/calc~accumulate" };

	//----------------------------------------------
	// Test_TryParsing
	//----------------------------------------------

	TEST( UniversalIdTests, Test_TryParsing_Case0 )
	{
		const std::string testCase = testData[0];

		ParsingErrors errors;
		std::optional<UniversalIdBuilder> uid;
		bool success = UniversalIdBuilder::tryParse( testCase, errors, uid );

		EXPECT_TRUE( success );
	}

	TEST( UniversalIdTests, Test_TryParsing_Case1 )
	{
		const std::string testCase = testData[1];

		ParsingErrors errors;
		std::optional<UniversalIdBuilder> uid;
		bool success = UniversalIdBuilder::tryParse( testCase, errors, uid );

		EXPECT_TRUE( success );
	}

	//----------------------------------------------
	// Test_Parsing
	//----------------------------------------------

	TEST( UniversalIdTests, Test_Parsing_Case0 )
	{
		const std::string testCase = testData[0];

		auto universalIdBuilder = UniversalIdBuilder::parse( testCase );

		EXPECT_TRUE( universalIdBuilder.imoNumber().has_value() );
		EXPECT_TRUE( universalIdBuilder.imoNumber().value() == ImoNumber( 1234567 ) );
	}

	TEST( UniversalIdTests, Test_Parsing_Case1 )
	{
		const std::string testCase = testData[1];

		auto universalIdBuilder = UniversalIdBuilder::parse( testCase );

		EXPECT_TRUE( universalIdBuilder.imoNumber().has_value() );
		EXPECT_TRUE( universalIdBuilder.imoNumber().value() == ImoNumber( 1234567 ) );
	}

	//----------------------------------------------
	// Test_ToString
	//----------------------------------------------

	TEST( UniversalIdTests, Test_ToString_Case0 )
	{
		const std::string testCase = testData[0];

		auto universalId = UniversalIdBuilder::parse( testCase );

		auto universalIdString = universalId.toString();
		EXPECT_EQ( testCase, universalIdString );
	}

	TEST( UniversalIdTests, Test_ToString_Case1 )
	{
		const std::string testCase = testData[1];

		auto universalId = UniversalIdBuilder::parse( testCase );

		auto universalIdString = universalId.toString();
		EXPECT_EQ( testCase, universalIdString );
	}

	//----------------------------------------------
	// Test_UniversalBuilder_Add_And_RemoveAll
	//----------------------------------------------

	TEST( UniversalIdTests, Test_UniversalBuilder_Add_And_RemoveAll_Case0 )
	{
		const std::string testCase = testData[0];

		std::optional<UniversalIdBuilder> universalIdBuilder;
		bool success = UniversalIdBuilder::tryParse( testCase, universalIdBuilder );

		ASSERT_TRUE( success );
		ASSERT_TRUE( universalIdBuilder.has_value() );
		EXPECT_TRUE( universalIdBuilder->localId().has_value() );
		EXPECT_TRUE( universalIdBuilder->imoNumber().has_value() );

		auto id = universalIdBuilder->withoutImoNumber().withoutLocalId();

		EXPECT_FALSE( id.localId().has_value() );
		EXPECT_FALSE( id.imoNumber().has_value() );
	}

	TEST( UniversalIdTests, Test_UniversalBuilder_Add_And_RemoveAll_Case1 )
	{
		const std::string testCase = testData[1];

		std::optional<UniversalIdBuilder> universalIdBuilder;
		bool success = UniversalIdBuilder::tryParse( testCase, universalIdBuilder );

		ASSERT_TRUE( success );
		ASSERT_TRUE( universalIdBuilder.has_value() );
		EXPECT_TRUE( universalIdBuilder->localId().has_value() );
		EXPECT_TRUE( universalIdBuilder->imoNumber().has_value() );

		auto id = universalIdBuilder->withoutImoNumber().withoutLocalId();

		EXPECT_FALSE( id.localId().has_value() );
		EXPECT_FALSE( id.imoNumber().has_value() );
	}

	//----------------------------------------------
	// Test_UniversalBuilder_TryWith
	//----------------------------------------------

	TEST( UniversalIdTests, Test_UniversalBuilder_TryWith )
	{ /* TODO Check this ! */
		auto universalBuilder = UniversalIdBuilder::create( VisVersion::v3_4a )
									.withoutLocalId()
									.withoutImoNumber();

		universalBuilder.tryWithLocalId( std::nullopt );
		universalBuilder.tryWithImoNumber( std::nullopt );

		EXPECT_FALSE( universalBuilder.localId().has_value() );
		EXPECT_FALSE( universalBuilder.imoNumber().has_value() );
	}
}
