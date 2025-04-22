#include "pch.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/Codebook.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	class CodebookTest
	{
	protected:
		VIS& vis = VIS::Instance();
		Codebooks codebooks;

	public:
		CodebookTest()
		{
			codebooks = vis.GetCodebooks( VisVersion::v3_4a );
		}
	};

	TEST_CASE_METHOD( CodebookTest, "Test Position Validation", "[codebook]" )
	{
		std::vector<std::pair<std::string, std::string>> testCases = {
			{ "upper", "Valid" },
			{ "lower", "Valid" } };

		auto codebookType = codebooks[CodebookName::Position];

		for ( const auto& [input, expectedOutput] : testCases )
		{
			INFO( "Testing position: " << input );
			auto validPosition = codebookType.ValidatePosition( input );
			auto parsedExpectedOutput = PositionValidationResults::FromString( expectedOutput );

			CHECK( parsedExpectedOutput == validPosition );
		}
	}

	TEST_CASE_METHOD( CodebookTest, "Test Positions", "[codebook]" )
	{
		std::vector<std::pair<std::string, std::string>> testCases = {
			{ "invalidValue", "upper" } };

		for ( const auto& [invalidStandardValue, validStandardValue] : testCases )
		{
			auto positions = codebooks[CodebookName::Position];

			CHECK_FALSE( positions.HasStandardValue( invalidStandardValue ) );
			CHECK( positions.HasStandardValue( validStandardValue ) );
		}
	}

	TEST_CASE_METHOD( CodebookTest, "Test Standard Values", "[codebook]" )
	{
		auto positions = codebooks[CodebookName::Position];

		CHECK( positions.HasStandardValue( "upper" ) );
		const auto& rawData = positions.GetRawData();
		CHECK( rawData.find( "Vertical" ) != rawData.end() );
		CHECK( std::find( rawData.at( "Vertical" ).begin(), rawData.at( "Vertical" ).end(), "upper" ) != rawData.at( "Vertical" ).end() );
	}

	TEST_CASE_METHOD( CodebookTest, "Test States", "[codebook]" )
	{
		struct StatesTestData
		{
			std::string invalidGroup;
			std::string validValue;
			std::string validGroup;
			std::string secondValidValue;
		};

		std::vector<StatesTestData> testCases = {
			{ "NonExistentGroup", "on", "On / off", "off" } };

		for ( const auto& data : testCases )
		{
			const auto& states = codebooks[CodebookName::State];

			CHECK_FALSE( states.HasGroup( data.invalidGroup ) );
			CHECK( states.HasStandardValue( data.validValue ) );
			CHECK( states.HasGroup( data.validGroup ) );
			CHECK( states.HasStandardValue( data.secondValidValue ) );
		}
	}

	TEST_CASE_METHOD( CodebookTest, "Test Create Tag", "[codebook]" )
	{
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

		std::vector<TagTestData> testCases = {
			{ "upper", "lower", "port", '-', "~customTag", '~', "##invalid1", "##invalid2" } };

		for ( const auto& data : testCases )
		{
			const auto& codebookType = codebooks[CodebookName::Position];

			auto metadataTag1 = codebookType.CreateTag( data.firstTag );
			CHECK( metadataTag1.GetValue() == data.firstTag );
			CHECK_FALSE( metadataTag1.IsCustom() );

			auto metadataTag2 = codebookType.CreateTag( data.secondTag );
			CHECK( metadataTag2.GetValue() == data.secondTag );
			CHECK_FALSE( metadataTag2.IsCustom() );

			auto metadataTag3 = codebookType.CreateTag( data.thirdTag );
			CHECK( metadataTag3.GetValue() == data.thirdTag );
			CHECK_FALSE( metadataTag3.IsCustom() );
			CHECK( metadataTag3.GetPrefix() == data.thirdTagPrefix );

			auto metadataTag4 = codebookType.CreateTag( data.customTag );
			CHECK( metadataTag4.GetValue() == data.customTag );
			CHECK( metadataTag4.IsCustom() );
			CHECK( metadataTag4.GetPrefix() == data.customTagPrefix );

			CHECK_THROWS_AS( codebookType.CreateTag( data.firstInvalidTag ), std::invalid_argument );
			CHECK( codebookType.TryCreateTag( data.firstInvalidTag ) == std::nullopt );

			CHECK_THROWS_AS( codebookType.CreateTag( data.secondInvalidTag ), std::invalid_argument );
			CHECK( codebookType.TryCreateTag( data.secondInvalidTag ) == std::nullopt );
		}
	}

	TEST_CASE_METHOD( CodebookTest, "Test Get Groups", "[codebook]" )
	{
		const auto& groups = codebooks[CodebookName::Position].GetGroups();
		CHECK( groups.Count() > 1 );

		CHECK( groups.Contains( "Vertical" ) );
		const auto& rawData = codebooks[CodebookName::Position].GetRawData();

		CHECK( groups.Count() == rawData.size() - 1 );
		CHECK( rawData.find( "Vertical" ) != rawData.end() );
	}

	TEST_CASE_METHOD( CodebookTest, "Test Iterate Groups", "[codebook]" )
	{
		const auto& groups = codebooks[CodebookName::Position].GetGroups();
		int count = 0;
		for ( const auto& group : groups )
		{
			(void)group;
			count++;
		}

		CHECK( count == 11 );
	}

	TEST_CASE_METHOD( CodebookTest, "Test Iterate Values", "[codebook]" )
	{
		const auto& values = codebooks[CodebookName::Position].GetStandardValues();
		int count = 0;
		for ( const auto& value : values )
		{
			(void)value;
			count++;
		}

		CHECK( count == 28 );
	}

	TEST_CASE_METHOD( CodebookTest, "Test Detail Tag", "[codebook]" )
	{
		struct DetailTagTestData
		{
			std::string validCustomTag;
			std::string firstInvalidCustomTag;
			std::string secondInvalidCustomTag;
		};

		std::vector<DetailTagTestData> testCases = {
			{ "validCustomTag", "#invalidTag1", "@invalidTag2" } };

		for ( const auto& data : testCases )
		{
			const auto& codebook = codebooks[CodebookName::Detail];

			CHECK( codebook.TryCreateTag( data.validCustomTag ).has_value() );
			CHECK( codebook.TryCreateTag( data.firstInvalidCustomTag ) == std::nullopt );
			CHECK( codebook.TryCreateTag( data.secondInvalidCustomTag ) == std::nullopt );

			CHECK_THROWS_AS( codebook.CreateTag( data.firstInvalidCustomTag ), std::invalid_argument );
			CHECK_THROWS_AS( codebook.CreateTag( data.secondInvalidCustomTag ), std::invalid_argument );
		}
	}
}
