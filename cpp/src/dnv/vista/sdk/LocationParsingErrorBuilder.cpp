/**
 * @file LocationParsingErrorBuilder.cpp
 * @brief Implementation of the LocationParsingErrorBuilder class.
 */

#include "pch.h"

#include "dnv/vista/sdk/LocationParsingErrorBuilder.h"

#include "dnv/vista/sdk/ParsingErrors.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Static Factory Method
	//=====================================================================

	LocationParsingErrorBuilder LocationParsingErrorBuilder::create()
	{
		return LocationParsingErrorBuilder();
	}

	//=====================================================================
	// Public Methods
	//=====================================================================

	LocationParsingErrorBuilder& LocationParsingErrorBuilder::addError( LocationValidationResult validationResult,
		const std::string& message )
	{
		m_errors.emplace_back( validationResult, message );
		return *this;
	}

	bool LocationParsingErrorBuilder::hasError() const noexcept
	{
		return !m_errors.empty();
	}

	//=====================================================================
	// Build Method
	//=====================================================================

	ParsingErrors LocationParsingErrorBuilder::build() const
	{
		if ( !hasError() )
		{
			return ParsingErrors::Empty;
		}

		std::vector<ParsingErrors::ErrorEntry> errors;

		errors.reserve( m_errors.size() );

		for ( const auto& err : m_errors )
		{
			std::string errorName;
			switch ( err.first )
			{
				case LocationValidationResult::Invalid:
					errorName = "Invalid";
					break;
				case LocationValidationResult::InvalidCode:
					errorName = "InvalidCode";
					break;
				case LocationValidationResult::InvalidOrder:
					errorName = "InvalidOrder";
					break;
				case LocationValidationResult::NullOrWhiteSpace:
					errorName = "NullOrWhiteSpace";
					break;
				case LocationValidationResult::Valid:
					errorName = "Valid";
					break;
				default:
					errorName = "UnknownError";
					break;
			}
			errors.emplace_back( errorName, err.second );
		}
		return ParsingErrors( errors );
	}
}
