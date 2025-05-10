#include "pch.h"

#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/LocalIdParsingErrorBuilder.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/Gmod.h"

using namespace dnv::vista::sdk;

struct GmodPathTestItem
{
	std::string visVersion;
	std::string path;
};

class GmodPathTest : public ::testing::Test
{
protected:
	GmodPathTest()
		: m_vis( nullptr )
	{
	}

	virtual void SetUp() override
	{
		m_vis = &VIS::instance();

		std::string jsonFilePath = "testdata/GmodPaths.json";
		std::ifstream jsonFile( jsonFilePath );

		if ( !jsonFile.is_open() )
		{
			FAIL() << "Failed to open GmodPaths.json at: " << jsonFilePath
				   << "\nPlease ensure the file exists and the path is correct relative to the test executable.";

			return;
		}

		try
		{
			jsonFile >> m_jsonData;
		}
		catch ( const nlohmann::json::parse_error& e )
		{
			FAIL() << "Failed to parse GmodPaths.json: " << e.what();

			return;
		}
	}

	VIS* m_vis;
	nlohmann::json m_jsonData;
};

//=====================================================================
// TEST_F
//=====================================================================

//----------------------------------------------
// TEST_F Passing :)
//----------------------------------------------

TEST_F( GmodPathTest, Test_GmodPath_Parse )
{
	if ( !m_jsonData.contains( "Valid" ) || !m_jsonData["Valid"].is_array() )
	{
		FAIL() << "GmodPaths.json does not contain a valid 'Valid' array.";
		return;
	}

	ASSERT_FALSE( m_jsonData["Valid"].empty() ) << "No valid test cases found in GmodPaths.json";

	for ( const auto& item : m_jsonData["Valid"] )
	{
		if ( item.is_object() && item.contains( "path" ) && item["path"].is_string() &&
			 item.contains( "visVersion" ) && item["visVersion"].is_string() )
		{
			std::string inputPath = item["path"].get<std::string>();
			std::string visVersionStr = item["visVersion"].get<std::string>();

			SCOPED_TRACE( "Testing valid path from JSON: " + inputPath + " (VIS: " + visVersionStr + ")" );

			VisVersion versionEnum = VisVersionExtensions::parse( visVersionStr );
			ASSERT_NE( VisVersion::Unknown, versionEnum ) << "Failed to parse VIS version: " << visVersionStr;

			const auto& gmod = m_vis->gmod( versionEnum );
			const auto& locations = m_vis->locations( versionEnum );

			GmodPath path;
			bool parsed = GmodPath::tryParse( inputPath, gmod, locations, path );

			ASSERT_TRUE( parsed ) << "Parsing should have succeeded for: " << inputPath;
			EXPECT_FALSE( path.isEmpty() ) << "Path should not be empty after successful parse for: " << inputPath;

			if ( parsed )
			{
				EXPECT_EQ( inputPath, path.toString() ) << "Parsed path's toString() should match the input short path.";
			}
		}
		else
		{
			ADD_FAILURE() << "Skipping invalid item in JSON 'Valid' array: " << item.dump( 4 );
		}
	}
}

