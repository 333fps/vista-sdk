/**
 * @file CodebooksTests.cpp
 * @brief Unit tests for the Codebooks and CodebookName functionalities.
 */

#include "pch.h"

#include "dnv/vista/sdk/Codebook.h"
#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	namespace tests
	{
		//----------------------------------------------
		// Test_Codebooks_Loads
		//----------------------------------------------

		TEST( CodebooksTests, Test_Codebooks_Loads )
		{
			VIS& vis = VIS::instance();
			const auto allVersions = VisVersionExtensions::allVersions();

			for ( const auto& version : allVersions )
			{
				if ( version == VisVersion::Unknown )
					continue;

				SCOPED_TRACE( "Testing VisVersion: " + VisVersionExtensions::toVersionString( version ) );

				const Codebooks* codebooksPtr = nullptr;
				ASSERT_NO_THROW( {
					codebooksPtr = &vis.codebooks( version );
				} );
				ASSERT_NE( nullptr, codebooksPtr );

				const Codebook* positionCodebookPtr = nullptr;

				ASSERT_NO_THROW( {
					positionCodebookPtr = &( *codebooksPtr )[CodebookName::Position];
				} );

				ASSERT_NE( nullptr, positionCodebookPtr );
			}
		}

		//----------------------------------------------
		// Test_Codebooks_Equality
		//----------------------------------------------

		TEST( CodebooksTests, Test_Codebooks_Equality )
		{
			VIS& vis = VIS::instance();

			const auto& codebooks = vis.codebooks( VisVersion::v3_4a );

			ASSERT_TRUE( codebooks.codebook( CodebookName::Position ).hasStandardValue( "centre" ) );
		}

		//----------------------------------------------
		// Test_CodebookName_Properties
		//----------------------------------------------

		TEST( CodebooksTests, Test_CodebookName_Properties )
		{
			const std::vector<dnv::vista::sdk::CodebookName> allCodebookNames = {
				dnv::vista::sdk::CodebookName::Quantity,
				dnv::vista::sdk::CodebookName::Content,
				dnv::vista::sdk::CodebookName::Calculation,
				dnv::vista::sdk::CodebookName::State,
				dnv::vista::sdk::CodebookName::Command,
				dnv::vista::sdk::CodebookName::Type,
				dnv::vista::sdk::CodebookName::FunctionalServices,
				dnv::vista::sdk::CodebookName::MaintenanceCategory,
				dnv::vista::sdk::CodebookName::ActivityType,
				dnv::vista::sdk::CodebookName::Position,
				dnv::vista::sdk::CodebookName::Detail };

			std::vector<int> actualEnumValues;
			actualEnumValues.reserve( allCodebookNames.size() );
			for ( const auto& cn : allCodebookNames )
			{
				actualEnumValues.push_back( static_cast<int>( cn ) );
			}

			/* Verify uniqueness of underlying integer values */
			std::set<int> uniqueValues( actualEnumValues.begin(), actualEnumValues.end() );
			ASSERT_EQ( actualEnumValues.size(), uniqueValues.size() );

			/*
				Verify that the enum values are sequential starting from 1
				This assumes the 'allCodebookNames' vector lists them in their natural enum
				order or that their integer values are inherently sequential as expected.
			*/
			std::vector<int> sortedValues = actualEnumValues;
			std::sort( sortedValues.begin(), sortedValues.end() );

			for ( size_t i = 0; i < sortedValues.size(); ++i )
			{
				ASSERT_EQ( static_cast<int>( i + 1 ), sortedValues[i] );
			}
		}
	}
}
