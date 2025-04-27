/**
 * @file ILocalId.hpp
 * @brief Implementation of template methods for ILocalId interface
 *
 * Contains the implementation of static template methods declared in ILocalId.h.
 * This file is included at the end of ILocalId.h and should not be included directly.
 */

#pragma once

#include <optional>
#include <string>
#include <stdexcept>
#include "spdlog/spdlog.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Static Parser Method Implementations
	//-------------------------------------------------------------------------

	template <typename T>
	inline bool ILocalId<T>::operator==( const T& other ) const noexcept
	{
		return equals( other );
	}

	template <typename T>
	inline bool ILocalId<T>::operator!=( const T& other ) const noexcept
	{
		return !equals( other );
	}

	template <typename T>
	T ILocalId<T>::parse( const std::string& localIdStr )
	{
		SPDLOG_TRACE( "ILocalId::parse called for: {}", localIdStr );

		ParsingErrors errors;
		std::optional<T> localId;
		if ( !tryParse( localIdStr, errors, localId ) )
		{
			SPDLOG_ERROR( "Failed to parse LocalId: {}", localIdStr );
			throw std::invalid_argument( "Failed to parse LocalId: " + errors.toString() );
		}

		if ( !localId.has_value() )
		{
			SPDLOG_ERROR( "Internal error: LocalId is null after successful parsing" );
			throw std::runtime_error( "Internal error: LocalId is null after successful parsing" );
		}

		return std::move( *localId );
	}

	template <typename T>
	bool ILocalId<T>::tryParse( const std::string& localIdStr, ParsingErrors& errors, std::optional<T>& localId )
	{
		SPDLOG_TRACE( "ILocalId::tryParse called for: {}", localIdStr );

		try
		{
			localId = T( localIdStr );
			SPDLOG_DEBUG( "Successfully parsed LocalId: {}", localIdStr );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_INFO( "LocalId parsing failed: {}", localIdStr );
			SPDLOG_DEBUG( "Exception details: {}", e.what() );

			std::string errorType = "ParseError";
			if ( dynamic_cast<const std::invalid_argument*>( &e ) )
				errorType = "InvalidFormatError";

			errors.addError( errorType, e.what() );

			localId.reset();
			return false;
		}
	}
}
