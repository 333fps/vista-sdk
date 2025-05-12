/**
 * @file GmodPathTests.cpp
 * @brief
 */

#include "pch.h"

#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/LocalIdParsingErrorBuilder.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodTraversal.h"

namespace dnv::vista::sdk
{
	namespace
	{
		static const nlohmann::json& testData()
		{
			static nlohmann::json globalJsonData;
			static bool loaded = false;
			if ( !loaded )
			{
				std::string jsonFilePath = "testdata/GmodPaths.json";
				std::ifstream jsonFile( jsonFilePath );
				if ( !jsonFile.is_open() )
				{
					throw std::runtime_error( "Failed to open global test data file: " + jsonFilePath );
				}
				try
				{
					jsonFile >> globalJsonData;
					loaded = true;
				}
				catch ( const nlohmann::json::parse_error& e )
				{
					throw std::runtime_error( std::string( "Failed to parse global test data file: " ) + jsonFilePath + " - " + e.what() );
				}
			}

			return globalJsonData;
		}
	}

	namespace GmodPathTestsFixture
	{
		class GmodPathTest : public ::testing::Test
		{
		protected:
			GmodPathTest()
				: m_vis{ VIS::instance() },
				  m_jsonData{ testData() }
			{
			}

			VIS& m_vis;
			const nlohmann::json& m_jsonData;
		};

		//=====================================================================
		// Tests
		//=====================================================================

		//----------------------------------------------
		// Test_GmodPath_Parse
		//----------------------------------------------

		/* 		TEST_F( GmodPathTest, Test_GmodPath_Parse )
				{
					struct TestCase
					{
						std::string visVersionStr;
						std::string pathStr;
						std::string description;
					};

					std::vector<TestCase> testCases;

					const std::string dataKey = "Valid";
					if ( !m_jsonData.contains( dataKey ) || !m_jsonData[dataKey].is_array() )
					{
						FAIL() << "GmodPaths.json does not contain a valid '" << dataKey << "' array.";
						return;
					}

					for ( const auto& item : m_jsonData[dataKey] )
					{
						if ( item.is_object() && item.contains( "path" ) && item["path"].is_string() &&
							 item.contains( "visVersion" ) && item["visVersion"].is_string() )
						{
							testCases.push_back( { item["visVersion"].get<std::string>(),
								item["path"].get<std::string>(),
								"Valid path from JSON: " + item["path"].get<std::string>() } );
						}
						else
						{
							ADD_FAILURE() << "Skipping invalid item in JSON '" << dataKey << "' array: " << item.dump( 4 );
						}
					}

					ASSERT_FALSE( testCases.empty() ) << "No valid test cases loaded from GmodPaths.json for key '" << dataKey << "'.";

					for ( const auto& tc : testCases )
					{
						SCOPED_TRACE( "Testing case: " + tc.description + " (Path: '" + tc.pathStr + "', VIS: " + tc.visVersionStr + ")" );

						VisVersion versionEnum = VisVersionExtensions::parse( tc.visVersionStr );
						ASSERT_NE( VisVersion::Unknown, versionEnum ) << "Failed to parse VIS version string: " << tc.visVersionStr;

						const auto& gmod = m_vis.gmod( versionEnum );
						const auto& locations = m_vis.locations( versionEnum );

						GmodPath path;
						bool parsed = GmodPath::tryParse( tc.pathStr, gmod, locations, path );

						ASSERT_TRUE( parsed ) << "Parsing should have succeeded for: " << tc.pathStr;
						EXPECT_FALSE( path.isEmpty() ) << "Path should not be empty after a successful parse for: " << tc.pathStr;
						EXPECT_EQ( tc.pathStr, path.toString() ) << "Parsed path string representation does not match input for: " << tc.pathStr;
					}
				} */

		//----------------------------------------------
		// Test_GmodPath_Parse_Invalid
		//----------------------------------------------

