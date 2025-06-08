/**
 * @file GmodTests.cpp
 * @brief Unit tests for the Generic Product Model (GMOD) functionality
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodTraversal.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk::tests
{
	namespace GmodTestsFixture
	{
		class GmodTests : public ::testing::Test
		{
		public:
			static std::pair<VIS&, const Gmod&> visAndGmod( VisVersion visVersion )
			{
				VIS& vis = VIS::instance();
				const auto& gmod = vis.gmod( visVersion );

				return { vis, gmod };
			}
		};

		//=====================================================================
		// Helper Functions
		//=====================================================================

		static std::size_t occurrences( const std::vector<const GmodNode*>& parents, const GmodNode& node )
		{
			return static_cast<size_t>(
				std::count_if(
					parents.begin(), parents.end(),
					[&node]( const GmodNode* p ) {
						return p != nullptr && p->code() == node.code();
					} ) );
		}

		//=====================================================================
		// Traversal Helper Struct
		//=====================================================================

		struct TraversalState
		{
			TraversalState( int stopAfter )
				: stopAfter{ stopAfter },
				  nodeCount{ 0 }
			{
			}

			int stopAfter;
			int nodeCount;
		};

		//=====================================================================
		// Tests
		//=====================================================================

		//----------------------------------------------
		// Test_Gmod_Node_Equality
		//----------------------------------------------

		TEST_F( GmodTests, Test_Gmod_Node_Equality )
		{
			auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

			const GmodNode& node1 = gmod["400a"];
			const GmodNode& node2 = gmod["400a"];

			EXPECT_EQ( node1, node2 );
			EXPECT_EQ( &node1, &node2 );

			GmodNode node3 = node2.withLocation( "1" );
			EXPECT_NE( node1, node3 );
			EXPECT_NE( &node1, &node3 );

			GmodNode node4 = node2;
			EXPECT_EQ( node1, node4 );
			EXPECT_NE( &node1, &node4 );
		}

		//----------------------------------------------
		// Test_Gmod_Node_Types
		//----------------------------------------------

		TEST_F( GmodTests, Test_Gmod_Node_Types )
		{
			auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

			std::unordered_set<std::string> set;
			Gmod::Enumerator enumerator = gmod.enumerator();

			while ( enumerator.next() )
			{
				const GmodNode& node = enumerator.current();
				const GmodNodeMetadata& metadata = node.metadata();
				std::string category_type_string = metadata.category();
				category_type_string += " | ";
				category_type_string += metadata.type();
				set.insert( category_type_string );
			}

			EXPECT_FALSE( set.empty() );
		}

		//----------------------------------------------
		// Test_Normal_Assignments
		//----------------------------------------------

		TEST_F( GmodTests, Test_Normal_Assignments )
		{
			auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

			const GmodNode& node1 = gmod["411.3"];
			EXPECT_NE( node1.productType(), nullptr );
			EXPECT_EQ( node1.productSelection(), nullptr );

			const GmodNode& node2 = gmod["H601"];
			EXPECT_EQ( node2.productType(), nullptr );
		}

		//----------------------------------------------
		// Test_Node_With_Product_Selection
		//----------------------------------------------

		TEST_F( GmodTests, Test_Node_With_Product_Selection )
		{
			auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

			const GmodNode& node1 = gmod["411.2"];
			EXPECT_NE( node1.productSelection(), nullptr );
			EXPECT_EQ( node1.productType(), nullptr );

			const GmodNode& node2 = gmod["H601"];
			EXPECT_EQ( node2.productSelection(), nullptr );
		}

		//----------------------------------------------
		// Test_Product_Selection
		//----------------------------------------------

		TEST_F( GmodTests, Test_Product_Selection )
		{
			auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

			const GmodNode& node = gmod["CS1"];
			EXPECT_TRUE( node.isProductSelection() );
		}

		//----------------------------------------------
		// Test_Full_Traversal
		//----------------------------------------------

		TEST_F( GmodTests, Test_Full_Traversal )
		{
			auto visVersionForPath = VisVersion::v3_4a;
			auto [vis, gmod] = visAndGmod( visVersionForPath );

			struct FullTraversalState
			{
				std::vector<GmodPath> paths;
				int maxOccurrence = 0;
				const Gmod& gmodRef;

				FullTraversalState( const Gmod& gmod ) : gmodRef( gmod ) {}
			};

			FullTraversalState traversalState( gmod );
			int maxExpected = TraversalOptions::DEFAULT_MAX_TRAVERSAL_OCCURRENCE;

			TraverseHandlerWithState<FullTraversalState> handler =
				[]( FullTraversalState& state, const std::vector<const GmodNode*>& parents, const GmodNode& node ) -> TraversalHandlerResult {
				EXPECT_TRUE( parents.empty() || parents[0]->isRoot() );

				bool isHG3Related = ( node.code() == "HG3" ) ||
									std::any_of( parents.begin(), parents.end(), []( const GmodNode* p ) {
										return p != nullptr && p->code() == "HG3";
									} );

				if ( isHG3Related )
				{
					std::vector<GmodNode*> nonConstParents;
					nonConstParents.reserve( parents.size() );
					for ( const GmodNode* p_const : parents )
					{
						nonConstParents.push_back( const_cast<GmodNode*>( p_const ) );
					}
					GmodNode* nonConstNode = const_cast<GmodNode*>( &node );

					state.paths.emplace_back( state.gmodRef, nonConstNode, std::move( nonConstParents ) );
				}

				const GmodNode* lastParent = parents.empty() ? nullptr : parents.back();

				bool skipOccurenceCheck = Gmod::isProductSelectionAssignment( lastParent, &node );
				if ( skipOccurenceCheck )
				{
					return TraversalHandlerResult::Continue;
				}

				int occ = static_cast<int>( occurrences( parents, node ) );
				if ( occ > state.maxOccurrence )
				{
					state.maxOccurrence = occ;
				}

				return TraversalHandlerResult::Continue;
			};

			bool completed = GmodTraversal::traverse( gmod, traversalState, handler );

			EXPECT_EQ( maxExpected, traversalState.maxOccurrence );
			EXPECT_TRUE( completed );
		}

		//----------------------------------------------
		// Test_Full_Traversal_With_Options
		//----------------------------------------------

		TEST_F( GmodTests, Test_Full_Traversal_With_Options )
		{
			auto visVersionForPath = VisVersion::v3_4a;
			auto [vis, gmod] = visAndGmod( visVersionForPath );

			struct MaxOccurrenceState
			{
				int maxOccurrence = 0;
			};

			MaxOccurrenceState state;
			size_t maxExpected = 2;
			TraversalOptions options;
			options.maxTraversalOccurrence = maxExpected;

			TraverseHandlerWithState<MaxOccurrenceState> handler =
				[]( MaxOccurrenceState& state, const std::vector<const GmodNode*>& parents, const GmodNode& node ) -> TraversalHandlerResult {
				const GmodNode* lastParent = parents.empty() ? nullptr : parents.back();
				bool skipOccurenceCheck = Gmod::isProductSelectionAssignment( lastParent, &node );
				if ( skipOccurenceCheck )
				{
					return TraversalHandlerResult::Continue;
				}

				int occ = static_cast<int>( occurrences( parents, node ) );
				if ( occ > state.maxOccurrence )
				{
					state.maxOccurrence = occ;
				}

				return TraversalHandlerResult::Continue;
			};

			bool completed = GmodTraversal::traverse( gmod, state, handler, options );

			EXPECT_EQ( maxExpected, state.maxOccurrence );
			EXPECT_TRUE( completed );
		}

		//----------------------------------------------
		// Test_Partial_Traversal
		//----------------------------------------------

		TEST_F( GmodTests, Test_Partial_Traversal )
		{
			auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

			TraversalState state( 5 );

			dnv::vista::sdk::TraverseHandlerWithState<TraversalState> handler =
				[]( TraversalState& currentState, const std::vector<const GmodNode*>& parents, [[maybe_unused]] const GmodNode& node ) -> TraversalHandlerResult {
				EXPECT_TRUE( parents.empty() || parents[0]->isRoot() );
				if ( ++currentState.nodeCount == currentState.stopAfter )
				{
					return TraversalHandlerResult::Stop;
				}

				return TraversalHandlerResult::Continue;
			};

			bool completed = GmodTraversal::traverse( gmod, state, handler );

			EXPECT_EQ( state.stopAfter, state.nodeCount );
			EXPECT_FALSE( completed );
		}

		//----------------------------------------------
		// Test_Full_Traversal_From
		//----------------------------------------------

		TEST_F( GmodTests, Test_Full_Traversal_From )
		{
			auto [vis, gmod] = visAndGmod( VisVersion::v3_4a );

			TraversalState state( 0 );

			const GmodNode& startNode = gmod["400a"];

			dnv::vista::sdk::TraverseHandlerWithState<TraversalState> handler =
				[]( TraversalState& currentState, const std::vector<const GmodNode*>& parents, [[maybe_unused]] const GmodNode& node ) -> TraversalHandlerResult {
				EXPECT_TRUE( parents.empty() || ( parents[0] != nullptr && parents[0]->code() == "400a" ) );

				++currentState.nodeCount;

				return TraversalHandlerResult::Continue;
			};

			bool completed = GmodTraversal::traverse( state, startNode, handler );

			EXPECT_TRUE( completed );
		}
	}

	namespace GmodTestsParametrized
	{
		class GmodTests : public ::testing::TestWithParam<VisVersion>
		{
		public:
			static std::pair<VIS&, const Gmod&> visAndGmod( VisVersion visVersion )
			{
				VIS& vis = VIS::instance();
				const auto& gmod = vis.gmod( visVersion );

				return { vis, gmod };
			}
		};

		//=====================================================================
		// Tests
		//=====================================================================

		//----------------------------------------------
		// Test_Gmod_Loads
		//----------------------------------------------

		TEST_P( GmodTests, Test_Gmod_Loads )
		{
			auto visVersion = GetParam();
			auto [vis, gmod] = visAndGmod( visVersion );

			const GmodNode* tempNodePtr = nullptr;
			ASSERT_TRUE( gmod.tryGetNode( std::string( "400a" ), tempNodePtr ) ) << "Node '400a' not found in GMOD for version " << VisVersionExtensions::toVersionString( visVersion );
			ASSERT_NE( tempNodePtr, nullptr ) << "tryGetNode succeeded but pointer is null for '400a' in GMOD version " << VisVersionExtensions::toVersionString( visVersion );
		}

		//----------------------------------------------
		// Test_Gmod_Properties
		//----------------------------------------------

		struct ExpectedValues
		{
			std::string maxCode;
			int nodeCount;
		};

		const std::map<VisVersion, ExpectedValues> expectedMaxes = {
			{ VisVersion::v3_4a, { "C1053.3114", 6420 } },
			{ VisVersion::v3_5a, { "C1053.3114", 6557 } },
			{ VisVersion::v3_6a, { "C1053.3114", 6557 } },
			{ VisVersion::v3_7a, { "H346.11113", 6672 } },
			{ VisVersion::v3_8a, { "H346.11113", 6335 } } };

		TEST_P( GmodTests, Test_Gmod_Properties )
		{
			/*
			 * This test ensures certain properties of the Gmod data
			 * that we make some design desicisions based on,
			 * i.e for hashing of the node code
			 */
			auto visVersion = GetParam();
			auto [vis, gmod] = visAndGmod( visVersion );

			const GmodNode* minLengthLexiographicallyOrderedNode = nullptr;
			const GmodNode* maxLengthLexiographicallyOrderedNode = nullptr;
			size_t currentMinLength = std::string::npos;
			size_t currentMaxLength = 0;
			int nodeCount = 0;

			Gmod::Enumerator enumerator = gmod.enumerator();
			while ( enumerator.next() )
			{
				const GmodNode& node = enumerator.current();
				nodeCount++;
				const std::string& code = node.code();
				size_t len = code.length();

				if ( !minLengthLexiographicallyOrderedNode || len < currentMinLength )
				{
					currentMinLength = len;
					minLengthLexiographicallyOrderedNode = &node;
				}
				else if ( len == currentMinLength )
				{
					if ( minLengthLexiographicallyOrderedNode && code < minLengthLexiographicallyOrderedNode->code() )
					{
						minLengthLexiographicallyOrderedNode = &node;
					}
				}

				if ( !maxLengthLexiographicallyOrderedNode || len > currentMaxLength )
				{
					currentMaxLength = len;
					maxLengthLexiographicallyOrderedNode = &node;
				}
				else if ( len == currentMaxLength )
				{
					if ( maxLengthLexiographicallyOrderedNode && code > maxLengthLexiographicallyOrderedNode->code() )
					{
						maxLengthLexiographicallyOrderedNode = &node;
					}
				}
			}

			ASSERT_NE( minLengthLexiographicallyOrderedNode, nullptr ) << "minLengthNode should not be null for " << VisVersionExtensions::toVersionString( visVersion );
			ASSERT_NE( maxLengthLexiographicallyOrderedNode, nullptr ) << "maxLengthNode should not be null for " << VisVersionExtensions::toVersionString( visVersion );

			ASSERT_EQ( minLengthLexiographicallyOrderedNode->code().length(), 2 ) << "Min code length mismatch for " << VisVersionExtensions::toVersionString( visVersion );
			ASSERT_EQ( minLengthLexiographicallyOrderedNode->code(), "VE" ) << "Min code value mismatch for " << VisVersionExtensions::toVersionString( visVersion );

			ASSERT_EQ( maxLengthLexiographicallyOrderedNode->code().length(), 10 ) << "Max code length mismatch for " << VisVersionExtensions::toVersionString( visVersion );

			auto expectedIt = expectedMaxes.find( visVersion );
			ASSERT_NE( expectedIt, expectedMaxes.end() ) << "Expected values not found for GMOD version " << VisVersionExtensions::toVersionString( visVersion );
			const auto& expectedValues = expectedIt->second;

			ASSERT_EQ( maxLengthLexiographicallyOrderedNode->code(), expectedValues.maxCode ) << "Max code value mismatch for " << VisVersionExtensions::toVersionString( visVersion );
			ASSERT_EQ( nodeCount, expectedValues.nodeCount ) << "Node count mismatch for " << VisVersionExtensions::toVersionString( visVersion );
		}

		//----------------------------------------------
		// Test_Gmod_Lookup
		//----------------------------------------------

		TEST_P( GmodTests, Test_Gmod_Lookup )
		{
			auto visVersion = GetParam();
			auto [vis, gmod] = visAndGmod( visVersion );

			const GmodDto& gmodDtoObject = vis.gmodDto( visVersion );

			{
				std::unordered_set<std::string> seen_codes;
				for ( const GmodNodeDto& nodeDto : gmodDtoObject.items() )
				{
					const std::string& dtoCode = nodeDto.code();
					ASSERT_FALSE( dtoCode.empty() ) << "DTO code is empty for version " << VisVersionExtensions::toVersionString( visVersion );

					auto insert_result = seen_codes.insert( dtoCode );
					ASSERT_TRUE( insert_result.second ) << "Duplicate DTO code: " << dtoCode << " for version " << VisVersionExtensions::toVersionString( visVersion );

					const GmodNode* foundNodePtr = nullptr;
					ASSERT_TRUE( gmod.tryGetNode( dtoCode, foundNodePtr ) ) << "Failed to find node from DTO code: " << dtoCode << " for version " << VisVersionExtensions::toVersionString( visVersion );
					ASSERT_NE( foundNodePtr, nullptr ) << "Found node pointer is null for DTO code: " << dtoCode << " for version " << VisVersionExtensions::toVersionString( visVersion );
					ASSERT_EQ( dtoCode, foundNodePtr->code() ) << "Mismatch between DTO code and found node code for: " << dtoCode << " for version " << VisVersionExtensions::toVersionString( visVersion );
				}
			}

			int gmodIteratedCount = 0;
			{
				std::unordered_set<std::string> seen_codes;
				Gmod::Enumerator enumerator = gmod.enumerator();
				while ( enumerator.next() )
				{
					const GmodNode& node = enumerator.current();
					const std::string& gmodNodeCode = node.code();
					ASSERT_FALSE( gmodNodeCode.empty() ) << "Gmod iterated node code is empty for version " << VisVersionExtensions::toVersionString( visVersion );

					auto insert_result = seen_codes.insert( gmodNodeCode );
					ASSERT_TRUE( insert_result.second ) << "Duplicate Gmod iterated code: " << gmodNodeCode << " for version " << VisVersionExtensions::toVersionString( visVersion );

					const GmodNode* foundNodePtr = nullptr;
					ASSERT_TRUE( gmod.tryGetNode( gmodNodeCode, foundNodePtr ) ) << "Failed to find node from Gmod iterated code: " << gmodNodeCode << " for version " << VisVersionExtensions::toVersionString( visVersion );
					ASSERT_NE( foundNodePtr, nullptr ) << "Found node pointer is null for Gmod iterated code: " << gmodNodeCode << " for version " << VisVersionExtensions::toVersionString( visVersion );
					ASSERT_EQ( gmodNodeCode, foundNodePtr->code() ) << "Mismatch between Gmod iterated code and found node code for: " << gmodNodeCode << " for version " << VisVersionExtensions::toVersionString( visVersion );
					gmodIteratedCount++;
				}
			}

			const GmodNode* tempNodePtr = nullptr;
			ASSERT_FALSE( gmod.tryGetNode( std::string( "ABC" ), tempNodePtr ) );
			ASSERT_FALSE( gmod.tryGetNode( std::string_view( "" ), tempNodePtr ) );
			ASSERT_FALSE( gmod.tryGetNode( std::string_view( "SDFASDFSDAFb" ), tempNodePtr ) );
			ASSERT_FALSE( gmod.tryGetNode( std::string_view( "✅" ), tempNodePtr ) );
			ASSERT_FALSE( gmod.tryGetNode( std::string_view( "a✅b" ), tempNodePtr ) );
			ASSERT_FALSE( gmod.tryGetNode( std::string_view( "ac✅bc" ), tempNodePtr ) );
			ASSERT_FALSE( gmod.tryGetNode( std::string_view( "✅bc" ), tempNodePtr ) );
			ASSERT_FALSE( gmod.tryGetNode( std::string_view( "a✅" ), tempNodePtr ) );
			ASSERT_FALSE( gmod.tryGetNode( std::string_view( "ag✅" ), tempNodePtr ) );
		}

		//----------------------------------------------
		// Test_Gmod_RootNode_Children
		//----------------------------------------------

		TEST_P( GmodTests, Test_Gmod_RootNode_Children )
		{
			auto visVersion = GetParam();
			auto [vis, gmod] = visAndGmod( visVersion );

			const GmodNode& node = gmod.rootNode();

			EXPECT_FALSE( node.children().empty() );
		}

		//----------------------------------------------
		// Test_Mappability
		//----------------------------------------------

		struct MappabilityTestData
		{
			std::string code;
			bool expectedMappable;
		};

		class GmodMappabilityTests : public ::testing::TestWithParam<MappabilityTestData>
		{
		};

		TEST_P( GmodMappabilityTests, Test_Mappability )
		{
			const auto& testData = GetParam();
			auto [vis, gmod] = GmodTests::visAndGmod( VisVersion::v3_4a );

			const GmodNode& node = gmod[testData.code];
			EXPECT_EQ( node.isMappable(), testData.expectedMappable );
		}

		//=====================================================================
		// Instantiate
		//=====================================================================

		INSTANTIATE_TEST_SUITE_P(
			GmodTestSuite,
			GmodTests,
			::testing::Values(
				VisVersion::v3_4a,
				VisVersion::v3_5a,
				VisVersion::v3_6a,
				VisVersion::v3_7a,
				VisVersion::v3_8a ) );

		INSTANTIATE_TEST_SUITE_P(
			GmodMappabilityTestSuite,
			GmodMappabilityTests,
			::testing::Values(
				MappabilityTestData{ "VE", false },
				MappabilityTestData{ "300a", false },
				MappabilityTestData{ "300", true },
				MappabilityTestData{ "411", true },
				MappabilityTestData{ "410", true },
				MappabilityTestData{ "651.21s", false },
				MappabilityTestData{ "924.2", true },
				MappabilityTestData{ "411.1", false },
				MappabilityTestData{ "C101", true },
				MappabilityTestData{ "CS1", false },
				MappabilityTestData{ "C101.663", true },
				MappabilityTestData{ "C101.4", true },
				MappabilityTestData{ "C101.21s", false },
				MappabilityTestData{ "F201.11", true },
				MappabilityTestData{ "C101.211", false } ) );
	}
}
