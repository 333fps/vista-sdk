#include "pch.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodDto.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/GmodPath.h"

namespace dnv::vista::sdk::tests
{
	struct ExpectedValues
	{
		std::string MaxCode;
		int NodeCount;
	};

	const std::map<VisVersion, ExpectedValues> ExpectedMaxes = {
		{ VisVersion::v3_4a, { "C1053.3112", 6420 } },
		{ VisVersion::v3_5a, { "C1053.3112", 6557 } },
		{ VisVersion::v3_6a, { "C1053.3112", 6557 } },
		{ VisVersion::v3_7a, { "H346.11113", 6672 } },
		{ VisVersion::v3_8a, { "H346.11112", 6335 } } };

	struct TraversalState
	{
		TraversalState( int stopAfter ) : StopAfter( stopAfter ), NodeCount( 0 ) {}
		int StopAfter;
		int NodeCount;
	};

	std::size_t occurrences( const std::vector<GmodNode>& parents, const GmodNode& node )
	{
		return static_cast<size_t>(
			std::count_if( parents.begin(), parents.end(),
				[&node]( const GmodNode& p ) {
					return p.code() == node.code();
				} ) );
	}

	class GmodTests : public ::testing::TestWithParam<VisVersion>
	{
	protected:
		static std::pair<VIS&, Gmod> visAndGmod( VisVersion visVersion )
		{
			VIS& vis = VIS::instance();
			Gmod gmod = vis.gmod( visVersion );
			return { vis, gmod };
		}
	};

	TEST_P( GmodTests, Test_Gmod_Loads )
	{
		auto visVersion = GetParam();
		auto [vis, gmod] = visAndGmod( visVersion );

		GmodNode tempNode;
		ASSERT_TRUE( gmod.tryGetNode( std::string( "400a" ), tempNode ) ) << "Node '400a' not found in GMOD.";
	}

	TEST_P( GmodTests, Test_Gmod_Properties )
	{
		auto visVersion = GetParam();
		auto [vis, gmod] = visAndGmod( visVersion );

		auto expectedIt = ExpectedMaxes.find( visVersion );
		ASSERT_NE( expectedIt, ExpectedMaxes.end() );
		std::string expectedMaxCode = expectedIt->second.MaxCode;

		const GmodNode* minLength = nullptr;
		const GmodNode* maxLength = nullptr;
		const GmodNode* expectedMax = nullptr;

		int nodeCount = 0;
		for ( auto nodeIt = gmod.begin(); nodeIt != gmod.end(); ++nodeIt )
		{
			const auto& node = *nodeIt;
			nodeCount++;

			if ( !node.code().empty() )
			{
				if ( minLength == nullptr || node.code().length() < minLength->code().length() )
					minLength = &node;

				if ( maxLength == nullptr || node.code().length() > maxLength->code().length() )
					maxLength = &node;

				if ( node.code() == expectedMaxCode )
					expectedMax = &node;
			}
		}

		SPDLOG_INFO( "Found min length node: {}", minLength ? minLength->code() : "null" );
		SPDLOG_INFO( "Found max length node: {}", maxLength ? maxLength->code() : "null" );

		if ( expectedMax )
		{
			SPDLOG_INFO( "Using expected max node: {} (length {})",
				expectedMax->code(), expectedMax->code().length() );
			ASSERT_EQ( expectedMax->code().length(), 10 );
			maxLength = expectedMax;
		}
		else
		{
			SPDLOG_ERROR( "Expected max node {} not found!", expectedMaxCode );
		}

		ASSERT_NE( minLength, nullptr );
		ASSERT_NE( maxLength, nullptr );
		EXPECT_EQ( minLength->code().length(), 2 );
		EXPECT_EQ( minLength->code(), "VE" );
		EXPECT_EQ( maxLength->code().length(), 10 );
		EXPECT_EQ( maxLength->code(), expectedMaxCode );

		EXPECT_EQ( nodeCount, expectedIt->second.NodeCount );
	}

