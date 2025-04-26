#include "pch.h"

#include "dnv/vista/sdk/GmodVersioning.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/Locations.h"

namespace dnv::vista::sdk::tests
{
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
					SPDLOG_INFO( "Loading versioning data" );
					auto versioningData = m_vis->gmodVersioningDto();
					m_gmodVersioning = std::make_unique<GmodVersioning>( versioningData );
					SPDLOG_INFO( "Successfully loaded versioning data with {} entries", versioningData.size() );
				}
				catch ( const std::exception& ex )
				{
					SPDLOG_ERROR( "Failed to load versioning data: {}", ex.what() );
					std::unordered_map<std::string, GmodVersioningDto> emptyDto;
					m_gmodVersioning = std::make_unique<GmodVersioning>( emptyDto );
					SPDLOG_WARN( "Using empty versioning data for tests" );
				}

				m_gmod_v3_4a = m_vis->gmod( VisVersion::v3_4a );
				m_gmod_v3_5a = m_vis->gmod( VisVersion::v3_5a );

				SPDLOG_INFO( "GmodVersioningTest setup complete" );

				ASSERT_FALSE( m_gmod_v3_4a->isEmpty() ) << "Dictionary is empty for GMOD 3-4a";
				ASSERT_FALSE( m_gmod_v3_5a->isEmpty() ) << "Dictionary is empty for GMOD 3-5a";

				m_setupSuccess = true;
			}
			catch ( const std::exception& ex )
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
		std::optional<Gmod> m_gmod_v3_4a;
		std::optional<Gmod> m_gmod_v3_5a;
		bool m_setupSuccess = false;
	};

	TEST_F( GmodVersioningTest, BasicSingleNodePathConversion )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		try
		{
			GmodNode rootNode;

			ASSERT_TRUE( m_gmod_v3_4a->tryGetNode( std::string( "411.1" ), rootNode ) ) << "Test node '411.1' not found in GMOD 3-4a";

			GmodPath sourcePath( {}, rootNode, true );

			SPDLOG_INFO( "Created source path with node code: {}", rootNode.code() );

			auto result = m_gmodVersioning->convertPath(
				VisVersion::v3_4a, sourcePath, VisVersion::v3_5a );

			ASSERT_TRUE( result.has_value() ) << "Path conversion failed unexpectedly";
			SPDLOG_INFO( "Path converted successfully to node: {}", result->node().code() );

			GmodNode verifyNode;
			EXPECT_TRUE( m_gmod_v3_5a->tryGetNode( result->node().code(), verifyNode ) ) << "Converted node not found in target GMOD";
		}
		catch ( const std::exception& ex )
		{
			FAIL() << "Exception during test: " << ex.what();
		}
	}

	TEST_F( GmodVersioningTest, MultiNodePathConversion )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		try
		{
			GmodNode parentNode, childNode;
			ASSERT_TRUE( m_gmod_v3_4a->tryGetNode( std::string( "411.1" ), parentNode ) )
				<< "Parent node not found";

			bool foundValidChild = false;
			for ( const auto* child : parentNode.children() )
			{
				if ( child != nullptr )
				{
					childNode = *child;
					foundValidChild = true;
					break;
				}
			}

			ASSERT_TRUE( foundValidChild )
				<< "No valid child nodes found for testing";

			SPDLOG_INFO( "Using parent node: {} and child node: {}",
				parentNode.code(), childNode.code() );

			std::vector<GmodNode> parents = { parentNode };
			GmodPath sourcePath( parents, childNode, true );

			auto result = m_gmodVersioning->convertPath(
				VisVersion::v3_4a, sourcePath, VisVersion::v3_5a );

			ASSERT_TRUE( result.has_value() )
				<< "Multi-node path conversion failed";

			SPDLOG_INFO( "Multi-node path converted successfully with {} parents",
				result->parents().size() );

			EXPECT_EQ( result->parents().size(), sourcePath.parents().size() )
				<< "Parent node count changed during conversion";
		}
		catch ( const std::exception& ex )
		{
			FAIL() << "Exception during test: " << ex.what();
		}
	}

	TEST_F( GmodVersioningTest, PathWithLocationConversion )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		try
		{
			GmodNode componentNode;
			ASSERT_TRUE( m_gmod_v3_4a->tryGetNode( std::string( "C101.31" ), componentNode ) )
				<< "Component node for location testing not found";

			if ( !componentNode.isIndividualizable( false, true ) )
			{
				GTEST_SKIP() << "Test node doesn't support location";
				return;
			}

			Location location( "2" );
			auto nodeWithLoc = componentNode.withLocation( location );

			ASSERT_TRUE( nodeWithLoc.location().has_value() )
				<< "Failed to set location on node";
			ASSERT_EQ( nodeWithLoc.location()->toString(), "2" )
				<< "Location not set correctly";

			GmodPath sourcePath( {}, nodeWithLoc, true );

			SPDLOG_INFO( "Created path with node {} and location {}",
				nodeWithLoc.code(), nodeWithLoc.location()->toString() );

			auto result = m_gmodVersioning->convertPath(
				VisVersion::v3_4a, sourcePath, VisVersion::v3_5a );

			ASSERT_TRUE( result.has_value() )
				<< "Path with location conversion failed";

			ASSERT_TRUE( result->node().location().has_value() )
				<< "Location lost during conversion";
			EXPECT_EQ( result->node().location()->toString(), "2" )
				<< "Location value changed during conversion";
		}
		catch ( const std::exception& ex )
		{
			FAIL() << "Exception during test: " << ex.what();
		}
	}

	TEST_F( GmodVersioningTest, VersionValidation )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		GmodNode node;
		ASSERT_TRUE( m_gmod_v3_4a->tryGetNode( std::string( "411.1" ), node ) );
		GmodPath testPath( {}, node, true );

		EXPECT_THROW( { m_gmodVersioning->convertPath(
							VisVersion::v3_4a, testPath, VisVersion::v3_4a ); }, std::invalid_argument ) << "Converting to same version should throw";

		EXPECT_THROW( { m_gmodVersioning->convertPath(
							VisVersion::v3_5a, testPath, VisVersion::v3_4a ); }, std::invalid_argument ) << "Converting to older version should throw";

		EXPECT_THROW( { m_gmodVersioning->convertPath(
							static_cast<VisVersion>( 999 ), testPath, VisVersion::v3_5a ); }, std::invalid_argument ) << "Using invalid source version should throw";
	}

	TEST_F( GmodVersioningTest, NonExistentNodeConversion )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		try
		{
			GmodNode fakeNode;

			GmodPath invalidPath( {}, fakeNode, false );

			auto result = m_gmodVersioning->convertPath(
				VisVersion::v3_4a, invalidPath, VisVersion::v3_5a );

			EXPECT_FALSE( result.has_value() )
				<< "Converting with non-existent node should fail gracefully";
		}
		catch ( const std::exception& ex )
		{
			FAIL() << "Exception during test: " << ex.what();
		}
	}

	TEST_F( GmodVersioningTest, ComplexPathHierarchyConversion )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		try
		{
			GmodNode node1, node2, node3;

			ASSERT_TRUE( m_gmod_v3_4a->tryGetNode( std::string( "411.1" ), node1 ) )
				<< "First test node not found";

			bool found2 = false;
			for ( const auto* child : node1.children() )
			{
				if ( child != nullptr )
				{
					node2 = *child;
					found2 = true;
					break;
				}
			}

			ASSERT_TRUE( found2 ) << "No level 2 node found for testing";

			bool found3 = false;
			for ( const auto* child : node2.children() )
			{
				if ( child != nullptr )
				{
					node3 = *child;
					found3 = true;
					break;
				}
			}

			if ( !found3 )
			{
				GTEST_SKIP() << "No level 3 node found for testing";
				return;
			}

			SPDLOG_INFO( "Testing complex path: {} -> {} -> {}",
				node1.code(), node2.code(), node3.code() );

			std::vector<GmodNode> parents = { node1, node2 };
			GmodPath complexPath( parents, node3, true );

			auto result = m_gmodVersioning->convertPath(
				VisVersion::v3_4a, complexPath, VisVersion::v3_5a );

			EXPECT_TRUE( result.has_value() )
				<< "Complex path conversion failed";

			if ( result.has_value() )
			{
				SPDLOG_INFO( "Complex path converted successfully with {} parents",
					result->parents().size() );
			}
		}
		catch ( const std::exception& ex )
		{
			FAIL() << "Exception during test: " << ex.what();
		}
	}

	TEST_F( GmodVersioningTest, MultiVersionSkipConversion )
	{
		ASSERT_TRUE( m_setupSuccess ) << "Test setup failed";

		try
		{
			GmodNode node;
			ASSERT_TRUE( m_gmod_v3_4a->tryGetNode( std::string( "411.1" ), node ) );
			GmodPath simplePath( {}, node, true );

			auto result = m_gmodVersioning->convertPath(
				VisVersion::v3_4a, simplePath, VisVersion::v3_6a );

			SPDLOG_INFO( "Multi-version conversion {} -> {} {}",
				"3-4a", "3-6a",
				result.has_value() ? "succeeded" : "failed" );
		}
		catch ( const std::exception& ex )
		{
			FAIL() << "Exception during multi-version conversion: " << ex.what();
		}
	}
}
