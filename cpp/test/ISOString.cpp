/**
 * @file ISOStringTests.cpp
 * @brief Unit tests for ISO string validation utilities.
 */

#include "pch.h"

#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	struct SmokeContext
	{
		int count = 0;
		int succeeded = 0;
		std::vector<std::pair<std::string, std::optional<std::string>>> errors;
	};

	const std::string AllAllowedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";

	TEST( IsISOStringTests, AllValidCharacters )
	{
		for ( const char ch : AllAllowedCharacters )
		{
			EXPECT_TRUE( VIS::isISOString( std::string( 1, ch ) ) )
				<< "Character: " << ch;
		}
	}

	TEST( IsISOStringTests, AllAllowedInOne )
	{
		EXPECT_TRUE( VIS::isISOString( AllAllowedCharacters ) );
	}

	TEST( IsISOStringTests, SpotTests )
	{
		struct TestCase
		{
			std::string input;
			bool expectedResult;
		};

		std::vector<TestCase> testCases = {
			{ "test", true },
			{ "TeST", true },
			{ "with space", false },
			{ "#%/*", false },
		};

		for ( const auto& testCase : testCases )
		{
			EXPECT_EQ( VIS::isISOString( testCase.input ), testCase.expectedResult ) << "Input: " << testCase.input;
		}
	}

	TEST( IsISOStringTests, SmokeTest_Parsing )
	{
		std::ifstream file( "testdata/LocalIds.txt" );
		ASSERT_TRUE( file.is_open() ) << "Failed to open testdata/LocalIds.txt";

		SmokeContext context;

		std::string localIdStr;
		while ( std::getline( file, localIdStr ) )
		{
			try
			{
				bool match = VIS::matchISOLocalIdString( localIdStr );
				if ( !match )
				{
					context.errors.push_back( { localIdStr, std::nullopt } );
				}
				else
				{
					context.succeeded++;
				}
			}
			catch ( const std::exception& ex )
			{
				context.errors.push_back( { localIdStr, ex.what() } );
			}
			context.count++;
		}

		if ( !context.errors.empty() )
		{
			for ( [[maybe_unused]] const auto& [localId, error] : context.errors )
			{
				SPDLOG_ERROR( "Failed to parse {} with error {}", localId, error.has_value() ? *error : "Not a match" );
			}
		}

		EXPECT_TRUE( context.errors.empty() ) << "Found " << context.errors.size() << " errors";
		EXPECT_EQ( context.count, context.succeeded );
	}
}
