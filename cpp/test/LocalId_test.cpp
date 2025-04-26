#include "pch.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/ParsingErrors.h"

namespace dnv::vista::sdk
{
	const ParsingErrors ParsingErrors::Empty = ParsingErrors();
}

namespace dnv::vista::sdk::tests
{
	struct Input
	{
		std::string PrimaryItem;
		std::optional<std::string> SecondaryItem;
		std::optional<std::string> Quantity;
		std::optional<std::string> Content;
		std::optional<std::string> Position;
		VisVersion visVersion = VisVersion::v3_4a;
		bool Verbose = false;

		Input( const std::string& primaryItem,
			const std::optional<std::string>& secondaryItem = std::nullopt,
			const std::optional<std::string>& quantity = std::nullopt,
			const std::optional<std::string>& content = std::nullopt,
			const std::optional<std::string>& position = std::nullopt,
			VisVersion visVersion = VisVersion::v3_4a,
			bool verbose = false )
			: PrimaryItem( primaryItem ),
			  SecondaryItem( secondaryItem ),
			  Quantity( quantity ),
			  Content( content ),
			  Position( position ),
			  visVersion( visVersion ),
			  Verbose( verbose )
		{
		}
	};

	inline std::pair<VIS&, const Gmod&> getVisAndGmod( VisVersion visVersion )
	{
		auto& vis = VIS::instance();
		const auto& gmod = vis.gmod( visVersion );

		return { vis, gmod };
	}

	std::vector<std::pair<Input, std::string>> getValidTestData()
	{
		return {
			{ Input( "411.1/C101.31-2" ), "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta" },
			{ Input( "411.1/C101.31-2", std::nullopt, "temperature", "exhaust.gas", "inlet" ),
				"/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" },
			{ Input( "411.1/C101.63/S206", std::nullopt, "temperature", "exhaust.gas", "inlet", VisVersion::v3_4a, true ),
				"/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" },
			{ Input( "411.1/C101.63/S206", "411.1/C101.31-5", "temperature", "exhaust.gas", "inlet", VisVersion::v3_4a, true ),
				"/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" },
			{ Input( "511.11/C101.67/S208", std::nullopt, "pressure", "starting.air", "inlet", VisVersion::v3_6a, true ),
				"/dnv-v2/vis-3-6a/511.11/C101.67/S208/~main.generator.engine/~starting.system.pneumatic/meta/qty-pressure/cnt-starting.air/pos-inlet" } };
	}

	std::vector<std::pair<Input, std::string>> getValidMqttTestData()
	{
		return {
			{ Input( "411.1/C101.31-2", std::nullopt, "temperature", "exhaust.gas", "inlet" ),
				"dnv-v2/vis-3-4a/411.1_C101.31-2/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" },
			{ Input( "411.1/C101.63/S206", std::nullopt, "temperature", "exhaust.gas", "inlet" ),
				"dnv-v2/vis-3-4a/411.1_C101.63_S206/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" },
			{ Input( "411.1/C101.63/S206", "411.1/C101.31-5", "temperature", "exhaust.gas", "inlet" ),
				"dnv-v2/vis-3-4a/411.1_C101.63_S206/411.1_C101.31-5/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" } };
	}

	class LocalIdValidTest : public ::testing::TestWithParam<std::pair<Input, std::string>>
	{
	};

	/* 	TEST( ParsingErrorsTests, Comparisons )
		{
			std::vector<ParsingErrors::ErrorEntry> errors1 = { std::make_tuple( "T1", "M1" ) };
			std::vector<ParsingErrors::ErrorEntry> errors2 = { std::make_tuple( "T1", "M1" ), std::make_tuple( "T2", "M1" ) };

			ParsingErrors e1( errors1 );
			ParsingErrors e2( errors1 );
			ParsingErrors e3( errors2 );
			ParsingErrors e4 = ParsingErrors::Empty;

			EXPECT_TRUE( e1 == e2 );
			EXPECT_TRUE( e1 == e1 );
			EXPECT_FALSE( e1 == e4 );

			EXPECT_TRUE( e1 != e3 );
			EXPECT_TRUE( e4 == ParsingErrors::Empty );
		} */

