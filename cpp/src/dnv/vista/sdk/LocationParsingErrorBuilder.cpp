/**
 * @file LocationParsingErrorBuilder.cpp
 * @brief Implementation of the LocationParsingErrorBuilder class.
 */

#include "pch.h"

#include "dnv/vista/sdk/LocationParsingErrorBuilder.h"

#include "dnv/vista/sdk/ParsingErrors.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Internal helper functions and constants
		//=====================================================================

		//----------------------------------------------
		// String view constants for Enum Conversion
		//----------------------------------------------

		static constexpr std::string_view INVALID_RESULT = "Invalid";
		static constexpr std::string_view INVALID_CODE_RESULT = "InvalidCode";
		static constexpr std::string_view INVALID_ORDER_RESULT = "InvalidOrder";
		static constexpr std::string_view NULL_OR_WHITESPACE_RESULT = "NullOrWhiteSpace";
		static constexpr std::string_view VALID_RESULT = "Valid";

		//----------------------------------------------
		// Enum to string view conversion function
		//----------------------------------------------

		/**
		 * @brief Converts LocationValidationResult enum to string_view representation.
		 * @details Uses constexpr switch for zero-cost, compile-time optimization.
		 *          Returns string_view to avoid memory allocations during conversion.
		 * @param result The validation result enum to convert
		 * @return Corresponding string_view representation
		 * @note This function is marked constexpr for compile-time evaluation
		 */

		constexpr std::string_view locationValidationResultToStringView( LocationValidationResult result ) noexcept
		{
			switch ( result )
			{
				case LocationValidationResult::Invalid:
					return INVALID_RESULT;
				case LocationValidationResult::InvalidCode:
					return INVALID_CODE_RESULT;
				case LocationValidationResult::InvalidOrder:
					return INVALID_ORDER_RESULT;
				case LocationValidationResult::NullOrWhiteSpace:
					return NULL_OR_WHITESPACE_RESULT;
				case LocationValidationResult::Valid:
					return VALID_RESULT;
				default:
					return "Unknown";
			}
		}
	}

	//=====================================================================
	// LocationParsingErrorBuilder class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	LocationParsingErrorBuilder::LocationParsingErrorBuilder()
	{
		/* Memory: 8 × sizeof(std::pair<LocationValidationResult, std::string>) = 8 × (4 + 32) = ~288 bytes */
		m_errors.reserve( 8 );
	}

	//----------------------------------------------
	// State inspection methods
	//----------------------------------------------

	bool LocationParsingErrorBuilder::hasError() const
	{
		return !m_errors.empty();
	}

	//----------------------------------------------
	// Static factory method
	//----------------------------------------------

	LocationParsingErrorBuilder LocationParsingErrorBuilder::create()
	{
		return LocationParsingErrorBuilder();
	}

	//----------------------------------------------
	// ParsingErrors construction
	//----------------------------------------------

	ParsingErrors LocationParsingErrorBuilder::build() const
	{
		if ( m_errors.empty() )
		{
			return ParsingErrors::Empty;
		}

		std::vector<ParsingErrors::ErrorEntry> errorEntries;
		errorEntries.reserve( m_errors.size() );

		for ( const auto& [validationResult, message] : m_errors )
		{
			errorEntries.emplace_back( locationValidationResultToStringView( validationResult ), message );
		}

		return ParsingErrors{ std::move( errorEntries ) };
	}

	//----------------------------------------------
	// Error addition
	//----------------------------------------------

	LocationParsingErrorBuilder& LocationParsingErrorBuilder::addError(
		LocationValidationResult validationResult,
		const std::optional<std::string>& message )
	{
		if ( m_errors.size() == m_errors.capacity() ) [[unlikely]]
		{
			/* 8 × sizeof(std::pair<LocationValidationResult, std::string>) = 8 × (4 + 32) = ~288 bytes */
			m_errors.reserve( std::max( static_cast<size_t>( 8 ), m_errors.capacity() * 2 ) );
		}

		if ( message.has_value() ) [[likely]]
		{
			if ( !message->empty() ) [[likely]]
			{
				m_errors.emplace_back( validationResult, *message );
			}
			else
			{
				m_errors.emplace_back( validationResult, std::string{} );
			}
		}
		else
		{
			m_errors.emplace_back( validationResult, std::string{} );
		}

		return *this;
	}
}
