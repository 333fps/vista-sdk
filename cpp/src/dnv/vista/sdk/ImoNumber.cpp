/**
 * @file ImoNumber.cpp
 * @brief Implementation of the ImoNumber class
 */

#include "pch.h"

#include "dnv/vista/sdk/ImoNumber.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Construction / Destruction
	//=====================================================================

	ImoNumber::ImoNumber( int value )
	{
		if ( !isValid( value ) )
		{
			SPDLOG_ERROR( "Invalid IMO number: {}", value );
			throw std::invalid_argument( "Invalid IMO number: " + std::to_string( value ) );
		}

		m_value = value;

		SPDLOG_INFO( "Created IMO number: {}", m_value );
	}

	ImoNumber::ImoNumber( const std::string& value )
	{
		auto result = tryParse( value );
		if ( !result )
		{
			SPDLOG_ERROR( "Invalid IMO number string: '{}'", value );
			throw std::invalid_argument( "Invalid IMO number: " + value );
		}

		m_value = static_cast<int>( *result );

		SPDLOG_INFO( "Created IMO number: {} from string '{}'", m_value, value );
	}

	ImoNumber::ImoNumber( int value, [[maybe_unused]] bool bUnused ) noexcept
		: m_value{ value }
	{
		SPDLOG_INFO( "Created pre-validated IMO number: {}", m_value );
	}

	ImoNumber::ImoNumber( const ImoNumber& ) = default; /* TODO - transfer in .h file later */

	ImoNumber::ImoNumber( ImoNumber&& ) noexcept = default; /* TODO - transfer in .h file later */

	//=====================================================================
	// Comparison Operators
	//=====================================================================

	bool ImoNumber::operator==( const ImoNumber& other ) const
	{
		return m_value == other.m_value;
	}

	bool ImoNumber::operator!=( const ImoNumber& other ) const
	{
		return !( *this == other );
	}

	//=====================================================================
	// Conversion Operator
	//=====================================================================

	ImoNumber::operator int() const
	{
		return m_value;
	}

	//=====================================================================
	// Public Member Methods
	//=====================================================================

	std::string ImoNumber::toString() const
	{
		return "IMO" + std::to_string( m_value );
	}

	size_t ImoNumber::hashCode() const noexcept
	{
		return std::hash<int>{}( m_value );
	}

	//=====================================================================
	// Static Public Methods
	//=====================================================================

	/*
		IMO number validation according to the standard:
		https://en.wikipedia.org/wiki/IMO_number
		An IMO number is made of the three letters "IMO" followed by a seven-digit number.
		This consists of a six-digit sequential unique number followed by a check digit.
		The integrity of an IMO number can be verified using its check digit.
		This is done by multiplying each of the first six digits by a factor
		of 2 to 7 corresponding to their position from right to left.
		The rightmost digit of this sum is the check digit.
		For example, for IMO 9074729: (9×7) + (0×6) + (7×5) + (4×4) + (7×3) + (2×2) = 139
		The rightmost digit (9) must equal checksum mod 10 (139 % 10 = 9)
	*/
	bool ImoNumber::isValid( int imoNumber )
	{
		if ( imoNumber < 1000000 || imoNumber > 9999999 )
		{
			SPDLOG_ERROR( "IMO number outside valid range: {}", imoNumber );

			return false;
		}

		int digits[7];
		int temp = imoNumber;
		for ( int i = 6; i >= 0; --i )
		{
			digits[i] = temp % 10;
			temp /= 10;
		}

		int checkSum = 0;
		for ( int i = 0; i < 6; i++ )
		{
			checkSum += digits[i] * ( 7 - i );
		}

		int calculatedCheckDigit = checkSum % 10;
		int providedCheckDigit = digits[6];

		bool isValid = ( providedCheckDigit == calculatedCheckDigit );

		SPDLOG_DEBUG( "Validating IMO {}: checksum={}, calculated={}, provided={}, valid={}", imoNumber, checkSum, calculatedCheckDigit, providedCheckDigit, isValid );

		return isValid;
	}

	ImoNumber ImoNumber::parse( const char* value )
	{
		if ( value == nullptr )
		{
			SPDLOG_ERROR( "Null IMO number string" );
			throw std::invalid_argument( "Null IMO number string" );
		}
		auto result = tryParse( std::string_view( value ) );
		if ( !result )
		{
			std::string error_message = "Failed to parse ImoNumber: ";
			error_message += value;
			SPDLOG_ERROR( "{}", error_message );
			throw std::invalid_argument( error_message );
		}
		SPDLOG_INFO( "Successfully parsed IMO number: {}", static_cast<int>( *result ) );

		return *result;
	}

	ImoNumber ImoNumber::parse( const std::string& value )
	{
		if ( value.empty() )
		{
			SPDLOG_ERROR( "Empty IMO number string" );
			throw std::invalid_argument( "Empty IMO number string" );
		}

		auto result = tryParse( value );
		if ( !result )
		{
			SPDLOG_ERROR( "Failed to parse ImoNumber: '{}'", value );
			throw std::invalid_argument( "Failed to parse ImoNumber: " + value );
		}

		SPDLOG_INFO( "Successfully parsed IMO number: {}", static_cast<int>( *result ) );

		return *result;
	}

	std::optional<ImoNumber> ImoNumber::tryParse( const std::string_view value )
	{
		if ( value.empty() )
		{
			SPDLOG_INFO( "Empty IMO number string" );

			return std::nullopt;
		}

		if ( value.find_first_of( " \t\n\r\f\v" ) != std::string::npos )
		{
			SPDLOG_INFO( "IMO number contains whitespace: '{}'", value );

			return std::nullopt;
		}

		std::string_view sv = value;
		bool hasImoPrefix = sv.length() >= 3 &&
							( std::toupper( static_cast<unsigned char>( sv[0] ) ) == 'I' ) &&
							( std::toupper( static_cast<unsigned char>( sv[1] ) ) == 'M' ) &&
							( std::toupper( static_cast<unsigned char>( sv[2] ) ) == 'O' );

		if ( hasImoPrefix )
		{
			sv = sv.substr( 3 );
			SPDLOG_INFO( "Removed IMO prefix, remaining: '{}'", fmt::string_view( sv.data(), sv.size() ) );
		}

		int num = 0;
		auto [ptr, ec] = std::from_chars( sv.data(), sv.data() + sv.length(), num );

		if ( ec != std::errc() )
		{
			if ( ec == std::errc::invalid_argument )
			{
				SPDLOG_ERROR( "Failed to convert '{}' to integer", fmt::string_view( sv.data(), sv.size() ) );
			}
			else if ( ec == std::errc::result_out_of_range )
			{
				SPDLOG_ERROR( "IMO number out of valid integer range: '{}'", fmt::string_view( sv.data(), sv.size() ) );
			}
			return std::nullopt;
		}

		if ( num == 0 || !isValid( num ) )
		{
			SPDLOG_ERROR( "Invalid IMO number format or checksum: {}", num );

			return std::nullopt;
		}

		SPDLOG_INFO( "Successfully parsed IMO number {} from string '{}'", num, value );

		return ImoNumber( num, true );
	}
}