	TEST_P( GmodTests, Test_Gmod_Lookup )
	{
		auto visVersion = GetParam();
		auto [vis, gmod] = visAndGmod( visVersion );

		auto gmodDto = vis.gmodDto( visVersion );

		{
			std::unordered_set<std::string> seen;
			int counter = 0;

			ASSERT_FALSE( gmodDto.items().empty() );

			for ( const auto& item : gmodDto.items() )
			{
				ASSERT_FALSE( item.code().empty() );
				auto insertResult = seen.insert( item.code() );
				EXPECT_TRUE( insertResult.second ) << "Code: " << item.code();

				GmodNode foundNode;
				ASSERT_TRUE( gmod.tryGetNode( item.code(), foundNode ) );
				EXPECT_EQ( item.code(), foundNode.code() );
				counter++;
			}
		}

		{
			std::unordered_set<std::string> seen;
			int counter = 0;
			for ( auto it = gmod.begin(); it != gmod.end(); ++it )
			{
				const auto& node = *it;
				ASSERT_FALSE( node.code().empty() );
				auto insertResult = seen.insert( node.code() );
				EXPECT_TRUE( insertResult.second ) << "Code: " << node.code();

				GmodNode foundNode;
				ASSERT_TRUE( gmod.tryGetNode( node.code(), foundNode ) );
				EXPECT_EQ( node.code(), foundNode.code() );
				counter++;
			}
		}

		GmodNode notFoundNode;
		EXPECT_FALSE( gmod.tryGetNode( std::string( "ABC" ), notFoundNode ) );
		EXPECT_FALSE( gmod.tryGetNode( std::string( "" ), notFoundNode ) );
		EXPECT_FALSE( gmod.tryGetNode( std::string( "SDFASDFSDAFb" ), notFoundNode ) );
		EXPECT_FALSE( gmod.tryGetNode( std::string( "✅" ), notFoundNode ) );
		EXPECT_FALSE( gmod.tryGetNode( std::string( "a✅b" ), notFoundNode ) );
		EXPECT_FALSE( gmod.tryGetNode( std::string( "ac✅bc" ), notFoundNode ) );
		EXPECT_FALSE( gmod.tryGetNode( std::string( "✅bc" ), notFoundNode ) );
		EXPECT_FALSE( gmod.tryGetNode( std::string( "a✅" ), notFoundNode ) );
		EXPECT_FALSE( gmod.tryGetNode( std::string( "ag✅" ), notFoundNode ) );
	}

	TEST_F( GmodTests, Test_Gmod_Node_Equality )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

		const auto& node1 = gmod["400a"];
		const auto& node2 = gmod["400a"];

		EXPECT_EQ( node1, node2 );
		EXPECT_EQ( &node1, &node2 );