		/* 		TEST_F( GmodPathTest, Test_GmodPath_Parse_Invalid )
				{
					struct TestCase
					{
						std::string visVersionStr;
						std::string pathStr;
						std::string description;
					};

					std::vector<TestCase> testCases;

					const std::string dataKey = "Invalid";
					if ( !m_jsonData.contains( dataKey ) || !m_jsonData[dataKey].is_array() )
					{
						FAIL() << "GmodPaths.json does not contain a valid '" << dataKey << "' array.";
						return;
					}

					for ( const auto& item : m_jsonData[dataKey] )
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
							ADD_FAILURE() << "Skipping invalid item in JSON '" << dataKey << "' array: " << item.dump( 4 );
						}
					}

					ASSERT_FALSE( testCases.empty() ) << "No invalid test cases loaded from GmodPaths.json for key '" << dataKey << "'.";

					for ( const auto& tc : testCases )
					{
						SCOPED_TRACE( "Testing case: " + tc.description + " (Path: '" + tc.pathStr + "', VIS: " + tc.visVersionStr + ")" );

						VisVersion versionEnum = VisVersionExtensions::parse( tc.visVersionStr );
						ASSERT_NE( VisVersion::Unknown, versionEnum ) << "Failed to parse VIS version string: " << tc.visVersionStr;

						const auto& gmod = m_vis.gmod( versionEnum );
						const auto& locations = m_vis.locations( versionEnum );

						GmodPath path;
						bool parsed = GmodPath::tryParse( tc.pathStr, gmod, locations, path );

						EXPECT_FALSE( parsed ) << "Parsing should have failed for: " << tc.pathStr;
						EXPECT_TRUE( path.isEmpty() ) << "Path should be empty after a failed parse for: " << tc.pathStr;
					}
				} */

		//----------------------------------------------
		// Test_GetFullPath
		//----------------------------------------------

		/* 		TEST_F( GmodPathTest, Test_GetFullPath )
				{
					const auto& gmod = m_vis.gmod( VisVersion::v3_4a );
					const auto& locations = m_vis.locations( VisVersion::v3_4a );

					std::string pathStr = "411.1/C101.72/I101";
					GmodPath path;
					ASSERT_TRUE( GmodPath::tryParse( pathStr, gmod, locations, path ) );

					std::map<int, std::string> expectation = {
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
						{ 10, "I101" },
					};

					std::set<int> seenDepths;
					int count = 0;
					for ( const auto& pair : path.fullPath() )
					{
						int depth = pair.first;
						const GmodNode& node = pair.second;

						ASSERT_TRUE( seenDepths.find( depth ) == seenDepths.end() ) << "Got same depth twice: " << depth;
						seenDepths.insert( depth );

						if ( count == 0 )
						{
							ASSERT_EQ( 0, depth );
						}
						ASSERT_TRUE( expectation.count( depth ) ) << "Depth " << depth << " not found in expectation.";
						EXPECT_EQ( expectation[depth], node.code() );
						count++;
					}

					std::vector<int> expectedDepths;
					for ( auto const& [depth, val] : expectation )
						expectedDepths.push_back( depth );
					std::sort( expectedDepths.begin(), expectedDepths.end() );

					std::vector<int> actualDepths( seenDepths.begin(), seenDepths.end() );
					std::sort( actualDepths.begin(), actualDepths.end() );

					EXPECT_EQ( expectedDepths, actualDepths );
				}
		 */
		//----------------------------------------------
		// Test_GetFullPathFrom
		//----------------------------------------------

