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
	TEST( ParsingErrorsTests, Comparisons )
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
	}

	TEST( ParsingErrorsTests, Enumerator )
	{
		std::vector<ParsingErrors::ErrorEntry> errors1 = { std::make_tuple( "T1", "M1" ) };
		std::vector<ParsingErrors::ErrorEntry> errors2 = { std::make_tuple( "T1", "M1" ), std::make_tuple( "T2", "M1" ) };

		ParsingErrors e1( errors1 );
		ParsingErrors e2( errors2 );
		ParsingErrors e3 = ParsingErrors::Empty;

		EXPECT_EQ( std::distance( e1.begin(), e1.end() ), 1 );
		EXPECT_EQ( std::distance( e2.begin(), e2.end() ), 2 );
		EXPECT_EQ( std::distance( e3.begin(), e3.end() ), 0 );
	}

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
			std::optional<std::string> secondaryItem = std::nullopt,
			std::optional<std::string> quantity = std::nullopt,
			std::optional<std::string> content = std::nullopt,
			std::optional<std::string> position = std::nullopt,
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

	inline std::pair<VIS&, const Gmod&> visAndGmod( VisVersion visVersion )
	{
		auto& vis = VIS::instance();
		const auto& gmod = vis.gmod( visVersion );

		return { vis, gmod };
	}

	std::vector<std::pair<Input, std::string>> validTestData()
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

	std::vector<std::pair<Input, std::string>> validMqttTestData()
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

	TEST_P( LocalIdValidTest, Test_LocalId_Build_Valid )
	{
		auto testCase = GetParam();
		Input input = testCase.first;
		input.Verbose = true;
		std::string expected = testCase.second;

		SPDLOG_INFO( "Testing: {}", input.PrimaryItem );

		auto [vis, gmod] = visAndGmod( input.visVersion );
		auto codebooks = vis.codebooks( input.visVersion );

		std::optional<GmodPath> primaryPath;
		ASSERT_TRUE( gmod.tryParsePath( input.PrimaryItem, primaryPath ) );
		ASSERT_TRUE( primaryPath.has_value() );

		LocalIdBuilder builder = LocalIdBuilder::create( input.visVersion )
									 .withPrimaryItem( *primaryPath )
									 .withVerboseMode( input.Verbose );

		if ( input.SecondaryItem.has_value() )
		{
			std::optional<GmodPath> secondaryPath;
			ASSERT_TRUE( gmod.tryParsePath( input.SecondaryItem.value(), secondaryPath ) );
			builder = builder.withSecondaryItem( *secondaryPath );
		}

		if ( input.Quantity.has_value() )
		{
			auto quantityTag = codebooks.codebook( CodebookName::Quantity ).createTag( input.Quantity.value() );
			builder = builder.withQuantity( quantityTag );
		}

		if ( input.Content.has_value() )
		{
			auto contentTag = codebooks.codebook( CodebookName::Content ).createTag( input.Content.value() );
			builder = builder.withContent( contentTag );
		}

		if ( input.Position.has_value() )
		{
			auto positionTag = codebooks.codebook( CodebookName::Position ).createTag( input.Position.value() );
			builder = builder.withPosition( positionTag );
		}

		std::string result = builder.toString();
		EXPECT_EQ( result, expected );
	}

	TEST( LocalIdTests, Test_LocalId_Build_AllWithout )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );
		auto codebooks = vis.codebooks( VisVersion::v3_4a );

		std::optional<GmodPath> primaryPath;
		std::optional<GmodPath> secondaryPath;

		ASSERT_TRUE( gmod.tryParsePath( "411.1/C101.31-2", primaryPath ) );
		ASSERT_TRUE( gmod.tryParsePath( "411.1/C101.31-5", secondaryPath ) );

		ASSERT_TRUE( primaryPath.has_value() );
		ASSERT_TRUE( secondaryPath.has_value() );

		SPDLOG_INFO( "Primary: {}", primaryPath->toString() );
		SPDLOG_INFO( "Secondary: {}", secondaryPath->toString() );

		LocalIdBuilder localId = LocalIdBuilder::create( VisVersion::v3_4a )
									 .withPrimaryItem( *primaryPath )
									 .withSecondaryItem( *secondaryPath )
									 .withVerboseMode( false )
									 .withQuantity( codebooks.codebook( CodebookName::Quantity ).createTag( "quantity" ) )
									 .withContent( codebooks.codebook( CodebookName::Content ).createTag( "content" ) )
									 .withPosition( codebooks.codebook( CodebookName::Position ).createTag( "position" ) )
									 .withState( codebooks.codebook( CodebookName::State ).createTag( "state" ) )
									 .withCalculation( codebooks.codebook( CodebookName::Calculation ).createTag( "calculate" ) );

		SPDLOG_INFO( "localId: {}", localId.toString() );

		EXPECT_TRUE( localId.isValid() );

		LocalIdBuilder allWithout = localId;

		SPDLOG_INFO( "allWithout.toString(): {}", allWithout.toString() );

		SPDLOG_INFO( "Quantity: {}", allWithout.quantity().value().toString() );
		SPDLOG_INFO( "Content: {}", allWithout.content().value().toString() );
		SPDLOG_INFO( "Position: {}", allWithout.position().value().toString() );
		SPDLOG_INFO( "State: {}", allWithout.state().value().toString() );
		SPDLOG_INFO( "Calculation: {}", allWithout.calculation().value().toString() );

		allWithout = allWithout
						 .withoutPrimaryItem()
						 .withoutSecondaryItem()
						 .withoutQuantity()
						 .withoutContent()
						 .withoutPosition()
						 .withoutState()
						 .withoutCalculation();

		SPDLOG_INFO( "AllWithout2: {}", allWithout.toString() );

		EXPECT_TRUE( allWithout.isEmpty() );
		EXPECT_FALSE( allWithout.isValid() );
		EXPECT_EQ( allWithout.toString(), "" );
	}

	/*
	class MqttLocalIdValidTest : public ::testing::TestWithParam<std::pair<Input, std::string>>
	{
	};

	TEST_P( MqttLocalIdValidTest, Test_Mqtt_LocalId_Build_Valid )
		{
			auto testCase = GetParam();
			Input input = testCase.first;
			std::string expected = testCase.second;

			auto [vis, gmod] = visAndGmod( input.visVersion );
			auto codebooks = vis.codebooks( input.visVersion );

			std::optional<GmodPath> primaryPath;
			ASSERT_TRUE( gmod.tryParsePath( input.PrimaryItem, primaryPath ) );

			std::optional<GmodPath> secondaryPath;
			if ( input.SecondaryItem.has_value() )
			{
				ASSERT_TRUE( gmod.tryParsePath( input.SecondaryItem.value(), secondaryPath ) );
			}

			LocalIdBuilder builder = LocalIdBuilder::create( input.visVersion )
										 .withPrimaryItem( *primaryPath )
										 .withVerboseMode( input.Verbose );

			if ( secondaryPath.has_value() )
			{
				builder = builder.withSecondaryItem( *secondaryPath );
			}

			if ( input.Quantity.has_value() )
			{
				auto quantityTag = codebooks.codebook( CodebookName::Quantity ).createTag( input.Quantity.value() );
				builder = builder.withQuantity( quantityTag );
			}

			if ( input.Content.has_value() )
			{
				auto contentTag = codebooks.codebook( CodebookName::Content ).createTag( input.Content.value() );
				builder = builder.withContent( contentTag );
			}

			if ( input.Position.has_value() )
			{
				auto positionTag = codebooks.codebook( CodebookName::Position ).createTag( input.Position.value() );
				builder = builder.withPosition( positionTag );
			}

			auto mqttLocalId = builder.buildMqtt();
			std::string result = mqttLocalId.toString();
			EXPECT_EQ( result, expected );
		}
			*/

	TEST( LocalIdTests, Test_LocalId_Equality )
	{
		auto testCases = validTestData();

		for ( const auto& testCase : testCases )
		{
			Input input = testCase.first;

			auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );
			auto codebooks = vis.codebooks( VisVersion::v3_4a );

			std::optional<GmodPath> primaryPath;
			ASSERT_TRUE( gmod.tryParsePath( input.PrimaryItem, primaryPath ) );

			std::optional<GmodPath> secondaryPath;
			if ( input.SecondaryItem.has_value() )
			{
				ASSERT_TRUE( gmod.tryParsePath( input.SecondaryItem.value(), secondaryPath ) );
			}

			LocalIdBuilder localId = LocalIdBuilder::create( VisVersion::v3_4a )
										 .withPrimaryItem( *primaryPath );

			if ( secondaryPath.has_value() )
			{
				localId = localId.withSecondaryItem( *secondaryPath );
			}

			if ( input.Quantity.has_value() )
			{
				localId = localId.withQuantity( codebooks.codebook( CodebookName::Quantity ).createTag( input.Quantity.value() ) );
			}

			if ( input.Content.has_value() )
			{
				localId = localId.withContent( codebooks.codebook( CodebookName::Content ).createTag( input.Content.value() ) );
			}

			if ( input.Position.has_value() )
			{
				localId = localId.withPosition( codebooks.codebook( CodebookName::Position ).createTag( input.Position.value() ) );
			}

			LocalIdBuilder otherLocalId = localId;
			EXPECT_EQ( localId, otherLocalId );

			LocalIdBuilder freshCopy = LocalIdBuilder::create( VisVersion::v3_4a );
			if ( localId.primaryItem().length() > 0 )
			{
				freshCopy = freshCopy.withPrimaryItem( localId.primaryItem() );
			}
			if ( localId.secondaryItem().has_value() )
			{
				freshCopy = freshCopy.withSecondaryItem( *localId.secondaryItem() );
			}
			if ( localId.quantity().has_value() )
			{
				freshCopy = freshCopy.withQuantity( *localId.quantity() );
			}
			if ( localId.content().has_value() )
			{
				freshCopy = freshCopy.withContent( *localId.content() );
			}
			if ( localId.position().has_value() )
			{
				freshCopy = freshCopy.withPosition( *localId.position() );
			}

			EXPECT_EQ( localId, freshCopy );

			if ( input.Position.has_value() )
			{
				otherLocalId = otherLocalId.withPosition( codebooks.codebook( CodebookName::Position ).createTag( "eqtestvalue" ) );
				EXPECT_NE( localId, otherLocalId );
			}
			else
			{
				otherLocalId = otherLocalId.withPosition( codebooks.codebook( CodebookName::Position ).createTag( "eqtestvalue" ) );
				EXPECT_NE( localId, otherLocalId );
			}

			if ( localId.position().has_value() )
			{
				otherLocalId = otherLocalId.withPosition( *localId.position() );
			}
			else
			{
				otherLocalId = otherLocalId.withoutPosition();
			}

			EXPECT_EQ( localId, otherLocalId );
		}
	}

	TEST( LocalIdTests, Test_Parsing )
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
			bool parsed = LocalIdBuilder::tryParse( localIdStr, errorBuilder, localId );

			EXPECT_TRUE( parsed );
			ASSERT_TRUE( localId.has_value() );
			EXPECT_EQ( localIdStr, localId->toString() );
		}
	}

	TEST( LocalIdTests, Test )
	{
		std::string localIdAsString = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet";

		ParsingErrors errorBuilder;
		std::optional<LocalIdBuilder> localId;
		bool success = LocalIdBuilder::tryParse( localIdAsString, errorBuilder, localId );
		EXPECT_TRUE( success );
		EXPECT_TRUE( localId.has_value() );
	}

	TEST( LocalIdTests, SmokeTest_Parsing )
	{
		std::vector<std::string> possiblePaths = {
			"testdata/LocalIds.txt",
			"../testdata/LocalIds.txt",
			"../../testdata/LocalIds.txt",
			"../../../testdata/LocalIds.txt",
		};

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
			ASSERT_TRUE( false ) << "Failed to open LocalIds.txt. Attempted paths: " << attemptedPaths;
			return;
		}

		std::vector<std::tuple<std::string, std::optional<LocalIdBuilder>, std::optional<std::string>, ParsingErrors>> errored;

		std::string localIdStr;
		while ( std::getline( file, localIdStr ) )
		{
			try
			{
				if ( localIdStr.find( "qty-content" ) != std::string::npos )
					continue;

				ParsingErrors errorBuilder;
				std::optional<LocalIdBuilder> localId;
				bool parsed = LocalIdBuilder::tryParse( localIdStr, errorBuilder, localId );

				if ( !parsed )
				{
					errored.push_back( std::make_tuple( localIdStr, localId, std::nullopt, errorBuilder ) );
				}
				else if ( !localId->isValid() || localId->isEmpty() )
				{
					errored.push_back( std::make_tuple( localIdStr, localId, std::nullopt, errorBuilder ) );
				}
			}
			catch ( const std::exception& ex )
			{
				if ( std::string( ex.what() ).find( "location" ) != std::string::npos )
					continue;

				errored.push_back( std::make_tuple( localIdStr, std::nullopt, ex.what(), ParsingErrors::Empty ) );
			}
		}

		for ( const auto& [id, builder, ex, errors] : errored )
		{
			SPDLOG_ERROR( "Failed to parse: {}", id );

			if ( ex.has_value() )
			{
				SPDLOG_ERROR( "  Exception: {}", *ex );
			}

			for ( const auto& [errorType, errorMsg] : errors )
			{
				SPDLOG_ERROR( "  Error: {} - {}", errorType, errorMsg );
			}
		}

		EXPECT_TRUE( errored.empty() ) << "Found " << errored.size() << " errors";
	}

	TEST( LocalIdTests, Test_Parsing_Validation )
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
			bool parsed = LocalIdBuilder::tryParse( testCase.localIdStr, errorBuilder, localId );

			EXPECT_FALSE( parsed );
			EXPECT_TRUE( errorBuilder.hasErrors() );

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
	}

	INSTANTIATE_TEST_SUITE_P(
		ValidTests,
		LocalIdValidTest,
		::testing::ValuesIn( validTestData() ) );

	/*
	INSTANTIATE_TEST_SUITE_P(
			ValidMqttTests,
			MqttLocalIdValidTest,
			::testing::ValuesIn( validMqttTestData() ) );
			*/
}
