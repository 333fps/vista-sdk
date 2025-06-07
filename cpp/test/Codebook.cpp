/**
 * @file CodebookTests.cpp
 * @brief Unit tests for the Codebook and related functionalities.
 */

#include "pch.h"

#include "TestDataLoader.h"

#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	namespace
	{
		constexpr const char* TEST_DATA_PATH = "testdata/Codebook.json";
	}

	namespace CodebookTestFixture
	{
		class CodebookTest : public ::testing::Test
		{
		protected:
			CodebookTest() : m_jsonData{ loadTestData( TEST_DATA_PATH ) } {}

			const Codebooks& getCodebooks()
			{
				VIS& vis = VIS::instance();
				return vis.codebooks( VisVersion::v3_4a );
			}

			const nlohmann::json& m_jsonData;
		};

		TEST_F( CodebookTest, Test_Standard_Values )
		{
			const auto& codebooks = getCodebooks();
			const auto& positions = codebooks.codebook( CodebookName::Position );

			EXPECT_TRUE( positions.hasStandardValue( "upper" ) );

			const auto& rawData = positions.rawData();
			EXPECT_TRUE( rawData.find( "Vertical" ) != rawData.end() );

			auto it = rawData.find( "Vertical" );
			ASSERT_NE( it, rawData.end() ) << "Group 'Vertical' not found in raw data.";

			const auto& verticalGroupValues = it->second;
			EXPECT_NE( std::find( verticalGroupValues.begin(), verticalGroupValues.end(), "upper" ), verticalGroupValues.end() );
		}

		TEST_F( CodebookTest, Test_Get_Groups )
		{
			const auto& codebooks = getCodebooks();
			const auto& groups = codebooks.codebook( CodebookName::Position ).groups();
			EXPECT_GT( groups.count(), 1 );

			EXPECT_TRUE( groups.contains( "Vertical" ) );

			const auto& rawData = codebooks.codebook( CodebookName::Position ).rawData();

			EXPECT_EQ( groups.count(), rawData.size() - 1 );
			EXPECT_TRUE( rawData.find( "Vertical" ) != rawData.end() );
		}

		TEST_F( CodebookTest, Test_Iterate_Groups )
		{
			const auto& codebooks = getCodebooks();
			const auto& groups = codebooks.codebook( CodebookName::Position ).groups();
			int iterated_count = 0;

			for ( [[maybe_unused]] const auto& group : groups )
			{
				iterated_count++;
			}
			EXPECT_EQ( iterated_count, 11 );
		}

		TEST_F( CodebookTest, Test_Iterate_Values )
		{
			const auto& codebooks = getCodebooks();
			const auto& positionCodebook = codebooks.codebook( CodebookName::Position );
			const auto& values = positionCodebook.standardValues();
			int iterated_count = 0;
			for ( [[maybe_unused]] const auto& value_item : values )
			{
				iterated_count++;
			}
			EXPECT_EQ( iterated_count, 28 );
		}
	}

	namespace CodebookTestParametrized
	{
		class CodebookBaseTest
		{
		protected:
			const Codebooks& getCodebooks()
			{
				VIS& vis = VIS::instance();
				return vis.codebooks( VisVersion::v3_4a );
			}
		};

		struct PositionValidationParam
		{
			std::string input;
			std::string expectedOutput;
		};

		class PositionValidationTest : public CodebookBaseTest,
									   public ::testing::TestWithParam<PositionValidationParam>
		{
		};

		static std::vector<PositionValidationParam> positionValidationData()
		{
			std::vector<PositionValidationParam> data;
			const nlohmann::json& jsonDataFromFile = loadTestData( TEST_DATA_PATH );

			if ( jsonDataFromFile.contains( "ValidPosition" ) && jsonDataFromFile["ValidPosition"].is_array() )
			{
				for ( const auto& item : jsonDataFromFile["ValidPosition"] )
				{
					if ( item.is_array() && item.size() == 2 && item[0].is_string() && item[1].is_string() )
					{
						data.push_back( { item[0].get<std::string>(), item[1].get<std::string>() } );
					}
				}
			}

			return data;
		}

		PositionValidationResult positionValidationResultsFromString( std::string_view name )
		{
			if ( name == "Valid" )
				return PositionValidationResult::Valid;
			if ( name == "Invalid" )
				return PositionValidationResult::Invalid;
			if ( name == "InvalidOrder" )
				return PositionValidationResult::InvalidOrder;
			if ( name == "InvalidGrouping" )
				return PositionValidationResult::InvalidGrouping;
			if ( name == "Custom" )
				return PositionValidationResult::Custom;

			throw std::invalid_argument( "Unknown position validation result: " + std::string( name ) );
		}

		TEST_P( PositionValidationTest, Test_Position_Validation )
		{
			const auto& param = GetParam();
			const auto& codebooks = getCodebooks();

			const auto& codebookType = codebooks.codebook( CodebookName::Position );
			auto validPosition = codebookType.validatePosition( param.input );
			auto parsedExpectedOutput = positionValidationResultsFromString( param.expectedOutput );

			EXPECT_EQ( parsedExpectedOutput, validPosition );
		}

		INSTANTIATE_TEST_SUITE_P(
			CodebookPositionValidationSuite,
			PositionValidationTest,
			::testing::ValuesIn( positionValidationData() ) );

		struct PositionsParam
		{
			std::string invalidStandardValue;
			std::string validStandardValue;
		};

		class PositionsTest : public CodebookBaseTest,
							  public ::testing::TestWithParam<PositionsParam>
		{
		};

		static std::vector<PositionsParam> positionsData()
		{
			std::vector<PositionsParam> data;
			const nlohmann::json& jsonDataFromFile = loadTestData( TEST_DATA_PATH );

			if ( jsonDataFromFile.contains( "Positions" ) && jsonDataFromFile["Positions"].is_array() )
			{
				for ( const auto& item : jsonDataFromFile["Positions"] )
				{
					if ( item.is_array() && item.size() == 2 && item[0].is_string() && item[1].is_string() )
					{
						data.push_back( { item[0].get<std::string>(), item[1].get<std::string>() } );
					}
				}
			}
			return data;
		}

		TEST_P( PositionsTest, Test_Positions )
		{
			const auto& param = GetParam();
			const auto& codebooks = getCodebooks();

			const auto& positions = codebooks.codebook( CodebookName::Position );

			EXPECT_FALSE( positions.hasStandardValue( param.invalidStandardValue ) );
			EXPECT_TRUE( positions.hasStandardValue( param.validStandardValue ) );
		}

		INSTANTIATE_TEST_SUITE_P(
			CodebookPositionsSuite,
			PositionsTest,
			::testing::ValuesIn( positionsData() ) );

		struct StatesParam
		{
			std::string invalidGroup;
			std::string validValue;
			std::string validGroup;
			std::string secondValidValue;
		};

		class StatesTest : public CodebookBaseTest,
						   public ::testing::TestWithParam<StatesParam>
		{
		};

		static std::vector<StatesParam> statesData()
		{
			std::vector<StatesParam> data;
			const nlohmann::json& jsonDataFromFile = loadTestData( TEST_DATA_PATH );

			if ( jsonDataFromFile.contains( "States" ) && jsonDataFromFile["States"].is_array() )
			{
				for ( const auto& item : jsonDataFromFile["States"] )
				{
					if ( item.is_array() && item.size() == 4 &&
						 item[0].is_string() && item[1].is_string() &&
						 item[2].is_string() && item[3].is_string() )
					{
						data.push_back( { item[0].get<std::string>(),
							item[1].get<std::string>(),
							item[2].get<std::string>(),
							item[3].get<std::string>() } );
					}
				}
			}
			return data;
		}

		TEST_P( StatesTest, Test_States )
		{
			const auto& param = GetParam();
			const auto& codebooks = getCodebooks();

			const auto& states = codebooks.codebook( CodebookName::State );

			EXPECT_FALSE( states.hasGroup( param.invalidGroup ) );
			EXPECT_TRUE( states.hasStandardValue( param.validValue ) );
			EXPECT_TRUE( states.hasGroup( param.validGroup ) );
			EXPECT_TRUE( states.hasStandardValue( param.secondValidValue ) );
		}

		INSTANTIATE_TEST_SUITE_P(
			CodebookStatesSuite,
			StatesTest,
			::testing::ValuesIn( statesData() ) );

		struct TagParam
		{
			std::string firstTag;
			std::string secondTag;
			std::string thirdTag;
			char thirdTagPrefix;
			std::string customTag;
			char customTagPrefix;
			std::string firstInvalidTag;
			std::string secondInvalidTag;
		};

		class TagTest : public CodebookBaseTest,
						public ::testing::TestWithParam<TagParam>
		{
		};

		static std::vector<TagParam> tagData()
		{
			std::vector<TagParam> data;
			const nlohmann::json& jsonDataFromFile = loadTestData( TEST_DATA_PATH );

			if ( jsonDataFromFile.contains( "Tag" ) && jsonDataFromFile["Tag"].is_array() )
			{
				for ( const auto& item : jsonDataFromFile["Tag"] )
				{
					if ( item.is_array() && item.size() == 8 &&
						 item[0].is_string() && item[1].is_string() && item[2].is_string() &&
						 item[3].is_string() && !item[3].get<std::string>().empty() &&
						 item[4].is_string() &&
						 item[5].is_string() && !item[5].get<std::string>().empty() &&
						 item[6].is_string() && item[7].is_string() )
					{
						data.push_back( { item[0].get<std::string>(),
							item[1].get<std::string>(),
							item[2].get<std::string>(),
							item[3].get<std::string>()[0],
							item[4].get<std::string>(),
							item[5].get<std::string>()[0],
							item[6].get<std::string>(),
							item[7].get<std::string>() } );
					}
				}
			}
			return data;
		}

		TEST_P( TagTest, Test_Create_Tag )
		{
			const auto& param = GetParam();
			const auto& codebooks = getCodebooks();

			const auto& codebookType = codebooks.codebook( CodebookName::Position );

			auto metadataTag1 = codebookType.createTag( param.firstTag );
			EXPECT_EQ( param.firstTag, metadataTag1.value() );
			EXPECT_FALSE( metadataTag1.isCustom() );

			auto metadataTag2 = codebookType.createTag( param.secondTag );
			EXPECT_EQ( param.secondTag, metadataTag2.value() );
			EXPECT_FALSE( metadataTag2.isCustom() );

			auto metadataTag3 = codebookType.createTag( param.thirdTag );
			EXPECT_EQ( param.thirdTag, metadataTag3.value() );
			EXPECT_FALSE( metadataTag3.isCustom() );
			EXPECT_EQ( param.thirdTagPrefix, metadataTag3.prefix() );

			auto metadataTag4 = codebookType.createTag( param.customTag );
			EXPECT_EQ( param.customTag, metadataTag4.value() );
			EXPECT_TRUE( metadataTag4.isCustom() );
			EXPECT_EQ( param.customTagPrefix, metadataTag4.prefix() );

			EXPECT_THROW( codebookType.createTag( param.firstInvalidTag ), std::invalid_argument );
			EXPECT_EQ( codebookType.tryCreateTag( param.firstInvalidTag ), std::nullopt );

			EXPECT_THROW( codebookType.createTag( param.secondInvalidTag ), std::invalid_argument );
			EXPECT_EQ( codebookType.tryCreateTag( param.secondInvalidTag ), std::nullopt );
		}

		INSTANTIATE_TEST_SUITE_P(
			CodebookTagSuite,
			TagTest,
			::testing::ValuesIn( tagData() ) );

		struct DetailTagParam
		{
			std::string validCustomTag;
			std::string firstInvalidCustomTag;
			std::string secondInvalidCustomTag;
		};

		class DetailTagTest : public CodebookBaseTest,
							  public ::testing::TestWithParam<DetailTagParam>
		{
		};

		static std::vector<DetailTagParam> detailTagData()
		{
			std::vector<DetailTagParam> data;
			const nlohmann::json& jsonDataFromFile = loadTestData( TEST_DATA_PATH );

			if ( jsonDataFromFile.contains( "DetailTag" ) && jsonDataFromFile["DetailTag"].is_array() )
			{
				for ( const auto& item : jsonDataFromFile["DetailTag"] )
				{
					if ( item.is_array() && item.size() == 3 &&
						 item[0].is_string() && item[1].is_string() && item[2].is_string() )
					{
						data.push_back( { item[0].get<std::string>(),
							item[1].get<std::string>(),
							item[2].get<std::string>() } );
					}
				}
			}
			return data;
		}

		TEST_P( DetailTagTest, Test_Detail_Tag )
		{
			const auto& param = GetParam();
			const auto& codebooks = getCodebooks();

			const auto& codebook = codebooks.codebook( CodebookName::Detail );

			EXPECT_NE( codebook.tryCreateTag( param.validCustomTag ), std::nullopt );
			EXPECT_EQ( codebook.tryCreateTag( param.firstInvalidCustomTag ), std::nullopt );
			EXPECT_EQ( codebook.tryCreateTag( param.secondInvalidCustomTag ), std::nullopt );

			EXPECT_THROW( codebook.createTag( param.firstInvalidCustomTag ), std::invalid_argument );
			EXPECT_THROW( codebook.createTag( param.secondInvalidCustomTag ), std::invalid_argument );
		}

		INSTANTIATE_TEST_SUITE_P(
			CodebookDetailTagSuite,
			DetailTagTest,
			::testing::ValuesIn( detailTagData() ) );
	}

	namespace CodebooksTests
	{
		TEST( CodebooksTests, Test_CodebookName_Prefix_Conversions )
		{
			const std::map<dnv::vista::sdk::CodebookName, std::string_view> expectedMappings = {
				{ dnv::vista::sdk::CodebookName::Quantity, "qty" },
				{ dnv::vista::sdk::CodebookName::Content, "cnt" },
				{ dnv::vista::sdk::CodebookName::Calculation, "calc" },
				{ dnv::vista::sdk::CodebookName::State, "state" },
				{ dnv::vista::sdk::CodebookName::Command, "cmd" },
				{ dnv::vista::sdk::CodebookName::Type, "type" },
				{ dnv::vista::sdk::CodebookName::FunctionalServices, "funct.svc" },
				{ dnv::vista::sdk::CodebookName::MaintenanceCategory, "maint.cat" },
				{ dnv::vista::sdk::CodebookName::ActivityType, "act.type" },
				{ dnv::vista::sdk::CodebookName::Position, "pos" },
				{ dnv::vista::sdk::CodebookName::Detail, "detail" } };

			for ( const auto& pair : expectedMappings )
			{
				const auto cbName = pair.first;
				const auto expectedPrefix = pair.second;

				SCOPED_TRACE( "Testing CodebookName toPrefix: " + std::string( expectedPrefix ) );
				std::string_view actualPrefix;
				ASSERT_NO_THROW( {
					actualPrefix = dnv::vista::sdk::CodebookNames::toPrefix( cbName );
				} );
				ASSERT_EQ( expectedPrefix, actualPrefix );

				SCOPED_TRACE( "Testing fromPrefix round trip for: " + std::string( expectedPrefix ) );
				dnv::vista::sdk::CodebookName roundTripName;
				ASSERT_NO_THROW( {
					roundTripName = dnv::vista::sdk::CodebookNames::fromPrefix( actualPrefix );
				} );
				ASSERT_EQ( cbName, roundTripName );
			}

			ASSERT_THROW( dnv::vista::sdk::CodebookNames::fromPrefix( "" ), std::invalid_argument );
			ASSERT_THROW( dnv::vista::sdk::CodebookNames::fromPrefix( "invalid_prefix" ), std::invalid_argument );
			ASSERT_THROW( dnv::vista::sdk::CodebookNames::fromPrefix( "po" ), std::invalid_argument );

			ASSERT_THROW( dnv::vista::sdk::CodebookNames::fromPrefix( "QTY" ), std::invalid_argument );
			ASSERT_THROW( dnv::vista::sdk::CodebookNames::fromPrefix( "Pos" ), std::invalid_argument );
			ASSERT_THROW( dnv::vista::sdk::CodebookNames::fromPrefix( "funct.SVC" ), std::invalid_argument );

			const auto invalidCbName = static_cast<dnv::vista::sdk::CodebookName>( 999 );
			ASSERT_THROW( (void)dnv::vista::sdk::CodebookNames::toPrefix( invalidCbName ), std::invalid_argument );
		}
	}
}
