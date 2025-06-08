/**
 * @file GmodVersioningTests.cpp
 * @brief Unit tests for the GmodVersioning class.
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodTraversal.h"
#include "dnv/vista/sdk/GmodVersioning.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk::tests
{
	//=====================================================================
	// TEST_F
	//=====================================================================

	//----------------------------------------------
	// ConvertLocalId
	//----------------------------------------------

	class GmodVersioningTest : public ::testing::Test
	{
	protected:
		virtual void SetUp() override
		{
			try
			{
				m_vis = &VIS::instance();

				try
				{
					auto versioningData = m_vis->gmodVersioningDto();
					m_gmodVersioning = std::make_unique<GmodVersioning>( versioningData );
				}
				catch ( [[maybe_unused]] const std::exception& ex )
				{
					std::unordered_map<std::string, GmodVersioningDto> emptyDto;
					m_gmodVersioning = std::make_unique<GmodVersioning>( emptyDto );
				}

				m_gmod_v3_4a = &m_vis->gmod( VisVersion::v3_4a );
				m_gmod_v3_6a = &m_vis->gmod( VisVersion::v3_6a );

				ASSERT_NE( m_gmod_v3_4a, nullptr );
				ASSERT_NE( m_gmod_v3_6a, nullptr );

				m_setupSuccess = true;
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "Test setup failed: {}", ex.what() );
				m_setupSuccess = false;
			}
		}

		virtual void TearDown() override
		{
		}

		VIS* m_vis = nullptr;
		std::unique_ptr<GmodVersioning> m_gmodVersioning;
		const Gmod* m_gmod_v3_4a = nullptr;
		const Gmod* m_gmod_v3_6a = nullptr;
		bool m_setupSuccess = false;
	};

	TEST_F( GmodVersioningTest, ConvertLocalId )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		std::string sourceLocalIdStr = "/dnv-v2/vis-3-4a/411.1/C101/sec/411.1/C101.64i/S201/meta/cnt-condensate";
		std::string targetLocalIdStr = "/dnv-v2/vis-3-5a/411.1/C101/sec/411.1/C101.64/S201/meta/cnt-condensate";

		ParsingErrors errors;
		std::optional<LocalIdBuilder> sourceLocalId;
		std::optional<LocalIdBuilder> targetLocalId;

		ASSERT_TRUE( LocalIdBuilder::tryParse( sourceLocalIdStr, errors, sourceLocalId ) );
		ASSERT_TRUE( LocalIdBuilder::tryParse( targetLocalIdStr, errors, targetLocalId ) );

		auto convertedLocalId = m_vis->convertLocalId( *sourceLocalId, VisVersion::v3_5a );
		ASSERT_TRUE( convertedLocalId.has_value() );
		EXPECT_EQ( *targetLocalId, *convertedLocalId );
		EXPECT_EQ( targetLocalIdStr, convertedLocalId->toString() );
	}

	//----------------------------------------------
	// Test_Finds_Path
	//----------------------------------------------

	TEST_F( GmodVersioningTest, Test_Finds_Path )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		const auto& gmod = m_vis->gmod( VisVersion::v3_4a );

		struct PathState
		{
			const Gmod& gmod;
			const std::string targetPath = "1012.22/S201.1/C151.2/S110.2/C101.61/S203.2/S101";

			PathState( const Gmod& g ) : gmod( g ) {}
		};

		PathState state( gmod );

		TraverseHandlerWithState<PathState> handler =
			[]( PathState& state, const std::vector<const GmodNode*>& parents, const GmodNode& node ) -> TraversalHandlerResult {
			if ( parents.empty() )
				return TraversalHandlerResult::Continue;

			std::vector<GmodNode*> nonConstParents;
			nonConstParents.reserve( parents.size() );
			for ( const GmodNode* p : parents )
				nonConstParents.push_back( const_cast<GmodNode*>( p ) );

			GmodPath path( state.gmod, const_cast<GmodNode*>( &node ), nonConstParents );
			if ( path.toString() == state.targetPath )
				return TraversalHandlerResult::Stop;

			return TraversalHandlerResult::Continue;
		};

		auto completed = GmodTraversal::traverse( gmod, state, handler );

		ASSERT_FALSE( completed );
	}

	//----------------------------------------------
	// Test_One_Path_To_Root_For_Asset_Functions
	//----------------------------------------------

	namespace
	{
		bool onePathToRoot( const dnv::vista::sdk::GmodNode& node )
		{
			if ( node.isRoot() )
			{
				return true;
			}
			if ( node.parents().empty() )
			{
				return false;
			}
			return node.parents().size() == 1 && onePathToRoot( *node.parents()[0] );
		}
	}

	TEST_F( GmodVersioningTest, Test_One_Path_To_Root_For_Asset_Functions )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		for ( const auto& version : dnv::vista::sdk::VisVersionExtensions::allVersions() )
		{
			if ( version == dnv::vista::sdk::VisVersion::Unknown )
				continue;

			const auto& gmod = m_vis->gmod( version );

			auto enumerator = gmod.enumerator();
			while ( enumerator.next() )
			{
				const auto& node = enumerator.current();
				if ( !dnv::vista::sdk::Gmod::isAssetFunctionNode( node.metadata() ) )
				{
					continue;
				}
				ASSERT_TRUE( onePathToRoot( node ) )
					<< "Node " << node.code() << " in GMOD " << dnv::vista::sdk::VisVersionExtensions::toVersionString( version )
					<< " is an asset function node but does not have one path to root.";
			}
		}
	}

	//----------------------------------------------
	// ConvertEveryNodeToLatest
	//----------------------------------------------

	TEST_F( GmodVersioningTest, ConvertEveryNodeToLatest )
	{
		GTEST_SKIP() << "3-8 S204 is not in 3-8a";

		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		std::vector<VisVersion> visVersionsToTest = { VisVersion::v3_7a };
		const VisVersion latestVisVersion = VisVersion::v3_8a;

		std::unordered_map<VisVersion, std::vector<std::string>> errored;

		for ( const auto& sourceVersion : visVersionsToTest )
		{
			const auto& gmod = m_vis->gmod( sourceVersion );

			std::vector<std::string>& currentVersionErrors = errored[sourceVersion];

			auto enumerator = gmod.enumerator();
			while ( enumerator.next() )
			{
				const auto& node = enumerator.current();
				auto targetNodeOpt = m_vis->convertNode( sourceVersion, node, latestVisVersion );
				if ( !targetNodeOpt.has_value() )
				{
					currentVersionErrors.push_back( node.code() );
				}
			}
		}

		for ( const auto& pair : errored )
		{
			EXPECT_TRUE( pair.second.empty() ) << "Failed to convert some nodes from version "
											   << dnv::vista::sdk::VisVersionExtensions::toVersionString( pair.first )
											   << " to latest. Error count: " << pair.second.size();
			if ( !pair.second.empty() )
			{
				std::string errorNodes;
				for ( const auto& code : pair.second )
				{
					errorNodes += code + ", ";
				}

				SPDLOG_ERROR( "Failed nodes for {}: {}", dnv::vista::sdk::VisVersionExtensions::toVersionString( pair.first ), errorNodes );
			}
		}
	}

	//=====================================================================
	// TEST_P
	//=====================================================================

	//----------------------------------------------
	// Test_GmodVersioning_ConvertPath
	//----------------------------------------------

	struct PathTestData
	{
		std::string inputPath;
		std::string expectedPath;
		VisVersion sourceVersion;
		VisVersion targetVersion;

		PathTestData( const std::string& input, const std::string& expected,
			VisVersion source = VisVersion::v3_4a, VisVersion target = VisVersion::v3_6a )
			: inputPath( input ), expectedPath( expected ), sourceVersion( source ), targetVersion( target )
		{
		}
	};

	std::vector<PathTestData> validPathTestData()
	{
		return {
			{ "411.1/C101.72/I101", "411.1/C101.72/I101" },
			{ "323.51/H362.1", "323.61/H362.1" },
			{ "321.38/C906", "321.39/C906" },
			{ "511.331/C221", "511.31/C121.31/C221" },
			{ "511.11/C101.663i/C663.5/CS6d", "511.11/C101.663i/C663.6/CS6d" },
			{ "001", "001" },
			{ "038.7/F101.2/F71", "038.7/F101.2/F71" },
			{ "000a", "000a" },
			{ "1012.21/C1147.221/C1051.7/C101.61/S203.2/S101", "1012.21/C1147.221/C1051.7/C101.61/S203.3/S110.1/S101" },
			{ "1012.22/S201.1/C151.2/S110.2/C101.64i", "1012.22/S201.1/C151.2/S110.2/C101.64" },
			{ "632.32i/S110.2/C111.42/G203.31/S90.5/C401", "632.32i/S110.2/C111.42/G203.31/S90.5/C401" },
			{ "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
				"864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51" },
			{ "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401", "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401" },
			{ "221.31/C1141.41/C664.2/C471", "221.31/C1141.41/C664.2/C471" },
			{ "514/E15", "514" },
			{ "244.1i/H101.111/H401", "244.1i/H101.11/H407.1/H401", VisVersion::v3_7a, VisVersion::v3_8a },
			{ "1346/S201.1/C151.31/S110.2/C111.1/C109.16/C509", "1346/S201.1/C151.31/S110.2/C111.1/C109.126/C509", VisVersion::v3_7a, VisVersion::v3_8a },
			{ "1012.21/C1147.221/C1051.7/C101.61/S203.6", "1012.21/C1147.221/C1051.7/C101.311/C467.5" },
			{ "1012.21/C1147.221/C1051.7/C101.61/S203.6/S61", "1012.21/C1147.221/C1051.7/C101.311/C467.5/S61" },
			{ "1012.21/C1147.221/C1051.7/C101.22", "1012.21/C1147.221/C1051.7/C101.93" },
			{ "1012.21/C1147.221/C1051.7/C101.661i/C624", "1012.21/C1147.221/C1051.7/C101.661i/C621" },
			{ "511.11/C101.663i/C663.5/CS6d", "511.11/C101.663i/C663.6/CS6d" },
			{ "511.11-1/C101.663i/C663.5/CS6d", "511.11-1/C101.663i/C663.6/CS6d" } };
	}

	class PathConversionTest : public ::testing::TestWithParam<PathTestData>
	{
	};

	TEST_P( PathConversionTest, Test_GmodVersioning_ConvertPath )
	{
		auto testData = GetParam();
		auto& vis = VIS::instance();
		const auto& sourceGmod = vis.gmod( testData.sourceVersion );
		const auto& targetGmod = vis.gmod( testData.targetVersion );

		std::optional<GmodPath> sourcePathOpt;

		auto res = sourceGmod.tryParsePath( testData.inputPath, sourcePathOpt );

		ASSERT_TRUE( res );
		ASSERT_TRUE( sourcePathOpt.has_value() );

		std::optional<GmodPath> parsedTargetPathOpt;
		bool parsedExpectedPath = targetGmod.tryParsePath( testData.expectedPath, parsedTargetPathOpt );

		auto targetPath = vis.convertPath( testData.sourceVersion, *sourcePathOpt, testData.targetVersion );

		EXPECT_EQ( testData.inputPath, sourcePathOpt->toString() );

		EXPECT_TRUE( parsedExpectedPath );
		ASSERT_TRUE( parsedTargetPathOpt.has_value() );
		EXPECT_EQ( testData.expectedPath, parsedTargetPathOpt->toString() );

		ASSERT_TRUE( targetPath.has_value() );
		EXPECT_EQ( testData.expectedPath, targetPath->toString() );
	}

	INSTANTIATE_TEST_SUITE_P(
		ValidPathTests,
		PathConversionTest,
		::testing::ValuesIn( validPathTestData() ) );

	//----------------------------------------------
	// Test_GmodVersioning_ConvertFullPath
	//----------------------------------------------

	struct FullPathTestData
	{
		std::string inputPath;
		std::string expectedPath;
		VisVersion sourceVersion;
		VisVersion targetVersion;

		FullPathTestData( const std::string& input, const std::string& expected,
			VisVersion source = VisVersion::v3_4a, VisVersion target = VisVersion::v3_6a )
			: inputPath( input ), expectedPath( expected ), sourceVersion( source ), targetVersion( target )
		{
		}
	};

	std::vector<FullPathTestData> validFullPathTestData()
	{
		return { { "VE/600a/630/632/632.3/632.32/632.32i-2/S110",
			"VE/600a/630/632/632.3/632.32/632.32i-2/SS5/S110" } };
	}

	class FullPathConversionTest : public ::testing::TestWithParam<FullPathTestData>
	{
	};

	TEST_P( FullPathConversionTest, Test_GmodVersioning_ConvertFullPath )
	{
		auto testData = GetParam();
		auto& vis = VIS::instance();
		const auto& sourceGmod = vis.gmod( testData.sourceVersion );
		const auto& targetGmod = vis.gmod( testData.targetVersion );

		std::optional<GmodPath> sourcePathOpt;
		ASSERT_TRUE( sourceGmod.tryParseFromFullPath( testData.inputPath, sourcePathOpt ) );
		ASSERT_TRUE( sourcePathOpt.has_value() );

		std::optional<GmodPath> parsedTargetPathOpt;
		bool parsedPath = targetGmod.tryParseFromFullPath( testData.expectedPath, parsedTargetPathOpt );

		auto targetPath = vis.convertPath( testData.sourceVersion, *sourcePathOpt, testData.targetVersion );

		ASSERT_TRUE( targetPath.has_value() ) << "Path conversion failed for input: " << testData.inputPath;

		EXPECT_EQ( testData.inputPath, sourcePathOpt->toFullPathString() );

		EXPECT_TRUE( parsedPath );
		ASSERT_TRUE( parsedTargetPathOpt.has_value() );
		EXPECT_EQ( testData.expectedPath, parsedTargetPathOpt->toFullPathString() );

		ASSERT_TRUE( targetPath.has_value() );
		EXPECT_EQ( testData.expectedPath, targetPath->toFullPathString() );
	}

	INSTANTIATE_TEST_SUITE_P(
		ValidFullPathTests,
		FullPathConversionTest,
		::testing::ValuesIn( validFullPathTestData() ) );

	//----------------------------------------------
	// Test_GmodVersioning_ConvertNode
	//----------------------------------------------

	struct NodeTestData
	{
		std::string inputCode;
		std::optional<std::string> location;
		std::string expectedCode;

		NodeTestData( const std::string& input, const std::optional<std::string>& loc, const std::string& expected )
			: inputCode( input ), location( loc ), expectedCode( expected )
		{
		}
	};

	std::vector<NodeTestData> validNodeTestData()
	{
		return {
			{ "1014.211", std::nullopt, "1014.211" },
			{ "323.5", std::nullopt, "323.6" },
			{ "412.72", std::nullopt, "412.7i" },
			{ "323.4", std::nullopt, "323.5" },
			{ "323.51", std::nullopt, "323.61" },
			{ "323.6", std::nullopt, "323.7" },
			{ "C101.212", std::nullopt, "C101.22" },
			{ "C101.22", std::nullopt, "C101.93" },
			{ "511.31", std::nullopt, "C121.1" },
			{ "C101.31", "5", "C101.31" } };
	}

	class NodeConversionTest : public ::testing::TestWithParam<NodeTestData>
	{
	};

	TEST_P( NodeConversionTest, Test_GmodVersioning_ConvertNode )
	{
		auto testData = GetParam();
		auto& vis = VIS::instance();

		const auto& gmod = vis.gmod( VisVersion::v3_4a );
		const auto& targetGmod = vis.gmod( VisVersion::v3_6a );

		const GmodNode* sourceNodePtr = nullptr;
		bool foundSource = gmod.tryGetNode( testData.inputCode, sourceNodePtr );
		ASSERT_TRUE( foundSource );
		ASSERT_NE( sourceNodePtr, nullptr );

		GmodNode sourceNode( *sourceNodePtr );

		if ( testData.location.has_value() )
		{
			Location location( testData.location.value() );
			sourceNode = sourceNode.withLocation( location.toString() );
		}

		const GmodNode* expectedNodePtr = nullptr;
		bool foundExpected = targetGmod.tryGetNode( testData.expectedCode, expectedNodePtr );
		ASSERT_TRUE( foundExpected );
		ASSERT_NE( expectedNodePtr, nullptr );

		GmodNode expectedNode( *expectedNodePtr );

		if ( testData.location.has_value() )
		{
			Location location( testData.location.value() );
			expectedNode = expectedNode.withLocation( location.toString() );
		}

		auto targetNodeOpt = vis.convertNode( VisVersion::v3_4a, sourceNode, VisVersion::v3_6a );

		ASSERT_TRUE( targetNodeOpt.has_value() );
		const auto& targetNode = *targetNodeOpt;

		EXPECT_EQ( expectedNode.code(), targetNode.code() );
		EXPECT_EQ( expectedNode.location(), targetNode.location() );
		EXPECT_EQ( expectedNode, targetNode );
	}

	INSTANTIATE_TEST_SUITE_P(
		ValidNodeTests,
		NodeConversionTest,
		::testing::ValuesIn( validNodeTestData() ) );
}
