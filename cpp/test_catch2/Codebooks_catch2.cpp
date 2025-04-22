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
		VIS& vis = VIS::Instance();
		Gmod gmod = vis.GetGmod( visVersion );
		return { vis, gmod };
	}

	TEST_CASE( "Codebooks load successfully", "[codebooks]" )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		const auto& codebooks = vis.GetCodebooks( VisVersion::v3_4a );
		const auto& positionCodebook = codebooks.GetCodebook( CodebookName::Position );

		SPDLOG_INFO( "Position codebook standard values count: {}", positionCodebook.GetStandardValues().Count() );
		for ( auto& val : positionCodebook.GetStandardValues() )
		{
			SPDLOG_INFO( "Standard value: {}", val );
		}

		REQUIRE( positionCodebook.GetStandardValues().Count() > 0 );
	}

	TEST_CASE( "Codebooks can be accessed by multiple methods", "[codebooks]" )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		const auto& codebooks = vis.GetCodebooks( VisVersion::v3_4a );

		const auto& positionCodebook1 = codebooks[CodebookName::Position];
		const auto& positionCodebook2 = codebooks.GetCodebook( CodebookName::Position );

		REQUIRE( positionCodebook1.HasStandardValue( "centre" ) );
		REQUIRE( positionCodebook2.HasStandardValue( "centre" ) );
	}

	TEST_CASE( "Codebooks accessed by different methods are equal", "[codebooks]" )
	{
		auto [vis, gmod] = GetVisAndGmod( VisVersion::v3_4a );

		const auto& codebooks = vis.GetCodebooks( VisVersion::v3_4a );

		const auto& positionCodebook1 = codebooks[CodebookName::Position];
		const auto& positionCodebook2 = codebooks.GetCodebook( CodebookName::Position );

		REQUIRE( positionCodebook1.HasStandardValue( "centre" ) );
		REQUIRE( positionCodebook2.HasStandardValue( "centre" ) );

		REQUIRE( positionCodebook1.GetStandardValues().Count() ==
				 positionCodebook2.GetStandardValues().Count() );
	}

	TEST_CASE( "CodebookName properties are unique and well-defined", "[codebooks]" )
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
		REQUIRE( values.size() == uniqueValues.size() );

		for ( size_t i = 0; i < values.size(); ++i )
		{
			REQUIRE( values[i] == expectedValues[i] );
		}
	}
}
