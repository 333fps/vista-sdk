#include "pch.h"

#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/Gmod.h"

using namespace dnv::vista::sdk;

class GmodPathTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		vis = &VIS::instance();
	}

	VIS* vis;
};

TEST_F( GmodPathTest, SimplePathParsing )
{
	const std::string path = "411.1/C101.31-2";
	const auto& gmod = vis->gmod( VisVersion::v3_4a );
	auto locations = vis->locations( VisVersion::v3_4a );

	GmodPath parsedPath;
	bool success = GmodPath::tryParse( path, gmod, locations, parsedPath );

	ASSERT_TRUE( success ) << "Failed to parse path: " << path;
	EXPECT_EQ( parsedPath.toString(), path );
}

TEST_F( GmodPathTest, LambdaStateCapture )
{
	const auto& gmod = vis->gmod( VisVersion::v3_4a );
	auto locations = vis->locations( VisVersion::v3_4a );

	struct TraversalTest
	{
		static bool test( const Gmod& gmod )
		{
			struct TraversalState
			{
				int visitCount = 0;
				std::vector<std::string> visitedNodes;
				bool foundTarget = false;
			};

			TraversalState initialState;

			auto handler = [state = initialState]( TraversalState& stateRef, [[maybe_unused]] const std::vector<GmodNode>& parents, const GmodNode& node ) mutable -> Gmod::TraversalHandlerResult {
				state.visitCount++;
				state.visitedNodes.push_back( node.code() );

				stateRef = state;

				return Gmod::TraversalHandlerResult::Continue;
			};

			TraversalState state;

			Gmod::TraversalOptions options;
			options.maxNodes = 10;
			bool result = gmod.traverse<TraversalState>( state, gmod.rootNode(), handler, options );

			return result && state.visitCount > 0 && !state.visitedNodes.empty();
		}
	};

	EXPECT_TRUE( TraversalTest::test( gmod ) );
}

TEST_F( GmodPathTest, SystemComponentRelationship )
{
	const auto& gmod = vis->gmod( VisVersion::v3_4a );
	auto locations = vis->locations( VisVersion::v3_4a );

	GmodNode systemNode, componentNode;
	ASSERT_TRUE( gmod.tryGetNode( std::string( "411.1" ), systemNode ) );
	ASSERT_TRUE( gmod.tryGetNode( std::string( "C101.31" ), componentNode ) );

	bool isSystemComponentRelationship =
		( systemNode.code().find( '.' ) != std::string::npos &&
			componentNode.code().find( 'C' ) == 0 &&
			componentNode.code().find( '.' ) != std::string::npos );

	EXPECT_TRUE( isSystemComponentRelationship ) << "System-component relationship not detected correctly";

	GmodPath parsedPath;
	bool success = GmodPath::tryParse( "411.1/C101.31-2", gmod, locations, parsedPath );
	EXPECT_TRUE( success ) << "Failed to parse system-component path";
}

TEST_F( GmodPathTest, FullPathWithLocation )
{
	const auto& gmod = vis->gmod( VisVersion::v3_4a );
	auto locations = vis->locations( VisVersion::v3_4a );

	std::string rootCode = gmod.rootNode().code();
	const std::string path = rootCode + "/411.1/C101.31-2";

	GmodPath parsedPath;
	bool success = GmodPath::tryParseFullPath( path, gmod, locations, parsedPath );

	ASSERT_TRUE( success ) << "Failed to parse full path: " << path;
	EXPECT_EQ( parsedPath.length(), 3 ); // ROOT + 411.1 + C101.31
	EXPECT_EQ( parsedPath.node().code(), "C101.31" );
	EXPECT_TRUE( parsedPath.node().location().has_value() );
	if ( parsedPath.node().location().has_value() )
	{
		EXPECT_EQ( parsedPath.node().location().value().toString(), "2" );
	}
}