/* TEST_F( GmodPathTest, Test_GmodPath_Parse_Invalid )
{
	struct TestCase
	{
		std::string visVersionStr;
		std::string pathStr;
		std::string description;
	};

	std::vector<TestCase> testCases;

	if ( !m_jsonData.contains( "Invalid" ) || !m_jsonData["Invalid"].is_array() )
	{
		FAIL() << "GmodPaths.json does not contain a valid 'Invalid' array.";
		return;
	}

	for ( const auto& item : m_jsonData["Invalid"] )
	{
		if ( item.is_object() && item.contains( "path" ) && item["path"].is_string() &&
			 item.contains( "visVersion" ) && item["visVersion"].is_string() )
		{
			testCases.push_back( { item["visVersion"].get<std::string>(),
				item["path"].get<std::string>(),
				"Invalid path from JSON: " + item["path"].get<std::string>() } );
		}
		else
		{
			ADD_FAILURE() << "Skipping invalid item in JSON 'Invalid' array: " << item.dump( 4 );
		}
	}

	ASSERT_FALSE( testCases.empty() ) << "No invalid test cases loaded from GmodPaths.json";

	for ( const auto& tc : testCases )
	{
		SCOPED_TRACE( "Testing case: " + tc.description + " (Path: '" + tc.pathStr + "', VIS: " + tc.visVersionStr + ")" );

		std::string visVersionForParse = tc.visVersionStr;

		VisVersion versionEnum = VisVersionExtensions::parse( visVersionForParse );
		ASSERT_NE( VisVersion::Unknown, versionEnum ) << "Failed to parse VIS version string: " << tc.visVersionStr << " (parsed as " << visVersionForParse << ")";

		const auto& gmod = m_vis->gmod( versionEnum );
		const auto& locations = m_vis->locations( versionEnum );

		GmodPath path;
		bool parsed = GmodPath::tryParse( tc.pathStr, gmod, locations, path );

		ASSERT_FALSE( parsed ) << "Parsing should have failed for: " << tc.pathStr;

		EXPECT_TRUE( path.isEmpty() ) << "Path should remain (or be reset to) empty after a failed parse for: " << tc.pathStr;
	}
}

TEST_F( GmodPathTest, Test_GmodPath_Does_Not_Individualize )
{
	auto version = VisVersion::v3_7a;
	const auto& gmod = m_vis->gmod( version );

	GmodPath path;
	bool parsed = gmod.tryParsePath( "500a-1", path );
	ASSERT_FALSE( parsed );
}

TEST_F( GmodPathTest, Test_GetFullPath )
{
	auto version = VisVersion::v3_4a;
	const auto& gmod = m_vis->gmod( version );
	const auto& locations = m_vis->locations( version );

	std::string pathStr = "411.1/C101.72/I101";
	std::map<size_t, std::string> expectation = {
		{ 0, "VE" },
		{ 1, "400a" },
		{ 2, "410" },
		{ 3, "411" },
		{ 4, "411i" },
		{ 5, "411.1" },
		{ 6, "CS1" },
		{ 7, "C101" },
		{ 8, "C101.7" },
		{ 9, "C101.72" },
		{ 10, "I101" } };

	std::optional<GmodPath> parsedPath;
	GmodPath actualPath;

	bool parsed = GmodPath::tryParse( pathStr, gmod, locations, actualPath );
	if ( parsed )
	{
		parsedPath = std::move( actualPath );
	}
	ASSERT_TRUE( parsed );

	std::set<size_t> seen;
	auto fullPath = parsedPath->fullPath();

	for ( const auto& [depth, node] : fullPath )
	{
		ASSERT_TRUE( seen.find( depth ) == seen.end() ) << "Got same depth twice";
		seen.insert( depth );

		if ( seen.size() == 1 )
		{
			EXPECT_EQ( 0, depth );
		}

		ASSERT_TRUE( expectation.find( depth ) != expectation.end() );
		EXPECT_EQ( expectation[depth], node.get().code() );
	}

	EXPECT_EQ( expectation.size(), seen.size() );
}

TEST_F( GmodPathTest, Test_GetFullPathFrom )
{
	auto version = VisVersion::v3_4a;
	const auto& gmod = m_vis->gmod( version );
	const auto& locations = m_vis->locations( version );

	std::string pathStr = "411.1/C101.72/I101";
	std::map<size_t, std::string> expectation = {
		{ 4, "411i" },
		{ 5, "411.1" },
		{ 6, "CS1" },
		{ 7, "C101" },
		{ 8, "C101.7" },
		{ 9, "C101.72" },
		{ 10, "I101" } };

	std::optional<GmodPath> parsedPath;
	GmodPath actualPath;

	bool parsed = GmodPath::tryParse( pathStr, gmod, locations, actualPath );
	if ( parsed )
	{
		parsedPath = std::move( actualPath );
	}
	ASSERT_TRUE( parsed );

	std::set<size_t> seen;
	auto partialPath = parsedPath->fullPathFrom( 4 );

	for ( const auto& [depth, node] : partialPath )
	{
		ASSERT_TRUE( seen.find( depth ) == seen.end() ) << "Got same depth twice";
		seen.insert( depth );

		if ( seen.size() == 1 )
		{
			EXPECT_EQ( 4, depth );
		}

		ASSERT_TRUE( expectation.find( depth ) != expectation.end() );
		EXPECT_EQ( expectation[depth], node.get().code() );
	}

	EXPECT_EQ( expectation.size(), seen.size() );
}

TEST_F( GmodPathTest, Test_ToFullPathString )
{
	auto version = VisVersion::v3_7a;
	const auto& gmod = m_vis->gmod( version );

	GmodPath path1;
	ASSERT_TRUE( gmod.tryParsePath( "511.11-1/C101.663i-1/C663", path1 ) );
	EXPECT_EQ( "VE/500a/510/511/511.1/511.1i-1/511.11-1/CS1/C101/C101.6/C101.66/C101.663/C101.663i-1/C663",
		path1.toFullPathString() );

	GmodPath path2;
	ASSERT_TRUE( gmod.tryParsePath( "846/G203.32-2/S110.2-1/E31", path2 ) );
	EXPECT_EQ( "VE/800a/840/846/G203/G203.3-2/G203.32-2/S110/S110.2-1/CS1/E31",
		path2.toFullPathString() );
} */

//=====================================================================
// TEST_P
//=====================================================================

