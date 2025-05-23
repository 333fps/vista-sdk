/**
 * @file Locations.cpp
 * @brief Implements the Locations, Location, RelativeLocation, and related helper classes.
 */

#include "pch.h"

#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/LocationParsingErrorBuilder.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Location Class
	//=====================================================================

	Location::Location( const std::string& value )
		: m_value{ value }
	{
		SPDLOG_DEBUG( "Created Location: '{}'", value );
	}

	//----------------------------------------------
	// Comparison Operators
	//----------------------------------------------

	bool Location::operator==( const Location& other ) const
	{
		return m_value == other.m_value;
	}

	bool Location::operator!=( const Location& other ) const
	{
		return !( m_value == other.m_value );
	}

	//----------------------------------------------
	// Conversion Operators
	//----------------------------------------------

	Location::operator std::string() const noexcept
	{
		return m_value;
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& Location::value() const noexcept
	{
		return m_value;
	}

	//----------------------------------------------
	// Conversion
	//----------------------------------------------

	std::string Location::toString() const noexcept
	{
		return m_value;
	}

	//=====================================================================
	// RelativeLocation Class
	//=====================================================================

	RelativeLocation::RelativeLocation( char code, const std::string& name,
		const Location& location,
		const std::optional<std::string> definition )
		: m_code( code ), m_name( name ), m_location( location ), m_definition( definition )
	{
		SPDLOG_DEBUG( "Created RelativeLocation: code={}, name={}", code, name );
	}

	//----------------------------------------------
	// Comparison Operators
	//----------------------------------------------

	bool RelativeLocation::operator==( const RelativeLocation& other ) const
	{
		return m_code == other.m_code;
	}

	bool RelativeLocation::operator!=( const RelativeLocation& other ) const
	{
		return !( m_code == other.m_code );
	}

	//----------------------------------------------
	// Public Methods
	//----------------------------------------------

	size_t RelativeLocation::hashCode() const noexcept
	{
		return std::hash<char>{}( m_code );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	char RelativeLocation::code() const noexcept
	{
		return m_code;
	}

	const std::string& RelativeLocation::name() const noexcept
	{
		return m_name;
	}

	const std::optional<std::string>& RelativeLocation::definition() const noexcept
	{
		return m_definition;
	}

	const Location& RelativeLocation::location() const noexcept
	{
		return m_location;
	}

	//=====================================================================
	// LocationCharDict Class
	//=====================================================================

	//----------------------------------------------
	// Lookup Operators
	//----------------------------------------------

	std::optional<char>& LocationCharDict::operator[]( LocationGroup key )
	{
		if ( static_cast<int>( key ) <= 0 )
		{
			SPDLOG_ERROR( "Invalid LocationGroup key: {}", static_cast<int>( key ) );
			throw std::runtime_error( "Unsupported code: " + std::to_string( static_cast<int>( key ) ) );
		}

		auto index{ static_cast<size_t>( key ) - 1 };
		if ( index >= m_table.size() )
		{
			SPDLOG_ERROR( "Index out of range for LocationCharDict: {}", index );
			throw std::runtime_error( "Unsupported code: " + std::to_string( static_cast<int>( key ) ) );
		}
		return m_table[index];
	}

	//----------------------------------------------
	// Public Methods
	//----------------------------------------------

	bool LocationCharDict::tryAdd( LocationGroup key, char value, std::optional<char>& existingValue )
	{
		auto& v = ( *this )[key];
		if ( v.has_value() )
		{
			existingValue = v;
			SPDLOG_INFO( "Failed to add '{}' for group {} - already has '{}'",
				value, static_cast<int>( key ), v.value() );
			return false;
		}

		existingValue = std::nullopt;
		v = value;
		SPDLOG_INFO( "Added '{}' for group {}", value, static_cast<int>( key ) );
		return true;
	}

	//=====================================================================
	// Locations Class
	//=====================================================================

	Locations::Locations( VisVersion version, const LocationsDto& dto )
		: m_visVersion( version )
	{
		SPDLOG_DEBUG( "Initializing Locations for VIS version {}", static_cast<int>( version ) );

		m_locationCodes.reserve( dto.items().size() );
		for ( const auto& item : dto.items() )
		{
			m_locationCodes.push_back( item.code() );
		}
		SPDLOG_INFO( "Collected {} location codes", m_locationCodes.size() );

		m_relativeLocations.reserve( dto.items().size() );
		for ( const auto& relLocDto : dto.items() )
		{
			Location loc( std::string( 1, relLocDto.code() ) );

			RelativeLocation relLoc(
				relLocDto.code(),
				relLocDto.name(),
				loc,
				relLocDto.definition() );

			m_relativeLocations.push_back( relLoc );

			if ( relLocDto.code() == 'H' || relLocDto.code() == 'V' )
			{
				SPDLOG_INFO( "Skipping special code: {}", relLocDto.code() );
				continue;
			}

			LocationGroup key;
			if ( relLocDto.code() == 'N' )
			{
				key = LocationGroup::Number;
			}
			else if ( relLocDto.code() == 'P' || relLocDto.code() == 'C' || relLocDto.code() == 'S' )
			{
				key = LocationGroup::Side;
			}
			else if ( relLocDto.code() == 'U' || relLocDto.code() == 'M' || relLocDto.code() == 'L' )
			{
				key = LocationGroup::Vertical;
			}
			else if ( relLocDto.code() == 'I' || relLocDto.code() == 'O' )
			{
				key = LocationGroup::Transverse;
			}
			else if ( relLocDto.code() == 'F' || relLocDto.code() == 'A' )
			{
				key = LocationGroup::Longitudinal;
			}
			else
			{
				SPDLOG_ERROR( "Unsupported code: {}", relLocDto.code() );
				throw std::runtime_error( std::string( "Unsupported code: " ) + relLocDto.code() );
			}

			if ( m_groups.find( key ) == m_groups.end() )
			{
				m_groups[key] = std::vector<RelativeLocation>();
			}

			if ( key == LocationGroup::Number )
			{
				continue;
			}

			m_reversedGroups[relLocDto.code()] = key;
			m_groups[key].push_back( relLoc );
		}

		SPDLOG_INFO( "Loaded {} relative locations in {} groups",
			m_relativeLocations.size(), m_groups.size() );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	VisVersion Locations::visVersion() const noexcept
	{
		return m_visVersion;
	}

	const std::vector<RelativeLocation>& Locations::relativeLocations() const noexcept
	{
		return m_relativeLocations;
	}

	const std::unordered_map<LocationGroup, std::vector<RelativeLocation>>& Locations::groups() const noexcept
	{
		return m_groups;
	}

	//----------------------------------------------
	// Public Methods - Parsing
	//----------------------------------------------

	Location Locations::parse( std::string_view locationStr ) const
	{
		SPDLOG_INFO( "Parsing location string view: '{}'", locationStr );
		Location location;
		if ( !tryParse( locationStr, location ) )
		{
			SPDLOG_ERROR( "Failed to parse location: '{}'", locationStr );
			throw std::invalid_argument( "Invalid location: " + std::string( locationStr ) );
		}

		SPDLOG_INFO( "Successfully parsed location: '{}'", location.value() );

		return location;
	}

	bool Locations::tryParse( const std::string& value, Location& location ) const
	{
		return tryParse( std::string_view( value ), location );
	}

	bool Locations::tryParse( const std::optional<std::string>& value, Location& location ) const
	{
		if ( !value.has_value() )
		{
			SPDLOG_INFO( "Can't parse null location" );
			return false;
		}

		LocationParsingErrorBuilder errorBuilder;

		return tryParseInternal( value.value(), value, location, errorBuilder );
	}

	bool Locations::tryParse( const std::optional<std::string>& value, Location& location, ParsingErrors& errors ) const
	{
		if ( !value.has_value() )
		{
			SPDLOG_INFO( "Can't parse null location" );

			std::vector<ParsingErrors::ErrorEntry> errorEntries;
			errorEntries.emplace_back( std::string( "0" ), std::string( "Location is null" ) );
			errors = ParsingErrors( errorEntries );

			return false;
		}

		LocationParsingErrorBuilder errorBuilder;
		bool result = tryParseInternal( value.value(), value, location, errorBuilder );
		if ( !result )
		{
			errors = errorBuilder.build();
		}

		return result;
	}

	bool Locations::tryParse( std::string_view value, Location& location ) const
	{
		LocationParsingErrorBuilder errorBuilder;

		return tryParseInternal( value, std::nullopt, location, errorBuilder );
	}

	bool Locations::tryParse( std::string_view value, Location& location, ParsingErrors& errors ) const
	{
		LocationParsingErrorBuilder errorBuilder;
		bool result = tryParseInternal( value, std::nullopt, location, errorBuilder );
		if ( !result )
		{
			errors = errorBuilder.build();
		}
		return result;
	}

	//----------------------------------------------
	// Private Static Helper Methods
	//----------------------------------------------

	bool Locations::tryParseInt( std::string_view span, int start, int length, int& number )
	{
		SPDLOG_INFO( "Parsing integer from position {} with length {}", start, length );
		if ( start < 0 || length <= 0 || static_cast<size_t>( start + length ) > span.length() )
		{
			SPDLOG_ERROR( "Invalid range for integer parsing: start={}, length={}, span length={}",
				start, length, span.length() );

			return false;
		}
		const char* begin = span.data() + start;
		const char* end = begin + length;
		auto result = std::from_chars( begin, end, number );
		if ( result.ec == std::errc() && result.ptr == end )
		{
			SPDLOG_INFO( "Successfully parsed integer: {}", number );

			return true;
		}

		SPDLOG_ERROR( "Failed to parse integer from string_view segment." );

		return false;
	}

	//----------------------------------------------
	// Private Methods
	//----------------------------------------------

	bool Locations::tryParseInternal( std::string_view span,
		const std::optional<std::string>& originalStr,
		Location& location,
		LocationParsingErrorBuilder& errorBuilder ) const
	{
		SPDLOG_INFO( "Parsing location: '{}'", span );

		auto displayString = [&span, &originalStr]() -> std::string {
			return originalStr.has_value() ? *originalStr : std::string( span );
		};

		if ( span.empty() )
		{
			errorBuilder.addError( LocationValidationResult::NullOrWhiteSpace,
				"Invalid location: contains only whitespace in '" + displayString() + "'" );
			return false;
		}

		bool isOnlyWhitespace = std::all_of( span.begin(), span.end(), []( unsigned char c_uc ) { return std::isspace( c_uc ); } );

		if ( isOnlyWhitespace )
		{
			errorBuilder.addError( LocationValidationResult::NullOrWhiteSpace,
				"Invalid location: contains only whitespace in '" + displayString() + "'" );
			return false;
		}

		std::string result;
		LocationCharDict charDict{};

		int digitStartIndex = -1;
		int prevDigitIndex = -1;
		int charsStartIndex = -1;

		for ( size_t i = 0; i < span.length(); i++ )
		{
			char ch = span[i];

			if ( std::isdigit( ch ) )
			{
				if ( charsStartIndex != -1 )
				{
					SPDLOG_INFO( "Digit found after location codes at position {}", i );
					errorBuilder.addError( LocationValidationResult::InvalidOrder,
						"Invalid location: numeric part must come before location codes in '" + displayString() + "'" );
					return false;
				}

				if ( prevDigitIndex != -1 && prevDigitIndex != static_cast<int>( i ) - 1 )
				{
					SPDLOG_INFO( "Discontinuous digit sequence at position {}", i );
					errorBuilder.addError( LocationValidationResult::Invalid,
						"Invalid location: cannot have multiple separated digits in '" + displayString() + "'" );
					return false;
				}

				if ( digitStartIndex == -1 )
				{
					digitStartIndex = static_cast<int>( i );
				}
				prevDigitIndex = static_cast<int>( i );

				SPDLOG_INFO( "Found digit at position {}: '{}'", i, ch );
				result.push_back( ch );
				continue;
			}

			if ( charsStartIndex == -1 )
			{
				charsStartIndex = static_cast<int>( i );
			}

			bool valid = false;
			for ( char code : m_locationCodes )
			{
				if ( code == ch )
				{
					valid = true;
					break;
				}
			}

			if ( !valid )
			{
				SPDLOG_INFO( "Invalid location code: '{}' at position {}", ch, i );

				std::string invalidChars;
				std::string source = displayString();
				bool first = true;

				for ( char c : source )
				{
					if ( !std::isdigit( c ) && ( c == 'N' ||
												   std::find( m_locationCodes.begin(), m_locationCodes.end(), c ) == m_locationCodes.end() ) )
					{
						if ( !first )
							invalidChars += ",";
						first = false;
						invalidChars += "'" + std::string( 1, c ) + "'";
					}
				}

				errorBuilder.addError( LocationValidationResult::InvalidCode,
					"Invalid location code: '" + displayString() + "' with invalid location code(s): " + invalidChars );
				return false;
			}

			if ( i > 0 && charsStartIndex != static_cast<int>( i ) )
			{
				char prevCh = span[i - 1];
				if ( !std::isdigit( prevCh ) && ch < prevCh )
				{
					SPDLOG_INFO( "Location codes not in alphabetical order: '{}' after '{}'", ch, prevCh );
					errorBuilder.addError( LocationValidationResult::InvalidOrder,
						"Invalid location: codes must be alphabetically sorted in location: '" + displayString() + "'" );
					return false;
				}
			}

			SPDLOG_INFO( "Found location code at position {}: '{}'", i, ch );
			result.push_back( ch );

			if ( m_reversedGroups.find( ch ) != m_reversedGroups.end() )
			{
				LocationGroup group = m_reversedGroups.at( ch );
				std::optional<char> existingValue;

				if ( !charDict.tryAdd( group, ch, existingValue ) )
				{
					SPDLOG_INFO( "Duplicate location code from group {}: '{}' and '{}'",
						static_cast<int>( group ), existingValue.value(), ch );
					errorBuilder.addError( LocationValidationResult::InvalidOrder,
						"Duplicate location code from the same group in '" + displayString() + "': " +
							std::string( 1, existingValue.value() ) + " and " + std::string( 1, ch ) );
					return false;
				}
			}
		}

		location = Location( originalStr.has_value() ? *originalStr : result );
		SPDLOG_INFO( "Successfully parsed location: '{}'", location.value() );
		return true;
	}
}
