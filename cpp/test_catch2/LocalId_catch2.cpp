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

	inline std::pair<VIS, Gmod> GetVisAndGmod( VisVersion visVersion )
	{
		VIS vis = VIS::Instance();
		Gmod gmod = vis.GetGmod( visVersion );
		return { vis, gmod };
	}

	static std::vector<std::pair<Input, std::string>> GetValidTestData()
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

	static std::vector<std::pair<Input, std::string>> GetValidMqttTestData()
	{
		return {
			{ Input( "411.1/C101.31-2", std::nullopt, "temperature", "exhaust.gas", "inlet" ),
				"dnv-v2/vis-3-4a/411.1_C101.31-2/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" },
			{ Input( "411.1/C101.63/S206", std::nullopt, "temperature", "exhaust.gas", "inlet" ),
				"dnv-v2/vis-3-4a/411.1_C101.63_S206/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" },
			{ Input( "411.1/C101.63/S206", "411.1/C101.31-5", "temperature", "exhaust.gas", "inlet" ),
				"dnv-v2/vis-3-4a/411.1_C101.63_S206/411.1_C101.31-5/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" } };
	}

	/*
	TEST_CASE( "ParsingErrors Comparisons", "[parsingerrors]" )
	{
		std::vector<ParsingErrors::ErrorEntry> errors1 = { std::make_tuple( "T1", "M1" ) };
		std::vector<ParsingErrors::ErrorEntry> errors2 = { std::make_tuple( "T1", "M1" ), std::make_tuple( "T2", "M1" ) };

		ParsingErrors e1( errors1 );
		ParsingErrors e2( errors1 );
		ParsingErrors e3( errors2 );
		ParsingErrors e4 = ParsingErrors::Empty;

		CHECK( e1 == e2 );
		CHECK( e1 == e1 );
		CHECK_FALSE( e1 == e4 );

		CHECK( e1 != e3 );
		CHECK( e4 == ParsingErrors::Empty );
	}

	TEST_CASE( "ParsingErrors Enumerator", "[parsingerrors]" )
	{
		std::vector<ParsingErrors::ErrorEntry> errors1 = { std::make_tuple( "T1", "M1" ) };
		std::vector<ParsingErrors::ErrorEntry> errors2 = { std::make_tuple( "T1", "M1" ), std::make_tuple( "T2", "M1" ) };

		ParsingErrors e1( errors1 );
		ParsingErrors e2( errors2 );
		ParsingErrors e3 = ParsingErrors::Empty;

		CHECK( std::distance( e1.begin(), e1.end() ) == 1 );
		CHECK( std::distance( e2.begin(), e2.end() ) == 2 );
		CHECK( std::distance( e3.begin(), e3.end() ) == 0 );
	}

	TEST_CASE( "Test_Parsing_Validation", "[localid]" )
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

			CHECK_FALSE( parsed );
			REQUIRE( errorBuilder.HasErrors() );

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
				INFO( "Expected error message not found: " << expectedMsg );
				CHECK( found );
			}
		}
	}
*/
	/*
	TEST_CASE( "Test", "[localid]" )
	{
		std::string localIdAsString = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet";

		std::optional<LocalIdBuilder> localId;
		ParsingErrors errors;
		bool success = LocalIdBuilder::TryParse( localIdAsString, errors, localId );
		REQUIRE( success );
		REQUIRE( localId.has_value() );
	}

	*/

	/*
		TEST_CASE( "Test_Parsing", "[localid]" )
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
				INFO( "Testing: " << localIdStr );

				std::optional<LocalIdBuilder> localId;
				ParsingErrors errors;
				bool parsed = LocalIdBuilder::TryParse( localIdStr, errors, localId );

				CHECK( parsed );
				REQUIRE( localId.has_value() );
				CHECK( localIdStr == localId->ToString() );
			}
		}

		*/

	TEST_CASE( "Test_LocalId_Build_Valid", "[localid]" )
	{
		SPDLOG_INFO( "\nTest_LocalId_Build_Valid" );

		static const auto testData = GetValidTestData();

		auto testCase = GENERATE( from_range( testData ) );

		Input input = testCase.first;
		std::string expected = testCase.second;

		SPDLOG_INFO( "Testing: {}", input.PrimaryItem );

		auto [vis, gmod] = GetVisAndGmod( input.visVersion );
		auto codebooks = vis.GetCodebooks( input.visVersion );

		std::optional<GmodPath> primaryPath;
		REQUIRE( gmod.TryParsePath( input.PrimaryItem, primaryPath ) );
		REQUIRE( primaryPath.has_value() );

		input.Verbose = true;
		LocalIdBuilder builder = LocalIdBuilder::Create( input.visVersion )
									 .WithPrimaryItem( *primaryPath )
									 .WithVerboseMode( input.Verbose );

		if ( input.SecondaryItem.has_value() )
		{
			std::optional<GmodPath> secondaryPath;
			REQUIRE( gmod.TryParsePath( input.SecondaryItem.value(), secondaryPath ) );
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
		CHECK( result == expected );
	}
	/*
		TEST_CASE( "Test_LocalId_Build_AllWithout", "[localid]" )
		{
			auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );
			auto codebooks = vis.GetCodebooks( VisVersion::v3_4a );

			std::optional<GmodPath> primaryPath;
			std::optional<GmodPath> secondaryPath;

			REQUIRE( gmod.TryParsePath( "411.1/C101.31-2", primaryPath ) );
			REQUIRE( gmod.TryParsePath( "411.1/C101.31-5", secondaryPath ) );

			REQUIRE( primaryPath.has_value() );
			REQUIRE( secondaryPath.has_value() );

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

			CHECK( localId.IsValid() );

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
			// SPDLOG_INFO( "Secondary: {}", allWithout.GetSecondaryItem().value().ToString() );
			// SPDLOG_INFO( "Quantity2: {}", allWithout.GetQuantity().value().ToString() );
			// SPDLOG_INFO( "Content2: {}", allWithout.GetContent().value().ToString() );
			// SPDLOG_INFO( "Position2: {}", allWithout.GetPosition().value().ToString() );
			// SPDLOG_INFO( "State2: {}", allWithout.GetState().value().ToString() );
			// SPDLOG_INFO( "Calculation2: {}", allWithout.GetCalculation().value().ToString() );

			CHECK( allWithout.IsEmpty() );
			CHECK_FALSE( allWithout.IsValid() );
			CHECK( allWithout.ToString() == "" );
		}

		TEST_CASE( "Test_LocalId_Equality", "[localid]" )
		{
			static const auto testData = GetValidTestData();

			auto testCase = GENERATE( from_range( testData ) );

			Input input = testCase.first;

			auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );
			auto codebooks = vis.GetCodebooks( VisVersion::v3_4a );

			std::optional<GmodPath> primaryPath;
			REQUIRE( gmod.TryParsePath( input.PrimaryItem, primaryPath ) );

			std::optional<GmodPath> secondaryPath;
			if ( input.SecondaryItem.has_value() )
			{
				REQUIRE( gmod.TryParsePath( input.SecondaryItem.value(), secondaryPath ) );
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
			CHECK( localId == otherLocalId );

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

			CHECK( localId == freshCopy );

			if ( input.Position.has_value() )
			{
				otherLocalId = otherLocalId.WithPosition( codebooks[CodebookName::Position].CreateTag( "eqtestvalue" ) );
				CHECK( localId != otherLocalId );
			}
			else
			{
				otherLocalId = otherLocalId.WithPosition( codebooks[CodebookName::Position].CreateTag( "eqtestvalue" ) );
				CHECK( localId != otherLocalId );
			}

			if ( localId.GetPosition().has_value() )
			{
				otherLocalId = otherLocalId.WithPosition( *localId.GetPosition() );
			}
			else
			{
				otherLocalId = otherLocalId.WithoutPosition();
			}

			CHECK( localId == otherLocalId );
		}
	*/

	/*
TEST_CASE( "SmokeTest_Parsing", "[localid][slow]" )
{
	std::string testDataPath = "testdata/LocalIds.txt";
	std::ifstream file( testDataPath );

	INFO( "Attempting to open test data file: " << testDataPath );
	REQUIRE( file.is_open() );

	struct ErrorEntry
	{
		std::string localIdStr;
		std::optional<LocalIdBuilder> localId;
		std::string errorMessage;
		ParsingErrors parsingErrors;
	};
	std::vector<ErrorEntry> errored;

	std::string localIdStr;
	while ( std::getline( file, localIdStr ) )
	{
		if ( localIdStr.empty() )
			continue;

		if ( localIdStr.find( "qty-content" ) != std::string::npos )
			continue;

		try
		{
			ParsingErrors errorBuilder;
			std::optional<LocalIdBuilder> localId;
			bool success = LocalIdBuilder::TryParse( localIdStr, errorBuilder, localId );

			if ( !success )
			{
				errored.push_back( { localIdStr, localId, "", errorBuilder } );
			}
			else if ( localId.has_value() && ( localId->IsEmpty() || !localId->IsValid() ) )
			{
				errored.push_back( { localIdStr, localId, "", errorBuilder } );
			}
		}
		catch ( const std::exception& ex )
		{
			if ( std::string( ex.what() ).find( "location" ) != std::string::npos )
				continue;

			errored.push_back( { localIdStr, std::nullopt, ex.what(), ParsingErrors::Empty } );
		}
	}

	if ( !errored.empty() )
	{
		std::stringstream errorMsg;
		errorMsg << "Failed to parse " << errored.size() << " LocalIds:" << std::endl;

		for ( const auto& entry : errored )
		{
			errorMsg << "LocalId: " << entry.localIdStr << std::endl;
			if ( !entry.errorMessage.empty() )
				errorMsg << "  Exception: " << entry.errorMessage << std::endl;
			if ( entry.parsingErrors.HasErrors() )
			{
				errorMsg << "  Errors: " << std::endl;
				for ( const auto& [errorType, errorMessage] : entry.parsingErrors )
				{
					errorMsg << "    " << errorType << ": " << errorMessage << std::endl;
				}
			}
		}

		FAIL( errorMsg.str() );
	}

	CHECK( errored.empty() );
}
*/
}