/* class GmodPathParamTest : public GmodPathTest,
						  public ::testing::WithParamInterface<GmodPathTestItem>
{
};

class FullPathParsingTest : public GmodPathTest,
							public ::testing::WithParamInterface<std::tuple<std::string, std::string>>
{
};

TEST_P( GmodPathParamTest, Test_GmodPath_Parse_Invalid )
{
	auto testItem = GetParam();
	auto visVersion = VisVersionExtensions::parse( testItem.visVersion );
	auto inputPath = testItem.path;

	const auto& gmod = m_vis->gmod( visVersion );
	const auto& locations = m_vis->locations( visVersion );

	std::optional<GmodPath> path;
	GmodPath actualPath;
	bool parsed = GmodPath::tryParse( inputPath, gmod, locations, actualPath );

	if ( parsed )
	{
		path = std::move( actualPath );
	}

	ASSERT_FALSE( parsed );
	ASSERT_FALSE( path.has_value() );
} */

//----------------------------------------------
// TEST_P Failing :(
//----------------------------------------------

/* TEST_P( GmodPathParamTest, Test_GmodPath_Parse_Valid )
{
	auto testItem = GetParam();
	auto visVersion = VisVersionExtensions::parse( testItem.visVersion );
	auto inputPath = testItem.path;

	const auto& gmod = m_vis->gmod( visVersion );
	const auto& locations = m_vis->locations( visVersion );

	std::optional<GmodPath> path;
	GmodPath actualPath;
	bool parsed = GmodPath::tryParse( inputPath, gmod, locations, actualPath );

	if ( parsed )
	{
		path = std::move( actualPath );
	}

	ASSERT_TRUE( parsed );
	ASSERT_TRUE( path.has_value() );
	EXPECT_EQ( inputPath, path->toString() );
}

TEST_P( FullPathParsingTest, Test_FullPathParsing )
{
	auto [shortPathStr, expectedFullPathStr] = GetParam();

	auto version = VisVersion::v3_4a;
	const auto& gmod = m_vis->gmod( version );
	const auto& locations = m_vis->locations( version );

	std::optional<GmodPath> shortPath;
	{
		GmodPath parsedPath;

		bool success = GmodPath::tryParse( shortPathStr, gmod, locations, parsedPath );
		if ( success )
		{
			shortPath = std::move( parsedPath );
		}
		ASSERT_TRUE( success ) << "Failed to parse short path: " << shortPathStr;
		ASSERT_TRUE( shortPath.has_value() );
	}

	std::string actualFullPathStr = shortPath->toFullPathString();
	EXPECT_EQ( expectedFullPathStr, actualFullPathStr );

	std::optional<GmodPath> fullPath;
	{
		GmodPath parsedPath;

		bool success = GmodPath::tryParseFullPath( actualFullPathStr, gmod, locations, parsedPath );
		if ( success )
		{
			fullPath = std::move( parsedPath );
		}
		ASSERT_TRUE( success ) << "Failed to parse full path: " << actualFullPathStr;
		ASSERT_TRUE( fullPath.has_value() );
	}

	EXPECT_EQ( *shortPath, *fullPath ) << "Round-trip path parsing failed";
	EXPECT_EQ( actualFullPathStr, shortPath->toFullPathString() );
	EXPECT_EQ( actualFullPathStr, fullPath->toFullPathString() );
	EXPECT_EQ( shortPathStr, shortPath->toString() );
	EXPECT_EQ( shortPathStr, fullPath->toString() );
}
 */
//=====================================================================
// Instantiate
//=====================================================================

/*
INSTANTIATE_TEST_SUITE_P(
	ValidPaths,
	GmodPathParamTest,
	::testing::Values(
		GmodPathTestItem{ "3.4a", "411.1/C101.31-2" },
		GmodPathTestItem{ "3.4a", "411.1/C101.72/I101" },
		GmodPathTestItem{ "3.4a", "612.21-1/C701.13/S93" } ) );

INSTANTIATE_TEST_SUITE_P(
	InvalidPaths,
	GmodPathParamTest,
	::testing::Values(
		GmodPathTestItem{ "3.4a", "invalid/path" },
		GmodPathTestItem{ "3.4a", "XXX.YYY/ZZZ" },
		GmodPathTestItem{ "3.4a", "500a-1" } ) );

INSTANTIATE_TEST_SUITE_P(
	FullPaths,
	FullPathParsingTest,
	::testing::Values(
		std::make_tuple( "411.1/C101.72/I101", "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101" ),
		std::make_tuple( "612.21-1/C701.13/S93", "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93" ) ) );

int main( int argc, char** argv )
{
	::testing::InitGoogleTest( &argc, argv );
	return RUN_ALL_TESTS();
}
*/
