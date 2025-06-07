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
}