TEST_F( GmodPathTest, PathVerificationWithSkipFlagTrue )
{
	const auto& gmod = vis->gmod( VisVersion::v3_4a );

	std::vector<GmodNode> parents;
	GmodNode rootNode = gmod.rootNode();
	GmodNode systemNode, componentNode;
	ASSERT_TRUE( gmod.tryGetNode( std::string( "411.1" ), systemNode ) );
	ASSERT_TRUE( gmod.tryGetNode( std::string( "C101.31" ), componentNode ) );

	parents.push_back( rootNode );
	parents.push_back( systemNode );

	ASSERT_NO_THROW( {
		GmodPath path( parents, componentNode, true );
		EXPECT_EQ( path.length(), 3 );
		EXPECT_EQ( path.node().code(), "C101.31" );
	} );
}

TEST_F( GmodPathTest, PathVerificationWithSkipFlagFalse )
{
	const auto& gmod = vis->gmod( VisVersion::v3_4a );

	std::vector<GmodNode> parents;
	GmodNode rootNode = gmod.rootNode();
	GmodNode systemNode, componentNode;
	ASSERT_TRUE( gmod.tryGetNode( std::string( "411.1" ), systemNode ) );
	ASSERT_TRUE( gmod.tryGetNode( std::string( "C101.31" ), componentNode ) );

	parents.push_back( rootNode );
	parents.push_back( systemNode );

	ASSERT_NO_THROW( {
		GmodPath path( parents, componentNode, false );
		EXPECT_EQ( path.length(), 3 );
		EXPECT_EQ( path.node().code(), "C101.31" );
	} );
}

TEST_F( GmodPathTest, ParentChildRelationships )
{
	const auto& gmod = vis->gmod( VisVersion::v3_4a );

	GmodNode rootNode = gmod.rootNode();
	GmodNode systemNode, componentNode;
	ASSERT_TRUE( gmod.tryGetNode( std::string( "411.1" ), systemNode ) );
	ASSERT_TRUE( gmod.tryGetNode( std::string( "C101.31" ), componentNode ) );

	std::cout << "Root node code: " << rootNode.code() << std::endl;

	bool systemIsChildOfRoot = rootNode.isChild( systemNode );
	std::cout << "Is " << systemNode.code() << " a child of " << rootNode.code()
			  << "? " << ( systemIsChildOfRoot ? "Yes" : "No" ) << std::endl;

	bool componentIsChildOfSystem = systemNode.isChild( componentNode );
	std::cout << "Is " << componentNode.code() << " a child of " << systemNode.code()
			  << "? " << ( componentIsChildOfSystem ? "Yes" : "No" ) << std::endl;

	std::vector<GmodNode> parents;
	parents.push_back( rootNode );
	parents.push_back( systemNode );

	int missingLinkAt = -1;
	bool isValid = GmodPath::isValid( parents, componentNode, missingLinkAt );
	std::cout << "Is path valid? " << ( isValid ? "Yes" : "No" );
	if ( !isValid )
	{
		std::cout << " (Missing link at position: " << missingLinkAt << ")";
	}
	std::cout << std::endl;
}

TEST_F( GmodPathTest, TraverseMethodDebug )
{
	const auto& gmod = vis->gmod( VisVersion::v3_4a );

	struct TraversalDebugState
	{
		std::vector<std::string> path;
		int visitedCount = 0;
	};

	TraversalDebugState state;

	auto handler = []( TraversalDebugState& state, [[maybe_unused]] const std::vector<GmodNode>& parents, const GmodNode& node ) -> Gmod::TraversalHandlerResult {
		state.visitedCount++;
		state.path.push_back( node.code() );

		if ( state.visitedCount > 100 )
		{
			return Gmod::TraversalHandlerResult::Stop;
		}

		return Gmod::TraversalHandlerResult::Continue;
	};

	Gmod::TraversalOptions options;
	options.maxNodes = 50;
	options.maxTraversalOccurrence = 1;

	bool result = gmod.traverse<TraversalDebugState>( state, gmod.rootNode(), handler, options );

	EXPECT_TRUE( result );
	EXPECT_GT( state.visitedCount, 0 );
	EXPECT_FALSE( state.path.empty() );

	std::cout << "Traversal path (first 10 nodes): ";
	for ( size_t i = 0; i < std::min<size_t>( 10, state.path.size() ); i++ )
	{
		std::cout << state.path[i] << " ";
	}
	std::cout << std::endl;
}

int main( int argc, char** argv )
{
	::testing::InitGoogleTest( &argc, argv );
	return RUN_ALL_TESTS();
}
