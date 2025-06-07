/**
 * @file GmodPathTests.cpp
 * @brief Unit tests for the GmodPath class.
 */

#include "pch.h"

#include "TestDataLoader.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk::tests
{
	namespace
	{
		constexpr const char* GMOD_PATH_TEST_DATA_PATH = "testdata/GmodPaths.json";
		constexpr const char* INDIVIDUALIZABLE_SETS_TEST_DATA_PATH = "testdata/IndividualizableSets.json";
	}

	namespace GmodTestsFixture
	{
	}

	namespace GmodPathTestsParametrized
	{
		//=====================================================================
		// GmodPathData
		//=====================================================================

		//----------------------------------------------
		// Valid
		//----------------------------------------------

		struct GmodPathParseValidParam
		{
			std::string visVersionString;
			std::string pathString;
		};

		static std::vector<GmodPathParseValidParam> loadValidGmodPathData()
		{
			std::vector<GmodPathParseValidParam> params;
			const nlohmann::json& jsonData = loadTestData( GMOD_PATH_TEST_DATA_PATH );
			const std::string dataKey = "Valid";

			if ( jsonData.contains( dataKey ) && jsonData[dataKey].is_array() )
			{
				for ( const auto& item : jsonData[dataKey] )
				{
					if ( item.is_object() && item.contains( "visVersion" ) && item["visVersion"].is_string() &&
						 item.contains( "path" ) && item["path"].is_string() )
					{
						std::string visVersionStr = item["visVersion"].get<std::string>();
						std::string pathStr = item["path"].get<std::string>();
						params.push_back( { visVersionStr, pathStr } );
					}
				}
			}
			return params;
		}

		//----------------------------------------------
		// Invalid
		//----------------------------------------------

		struct GmodPathParseInvalidParam
		{
			std::string visVersionString;
			std::string pathString;
		};

		static std::vector<GmodPathParseInvalidParam> loadInvalidGmodPathData()
		{
			std::vector<GmodPathParseInvalidParam> params;
			const nlohmann::json& jsonData = loadTestData( GMOD_PATH_TEST_DATA_PATH );
			const std::string dataKey = "Invalid";

			if ( jsonData.contains( dataKey ) && jsonData[dataKey].is_array() )
			{
				for ( const auto& item : jsonData[dataKey] )
				{
					if ( item.is_object() && item.contains( "visVersion" ) && item["visVersion"].is_string() &&
						 item.contains( "path" ) && item["path"].is_string() )
					{
						std::string visVersionStr = item["visVersion"].get<std::string>();
						std::string pathStr = item["path"].get<std::string>();
						params.push_back( { visVersionStr, pathStr } );
					}
				}
			}
			return params;
		}

		//=====================================================================
		// IndividualizableSetsData
		//=====================================================================

		struct IndividualizableSetsTestData
		{
			bool isFullPath;
			std::string visVersionString;
			std::string path;
			std::optional<std::vector<std::vector<std::string>>> expected;
		};

		static std::vector<IndividualizableSetsTestData> LoadIndividualizableSetsData()
		{
			std::vector<IndividualizableSetsTestData> params;
			const nlohmann::json& jsonData = loadTestData( INDIVIDUALIZABLE_SETS_TEST_DATA_PATH );

			if ( jsonData.is_array() )
			{
				for ( const auto& item : jsonData )
				{
					if ( item.is_object() &&
						 item.contains( "isFullPath" ) && item["isFullPath"].is_boolean() &&
						 item.contains( "visVersion" ) && item["visVersion"].is_string() &&
						 item.contains( "path" ) && item["path"].is_string() &&
						 item.contains( "expected" ) )
					{
						bool isFullPath = item["isFullPath"].get<bool>();
						std::string visVersionStr = item["visVersion"].get<std::string>();
						std::string pathStr = item["path"].get<std::string>();
						std::optional<std::vector<std::vector<std::string>>> expectedSetsOpt;

						const auto& expectedJson = item["expected"];
						if ( expectedJson.is_null() )
						{
							expectedSetsOpt = std::nullopt;
						}
						else if ( expectedJson.is_array() )
						{
							std::vector<std::vector<std::string>> outerVector;
							for ( const auto& innerJsonArray : expectedJson )
							{
								if ( innerJsonArray.is_array() )
								{
									std::vector<std::string> innerVector;
									for ( const auto& elementJson : innerJsonArray )
									{
										if ( elementJson.is_string() )
										{
											innerVector.push_back( elementJson.get<std::string>() );
										}
									}
									outerVector.push_back( innerVector );
								}
							}
							expectedSetsOpt = outerVector;
						}
						params.push_back( { isFullPath, visVersionStr, pathStr, expectedSetsOpt } );
					}
				}
			}
			return params;
		}

		//----------------------------------------------
		// Test_GmodPath_Parse
		//----------------------------------------------

		class GmodPathParseValidTest : public ::testing::TestWithParam<GmodPathParseValidParam>
		{
		protected:
			GmodPathParseValidTest() : m_vis( VIS::instance() )
			{
			}

			VIS& m_vis;
		};

		TEST_P( GmodPathParseValidTest, Test_GmodPath_Parse )
		{
			const auto& param = GetParam();

			VisVersion visVersion = VisVersionExtensions::parse( param.visVersionString );

			std::optional<GmodPath> parsedGmodPathOptional;
			bool parsed = GmodPath::tryParse( param.pathString, visVersion, parsedGmodPathOptional );

			ASSERT_TRUE( parsed );
			ASSERT_TRUE( parsedGmodPathOptional.has_value() );
			ASSERT_EQ( param.pathString, parsedGmodPathOptional.value().toString() );
		}

		INSTANTIATE_TEST_SUITE_P(
			GmodPathParseValidSuite,
			GmodPathParseValidTest,
			::testing::ValuesIn( loadValidGmodPathData() ) );

		//----------------------------------------------
		// Test_GmodPath_Parse_Invalid
		//----------------------------------------------

		class GmodPathParseInvalidTest : public ::testing::TestWithParam<GmodPathParseInvalidParam>
		{
		protected:
			GmodPathParseInvalidTest() : m_vis( VIS::instance() )
			{
			}

			VIS& m_vis;
		};

		TEST_P( GmodPathParseInvalidTest, Test_GmodPath_Parse_Invalid )
		{
			const auto& param = GetParam();

			VisVersion visVersion = VisVersionExtensions::parse( param.visVersionString );

			std::optional<GmodPath> parsedGmodPathOptional;
			bool parsed = GmodPath::tryParse( param.pathString, visVersion, parsedGmodPathOptional );

			ASSERT_FALSE( parsed );
			ASSERT_FALSE( parsedGmodPathOptional.has_value() );
		}

		INSTANTIATE_TEST_SUITE_P(
			GmodPathParseInvalidSuite,
			GmodPathParseInvalidTest,
			::testing::ValuesIn( loadInvalidGmodPathData() ) );

		//----------------------------------------------
		// Test_FullPathParsing
		//----------------------------------------------

		struct FullPathParsingTestData
		{
			std::string shortPathStr;
			std::string expectedFullPathStr;
			VisVersion version;
		};

		class GmodPathFullPathParsingTest : public ::testing::TestWithParam<FullPathParsingTestData>
		{
		};

		TEST_P( GmodPathFullPathParsingTest, Test_FullPathParsing )
		{
			const auto& param = GetParam();

			std::optional<GmodPath> pathOptional;
			bool parsedOk = GmodPath::tryParse( param.shortPathStr, param.version, pathOptional );
			ASSERT_TRUE( parsedOk );
			ASSERT_TRUE( pathOptional.has_value() );
			GmodPath path = pathOptional.value();

			std::string fullString = path.toFullPathString();
			ASSERT_EQ( param.expectedFullPathStr, fullString );

			std::optional<GmodPath> parsedPathOptional;
			parsedOk = GmodPath::tryParseFullPath( fullString, param.version, parsedPathOptional );
			ASSERT_TRUE( parsedOk );
			ASSERT_TRUE( parsedPathOptional.has_value() );
			GmodPath parsedPath = parsedPathOptional.value();

			ASSERT_EQ( path, parsedPath );
			ASSERT_EQ( fullString, path.toFullPathString() );
			ASSERT_EQ( fullString, parsedPath.toFullPathString() );
			ASSERT_EQ( param.shortPathStr, path.toString() );
			ASSERT_EQ( param.shortPathStr, parsedPath.toString() );

			GmodPath parsedPathNonOptional = GmodPath::parseFullPath( fullString, param.version );
			ASSERT_EQ( path, parsedPathNonOptional );
			ASSERT_EQ( fullString, parsedPathNonOptional.toFullPathString() );
			ASSERT_EQ( param.shortPathStr, parsedPathNonOptional.toString() );
		}

		INSTANTIATE_TEST_SUITE_P(
			GmodPathFullPathParsingSuite,
			GmodPathFullPathParsingTest,
			::testing::Values(
				FullPathParsingTestData{ "411.1/C101.72/I101", "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101", VisVersion::v3_4a },
				FullPathParsingTestData{ "612.21-1/C701.13/S93", "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93", VisVersion::v3_4a } ) );

		//----------------------------------------------
		// Test_IndividualizableSets
		//----------------------------------------------

		class GmodPathIndividualizableSetsTest : public ::testing::TestWithParam<IndividualizableSetsTestData>
		{
		protected:
			GmodPathIndividualizableSetsTest() : m_vis( VIS::instance() )
			{
			}

			VIS& m_vis;
		};

		TEST_P( GmodPathIndividualizableSetsTest, Test_IndividualizableSets )
		{
			const auto& param = GetParam();
			VisVersion version = VisVersionExtensions::parse( param.visVersionString );
			const Gmod& gmod = VIS::instance().gmod( version );

			if ( !param.expected.has_value() )
			{
				std::optional<GmodPath> parsedPathOptional;
				bool success;
				if ( param.isFullPath )
				{
					success = gmod.tryParseFromFullPath( param.path, parsedPathOptional );
				}
				else
				{
					success = gmod.tryParsePath( param.path, parsedPathOptional );
				}
				ASSERT_FALSE( success );
				ASSERT_FALSE( parsedPathOptional.has_value() );
				return;
			}

			GmodPath path;
			if ( param.isFullPath )
			{
				path = gmod.parseFromFullPath( param.path );
			}
			else
			{
				path = gmod.parsePath( param.path );
			}

			std::vector<GmodIndividualizableSet> sets = path.individualizableSets();
			const auto& expectedOuterVector = param.expected.value();

			ASSERT_EQ( expectedOuterVector.size(), sets.size() );
			for ( size_t i = 0; i < expectedOuterVector.size(); ++i )
			{
				const auto& expectedInnerVector = expectedOuterVector[i];
				const auto& actualNodes = sets[i].nodes();
				ASSERT_EQ( expectedInnerVector.size(), actualNodes.size() );
				for ( size_t j = 0; j < expectedInnerVector.size(); ++j )
				{
					ASSERT_EQ( expectedInnerVector[j], actualNodes[j]->code().data() );
				}
			}
		}

		INSTANTIATE_TEST_SUITE_P(
			GmodPathIndividualizableSetsTestSuite,
			GmodPathIndividualizableSetsTest,
			::testing::ValuesIn( LoadIndividualizableSetsData() ) );

		//----------------------------------------------
		// Test_IndividualizableSets_FullPath
		//----------------------------------------------

		class GmodPathIndividualizableSetsFullPathTest : public ::testing::TestWithParam<IndividualizableSetsTestData>
		{
		protected:
			GmodPathIndividualizableSetsFullPathTest() : m_vis( VIS::instance() )
			{
			}

			VIS& m_vis;
		};

		TEST_P( GmodPathIndividualizableSetsFullPathTest, Test_IndividualizableSets_FullPath )
		{
			const auto& param = GetParam();
			VisVersion version = VisVersionExtensions::parse( param.visVersionString );
			const Gmod& gmod = VIS::instance().gmod( version );

			if ( param.isFullPath )
			{
				return;
			}

			if ( !param.expected.has_value() )
			{
				std::optional<GmodPath> parsedPathOptional;
				bool success = gmod.tryParsePath( param.path, parsedPathOptional );
				ASSERT_FALSE( success );
				ASSERT_FALSE( parsedPathOptional.has_value() );
				return;
			}

			GmodPath shortPath = gmod.parsePath( param.path );
			std::string fullPathString = shortPath.toFullPathString();
			GmodPath path = GmodPath::parseFullPath( fullPathString, version );

			std::vector<GmodIndividualizableSet> sets = path.individualizableSets();
			const auto& expectedOuterVector = param.expected.value();

			ASSERT_EQ( expectedOuterVector.size(), sets.size() );
			for ( size_t i = 0; i < expectedOuterVector.size(); ++i )
			{
				const auto& expectedInnerVector = expectedOuterVector[i];
				const auto& actualNodes = sets[i].nodes();
				ASSERT_EQ( expectedInnerVector.size(), actualNodes.size() );
				for ( size_t j = 0; j < expectedInnerVector.size(); ++j )
				{
					ASSERT_EQ( expectedInnerVector[j], actualNodes[j]->code().data() );
				}
			}
		}

		INSTANTIATE_TEST_SUITE_P(
			GmodPathIndividualizableSetsFullPathTestSuite,
			GmodPathIndividualizableSetsFullPathTest,
			::testing::ValuesIn( LoadIndividualizableSetsData() ) );

		//----------------------------------------------
		// Test_Valid_GmodPath_IndividualizableSets
		//----------------------------------------------

		class GmodPathValidIndividualizableSetsTest : public ::testing::TestWithParam<GmodPathParseValidParam>
		{
		protected:
			GmodPathValidIndividualizableSetsTest() : m_vis( VIS::instance() )
			{
			}

			VIS& m_vis;
		};

		TEST_P( GmodPathValidIndividualizableSetsTest, Test_Valid_GmodPath_IndividualizableSets )
		{
			const auto& param = GetParam();
			VisVersion version = VisVersionExtensions::parse( param.visVersionString );
			const std::string& inputPath = param.pathString;

			const Gmod& gmod = VIS::instance().gmod( version );

			GmodPath path = gmod.parsePath( inputPath );
			std::vector<GmodIndividualizableSet> sets = path.individualizableSets();

			std::set<std::string> uniqueCodes;
			for ( const auto& set : sets )
			{
				for ( const auto& node : set.nodes() )
				{
					ASSERT_NE( node, nullptr );
					auto insertResult = uniqueCodes.insert( std::string( node->code().data() ) );
					ASSERT_TRUE( insertResult.second );
				}
			}
		}

		INSTANTIATE_TEST_SUITE_P(
			GmodPathValidIndividualizableSetsTestSuite,
			GmodPathValidIndividualizableSetsTest,
			::testing::ValuesIn( loadValidGmodPathData() ) );

		//----------------------------------------------
		// Test_Valid_GmodPath_IndividualizableSets_FullPath
		//----------------------------------------------

		class GmodPathValidIndividualizableSetsFullPathTest : public ::testing::TestWithParam<GmodPathParseValidParam>
		{
		};

		TEST_P( GmodPathValidIndividualizableSetsFullPathTest, Test_Valid_GmodPath_IndividualizableSets_FullPath )
		{
			const auto& param = GetParam();
			VisVersion version = VisVersionExtensions::parse( param.visVersionString );
			const std::string& inputPath = param.pathString;

			const Gmod& gmod = VIS::instance().gmod( version );

			GmodPath shortPath = gmod.parsePath( inputPath );
			std::string fullPathString = shortPath.toFullPathString();
			GmodPath path = GmodPath::parseFullPath( fullPathString, version );

			std::vector<GmodIndividualizableSet> sets = path.individualizableSets();

			std::set<std::string> uniqueCodes;
			for ( const auto& set : sets )
			{
				for ( const auto& node : set.nodes() )
				{
					ASSERT_NE( node, nullptr );
					auto insertResult = uniqueCodes.insert( std::string( node->code().data() ) );
					ASSERT_TRUE( insertResult.second );
				}
			}
		}

		INSTANTIATE_TEST_SUITE_P(
			GmodPathValidIndividualizableSetsFullPathSuite,
			GmodPathValidIndividualizableSetsFullPathTest,
			::testing::ValuesIn( loadValidGmodPathData() ) );
	}

	namespace Tests
	{
		//----------------------------------------------
		// Test_GetFullPath
		//----------------------------------------------

		TEST( GmodPathGetFullPathTest, Test_GetFullPath )
		{
			const VisVersion visVersion = VisVersion::v3_4a;
			const std::string pathStr = "411.1/C101.72/I101";

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
				{ 10, "I101" },
			};

			std::optional<GmodPath> parsedGmodPathOptional;
			bool parsed = GmodPath::tryParse( pathStr, visVersion, parsedGmodPathOptional );
			ASSERT_TRUE( parsed );
			ASSERT_TRUE( parsedGmodPathOptional.has_value() );

			GmodPath path = parsedGmodPathOptional.value();

			std::set<size_t> seenDepths;
			size_t iterationCount = 0;

			auto enumerator = path.enumerator();
			while ( enumerator.next() )
			{
				const auto& [depth, node] = enumerator.current();

				ASSERT_NE( node, nullptr );

				auto insertResult = seenDepths.insert( depth );
				ASSERT_TRUE( insertResult.second ) << "Got same depth twice: " << depth;

				if ( iterationCount == 0 )
				{
					ASSERT_EQ( 0, depth );
				}
				ASSERT_EQ( expectation[depth], node->code().data() );
				iterationCount++;
			}

			ASSERT_EQ( expectation.size(), seenDepths.size() );
			for ( const auto& expectedPair : expectation )
			{
				ASSERT_TRUE( seenDepths.count( expectedPair.first ) ) << "Expected depth not seen: " << expectedPair.first;
			}
		}

		//----------------------------------------------
		// Test_GetFullPathFrom
		//----------------------------------------------

		TEST( GmodPathGetFullPathTest, Test_GetFullPathFrom )
		{
			const VisVersion visVersion = VisVersion::v3_4a;
			const std::string pathStr = "411.1/C101.72/I101";
			const size_t startDepth = 4;

			std::map<size_t, std::string> expectation = {
				{ 4, "411i" },
				{ 5, "411.1" },
				{ 6, "CS1" },
				{ 7, "C101" },
				{ 8, "C101.7" },
				{ 9, "C101.72" },
				{ 10, "I101" },
			};

			std::optional<GmodPath> parsedGmodPathOptional;
			bool parsed = GmodPath::tryParse( pathStr, visVersion, parsedGmodPathOptional );
			ASSERT_TRUE( parsed );
			ASSERT_TRUE( parsedGmodPathOptional.has_value() );

			GmodPath path = parsedGmodPathOptional.value();

			std::set<size_t> seenDepths;
			size_t iterationCount = 0;

			auto enumerator = path.enumerator( startDepth );
			while ( enumerator.next() )
			{
				const auto& [depth, node] = enumerator.current();

				ASSERT_NE( node, nullptr );

				auto insertResult = seenDepths.insert( depth );
				ASSERT_TRUE( insertResult.second ) << "Got same depth twice: " << depth;

				if ( iterationCount == 0 )
				{
					ASSERT_EQ( startDepth, depth );
				}
				ASSERT_EQ( expectation.at( depth ), node->code().data() );
				iterationCount++;
			}

			ASSERT_EQ( expectation.size(), seenDepths.size() );
			for ( const auto& expectedPair : expectation )
			{
				ASSERT_TRUE( seenDepths.count( expectedPair.first ) ) << "Expected depth not seen: " << expectedPair.first;
			}
		}

		//----------------------------------------------
		// Test_GmodPath_Does_Not_Individualize
		//----------------------------------------------

		TEST( GmodPathTest, Test_GmodPath_Does_Not_Individualize )
		{
			VisVersion version = VisVersion::v3_7a;
			const Gmod& gmod = VIS::instance().gmod( version );
			std::optional<GmodPath> pathOptional;
			bool parsed = gmod.tryParsePath( "500a-1", pathOptional );
			ASSERT_FALSE( parsed );
			ASSERT_FALSE( pathOptional.has_value() );
		}

		//----------------------------------------------
		// Test_ToFullPathString
		//----------------------------------------------

		TEST( GmodPathTest, Test_ToFullPathString )
		{
			VisVersion version = VisVersion::v3_7a;
			const Gmod& gmod = VIS::instance().gmod( version );

			GmodPath path = gmod.parsePath( "511.11-1/C101.663i-1/C663" );
			ASSERT_EQ(
				"VE/500a/510/511/511.1/511.1i-1/511.11-1/CS1/C101/C101.6/C101.66/C101.663/C101.663i-1/C663",
				path.toFullPathString() );

			path = gmod.parsePath( "846/G203.32-2/S110.2-1/E31" );
			ASSERT_EQ(
				"VE/800a/840/846/G203/G203.3-2/G203.32-2/S110/S110.2-1/CS1/E31",
				path.toFullPathString() );
		}
	}
}