	/* 	TEST( ParsingErrorsTests, Enumerator )
		{
			std::vector<ParsingErrors::ErrorEntry> errors1 = { std::make_tuple( "T1", "M1" ) };
			std::vector<ParsingErrors::ErrorEntry> errors2 = { std::make_tuple( "T1", "M1" ), std::make_tuple( "T2", "M1" ) };

			ParsingErrors e1( errors1 );
			ParsingErrors e2( errors2 );
			ParsingErrors e3 = ParsingErrors::Empty;

			EXPECT_EQ( std::distance( e1.begin(), e1.end() ), 1 );
			EXPECT_EQ( std::distance( e2.begin(), e2.end() ), 2 );
			EXPECT_EQ( std::distance( e3.begin(), e3.end() ), 0 );
		} */

	/*
	TEST_P( LocalIdValidTest, BuildValid )
	{
		auto testCase = GetParam();
		Input input = testCase.first;
		input.Verbose = true;
		std::string expected = testCase.second;

		SPDLOG_INFO( "Testing: {}", input.PrimaryItem );

		auto [vis, gmod] = getVisAndGmod( input.visVersion );
		auto codebooks = vis.GetCodebooks( input.visVersion );

		std::optional<GmodPath> primaryPath;
		ASSERT_TRUE( gmod.TryParsePath( input.PrimaryItem, primaryPath ) );
		ASSERT_TRUE( primaryPath.has_value() );

		LocalIdBuilder builder = LocalIdBuilder::Create( input.visVersion )
									 .WithPrimaryItem( *primaryPath )
									 .WithVerboseMode( input.Verbose );

		if ( input.SecondaryItem.has_value() )
		{
			std::optional<GmodPath> secondaryPath;
			ASSERT_TRUE( gmod.TryParsePath( input.SecondaryItem.value(), secondaryPath ) );
			builder = builder.WithSecondaryItem( *secondaryPath );
		}

		if ( input.Quantity.has_value() )
		{
			auto quantityTag = codebooks[CodebookName::Quantity].CreateTag( input.Quantity.value() );
			builder = builder.WithQuantity( quantityTag );
		}

		if ( input.Content.has_value() )
		{
			auto contentTag = codebooks[CodebookName::Content].CreateTag( input.Content.value() );
			builder = builder.WithContent( contentTag );
		}

		if ( input.Position.has_value() )
		{
			auto positionTag = codebooks[CodebookName::Position].CreateTag( input.Position.value() );
			builder = builder.WithPosition( positionTag );
		}

		std::string result = builder.ToString();
		EXPECT_EQ( result, expected );
	}
*/
	/*

	TEST( LocalIdTests, BuildAllWithout )
	{
		auto [vis, gmod] = getVisAndGmod( VisVersion::v3_4a );
		auto codebooks = vis.GetCodebooks( VisVersion::v3_4a );

		std::optional<GmodPath> primaryPath;
		std::optional<GmodPath> secondaryPath;

		ASSERT_TRUE( gmod.TryParsePath( "411.1/C101.31-2", primaryPath ) );
		ASSERT_TRUE( gmod.TryParsePath( "411.1/C101.31-5", secondaryPath ) );

		ASSERT_TRUE( primaryPath.has_value() );
		ASSERT_TRUE( secondaryPath.has_value() );

		SPDLOG_INFO( "Primary: {}", primaryPath->ToString() );
		SPDLOG_INFO( "Secondary: {}", secondaryPath->ToString() );

		LocalIdBuilder localId = LocalIdBuilder::Create( VisVersion::v3_4a )
									 .WithPrimaryItem( *primaryPath )
									 .WithSecondaryItem( *secondaryPath )
									 .WithVerboseMode( false )
									 .WithQuantity( codebooks[CodebookName::Quantity].CreateTag( "quantity" ) )
									 .WithContent( codebooks[CodebookName::Content].CreateTag( "content" ) )
									 .WithPosition( codebooks[CodebookName::Position].CreateTag( "position" ) )
									 .WithState( codebooks[CodebookName::State].CreateTag( "state" ) )
									 .WithCalculation( codebooks[CodebookName::Calculation].CreateTag( "calculate" ) );

		SPDLOG_INFO( "localId: {}", localId.ToString() );

		EXPECT_TRUE( localId.IsValid() );

		LocalIdBuilder allWithout = localId;

		SPDLOG_INFO( "allWithout.ToString(): {}", allWithout.ToString() );

		SPDLOG_INFO( "Quantity: {}", allWithout.GetQuantity().value().ToString() );
		SPDLOG_INFO( "Content: {}", allWithout.GetContent().value().ToString() );
		SPDLOG_INFO( "Position: {}", allWithout.GetPosition().value().ToString() );
		SPDLOG_INFO( "State: {}", allWithout.GetState().value().ToString() );
		SPDLOG_INFO( "Calculation: {}", allWithout.GetCalculation().value().ToString() );

		allWithout = allWithout
						 .WithoutPrimaryItem()
						 .WithoutSecondaryItem()
						 .WithoutQuantity()
						 .WithoutContent()
						 .WithoutPosition()
						 .WithoutState()
						 .WithoutCalculation();

		SPDLOG_INFO( "AllWithout2: {}", allWithout.ToString() );

		EXPECT_TRUE( allWithout.IsEmpty() );
		EXPECT_FALSE( allWithout.IsValid() );
		EXPECT_EQ( allWithout.ToString(), "" );
	}
*/
	/*
		TEST( LocalIdTests, Equality )
		{
			auto testCases = getValidTestData();

			for ( const auto& testCase : testCases )
			{
				Input input = testCase.first;

				auto [vis, gmod] = getVisAndGmod( VisVersion::v3_4a );
				auto codebooks = vis.GetCodebooks( VisVersion::v3_4a );

				std::optional<GmodPath> primaryPath;
				ASSERT_TRUE( gmod.TryParsePath( input.PrimaryItem, primaryPath ) );

				std::optional<GmodPath> secondaryPath;
				if ( input.SecondaryItem.has_value() )
				{
					ASSERT_TRUE( gmod.TryParsePath( input.SecondaryItem.value(), secondaryPath ) );
				}

				LocalIdBuilder localId = LocalIdBuilder::Create( VisVersion::v3_4a )
											 .WithPrimaryItem( *primaryPath );

				if ( secondaryPath.has_value() )
				{
					localId = localId.WithSecondaryItem( *secondaryPath );
				}

				if ( input.Quantity.has_value() )
				{
					localId = localId.WithQuantity( codebooks[CodebookName::Quantity].CreateTag( input.Quantity.value() ) );
				}

				if ( input.Content.has_value() )
				{
					localId = localId.WithContent( codebooks[CodebookName::Content].CreateTag( input.Content.value() ) );
				}

				if ( input.Position.has_value() )
				{
					localId = localId.WithPosition( codebooks[CodebookName::Position].CreateTag( input.Position.value() ) );
				}

				LocalIdBuilder otherLocalId = localId;
				EXPECT_EQ( localId, otherLocalId );

				LocalIdBuilder freshCopy = LocalIdBuilder::Create( VisVersion::v3_4a );
				if ( localId.GetPrimaryItem().has_value() )
				{
					freshCopy = freshCopy.WithPrimaryItem( *localId.GetPrimaryItem() );
				}
				if ( localId.GetSecondaryItem().has_value() )
				{
					freshCopy = freshCopy.WithSecondaryItem( *localId.GetSecondaryItem() );
				}
				if ( localId.GetQuantity().has_value() )
				{
					freshCopy = freshCopy.WithQuantity( *localId.GetQuantity() );
				}
				if ( localId.GetContent().has_value() )
				{
					freshCopy = freshCopy.WithContent( *localId.GetContent() );
				}
				if ( localId.GetPosition().has_value() )
				{
					freshCopy = freshCopy.WithPosition( *localId.GetPosition() );
				}

				EXPECT_EQ( localId, freshCopy );

				if ( input.Position.has_value() )
				{
					otherLocalId = otherLocalId.WithPosition( codebooks[CodebookName::Position].CreateTag( "eqtestvalue" ) );
					EXPECT_NE( localId, otherLocalId );
				}
				else
				{
					otherLocalId = otherLocalId.WithPosition( codebooks[CodebookName::Position].CreateTag( "eqtestvalue" ) );
					EXPECT_NE( localId, otherLocalId );
				}

				if ( localId.GetPosition().has_value() )
				{
					otherLocalId = otherLocalId.WithPosition( *localId.GetPosition() );
				}
				else
				{
					otherLocalId = otherLocalId.WithoutPosition();
				}

				EXPECT_EQ( localId, otherLocalId );
			}
		}
*/

