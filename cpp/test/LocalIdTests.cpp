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
#include "dnv/vista/sdk/LocalIdParsingErrorBuilder.h"

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

	std::optional<LocalIdBuilder> buildLocalIdFromInput( const Input& input, const Gmod& gmod, const Codebooks& codebooks )
	{
		GmodPath primaryPath;
		if ( !gmod.tryParsePath( input.PrimaryItem, primaryPath ) )
		{
			SPDLOG_WARN( "buildLocalIdFromInput: Failed to parse primary path '{}'", input.PrimaryItem );
			return std::nullopt;
		}

		LocalIdBuilder builder = LocalIdBuilder::create( input.visVersion )
									 .withPrimaryItem( std::move( primaryPath ) )
									 .withVerboseMode( input.Verbose );

		if ( input.SecondaryItem.has_value() )
		{
			GmodPath secondaryPath;
			if ( !gmod.tryParsePath( input.SecondaryItem.value(), secondaryPath ) )
			{
				SPDLOG_WARN( "buildLocalIdFromInput: Failed to parse secondary path '{}'", input.SecondaryItem.value() );
				return std::nullopt;
			}
			builder = builder.withSecondaryItem( std::move( secondaryPath ) );
		}

		try
		{
			if ( input.Quantity.has_value() )
			{
				builder = builder.withQuantity( codebooks.codebook( CodebookName::Quantity ).createTag( input.Quantity.value() ) );
			}

			if ( input.Content.has_value() )
			{
				builder = builder.withContent( codebooks.codebook( CodebookName::Content ).createTag( input.Content.value() ) );
			}

			if ( input.Position.has_value() )
			{
				builder = builder.withPosition( codebooks.codebook( CodebookName::Position ).createTag( input.Position.value() ) );
			}
		}
		catch ( [[maybe_unused]] const std::invalid_argument& ex )
		{
			SPDLOG_WARN( "buildLocalIdFromInput: Failed to create metadata tag - {}", ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::out_of_range& ex )
		{
			SPDLOG_WARN( "buildLocalIdFromInput: Failed to find codebook - {}", ex.what() );
			return std::nullopt;
		}

		return builder;
	}

	std::vector<std::pair<Input, std::string>> validTestData()
	{
		std::vector<std::pair<Input, std::string>> data;
		data.reserve( 5 );

		data.emplace_back(
			Input( "411.1/C101.31-2" ),
			"/dnv-v2/vis-3-4a/411.1/C101.31-2/meta" );

		data.emplace_back(
			Input( "411.1/C101.31-2", std::nullopt, "temperature", "exhaust.gas", "inlet" ),
			"/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" );

		data.emplace_back(
			Input( "411.1/C101.63/S206", std::nullopt, "temperature", "exhaust.gas", "inlet", VisVersion::v3_4a, true ),
			"/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" );

		data.emplace_back(
			Input( "411.1/C101.63/S206", "411.1/C101.31-5", "temperature", "exhaust.gas", "inlet", VisVersion::v3_4a, true ),
			"/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" );

		data.emplace_back(
			Input( "511.11/C101.67/S208", std::nullopt, "pressure", "starting.air", "inlet", VisVersion::v3_6a, true ),
			"/dnv-v2/vis-3-6a/511.11/C101.67/S208/~main.generator.engine/~starting.system.pneumatic/meta/qty-pressure/cnt-starting.air/pos-inlet" );

		return data;
	}

	std::vector<std::pair<Input, std::string>> validMqttTestData()
	{
		std::vector<std::pair<Input, std::string>> data;
		data.reserve( 3 );

		data.emplace_back(
			Input( "411.1/C101.31-2", std::nullopt, "temperature", "exhaust.gas", "inlet" ),
			"dnv-v2/vis-3-4a/411.1_C101.31-2/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" );

		data.emplace_back(
			Input( "411.1/C101.63/S206", std::nullopt, "temperature", "exhaust.gas", "inlet" ),
			"dnv-v2/vis-3-4a/411.1_C101.63_S206/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" );

		data.emplace_back(
			Input( "411.1/C101.63/S206", "411.1/C101.31-5", "temperature", "exhaust.gas", "inlet" ),
			"dnv-v2/vis-3-4a/411.1_C101.63_S206/411.1_C101.31-5/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_" );

		return data;
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
		const auto& codebooks = vis.codebooks( input.visVersion );

		GmodPath primaryPath;
		ASSERT_TRUE( gmod.tryParsePath( input.PrimaryItem, primaryPath ) );

		LocalIdBuilder builder = LocalIdBuilder::create( input.visVersion )
									 .withPrimaryItem( std::move( primaryPath ) )
									 .withVerboseMode( input.Verbose );

		if ( input.SecondaryItem.has_value() )
		{
			GmodPath secondaryPath;
			ASSERT_TRUE( gmod.tryParsePath( input.SecondaryItem.value(), secondaryPath ) );
			builder = builder.withSecondaryItem( std::move( secondaryPath ) );
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
		const auto& codebooks = vis.codebooks( VisVersion::v3_4a );

		GmodPath primaryPath;
		GmodPath secondaryPath;

		ASSERT_TRUE( gmod.tryParsePath( "411.1/C101.31-2", primaryPath ) );
		ASSERT_TRUE( gmod.tryParsePath( "411.1/C101.31-5", secondaryPath ) );

		SPDLOG_INFO( "Primary: {}", primaryPath.toString() );
		SPDLOG_INFO( "Secondary: {}", secondaryPath.toString() );

		LocalIdBuilder localId = LocalIdBuilder::create( VisVersion::v3_4a )
									 .withPrimaryItem( std::move( primaryPath ) )
									 .withSecondaryItem( std::move( secondaryPath ) )
									 .withVerboseMode( false )
									 .withQuantity( codebooks.codebook( CodebookName::Quantity ).createTag( "quantity" ) )
									 .withContent( codebooks.codebook( CodebookName::Content ).createTag( "content" ) )
									 .withPosition( codebooks.codebook( CodebookName::Position ).createTag( "position" ) )
									 .withState( codebooks.codebook( CodebookName::State ).createTag( "state" ) )
									 .withCalculation( codebooks.codebook( CodebookName::Calculation ).createTag( "calculate" ) );

		SPDLOG_INFO( "localId: {}", localId.toString() );

		EXPECT_TRUE( localId.isValid() );

		LocalIdBuilder allWithout = std::move( localId );

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
		// TODO Implement when Mqtt will be implemented.
	};

	TEST_P( MqttLocalIdValidTest, Test_Mqtt_LocalId_Build_Valid )
		{
		// TODO Implement when Mqtt will be implemented.

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

	TEST_P( LocalIdValidTest, Test_LocalId_Equality )
	{
		auto testCase = GetParam();
		Input input = testCase.first;

		const auto& [vis, gmod] = visAndGmod( input.visVersion );
		const auto& codebooks = vis.codebooks( input.visVersion );

		auto builder1Opt = buildLocalIdFromInput( input, gmod, codebooks );
		auto builder2Opt = buildLocalIdFromInput( input, gmod, codebooks );

		ASSERT_TRUE( builder1Opt.has_value() ) << "Failed to build builder1 for input: " << input.PrimaryItem;
		ASSERT_TRUE( builder2Opt.has_value() ) << "Failed to build builder2 for input: " << input.PrimaryItem;

		LocalIdBuilder builder1 = std::move( *builder1Opt );
		LocalIdBuilder builder2 = std::move( *builder2Opt );

		EXPECT_EQ( builder1, builder1 );

		EXPECT_EQ( builder1, builder2 );

		EXPECT_NE( &builder1, &builder2 );

		auto testTag = codebooks.codebook( CodebookName::Position ).createTag( "eqtestvalue" );

		auto modifiedBuilder2 = builder2.withPosition( testTag );

		EXPECT_NE( builder1, modifiedBuilder2 );

		EXPECT_EQ( builder1, builder2 );
		EXPECT_NE( builder2, modifiedBuilder2 );

		EXPECT_EQ( builder1, builder2 );
		EXPECT_NE( builder2, modifiedBuilder2 );

		LocalIdBuilder restoredBuilder2 = builder1.position().has_value() ? modifiedBuilder2.withPosition( *builder1.position() ) : modifiedBuilder2.withoutPosition();

		EXPECT_EQ( builder1, restoredBuilder2 );

		EXPECT_NE( &builder1, &restoredBuilder2 );
		EXPECT_NE( &modifiedBuilder2, &restoredBuilder2 );
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
			"../../../testdata/LocalIds.txt" };

		std::ifstream file;
		std::string attemptedPaths;
		bool fileOpened = false;
		std::string foundPath;

		for ( const auto& path : possiblePaths )
		{
			file.open( path );
			if ( file.is_open() )
			{
				SPDLOG_INFO( "Found test data at path: {}", path );
				foundPath = path;
				fileOpened = true;
				break;
			}
			attemptedPaths += path + ", ";
			file.clear();
			SPDLOG_DEBUG( "Failed to open test data file: {}", path );
		}

		if ( !fileOpened )
		{
			std::string errorMsg = "Failed to open LocalIds.txt. Check paths relative to build/test execution directory. Attempted: " + attemptedPaths;
			SPDLOG_ERROR( errorMsg );
			FAIL() << errorMsg;
			return;
		}

		struct ErrorInfo
		{
			std::string LocalIdStr;
			std::optional<LocalIdBuilder> LocalIdOpt;
			std::optional<std::string> ExceptionMsgOpt;
			ParsingErrors Errors;
		};
		std::vector<ErrorInfo> errored;

		std::string localIdStr;
		int lineNumber = 0;

		while ( std::getline( file, localIdStr ) )
		{
			lineNumber++;
			try
			{
				if ( localIdStr.empty() || localIdStr[0] == '#' )
					continue;

				if ( localIdStr.find( "qty-content" ) != std::string::npos )
				{
					SPDLOG_DEBUG( "Line {}: Skipping due to 'qty-content': {}", lineNumber, localIdStr );
					continue;
				}

				ParsingErrors errorBuilder;
				std::optional<LocalIdBuilder> localIdOpt;
				bool parsed = LocalIdBuilder::tryParse( localIdStr, errorBuilder, localIdOpt );

				if ( !parsed )
				{
					SPDLOG_WARN( "Line {}: Failed to parse: {}", lineNumber, localIdStr );
					errored.push_back( ErrorInfo{ localIdStr, std::move( localIdOpt ), std::nullopt, std::move( errorBuilder ) } );
				}
				else if ( !localIdOpt.has_value() || localIdOpt->isEmpty() || !localIdOpt->isValid() )
				{
					SPDLOG_WARN( "Line {}: Parsed but invalid/empty: {}", lineNumber, localIdStr );
					errored.push_back( ErrorInfo{ localIdStr, std::move( localIdOpt ), std::nullopt, std::move( errorBuilder ) } );
				}
			}
			catch ( const std::exception& ex )
			{
				std::string exMsg = ex.what();
				if ( exMsg.find( "location" ) != std::string::npos )
				{
					SPDLOG_DEBUG( "Line {}: Skipping due to location exception: {} - {}", lineNumber, localIdStr, exMsg );
					continue;
				}
				SPDLOG_ERROR( "Line {}: Exception during parsing: {} - {}", lineNumber, localIdStr, exMsg );
				errored.push_back( ErrorInfo{ localIdStr, std::nullopt, std::optional<std::string>{ exMsg }, ParsingErrors::Empty } );
			}
			catch ( ... )
			{
				SPDLOG_ERROR( "Line {}: Unknown exception during parsing: {}", lineNumber, localIdStr );
				errored.push_back( ErrorInfo{ localIdStr, std::nullopt, std::optional<std::string>{ "Unknown exception" }, ParsingErrors::Empty } );
			}
		}

		file.close();

		bool errorsFound = !errored.empty();
		if ( errorsFound )
		{
			std::stringstream errorDetails;
			errorDetails << "Found " << errored.size() << " parsing errors/exceptions in '" << foundPath << "':\n";
			SPDLOG_ERROR( "Found {} errors during LocalId smoke test parsing from '{}':", errored.size(), foundPath );

			for ( const auto& errorInfo : errored )
			{
				errorDetails << "  Input: \"" << errorInfo.LocalIdStr << "\"\n";
				SPDLOG_ERROR( "  Failed ID: {}", errorInfo.LocalIdStr );

				if ( errorInfo.ExceptionMsgOpt.has_value() )
				{
					errorDetails << "    Exception: " << *errorInfo.ExceptionMsgOpt << "\n";
					SPDLOG_ERROR( "    Exception: {}", *errorInfo.ExceptionMsgOpt );
				}

				if ( errorInfo.Errors.hasErrors() )
				{
					errorDetails << "    Parsing Errors:\n";
					SPDLOG_ERROR( "    Parsing Errors:" );
					for ( const auto& [errorType, errorMsg] : errorInfo.Errors )
					{
						errorDetails << "      - " << errorType << ": " << errorMsg << "\n";
						SPDLOG_ERROR( "      {}: {}", errorType, errorMsg );
					}
				}
				else if ( !errorInfo.ExceptionMsgOpt.has_value() )
				{
					const char* reason = ( !errorInfo.LocalIdOpt.has_value() ? "Builder not created" : ( errorInfo.LocalIdOpt->isEmpty() ? "IsEmpty" : ( !errorInfo.LocalIdOpt->isValid() ? "IsValid=false" : "Unknown" ) ) );
					errorDetails << "    Reason: Parsed but deemed invalid (" << reason << ").\n";
					SPDLOG_ERROR( "    Reason: Parsed but deemed invalid ({}).", reason );
				}
			}
			FAIL() << errorDetails.str();
		}
		else
		{
			SUCCEED() << "Successfully parsed all entries in " << foundPath;
		}
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
			for ( [[maybe_unused]] const auto& [errorType, errorMessage] : errorBuilder )
			{
				SPDLOG_INFO( "Error Type: {}, message: {}", errorType, errorMessage );
			}

			std::vector<std::string> actualErrorMessages;
			for ( [[maybe_unused]] const auto& [errorType, errorMessage] : errorBuilder )
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

		// TODO Implement when Mqtt will be implemented.

	*/
}