		/* 		TEST_F( GmodPathTest, Test_GetFullPathFrom )
				{
					const auto& gmod = m_vis.gmod( VisVersion::v3_4a );
					const auto& locations = m_vis.locations( VisVersion::v3_4a );

					std::string pathStr = "411.1/C101.72/I101";
					GmodPath path;
					ASSERT_TRUE( GmodPath::tryParse( pathStr, gmod, locations, path ) );

					std::map<int, std::string> expectation = {
						{ 4, "411i" },
						{ 5, "411.1" },
						{ 6, "CS1" },
						{ 7, "C101" },
						{ 8, "C101.7" },
						{ 9, "C101.72" },
						{ 10, "I101" },
					};

					std::set<int> seenDepths;
					int count = 0;
					for ( const auto& pair : path.fullPathFrom( 4 ) )
					{
						int depth = pair.first;
						const GmodNode& node = pair.second;

						ASSERT_TRUE( seenDepths.find( depth ) == seenDepths.end() ) << "Got same depth twice: " << depth;
						seenDepths.insert( depth );

						if ( count == 0 )
						{
							ASSERT_EQ( 4, depth );
						}
						ASSERT_TRUE( expectation.count( depth ) ) << "Depth " << depth << " not found in expectation.";
						EXPECT_EQ( expectation[depth], node.code() );
						count++;
					}

					std::vector<int> expectedDepths;
					for ( auto const& [depth, val] : expectation )
						expectedDepths.push_back( depth );
					std::sort( expectedDepths.begin(), expectedDepths.end() );

					std::vector<int> actualDepths( seenDepths.begin(), seenDepths.end() );
					std::sort( actualDepths.begin(), actualDepths.end() );

					EXPECT_EQ( expectedDepths, actualDepths );
				} */

		//----------------------------------------------
		// Test_GmodPath_Does_Not_Individualize
		//----------------------------------------------

		/* 		TEST_F( GmodPathTest, Test_GmodPath_Does_Not_Individualize )
				{
					auto version = VisVersion::v3_7a;
					const auto& gmod = m_vis.gmod( version );

					std::optional<GmodPath> pathOpt;
					bool parsed = gmod.tryParsePath( "500a-1", pathOpt );

					ASSERT_FALSE( parsed );
					EXPECT_FALSE( pathOpt.has_value() );
				} */

		//----------------------------------------------
		// Test_ToFullPathString
		//----------------------------------------------

		/*
		TEST_F( GmodPathTest, Test_ToFullPathString )
		{
			auto version = VisVersion::v3_7a;
			const auto& gmod = m_vis.gmod( version );

			std::optional<GmodPath> path;

			path = gmod.parsePath( "511.11-1/C101.663i-1/C663" );
			EXPECT_EQ( "VE/500a/510/511/511.1/511.1i-1/511.11-1/CS1/C101/C101.6/C101.66/C101.663/C101.663i-1/C663",
				path->toFullPathString() );

			path = gmod.parsePath( "846/G203.32-2/S110.2-1/E31" );
			EXPECT_EQ( "VE/800a/840/846/G203/G203.3-2/G203.32-2/S110/S110.2-1/CS1/E31",
				path->toFullPathString() );
		}
		*/
	}

	namespace GmodPathTestsParametrized
	{
		//=====================================================================
		// Tests
		//=====================================================================

		//----------------------------------------------
		// Test_ToFullPathString
		//----------------------------------------------

		/*
		struct FullPathParsingParam
		{
			std::string shortPathStr;
			std::string expectedFullPathStr;
		};

		class FullPathParsingTest : public ::testing::TestWithParam<FullPathParsingParam>
		{
		protected:
			FullPathParsingTest()
				: m_vis( VIS::instance() )
			{
			}
			VIS& m_vis;
		};

		static std::vector<FullPathParsingParam> fullPathParsingData()
		{
			return {
				{ "411.1/C101.72/I101", "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101" },
				{ "612.21-1/C701.13/S93", "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93" } };
		}

		TEST_P( FullPathParsingTest, Test_FullPathParsing )
		{
			const auto& param = GetParam();
			auto version = VisVersion::v3_4a;
			const auto& gmod = m_vis.gmod( version );
			const auto& locations = m_vis.locations( version );

			GmodPath path1;
			ASSERT_TRUE( GmodPath::tryParse( param.shortPathStr, gmod, locations, path1 ) );

			std::string fullString1 = path1.toFullPathString();
			EXPECT_EQ( param.expectedFullPathStr, fullString1 );

			GmodPath path2;
			bool parsed = GmodPath::tryParseFullPath( fullString1, gmod, locations, path2 );
			ASSERT_TRUE( parsed );
			ASSERT_FALSE( path2.isEmpty() );

			EXPECT_EQ( path1, path2 );
			EXPECT_EQ( fullString1, path1.toFullPathString() );
			EXPECT_EQ( fullString1, path2.toFullPathString() );
			EXPECT_EQ( param.shortPathStr, path1.toString() );
			EXPECT_EQ( param.shortPathStr, path2.toString() );

			GmodPath path3;
			ASSERT_NO_THROW( {
				path3 = GmodPath::parseFullPath( fullString1, gmod, locations );
			} );
			ASSERT_FALSE( path3.isEmpty() );
			EXPECT_EQ( path1, path3 );
			EXPECT_EQ( fullString1, path3.toFullPathString() );
			EXPECT_EQ( param.shortPathStr, path3.toString() );
		}
		*/

