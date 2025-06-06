/**
 * @file LocalIdTests.cpp
 * @brief Unit tests for LocalId and LocalIdBuilder classes.
 */

#include "pch.h"

#include "TestDataLoader.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/Locations.h"

#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/LocalIdParsingErrorBuilder.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VISVersion.h"

namespace dnv::vista::sdk
{
	const ParsingErrors ParsingErrors::Empty = ParsingErrors();
}

namespace dnv::vista::sdk::tests
{
	namespace
	{
		constexpr const char* INVALID_LOCAL_IDS_TEST_DATA_PATH = "testdata/InvalidLocalIds.json";
	}

	//=====================================================================
	// ParsingErrorsTests
	//=====================================================================

	//----------------------------------------------
	// Comparisons
	//----------------------------------------------

	TEST( ParsingErrorsTests, Comparisons )
	{
		LocalIdParsingErrorBuilder builder1;
		builder1.addError( LocalIdParsingState::NamingRule, "M1" );
		ParsingErrors e1 = builder1.build();

		LocalIdParsingErrorBuilder builder2;
		builder2.addError( LocalIdParsingState::NamingRule, "M1" );
		ParsingErrors e2 = builder2.build();

		LocalIdParsingErrorBuilder builder3;
		builder3.addError( LocalIdParsingState::NamingRule, "M1" );
		builder3.addError( LocalIdParsingState::VisVersion, "M1" );
		ParsingErrors e3 = builder3.build();

		ParsingErrors e4 = ParsingErrors::Empty;

		EXPECT_EQ( e1, e2 );
		EXPECT_TRUE( e1 == e2 );
		EXPECT_TRUE( e1 == e1 );
		EXPECT_FALSE( e1 == ParsingErrors{} );
		EXPECT_FALSE( e1 == e4 );

		EXPECT_NE( e1, e3 );
		EXPECT_FALSE( e1 == e3 );
		EXPECT_TRUE( e4 == ParsingErrors::Empty );
		EXPECT_EQ( e4, ParsingErrors::Empty );
		EXPECT_TRUE( e4.equals( ParsingErrors::Empty ) );

		ParsingErrors empty = ParsingErrors::Empty;
		EXPECT_TRUE( e4.equals( empty ) );
	}

	//----------------------------------------------
	// Enumerator
	//----------------------------------------------

	TEST( ParsingErrorsTests, Enumerator )
	{
		LocalIdParsingErrorBuilder builder1;
		builder1.addError( LocalIdParsingState::NamingRule, "M1" );
		ParsingErrors e1 = builder1.build();

		LocalIdParsingErrorBuilder builder2;
		builder2.addError( LocalIdParsingState::NamingRule, "M1" );
		builder2.addError( LocalIdParsingState::VisVersion, "M1" );
		ParsingErrors e2 = builder2.build();

		ParsingErrors e3 = ParsingErrors::Empty;

		EXPECT_EQ( 1, e1.count() );
		EXPECT_EQ( 2, e2.count() );
		EXPECT_EQ( 0, e3.count() );
	}

	//=====================================================================
	// LocalIdTests
	//=====================================================================

	struct Input
	{
		std::string primaryItem;
		std::string secondaryItem = "";
		std::string quantity = "";
		std::string content = "";
		std::string position = "";
		VisVersion visVersion = VisVersion::v3_4a;
		bool verbose = false;
	};

