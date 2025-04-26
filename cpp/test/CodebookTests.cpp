#include "pch.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/Codebook.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	namespace TestData
	{
		static std::vector<std::pair<std::string, std::string>> getPositionValidationTestData()
		{
			return { { "upper", "Valid" }, { "lower", "Valid" } };
		}

		static std::vector<std::pair<std::string, std::string>> getPositionStandardValuesTestData()
		{
			return { { "invalidValue", "upper" } };
		}

		struct StatesTestData
		{
			std::string invalidGroup;
			std::string validValue;
			std::string validGroup;
			std::string secondValidValue;
		};

		static std::vector<StatesTestData> getStatesTestData()
		{
			return { { "NonExistentGroup", "on", "On / off", "off" } };
		}

		struct TagTestData
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

		static std::vector<TagTestData> getTagTestData()
		{
			return { { "upper", "lower", "port", '-', "~customTag", '~', "##invalid1", "##invalid2" } };
		}

		struct DetailTagTestData
		{
			std::string validCustomTag;
			std::string firstInvalidCustomTag;
			std::string secondInvalidCustomTag;
		};

		static std::vector<DetailTagTestData> getDetailTagTestData()
		{
			return { { "validCustomTag", "#invalidTag1", "@invalidTag2" } };
		}
	}

	class CodebookTest : public ::testing::Test
	{
	protected:
		VIS& Vis = VIS::instance();
		Codebooks Codebooks;

		virtual void SetUp() override
		{
			Codebooks = Vis.codebooks( VisVersion::v3_4a );
		}
	};

	TEST_F( CodebookTest, Test_Position_Validation )
	{
		auto testCases = TestData::getPositionValidationTestData();
		auto codebookType = Codebooks[CodebookName::Position];

		for ( const auto& [input, expectedOutput] : testCases )
		{
			auto validPosition = codebookType.validatePosition( input );
			auto parsedExpectedOutput = PositionValidationResults::fromString( expectedOutput );

			EXPECT_EQ( parsedExpectedOutput, validPosition ) << "Failed for position: " << input;
		}
	}

	TEST_F( CodebookTest, Test_Positions )
	{
		auto testCases = TestData::getPositionStandardValuesTestData();

		for ( const auto& [invalidStandardValue, validStandardValue] : testCases )
		{
			auto positions = Codebooks[CodebookName::Position];

			EXPECT_FALSE( positions.hasStandardValue( invalidStandardValue ) );
			EXPECT_TRUE( positions.hasStandardValue( validStandardValue ) );
		}
	}

	TEST_F( CodebookTest, Test_Standard_Values )
	{
		auto positions = Codebooks[CodebookName::Position];

		EXPECT_TRUE( positions.hasStandardValue( "upper" ) );

		const auto& rawData = positions.rawData();
		EXPECT_TRUE( rawData.find( "Vertical" ) != rawData.end() );
		EXPECT_NE( std::find( rawData.at( "Vertical" ).begin(), rawData.at( "Vertical" ).end(), "upper" ), rawData.at( "Vertical" ).end() );
	}

	TEST_F( CodebookTest, Test_States )
	{
		auto testCases = TestData::getStatesTestData();

		for ( const auto& data : testCases )
		{
			const auto& states = Codebooks[CodebookName::State];

			EXPECT_FALSE( states.hasGroup( data.invalidGroup ) );
			EXPECT_TRUE( states.hasStandardValue( data.validValue ) );
			EXPECT_TRUE( states.hasGroup( data.validGroup ) );
			EXPECT_TRUE( states.hasStandardValue( data.secondValidValue ) );
		}
	}

	TEST_F( CodebookTest, Test_Create_Tag )
	{
		auto testCases = TestData::getTagTestData();

		for ( const auto& data : testCases )
		{
			const auto& codebookType = Codebooks[CodebookName::Position];

			auto metadataTag1 = codebookType.createTag( data.firstTag );
			EXPECT_EQ( metadataTag1.value(), data.firstTag );
			EXPECT_FALSE( metadataTag1.isCustom() );

			auto metadataTag2 = codebookType.createTag( data.secondTag );
			EXPECT_EQ( metadataTag2.value(), data.secondTag );
			EXPECT_FALSE( metadataTag2.isCustom() );

			auto metadataTag3 = codebookType.createTag( data.thirdTag );
			EXPECT_EQ( metadataTag3.value(), data.thirdTag );
			EXPECT_FALSE( metadataTag3.isCustom() );
			EXPECT_EQ( metadataTag3.prefix(), data.thirdTagPrefix );

			auto metadataTag4 = codebookType.createTag( data.customTag );
			EXPECT_EQ( metadataTag4.value(), data.customTag );
			EXPECT_TRUE( metadataTag4.isCustom() );
			EXPECT_EQ( metadataTag4.prefix(), data.customTagPrefix );

			EXPECT_THROW( codebookType.createTag( data.firstInvalidTag ), std::invalid_argument );
			EXPECT_EQ( codebookType.tryCreateTag( data.firstInvalidTag ), std::nullopt );

			EXPECT_THROW( codebookType.createTag( data.secondInvalidTag ), std::invalid_argument );
			EXPECT_EQ( codebookType.tryCreateTag( data.secondInvalidTag ), std::nullopt );
		}
	}

	TEST_F( CodebookTest, Test_Get_Groups )
	{
		const auto& groups = Codebooks[CodebookName::Position].groups();
		EXPECT_GT( groups.count(), 1 );

		EXPECT_TRUE( groups.contains( "Vertical" ) );

		const auto& rawData = Codebooks[CodebookName::Position].rawData();

		EXPECT_EQ( groups.count(), rawData.size() - 1 );
		EXPECT_TRUE( rawData.find( "Vertical" ) != rawData.end() );
	}

	TEST_F( CodebookTest, Test_Iterate_Groups )
	{
		const auto& groups = Codebooks[CodebookName::Position].groups();
		int count = 0;

		for ( const auto& group : groups )
		{
			(void)group;
			count++;
		}

		EXPECT_EQ( count, 11 );
	}

	TEST_F( CodebookTest, Test_Iterate_Values )
	{
		const auto& values = Codebooks[CodebookName::Position].standardValues();
		int count = 0;

		for ( const auto& value : values )
		{
			(void)value;
			count++;
		}

		EXPECT_EQ( count, 28 );
	}

	TEST_F( CodebookTest, Test_Detail_Tag )
	{
		auto testCases = TestData::getDetailTagTestData();

		for ( const auto& data : testCases )
		{
			const auto& codebook = Codebooks[CodebookName::Detail];

			EXPECT_TRUE( codebook.tryCreateTag( data.validCustomTag ).has_value() );
			EXPECT_EQ( codebook.tryCreateTag( data.firstInvalidCustomTag ), std::nullopt );
			EXPECT_EQ( codebook.tryCreateTag( data.secondInvalidCustomTag ), std::nullopt );

			EXPECT_THROW( codebook.createTag( data.firstInvalidCustomTag ), std::invalid_argument );
			EXPECT_THROW( codebook.createTag( data.secondInvalidCustomTag ), std::invalid_argument );
		}
	}

	class PositionValidationTest : public ::testing::TestWithParam<std::pair<std::string, std::string>>
	{
	};

	TEST_P( PositionValidationTest, ValidatesPositionsCorrectly )
	{
		auto [input, expectedOutput] = GetParam();

		VIS& vis = VIS::instance();
		auto codebooks = vis.codebooks( VisVersion::v3_4a );
		auto codebookType = codebooks[CodebookName::Position];

		auto validPosition = codebookType.validatePosition( input );
		auto parsedExpectedOutput = PositionValidationResults::fromString( expectedOutput );

		EXPECT_EQ( parsedExpectedOutput, validPosition ) << "Failed for position: " << input;
	}

	INSTANTIATE_TEST_SUITE_P(
		PositionValidation,
		PositionValidationTest,
		::testing::ValuesIn( TestData::getPositionValidationTestData() ) );
}
