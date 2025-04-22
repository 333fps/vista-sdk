#include "pch.h"

#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	std::vector<std::string> ReadLinesFromFile( const std::string& filePath )
	{
		std::vector<std::string> lines;
		std::ifstream file( filePath );
		if ( !file.is_open() )
		{
			SPDLOG_ERROR( "Failed to open file: {}", filePath );
			throw std::runtime_error( "Failed to open file: " + filePath );
		}

		std::string line;
		while ( std::getline( file, line ) )
		{
			lines.push_back( line );
		}
		return lines;
	}

	TEST_CASE( "All valid characters are accepted", "[ISOString]" )
	{
		const std::string allAllowedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";

		for ( const char ch : allAllowedCharacters )
		{
			INFO( "Character: " << ch );
			CHECK( VIS::IsISOString( std::string( 1, ch ) ) );
		}
	}

	TEST_CASE( "Spot tests for ISO string validation", "[ISOString]" )
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
			INFO( "Input: " << testCase.input );
			CHECK( VIS::IsISOString( testCase.input ) == testCase.expectedResult );
		}
	}

	TEST_CASE( "Smoke test with real-world local IDs", "[ISOString]" )
	{
		const std::string filePath = "testdata/LocalIds.txt";
		std::vector<std::string> lines = ReadLinesFromFile( filePath );

		int count = 0;
		int succeeded = 0;
		std::vector<std::string> errored;

		for ( const auto& localIdStr : lines )
		{
			try
			{
				bool match = VIS::IsISOLocalIdString( localIdStr );
				if ( !match )
				{
					errored.push_back( localIdStr );
				}
				else
				{
					succeeded++;
				}
			}
			catch ( ... )
			{
				errored.push_back( localIdStr );
			}
			count++;
		}

		INFO( "Errored LocalIds: " << errored.size() );
		CHECK( errored.size() == 0 );
		CHECK( count == succeeded );
	}
}