		//----------------------------------------------
		// Test_IndividualizableSets
		//----------------------------------------------

		/*
		struct IndividualizableSetsParam
		{
			bool isFullPath;
			std::string visVersionStr;
			std::string inputPath;
			std::optional<std::vector<std::vector<std::string>>> expectedNodeCodes;
			std::string description;
		};

		class IndividualizableSetsTest : public ::testing::TestWithParam<IndividualizableSetsParam>
		{
		protected:
			IndividualizableSetsTest()
				: m_vis( VIS::instance() )
			{
			}
			VIS& m_vis;
		};

		static std::vector<IndividualizableSetsParam> individualizableSetsData()
		{
			std::vector<IndividualizableSetsParam> data;
			const nlohmann::json& jsonDataFromFile = testData();
			const std::string dataKey = "IndividualizableSets";

			if ( !jsonDataFromFile.contains( dataKey ) || !jsonDataFromFile[dataKey].is_array() )
			{
				return data;
			}

			for ( const auto& item : jsonDataFromFile[dataKey] )
			{
				if ( item.is_object() && item.contains( "isFullPath" ) && item["isFullPath"].is_boolean() &&
					 item.contains( "visVersion" ) && item["visVersion"].is_string() &&
					 item.contains( "inputPath" ) && item["inputPath"].is_string() &&
					 item.contains( "expected" ) )
				{
					std::optional<std::vector<std::vector<std::string>>> expected;
					if ( !item["expected"].is_null() )
					{
						if ( item["expected"].is_array() )
						{
							std::vector<std::vector<std::string>> outerVec;
							for ( const auto& innerItem : item["expected"] )
							{
								if ( innerItem.is_array() )
								{
									outerVec.push_back( innerItem.get<std::vector<std::string>>() );
								}
							}
							expected = outerVec;
						}
					}

					std::string desc = "isFullPath: " + std::string( item["isFullPath"].get<bool>() ? "true" : "false" ) +
									   ", Version: " + item["visVersion"].get<std::string>() +
									   ", Path: " + item["inputPath"].get<std::string>();
					data.push_back( { item["isFullPath"].get<bool>(),
						item["visVersion"].get<std::string>(),
						item["inputPath"].get<std::string>(),
						expected,
						desc } );
				}
			}
			return data;
		}

		TEST_P( IndividualizableSetsTest, Test_IndividualizableSets )
		{
			const auto& param = GetParam();
			SCOPED_TRACE( param.description );

			VisVersion versionEnum = VisVersionExtensions::parse( param.visVersionStr );
			ASSERT_NE( VisVersion::Unknown, versionEnum ) << "Failed to parse VIS version string: " << param.visVersionStr;

			const auto& gmod = m_vis.gmod( versionEnum );
			const auto& locations = m_vis.locations( versionEnum );

			if ( !param.expectedNodeCodes.has_value() )
			{
				std::optional<GmodPath> parsedPathOpt;
				bool parsedSuccess = param.isFullPath ? GmodPath::tryParseFullPath( param.inputPath, gmod, locations, parsedPathOpt ) : GmodPath::tryParse( param.inputPath, gmod, locations, parsedPathOpt );

				EXPECT_FALSE( parsedSuccess );
				EXPECT_FALSE( parsedPathOpt.has_value() );
			}
			else
			{
				GmodPath path;
				if ( param.isFullPath )
				{
					ASSERT_NO_THROW( path = GmodPath::parseFullPath( param.inputPath, gmod, locations ) );
				}
				else
				{
					ASSERT_NO_THROW( path = GmodPath::parse( param.inputPath, gmod, locations ) );
				}
				ASSERT_FALSE( path.isEmpty() );

				auto actualSets = path.individualizableSets();
				const auto& expectedOuterVec = param.expectedNodeCodes.value();

				ASSERT_EQ( expectedOuterVec.size(), actualSets.size() );
				for ( size_t i = 0; i < expectedOuterVec.size(); ++i )
				{
					const auto& expectedInnerVec = expectedOuterVec[i];
					const auto& actualSetNodes = actualSets[i].nodes();

					std::vector<std::string> actualNodeCodesInSet;
					std::transform( actualSetNodes.begin(), actualSetNodes.end(), std::back_inserter( actualNodeCodesInSet ),
						[]( const GmodNode& node ) { return node.code(); } );

					EXPECT_EQ( expectedInnerVec, actualNodeCodesInSet );
				}
			}
		}
		*/

