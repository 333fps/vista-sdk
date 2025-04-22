#include "pch.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/Codebook.h"
#include "dnv/vista/sdk/CodebookName.h"

namespace dnv::vista::sdk::tests
{
	inline std::pair<VIS&, Gmod> GetVisAndGmod( VisVersion visVersion )
	{
		VIS& vis = VIS::instance();
		Gmod gmod = vis.gmod( visVersion );
		return { vis, gmod };
	}

	TEST( CodebooksTests, Test_Codebooks_Loads )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		const auto& codebooks = vis.codebooks( VisVersion::v3_4a );
		const auto& positionCodebook = codebooks.codebook( CodebookName::Position );

		SPDLOG_INFO( "Position codebook standard values count: {}", positionCodebook.standardValues().count() );
		for ( auto& val : positionCodebook.standardValues() )
		{
			// SPDLOG_INFO( "Standard value: {}", val );
		}

		ASSERT_GT( positionCodebook.standardValues().count(), 0 );
	}

	TEST( CodebooksTests, Test_Codebooks_AccessMethods )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		const auto& codebooks = vis.codebooks( VisVersion::v3_4a );

		const auto& positionCodebook1 = codebooks[CodebookName::Position];
		const auto& positionCodebook2 = codebooks.codebook( CodebookName::Position );

		ASSERT_TRUE( positionCodebook1.hasStandardValue( "centre" ) );
		ASSERT_TRUE( positionCodebook2.hasStandardValue( "centre" ) );
	}

	TEST( CodebooksTests, Test_Codebooks_Equality )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		const auto& codebooks = vis.codebooks( VisVersion::v3_4a );

		const auto& positionCodebook1 = codebooks[CodebookName::Position];
		const auto& positionCodebook2 = codebooks.codebook( CodebookName::Position );

		ASSERT_TRUE( positionCodebook1.hasStandardValue( "centre" ) );
		ASSERT_TRUE( positionCodebook2.hasStandardValue( "centre" ) );

		ASSERT_EQ( positionCodebook1.standardValues().count(),
			positionCodebook2.standardValues().count() );
	}

	TEST( CodebooksTests, Test_CodebookName_Properties )
	{
		std::vector<int> expectedValues = {
			static_cast<int>( CodebookName::Position ),
			static_cast<int>( CodebookName::Quantity ),
			static_cast<int>( CodebookName::Content ),
			static_cast<int>( CodebookName::Calculation ),
			static_cast<int>( CodebookName::State ),
			static_cast<int>( CodebookName::Command ),
			static_cast<int>( CodebookName::Type ),
			static_cast<int>( CodebookName::Detail ) };

		std::vector<int> values;
		for ( const auto& name : expectedValues )
		{
			values.push_back( name );
		}

		std::set<int> uniqueValues( values.begin(), values.end() );
		ASSERT_EQ( values.size(), uniqueValues.size() );

		for ( size_t i = 0; i < values.size(); ++i )
		{
			ASSERT_EQ( values[i], expectedValues[i] );
		}
	}
}
