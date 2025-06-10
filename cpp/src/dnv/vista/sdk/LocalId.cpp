/**
 * @file LocalId.cpp
 * @brief Implementation of the LocalId class
 */

#include "pch.h"

#include "dnv/vista/sdk/LocalId.h"

#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VISVersion.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// LocalId class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	LocalId::LocalId( LocalIdBuilder builder ) : m_builder( std::move( builder ) )
	{
		if ( m_builder.isEmpty() || !m_builder.isValid() )
		{
			throw std::invalid_argument( "Invalid LocalIdBuilder" );
		}
	}

	//----------------------------------------------
	// Static parsing methods
	//----------------------------------------------

	LocalId LocalId::parse( std::string_view localIdStr )
	{
		return LocalId( LocalIdBuilder::parse( localIdStr ) );
	}

	bool LocalId::tryParse( std::string_view localIdStr, ParsingErrors& errors, std::optional<LocalId>& localId )
	{
		std::optional<LocalIdBuilder> builder;
		bool success = LocalIdBuilder::tryParse( localIdStr, errors, builder );
		if ( success && builder.has_value() )
		{
			localId = LocalId( std::move( builder.value() ) );
		}

		return success;
	}

	bool LocalId::tryParse( std::string_view localIdStr, std::optional<LocalId>& localId )
	{
		std::optional<LocalIdBuilder> builder;
		bool success = LocalIdBuilder::tryParse( localIdStr, builder );
		if ( success && builder.has_value() )
		{
			localId = LocalId( std::move( builder.value() ) );
		}

		return success;
	}
}