		//----------------------------------------------
		// Test_FullPathParsing
		//----------------------------------------------

		/*
		struct FullPathParsingParam
		{
			std::string shortPathStr;
			std::string expectedFullPathStr;
		};

		class FullPathParsingTest : public ::testing::TestWithParam<FullPathParsingParam>
		{
		protected:
			FullPathParsingTest()
				: m_vis( VIS::instance() )
			{
			}
			VIS& m_vis;
		};

		static std::vector<FullPathParsingParam> fullPathParsingData()
		{
			return {
				{ "411.1/C101.72/I101", "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101" },
				{ "612.21-1/C701.13/S93", "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93" } };
		}

		TEST_P( FullPathParsingTest, Test_FullPathParsing )
		{
			const auto& param = GetParam();
			auto version = VisVersion::v3_4a;
			const auto& gmod = m_vis.gmod( version );
			const auto& locations = m_vis.locations( version );

			GmodPath path1;
			ASSERT_TRUE( GmodPath::tryParse( param.shortPathStr, gmod, locations, path1 ) );

			std::string fullString1 = path1.toFullPathString();
			EXPECT_EQ( param.expectedFullPathStr, fullString1 );

			GmodPath path2;
			bool parsed = GmodPath::tryParseFullPath( fullString1, gmod, locations, path2 );
			ASSERT_TRUE( parsed );
			ASSERT_FALSE( path2.isEmpty() );

			EXPECT_EQ( path1, path2 );
			EXPECT_EQ( fullString1, path1.toFullPathString() );
			EXPECT_EQ( fullString1, path2.toFullPathString() );
			EXPECT_EQ( param.shortPathStr, path1.toString() );
			EXPECT_EQ( param.shortPathStr, path2.toString() );

			GmodPath path3;
			ASSERT_NO_THROW( {
				path3 = GmodPath::parseFullPath( fullString1, gmod, locations );
			} );
			ASSERT_FALSE( path3.isEmpty() );
			EXPECT_EQ( path1, path3 );
			EXPECT_EQ( fullString1, path3.toFullPathString() );
			EXPECT_EQ( param.shortPathStr, path3.toString() );
		}
		*/

		//----------------------------------------------
		// Test_Valid_GmodPath_IndividualizableSets
		//----------------------------------------------

