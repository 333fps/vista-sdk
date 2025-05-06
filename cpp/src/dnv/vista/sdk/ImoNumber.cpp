#include "pch.h"

#include "dnv/vista/sdk/ImoNumber.h"

namespace dnv::vista::sdk
{
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
		: m_value( value )
	{
		SPDLOG_INFO( "Created pre-validated IMO number: {}", m_value );
	}

	ImoNumber ImoNumber::parse( const char* value )
	{
		if ( value == nullptr )
		{
			SPDLOG_ERROR( "Null IMO number string" );
			throw std::invalid_argument( "Null IMO number string" );
		}

		return parse( std::string( value ) );
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

	std::optional<ImoNumber> ImoNumber::tryParse( const std::string& value )
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

		std::string processed = value;

		bool hasImoPrefix = processed.size() >= 3 &&
							( std::toupper( processed[0] ) == 'I' ) &&
							( std::toupper( processed[1] ) == 'M' ) &&
							( std::toupper( processed[2] ) == 'O' );

		if ( hasImoPrefix )
		{
			processed = processed.substr( 3 );
			SPDLOG_INFO( "Removed IMO prefix, remaining: '{}'", processed );
		}

		int num = 0;
		try
		{
			num = std::stoi( processed );
		}
		catch ( const std::invalid_argument& )
		{
			SPDLOG_ERROR( "Failed to convert '{}' to integer", processed );
			return std::nullopt;
		}
		catch ( const std::out_of_range& )
		{
			SPDLOG_ERROR( "IMO number out of valid integer range: '{}'", processed );
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

	// IMO number validation according to the standard:
	// https://en.wikipedia.org/wiki/IMO_number
	// An IMO number is made of the three letters "IMO" followed by a seven-digit number.
	// This consists of a six-digit sequential unique number followed by a check digit.
	// The integrity of an IMO number can be verified using its check digit.
	// This is done by multiplying each of the first six digits by a factor
	// of 2 to 7 corresponding to their position from right to left.
	// The rightmost digit of this sum is the check digit.
	// For example, for IMO 9074729: (9×7) + (0×6) + (7×5) + (4×4) + (7×3) + (2×2) = 139
	// The rightmost digit (9) must equal checksum mod 10 (139 % 10 = 9)
	bool ImoNumber::isValid( int imoNumber )
	{
		if ( imoNumber < 1000000 || imoNumber > 9999999 )
		{
			SPDLOG_ERROR( "IMO number outside valid range: {}", imoNumber );
			return false;
		}

		int digits[7];
		int temp = imoNumber;
		for ( int i = 6; i >= 0; i-- )
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

		SPDLOG_INFO( "Validating IMO {}: checksum={}, calculated={}, provided={}, valid={}", imoNumber, checkSum, calculatedCheckDigit, providedCheckDigit, isValid );

		return isValid;
	}

	void ImoNumber::digits( int number, unsigned char* digits )
	{
		int current = number;
		int index = 0;

		while ( current > 0 )
		{
			digits[index++] = static_cast<unsigned char>( current % 10 );
			current /= 10;
		}
	}

	std::string ImoNumber::toString() const
	{
		return "IMO" + std::to_string( m_value );
	}

	ImoNumber::operator int() const
	{
		return m_value;
	}

	bool ImoNumber::operator==( const ImoNumber& other ) const
	{
		return m_value == other.m_value;
	}

	std::size_t ImoNumber::Hash::operator()( const ImoNumber& imoNumber ) const noexcept
	{
		return std::hash<int>{}( imoNumber.m_value );
	}
}