	/* 	TEST( LocalIdTests, Parsing )
		{
			std::vector<std::string> testCases = {
				"/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking",
				"/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
				"/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
				"/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
				"/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
				"/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
				"/dnv-v2/vis-3-4a/511.11-21O/C101.67/S208/meta/qty-pressure/cnt-air/state-low" };

			for ( const auto& localIdStr : testCases )
			{
				ParsingErrors errorBuilder;
				std::optional<LocalIdBuilder> localId;
				bool parsed = LocalIdBuilder::TryParse( localIdStr, errorBuilder, localId );

				EXPECT_TRUE( parsed );
				ASSERT_TRUE( localId.has_value() );
				EXPECT_EQ( localIdStr, localId->ToString() );
			}
		} */

	TEST( LocalIdTests, SimpleParse )
	{
		std::string localIdAsString = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet";

		ParsingErrors errorBuilder;
		std::optional<LocalIdBuilder> localId;
		bool success = LocalIdBuilder::tryParse( localIdAsString, errorBuilder, localId );
		EXPECT_TRUE( success );
		EXPECT_TRUE( localId.has_value() );
	}

	/* 	TEST( LocalIdTests, ParsingValidation )
		{
			struct TestCase
			{
				std::string localIdStr;
				std::vector<std::string> expectedErrorMessages;
			};

			std::vector<TestCase> testCases = {
				{ "/invalid-naming/vis-3-4a/400a/meta/cnt-refrigerant/state-leaking", { "Invalid naming rule prefix" } },
				{ "/dnv-v2/vis-invalid/400a/meta/cnt-refrigerant/state-leaking", { "Invalid VIS version: invalid" } },
				{ "", { "LocalId string is empty" } },
				{ "something_invalid", { "Invalid string format" } } };

			for ( const auto& testCase : testCases )
			{
				ParsingErrors errorBuilder;
				std::optional<LocalIdBuilder> localId;
				bool parsed = LocalIdBuilder::TryParse( testCase.localIdStr, errorBuilder, localId );

				EXPECT_FALSE( parsed );
				EXPECT_TRUE( errorBuilder.HasErrors() );

				SPDLOG_INFO( "Errors for LocalId: {}", testCase.localIdStr );
				for ( const auto& [errorType, errorMessage] : errorBuilder )
				{
					SPDLOG_INFO( "Error Type: {}, message: {}", errorType, errorMessage );
				}

				std::vector<std::string> actualErrorMessages;
				for ( const auto& [errorType, errorMessage] : errorBuilder )
				{
					actualErrorMessages.push_back( errorMessage );
				}

				for ( const auto& expectedMsg : testCase.expectedErrorMessages )
				{
					bool found = false;
					for ( const auto& actualMsg : actualErrorMessages )
					{
						if ( actualMsg.find( expectedMsg ) != std::string::npos )
						{
							found = true;
							break;
						}
					}
					EXPECT_TRUE( found ) << "Expected error message not found: " << expectedMsg;
				}
			}
		} */

	/*
	INSTANTIATE_TEST_SUITE_P(
			ValidTests,
			LocalIdValidTest,
			::testing::ValuesIn( getValidTestData() ) );
			*/
}