		/*
		struct ValidGmodPathIndividualizableSetsParam
		{
			std::string visVersionStr;
			std::string pathStr;
			std::string description;
		};

		class ValidGmodPathIndividualizableSetsTest : public ::testing::TestWithParam<ValidGmodPathIndividualizableSetsParam>
		{
		protected:
			ValidGmodPathIndividualizableSetsTest()
				: m_vis( VIS::instance() )
			{
			}
			VIS& m_vis;
		};

		static std::vector<ValidGmodPathIndividualizableSetsParam> validGmodPathIndividualizableSetsData()
		{
			std::vector<ValidGmodPathIndividualizableSetsParam> data;
			const nlohmann::json& jsonDataFromFile = testData();
			const std::string dataKey = "Valid";

			if ( jsonDataFromFile.contains( dataKey ) && jsonDataFromFile[dataKey].is_array() )
			{
				for ( const auto& item : jsonDataFromFile[dataKey] )
				{
					if ( item.is_object() && item.contains( "path" ) && item["path"].is_string() &&
						 item.contains( "visVersion" ) && item["visVersion"].is_string() )
					{
						std::string path = item["path"].get<std::string>();
						std::string version = item["visVersion"].get<std::string>();
						data.push_back( { version, path, "Version: " + version + ", Path: " + path } );
					}
				}
			}
			return data;
		}

		TEST_P( ValidGmodPathIndividualizableSetsTest, Test_Valid_GmodPath_IndividualizableSets )
		{
			const auto& param = GetParam();
			SCOPED_TRACE( param.description );

			VisVersion versionEnum = VisVersionExtensions::parse( param.visVersionStr );
			ASSERT_NE( VisVersion::Unknown, versionEnum );

			const auto& gmod = m_vis.gmod( versionEnum );
			const auto& locations = m_vis.locations( versionEnum );

			GmodPath path;
			ASSERT_NO_THROW( path = GmodPath::parse( param.pathStr, gmod, locations ) );
			ASSERT_FALSE( path.isEmpty() );

			auto sets = path.individualizableSets();

			std::set<std::string> uniqueCodes;
			for ( const auto& set : sets )
			{
				for ( const auto& node : set.nodes() )
				{
					EXPECT_TRUE( uniqueCodes.insert( node.code() ).second ) << "Duplicate code found: " << node.code() << " in path " << param.pathStr;
				}
			}
		}
		*/

		//----------------------------------------------
		// Test_Valid_GmodPath_IndividualizableSets_FullPath
		//----------------------------------------------

		/*
		TEST_P( ValidGmodPathIndividualizableSetsTest, Test_Valid_GmodPath_IndividualizableSets_FullPath )
		{
			const auto& param = GetParam();
			SCOPED_TRACE( param.description );

			VisVersion versionEnum = VisVersionExtensions::parse( param.visVersionStr );
			ASSERT_NE( VisVersion::Unknown, versionEnum );

			const auto& gmod = m_vis.gmod( versionEnum );
			const auto& locations = m_vis.locations( versionEnum );

			GmodPath shortPath;
			ASSERT_NO_THROW( shortPath = GmodPath::parse( param.pathStr, gmod, locations ) );
			ASSERT_FALSE( shortPath.isEmpty() );

			std::string fullPathStr = shortPath.toFullPathString();

			GmodPath pathFromFullPath;
			ASSERT_NO_THROW( pathFromFullPath = GmodPath::parseFullPath( fullPathStr, gmod, locations ) );
			ASSERT_FALSE( pathFromFullPath.isEmpty() );

			auto sets = pathFromFullPath.individualizableSets();

			std::set<std::string> uniqueCodes;
			for ( const auto& set : sets )
			{
				for ( const auto& node : set.nodes() )
				{
					EXPECT_TRUE( uniqueCodes.insert( node.code() ).second ) << "Duplicate code found: " << node.code() << " in full path derived from " << param.pathStr;
				}
			}
		}
		*/

		//=====================================================================
		// Instantiate
		//=====================================================================

		/*
		INSTANTIATE_TEST_SUITE_P(
			GmodPathParsingSuite,
			GmodPathTestsFixture::FullPathParsingTest,
			::testing::ValuesIn( GmodPathTestsFixture::fullPathParsingData() ) );
		*/

		/*
		INSTANTIATE_TEST_SUITE_P(
			GmodIndividualizableSetsSuite,
			IndividualizableSetsTest,
			::testing::ValuesIn( individualizableSetsData() ) );
		*/

		/*
		INSTANTIATE_TEST_SUITE_P(
			GmodPathParsingSuite,
			GmodPathTestsFixture::FullPathParsingTest,
			::testing::ValuesIn( GmodPathTestsFixture::fullPathParsingData() ) );
		*/

		/*
		INSTANTIATE_TEST_SUITE_P(
			GmodValidPathIndividualizableSetsSuite,
			ValidGmodPathIndividualizableSetsTest,
			::testing::ValuesIn( validGmodPathIndividualizableSetsData() ) );
		*/
	}
}