	class LocalIdValidTest : public ::testing::TestWithParam<std::pair<Input, std::string>>
	{
	public:
		static std::vector<std::pair<Input, std::string>> testData()
		{
			return {
				{ Input{ "411.1/C101.31-2" }, "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta" },
				{ Input{ "411.1/C101.31-2", "", "temperature", "exhaust.gas", "inlet" }, "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" },
				{ Input{ "411.1/C101.63/S206", "", "temperature", "exhaust.gas", "inlet", VisVersion::v3_4a, true }, "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" },
				{ Input{ "411.1/C101.63/S206", "411.1/C101.31-5", "temperature", "exhaust.gas", "inlet", VisVersion::v3_4a, true }, "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet" },
				{ Input{ "511.11/C101.67/S208", "", "pressure", "starting.air", "inlet", VisVersion::v3_6a, true }, "/dnv-v2/vis-3-6a/511.11/C101.67/S208/~main.generator.engine/~starting.system.pneumatic/meta/qty-pressure/cnt-starting.air/pos-inlet" } };
		}
	};

	//----------------------------------------------
	// Test_LocalId_Build_Valid
	//----------------------------------------------

	TEST_P( LocalIdValidTest, Test_LocalId_Build_Valid )
	{
		const auto& [input, expectedOutput] = GetParam();

		VIS& vis = VIS::instance();

		auto visVersion = input.visVersion;
		const auto& gmod = vis.gmod( visVersion );
		const auto& codebooks = vis.codebooks( visVersion );

		auto primaryItem = gmod.parsePath( input.primaryItem );

		std::optional<GmodPath> secondaryItem;
		if ( !input.secondaryItem.empty() )
		{
			secondaryItem = gmod.parsePath( input.secondaryItem );
		}

		auto localId = LocalIdBuilder::create( visVersion )
						   .withPrimaryItem( std::move( primaryItem ) )
						   .tryWithSecondaryItem( std::move( secondaryItem ) )
						   .withVerboseMode( input.verbose )
						   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Quantity, input.quantity ) )
						   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Content, input.content ) )
						   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Position, input.position ) );

		auto localIdStr = localId.toString();

		EXPECT_EQ( expectedOutput, localIdStr );
	}

	INSTANTIATE_TEST_SUITE_P(
		ValidCases,
		LocalIdValidTest,
		::testing::ValuesIn( LocalIdValidTest::testData() ) );

	//----------------------------------------------
	// Test_LocalId_Build_AllWithout
	//----------------------------------------------

	TEST( LocalIdValidTest, Test_LocalId_Build_AllWithout )
	{
		VIS& vis = VIS::instance();

		auto visVersion = VisVersion::v3_4a;
		const auto& gmod = vis.gmod( visVersion );
		const auto& codebooks = vis.codebooks( visVersion );

		auto primaryItem = gmod.parsePath( "411.1/C101.31-2" );
		auto secondaryItem = gmod.parsePath( "411.1/C101.31-5" );

		auto localId = LocalIdBuilder::create( visVersion )
						   .withPrimaryItem( std::move( primaryItem ) )
						   .tryWithSecondaryItem( std::move( secondaryItem ) )
						   .withVerboseMode( true )
						   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Quantity, "quantity" ) )
						   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Content, "content" ) )
						   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Position, "position" ) )
						   .tryWithMetadataTag( codebooks.createTag( CodebookName::State, "state" ) )
						   .tryWithMetadataTag( codebooks.createTag( CodebookName::Content, "content" ) )
						   .tryWithMetadataTag( codebooks.createTag( CodebookName::Calculation, "calculate" ) );

		EXPECT_TRUE( localId.isValid() );

		auto allWithout = localId
							  .withoutPrimaryItem()
							  .withoutSecondaryItem()
							  .withoutQuantity()
							  .withoutPosition()
							  .withoutState()
							  .withoutContent()
							  .withoutCalculation();

		EXPECT_TRUE( allWithout.isEmpty() );
	}

	//----------------------------------------------
	// Test_LocalId_Equality
	//----------------------------------------------

	TEST_P( LocalIdValidTest, Test_LocalId_Equality )
	{
		const auto& [input, _] = GetParam();

		VIS& vis = VIS::instance();
		auto visVersion = VisVersion::v3_4a;
		const auto& gmod = vis.gmod( visVersion );
		const auto& codebooks = vis.codebooks( visVersion );

		auto primaryItem = gmod.parsePath( input.primaryItem );
		std::optional<GmodPath> secondaryItem;
		if ( !input.secondaryItem.empty() )
		{
			secondaryItem = gmod.parsePath( input.secondaryItem );
		}

		auto localId = LocalIdBuilder::create( visVersion )
						   .withPrimaryItem( std::move( primaryItem ) )
						   .tryWithSecondaryItem( std::move( secondaryItem ) )
						   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Quantity, input.quantity ) )
						   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Content, input.content ) )
						   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Position, input.position ) );

		auto otherLocalId = std::move( localId );

		auto localId1 = LocalIdBuilder::create( visVersion )
							.withPrimaryItem( gmod.parsePath( input.primaryItem ) )
							.tryWithSecondaryItem( !input.secondaryItem.empty() ? std::make_optional( gmod.parsePath( input.secondaryItem ) ) : std::nullopt )
							.tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Quantity, input.quantity ) )
							.tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Content, input.content ) )
							.tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Position, input.position ) );

		auto localId2 = LocalIdBuilder::create( visVersion )
							.withPrimaryItem( gmod.parsePath( input.primaryItem ) )
							.tryWithSecondaryItem( !input.secondaryItem.empty() ? std::make_optional( gmod.parsePath( input.secondaryItem ) ) : std::nullopt )
							.tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Quantity, input.quantity ) )
							.tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Content, input.content ) )
							.tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Position, input.position ) );

		EXPECT_EQ( localId1, localId2 );
		EXPECT_TRUE( localId1.equals( localId2 ) );

		auto modifiedLocalId = LocalIdBuilder::create( visVersion )
								   .withPrimaryItem( gmod.parsePath( input.primaryItem ) )
								   .tryWithSecondaryItem( !input.secondaryItem.empty() ? std::make_optional( gmod.parsePath( input.secondaryItem ) ) : std::nullopt )
								   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Quantity, input.quantity ) )
								   .tryWithMetadataTag( codebooks.tryCreateTag( CodebookName::Content, input.content ) )
								   .tryWithMetadataTag( codebooks.createTag( CodebookName::Position, "eqtestvalue" ) );

		EXPECT_NE( localId1, modifiedLocalId );
		EXPECT_FALSE( localId1.equals( modifiedLocalId ) );
	}

	//----------------------------------------------
	// Test_Parsing
	//----------------------------------------------

	class LocalIdParsingTest : public ::testing::TestWithParam<std::string>
	{
	public:
		static std::vector<std::string> testData()
		{
			return {
				"/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking",
				"/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
				"/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
				"/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
				"/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
				"/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
				"/dnv-v2/vis-3-4a/511.11-21O/C101.67/S208/meta/qty-pressure/cnt-air/state-low" };
		}
	};

	TEST_P( LocalIdParsingTest, Test_Parsing )
	{
		const std::string& localIdStr = GetParam();

		std::optional<LocalIdBuilder> localId;
		bool parsed = LocalIdBuilder::tryParse( localIdStr, localId );

		EXPECT_TRUE( parsed );
		ASSERT_TRUE( localId.has_value() );
		EXPECT_EQ( localIdStr, localId->toString() );
	}

	INSTANTIATE_TEST_SUITE_P(
		ParsingCases,
		LocalIdParsingTest,
		::testing::ValuesIn( LocalIdParsingTest::testData() ) );

	//----------------------------------------------
	// SmokeTest_Parsing
	//----------------------------------------------

	TEST( LocalIdTests, SmokeTest_Parsing )
	{
		std::ifstream file( "testdata/LocalIds.txt" );
		if ( !file.is_open() )
		{
			FAIL() << "Failed to open testdata/LocalIds.txt";
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
		while ( std::getline( file, localIdStr ) )
		{
			try
			{
				if ( localIdStr.find( "qty-content" ) != std::string::npos )
					continue;

				ParsingErrors errorBuilder;
				std::optional<LocalIdBuilder> localIdOpt;
				bool parsed = LocalIdBuilder::tryParse( localIdStr, errorBuilder, localIdOpt );

				if ( !parsed )
				{
					errored.push_back( ErrorInfo{ localIdStr, std::move( localIdOpt ), std::nullopt, std::move( errorBuilder ) } );
				}
				else if ( localIdOpt.has_value() && ( localIdOpt->isEmpty() || !localIdOpt->isValid() ) )
				{
					errored.push_back( ErrorInfo{ localIdStr, std::move( localIdOpt ), std::nullopt, std::move( errorBuilder ) } );
				}
			}
			catch ( const std::exception& ex )
			{
				if ( std::string( ex.what() ).find( "location" ) != std::string::npos )
					continue;
				errored.push_back( ErrorInfo{ localIdStr, std::nullopt, std::string( ex.what() ), ParsingErrors::Empty } );
			}
		}

		file.close();

		bool hasParsingErrors = false;
		for ( const auto& errorInfo : errored )
		{
			if ( errorInfo.Errors.hasErrors() )
			{
				hasParsingErrors = true;
				break;
			}
		}

		if ( hasParsingErrors )
		{
			std::cout << "";
		}

		std::vector<ParsingErrors> allErrors;
		for ( const auto& errorInfo : errored )
		{
			if ( errorInfo.Errors.hasErrors() )
			{
				allErrors.push_back( errorInfo.Errors );
			}
		}

		EXPECT_TRUE( allErrors.empty() );
		EXPECT_TRUE( errored.empty() );
	}

	//----------------------------------------------
	// Test_Parsing_Validation
	//----------------------------------------------

	class LocalIdValidationTest : public ::testing::TestWithParam<std::pair<std::string, std::vector<std::string>>>
	{
	};

	static std::vector<std::pair<std::string, std::vector<std::string>>> invalidLocalIdsData()
	{
		std::vector<std::pair<std::string, std::vector<std::string>>> data;
		const nlohmann::json& jsonDataFromFile = loadTestData( INVALID_LOCAL_IDS_TEST_DATA_PATH );

		if ( jsonDataFromFile.contains( "InvalidLocalIds" ) && jsonDataFromFile["InvalidLocalIds"].is_array() )
		{
			for ( const auto& item : jsonDataFromFile["InvalidLocalIds"] )
			{
				if ( item.contains( "input" ) && item.contains( "expectedErrorMessages" ) &&
					 item["input"].is_string() && item["expectedErrorMessages"].is_array() )
				{
					std::string input = item["input"].get<std::string>();
					std::vector<std::string> expectedMessages;

					for ( const auto& msg : item["expectedErrorMessages"] )
					{
						if ( msg.is_string() )
						{
							expectedMessages.push_back( msg.get<std::string>() );
						}
					}

					data.push_back( { input, expectedMessages } );
				}
			}
		}

		return data;
	}

	TEST_P( LocalIdValidationTest, Test_Parsing_Validation )
	{
		const auto& [localIdStr, expectedErrorMessages] = GetParam();

		ParsingErrors errorBuilder;
		std::optional<LocalIdBuilder> localIdOpt;
		bool parsed = LocalIdBuilder::tryParse( localIdStr, errorBuilder, localIdOpt );

		std::vector<std::string> actualErrorMessages;
		auto enumerator = errorBuilder.enumerator();
		while ( enumerator.next() )
		{
			const auto& [type, message] = enumerator.current();
			actualErrorMessages.push_back( message );
		}

		EXPECT_EQ( expectedErrorMessages, actualErrorMessages );
		EXPECT_FALSE( parsed );
	}

	INSTANTIATE_TEST_SUITE_P(
		ValidationCases,
		LocalIdValidationTest,
		::testing::ValuesIn( invalidLocalIdsData() ) );
}
