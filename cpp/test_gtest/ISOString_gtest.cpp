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

	TEST( ISOStringTests, AllValidCharacters )
	{
		const std::string allAllowedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";

		for ( const char ch : allAllowedCharacters )
		{
			EXPECT_TRUE( VIS::isISOString( std::string( 1, ch ) ) ) << "Character: " << ch;
		}
	}

	TEST( ISOStringTests, SpotTests )
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
			EXPECT_EQ( VIS::isISOString( testCase.input ), testCase.expectedResult )
				<< "Input: " << testCase.input;
		}
	}

	TEST( ISOStringTests, SmokeTest )
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
				bool match = VIS::isISOLocalIdString( localIdStr );
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

		EXPECT_EQ( errored.size(), 0 ) << "Errored LocalIds: " << errored.size();
		EXPECT_EQ( count, succeeded );
	}
}