		auto node3 = node1.withLocation( "1" );
		EXPECT_NE( node1, node3 );
		EXPECT_NE( &node1, &node3 );
	}

	TEST_F( GmodTests, Test_Gmod_Node_Types )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

		std::unordered_set<std::string> types;
		for ( auto it = gmod.begin(); it != gmod.end(); ++it )
		{
			const auto& node = *it;
			types.insert( node.metadata().category() + " | " + node.metadata().type() );
		}

		EXPECT_FALSE( types.empty() );
	}

	TEST_P( GmodTests, Test_Gmod_RootNode_Children )
	{
		auto visVersion = GetParam();
		auto [vis, gmod] = visAndGmod( visVersion );

		const auto& node = gmod.rootNode();
		EXPECT_FALSE( node.children().empty() );
	}

	TEST_F( GmodTests, Test_Normal_Assignments )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

		auto node = gmod["411.3"];
		EXPECT_TRUE( node.productType() != nullptr );
		EXPECT_TRUE( node.productSelection() == nullptr );

		node = gmod["H601"];
		EXPECT_TRUE( node.productType() == nullptr );
	}

	TEST_F( GmodTests, Test_Node_With_Product_Selection )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

		auto node = gmod["411.2"];
		EXPECT_TRUE( node.productSelection() != nullptr );
		EXPECT_TRUE( node.productType() == nullptr );

		node = gmod["H601"];
		EXPECT_TRUE( node.productSelection() == nullptr );
	}

	TEST_F( GmodTests, Test_Product_Selection )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

		auto node = gmod["CS1"];
		EXPECT_TRUE( node.isProductSelection() );
	}

	struct MappabilityTestCase
	{
		std::string Code;
		bool Mappable;
	};

	class MappabilityTests : public ::testing::TestWithParam<MappabilityTestCase>
	{
	};

	TEST_P( MappabilityTests, Test_Mappability )
	{
		try
		{
			const auto& testCase = GetParam();

			auto& vis = VIS::instance();
			auto gmod = vis.gmod( VisVersion::v3_4a );

			GmodNode node;
			if ( !gmod.tryGetNode( testCase.Code, node ) )
			{
				FAIL() << "Node '" << testCase.Code << "' not found.";
				return;
			}

			EXPECT_EQ( node.isMappable(), testCase.Mappable )
				<< "Mappability mismatch for node '" << testCase.Code << "'.";
		}
		catch ( const std::exception& ex )
		{
			FAIL() << "Exception during mappability test: " << ex.what();
		}
		catch ( ... )
		{
			FAIL() << "Unknown exception during mappability test";
		}
	}

	INSTANTIATE_TEST_SUITE_P(
		GmodTests,
		MappabilityTests,
		::testing::Values(
			MappabilityTestCase{ "VE", false },
			MappabilityTestCase{ "300a", false },
			MappabilityTestCase{ "300", true },
			MappabilityTestCase{ "411", true },
			MappabilityTestCase{ "410", true },
			MappabilityTestCase{ "651.21s", false },
			MappabilityTestCase{ "924.2", true },
			MappabilityTestCase{ "411.1", false },
			MappabilityTestCase{ "C101", true },
			MappabilityTestCase{ "CS1", false },
			MappabilityTestCase{ "C101.663", true },
			MappabilityTestCase{ "C101.4", true },
			MappabilityTestCase{ "C101.21s", false },
			MappabilityTestCase{ "F201.11", true },
			MappabilityTestCase{ "C101.211", false } ) );

	TEST_F( GmodTests, Test_Full_Traversal )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

		int pathCount = 0;
		const int maxExpected = Gmod::TraversalOptions::DEFAULT_MAX_TRAVERSAL_OCCURRENCE;
		size_t maxOccurrence = 0;

		bool completed = gmod.traverse(
			[&]( const std::vector<GmodNode>& parents, const GmodNode& node ) {
				EXPECT_TRUE( parents.empty() || parents[0].isRoot() );

				if ( std::any_of( parents.begin(), parents.end(),
						 []( const auto& p ) { return p.code() == "HG3"; } ) ||
					 node.code() == "HG3" )
				{
					pathCount++;
				}

				bool skipOccurrenceCheck = Gmod::isProductSelectionAssignment(
					parents.empty() ? nullptr : &parents.back(), &node );

				if ( skipOccurrenceCheck )
					return Gmod::TraversalHandlerResult::Continue;

				size_t occ = occurrences( parents, node );
				if ( occ > maxOccurrence )
					maxOccurrence = occ;

				return Gmod::TraversalHandlerResult::Continue;
			} );

		EXPECT_EQ( maxExpected, maxOccurrence );
		EXPECT_TRUE( completed );
		EXPECT_GT( pathCount, 0 );
	}

	TEST_F( GmodTests, Test_Full_Traversal_With_Options )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

		const int maxExpected = 2;
		size_t maxOccurrence = 0;

		Gmod::TraversalOptions options;
		options.maxTraversalOccurrence = maxExpected;

		bool completed = gmod.traverse(
			[&]( const std::vector<GmodNode>& parents, const GmodNode& node ) {
				bool skipOccurrenceCheck = Gmod::isProductSelectionAssignment(
					parents.empty() ? nullptr : &parents.back(), &node );

				if ( skipOccurrenceCheck )
					return Gmod::TraversalHandlerResult::Continue;

				size_t occ = occurrences( parents, node );
				if ( occ > maxOccurrence )
					maxOccurrence = occ;

				return Gmod::TraversalHandlerResult::Continue;
			},
			options );

		EXPECT_EQ( maxExpected, maxOccurrence );
		EXPECT_TRUE( completed );
	}

	TEST_F( GmodTests, Test_Partial_Traversal )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

		TraversalState state( 5 );

		bool completed = gmod.traverse(
			[&state]( const std::vector<GmodNode>& parents, [[maybe_unused]] const GmodNode& node ) {
				EXPECT_TRUE( parents.empty() || parents[0].isRoot() );
				if ( ++state.NodeCount == state.StopAfter )
					return Gmod::TraversalHandlerResult::Stop;
				return Gmod::TraversalHandlerResult::Continue;
			} );

		EXPECT_EQ( state.StopAfter, state.NodeCount );
		EXPECT_FALSE( completed );
	}

	TEST_F( GmodTests, Test_Full_Traversal_From )
	{
		auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

		TraversalState state( 0 );
		GmodNode startNode = gmod["400a"];

		bool completed = gmod.traverse<TraversalState>(
			state,
			startNode,
			[]( TraversalState& state, const std::vector<GmodNode>& parents, const GmodNode& node ) -> Gmod::TraversalHandlerResult {
				(void)node;
				EXPECT_TRUE( parents.empty() || parents[0].code() == "400a" );
				++state.NodeCount;
				return Gmod::TraversalHandlerResult::Continue;
			} );

		EXPECT_TRUE( completed );
		EXPECT_GT( state.NodeCount, 0 );
	}

	INSTANTIATE_TEST_SUITE_P(
		GmodTestSuite,
		GmodTests,
		::testing::Values(
			VisVersion::v3_4a,
			VisVersion::v3_5a,
			VisVersion::v3_6a,
			VisVersion::v3_7a,
			VisVersion::v3_8a ) );
}
