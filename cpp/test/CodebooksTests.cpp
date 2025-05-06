#include "pch.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/Codebook.h"
#include "dnv/vista/sdk/CodebookName.h"

namespace dnv::vista::sdk::tests
{
	inline std::pair<VIS&, const Gmod&> visAndGmod()
	{
		VIS& vis = VIS::instance();
		const Gmod& gmod = vis.gmod( VisVersion::v3_4a );
		return { vis, gmod };
	}

	TEST( CodebooksTests, Test_Codebooks_Loads )
	{
		auto [vis, gmod] = visAndGmod();

		const auto& codebooks = vis.codebooks( VisVersion::v3_4a );

		ASSERT_NO_THROW( {
			[[maybe_unused]] const auto& positionCodebook = codebooks.codebook( CodebookName::Position );
		} );
	}

	TEST( CodebooksTests, Test_Codebooks_Equality )
	{
		auto [vis, gmod] = visAndGmod();

		const auto& codebooks = vis.codebooks( VisVersion::v3_4a );

		ASSERT_TRUE( codebooks[CodebookName::Position].hasStandardValue( "centre" ) );
	}

	TEST( CodebooksTests, Test_CodebookName_Properties )
	{
		std::vector<int> values;
		for ( int i = 1; i <= static_cast<int>( CodebookName::Detail ); i++ )
		{
			values.push_back( i );
		}

		std::set<int> uniqueValues( values.begin(), values.end() );
		ASSERT_EQ( values.size(), uniqueValues.size() );

		for ( size_t i = 0; i < values.size(); i++ )
		{
			ASSERT_EQ( i, values[i] - 1 );
		}
	}
}
