#include "pch.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/Gmod.h"
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

	std::size_t Occurrences( const std::vector<GmodNode>& parents, const GmodNode& node )
	{
		return std::count_if( parents.begin(), parents.end(),
			[&node]( const GmodNode& p ) { return p.GetCode() == node.GetCode(); } );
	}

	class GmodTests : public ::testing::TestWithParam<VisVersion>
	{
	protected:
		static std::pair<VIS&, Gmod> GetVisAndGmod( VisVersion visVersion )
		{
			VIS& vis = VIS::Instance();
			Gmod gmod = vis.GetGmod( visVersion );
			return { vis, gmod };
		}
	};

	TEST_P( GmodTests, Test_Gmod_Loads )
	{
		auto visVersion = GetParam();
		auto [vis, gmod] = GetVisAndGmod( visVersion );

		GmodNode tempNode;
		ASSERT_TRUE( gmod.TryGetNode( std::string( "400a" ), tempNode ) ) << "Node '400a' not found in GMOD.";
	}

	TEST_P( GmodTests, Test_Gmod_Properties )
	{
		auto visVersion = GetParam();
		auto [vis, gmod] = GetVisAndGmod( visVersion );

		auto it = ExpectedMaxes.find( visVersion );
		ASSERT_NE( it, ExpectedMaxes.end() );
		std::string expectedMaxCode = it->second.MaxCode;

		const GmodNode* minLength = nullptr;
		const GmodNode* maxLength = nullptr;
		const GmodNode* expectedMax = nullptr;

		int nodeCount = 0;
		for ( auto it = gmod.begin(); it != gmod.end(); ++it )
		{
			const auto& node = *it;
			nodeCount++;

			if ( !node.GetCode().empty() )
			{
				if ( minLength == nullptr || node.GetCode().length() < minLength->GetCode().length() )
					minLength = &node;

				if ( maxLength == nullptr || node.GetCode().length() > maxLength->GetCode().length() )
					maxLength = &node;

				if ( node.GetCode() == expectedMaxCode )
					expectedMax = &node;
			}
		}

		SPDLOG_INFO( "Found min length node: {}", minLength ? minLength->GetCode() : "null" );
		SPDLOG_INFO( "Found max length node: {}", maxLength ? maxLength->GetCode() : "null" );

		if ( expectedMax )
		{
			SPDLOG_INFO( "Using expected max node: {} (length {})",
				expectedMax->GetCode(), expectedMax->GetCode().length() );
			ASSERT_EQ( expectedMax->GetCode().length(), 10 );
			maxLength = expectedMax;
		}
		else
		{
			SPDLOG_ERROR( "Expected max node {} not found!", expectedMaxCode );
		}

		ASSERT_NE( minLength, nullptr );
		ASSERT_NE( maxLength, nullptr );
		EXPECT_EQ( minLength->GetCode().length(), 2 );
		EXPECT_EQ( minLength->GetCode(), "VE" );
		EXPECT_EQ( maxLength->GetCode().length(), 10 );
		EXPECT_EQ( maxLength->GetCode(), expectedMaxCode );

		EXPECT_EQ( nodeCount, it->second.NodeCount );
	}

	TEST_F( GmodTests, Test_Gmod_Node_Equality )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		const auto& node1 = gmod["400a"];
		const auto& node2 = gmod["400a"];

		EXPECT_EQ( node1, node2 );
		EXPECT_EQ( &node1, &node2 );

		auto node3 = node1.WithLocation( "1" );
		EXPECT_NE( node1, node3 );
		EXPECT_NE( &node1, &node3 );
	}

	TEST_F( GmodTests, Test_Gmod_Node_Types )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		std::unordered_set<std::string> types;
		for ( auto it = gmod.begin(); it != gmod.end(); ++it )
		{
			const auto& node = *it;
			types.insert( node.GetMetadata().GetCategory() + " | " + node.GetMetadata().GetType() );
		}

		EXPECT_FALSE( types.empty() );
	}

	TEST_P( GmodTests, Test_Gmod_RootNode_Children )
	{
		auto visVersion = GetParam();
		auto [vis, gmod] = GetVisAndGmod( visVersion );

		const auto& node = gmod.GetRootNode();
		EXPECT_FALSE( node.GetChildren().empty() );
	}

	TEST_F( GmodTests, Test_Product_Selection )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		auto node = gmod["CS1"];
		EXPECT_TRUE( node.IsProductSelection() );
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

			auto& vis = VIS::Instance();
			auto gmod = vis.GetGmod( VisVersion::v3_4a );

			GmodNode node;
			if ( !gmod.TryGetNode( testCase.Code, node ) )
			{
				FAIL() << "Node '" << testCase.Code << "' not found.";
				return;
			}

			EXPECT_EQ( node.IsMappable(), testCase.Mappable )
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
			MappabilityTestCase{ "411", true }, //	<NOK
			MappabilityTestCase{ "410", true },
			MappabilityTestCase{ "651.21s", false },
			MappabilityTestCase{ "924.2", true },
			MappabilityTestCase{ "411.1", false }, //	<NOK
			MappabilityTestCase{ "C101", true },
			MappabilityTestCase{ "CS1", false },
			MappabilityTestCase{ "C101.663", true }, //	<NOK
			MappabilityTestCase{ "C101.4", true },
			MappabilityTestCase{ "C101.21s", false },
			MappabilityTestCase{ "F201.11", true },
			MappabilityTestCase{ "C101.211", false } //	<NOK
			) );

	TEST_F( GmodTests, Test_Full_Traversal )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		const int maxExpected = Gmod::TraversalOptions::DEFAULT_MAX_TRAVERSAL_OCCURRENCE;
		int maxOccurrence = 0;

		try
		{
			bool completed = gmod.Traverse(
				[&]( const std::vector<GmodNode>& parents, const GmodNode& node ) {
					EXPECT_TRUE( parents.empty() || parents[0].IsRoot() );

					/*
					if (std::any_of(parents.begin(), parents.end(),
							 [](const auto& p) { return p.GetCode() == "HG3"; }) ||
						 node.GetCode() == "HG3")
					{
						paths.emplace_back(GmodPath(parents, node));
					}
					*/

					bool skipOccurrenceCheck = Gmod::IsProductSelectionAssignment(
						parents.empty() ? nullptr : &parents.back(), &node );

					if ( skipOccurrenceCheck )
						return Gmod::TraversalHandlerResult::Continue;

					int occ = Occurrences( parents, node );
					if ( occ > maxOccurrence )
						maxOccurrence = occ;

					return Gmod::TraversalHandlerResult::Continue;
				} );

			EXPECT_EQ( maxExpected, maxOccurrence );
		}
		catch ( const std::exception& ex )
		{
			FAIL() << "Exception during traversal: " << ex.what();
		}
		catch ( ... )
		{
			FAIL() << "Unknown exception during traversal";
		}
	}

	TEST_F( GmodTests, Test_Full_Traversal_With_Options )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		const int maxExpected = 2;
		int maxOccurrence = 0;

		Gmod::TraversalOptions options;
		options.MaxOccurrence = maxExpected;

		bool completed = gmod.Traverse(
			[&]( const std::vector<GmodNode>& parents, const GmodNode& node ) {
				bool skipOccurrenceCheck = Gmod::IsProductSelectionAssignment(
					parents.empty() ? nullptr : &parents.back(), &node );

				if ( skipOccurrenceCheck )
					return Gmod::TraversalHandlerResult::Continue;

				int occ = Occurrences( parents, node );
				if ( occ > maxOccurrence )
					maxOccurrence = occ;

				return Gmod::TraversalHandlerResult::Continue;
			},
			options );

		EXPECT_EQ( maxExpected, maxOccurrence );
		EXPECT_TRUE( completed );
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
