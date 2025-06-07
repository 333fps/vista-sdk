/**
 * @file Locations.cpp
 * @brief Implements the Locations, Location, RelativeLocation, and related helper classes.
 */

#include "pch.h"

#include "dnv/vista/sdk/Locations.h"

#include "dnv/vista/sdk/LocationParsingErrorBuilder.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VISVersion.h"

namespace dnv::vista::sdk
{
	namespace
	{
		static std::string groupNameToString( LocationGroup group )
		{
			switch ( group )
			{
				case LocationGroup::Number:
					return "Number";
				case LocationGroup::Side:
					return "Side";
				case LocationGroup::Vertical:
					return "Vertical";
				case LocationGroup::Transverse:
					return "Transverse";
				case LocationGroup::Longitudinal:
					return "Longitudinal";
				default:
					return "Unknown";
			}
		}
	}

	//=====================================================================
	// Location Class
	//=====================================================================

	Location::Location( std::string_view value ) : m_value{ value }
	{
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
		: m_code{ code },
		  m_name{ name },
		  m_location{ location },
		  m_definition{ definition }
	{
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
	// Public methods
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
	// Lookup operators
	//----------------------------------------------

	std::optional<char>& LocationCharDict::operator[]( LocationGroup key )
	{
		if ( static_cast<int>( key ) <= 0 )
		{
			throw std::runtime_error( "Unsupported code: " + std::to_string( static_cast<int>( key ) ) );
		}

		auto index{ static_cast<size_t>( key ) - 1 };
		if ( index >= m_table.size() )
		{
			throw std::runtime_error( "Unsupported code: " + std::to_string( static_cast<int>( key ) ) );
		}

		return m_table[index];
	}

	//----------------------------------------------
	// Public methods
	//----------------------------------------------

	bool LocationCharDict::tryAdd( LocationGroup key, char value, std::optional<char>& existingValue )
	{
		auto& v = ( *this )[key];
		if ( v.has_value() )
		{
			existingValue = v;

			return false;
		}

		existingValue = std::nullopt;
		v = value;

		return true;
	}

	//=====================================================================
	// Locations Class
	//=====================================================================

	Locations::Locations( VisVersion version, const LocationsDto& dto )
		: m_visVersion{ version }
	{
		m_locationCodes.reserve( dto.items().size() );
		for ( const auto& item : dto.items() )
		{
			m_locationCodes.push_back( item.code() );
		}

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

	const std::map<char, LocationGroup>& Locations::reversedGroups() const noexcept
	{
		return m_reversedGroups;
	}

	//----------------------------------------------
	// Public methods - Parsing
	//----------------------------------------------

	Location Locations::parse( std::string_view locationStr ) const
	{
		Location location;
		if ( !tryParse( locationStr, location ) )
		{
			throw std::invalid_argument( "Invalid location: " + std::string( locationStr ) );
		}

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
			return false;
		}

		LocationParsingErrorBuilder errorBuilder;

		return tryParseInternal( value.value(), value, location, errorBuilder );
	}

	bool Locations::tryParse( const std::optional<std::string>& value, Location& location, ParsingErrors& errors ) const
	{
		if ( !value.has_value() )
		{
			LocationParsingErrorBuilder errorBuilder = LocationParsingErrorBuilder::create();

			errorBuilder.addError( LocationValidationResult::NullOrWhiteSpace, "Location is null" );

			errors = errorBuilder.build();

			location = Location{};

			return false;
		}

		LocationParsingErrorBuilder errorBuilder = LocationParsingErrorBuilder::create();

		bool result = tryParseInternal( value.value(), value, location, errorBuilder );
		errors = errorBuilder.build();

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
	// Public static helper methods
	//----------------------------------------------

	bool Locations::tryParseInt( std::string_view span, int start, int length, int& number )
	{
		if ( start < 0 || length <= 0 || static_cast<size_t>( start + length ) > span.length() )
		{
			return false;
		}

		const char* begin = span.data() + start;
		const char* end = begin + length;
		auto result = std::from_chars( begin, end, number );
		if ( result.ec == std::errc() && result.ptr == end )
		{
			return true;
		}

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
		auto displayString = [&span, &originalStr]() -> std::string {
			return originalStr.has_value() ? *originalStr : std::string( span );
		};

		if ( span.empty() )
		{
			errorBuilder.addError( LocationValidationResult::NullOrWhiteSpace,
				"Invalid location: contains only whitespace" );

			return false;
		}

		bool isOnlyWhitespace = std::all_of( span.begin(), span.end(), []( unsigned char c_uc ) { return std::isspace( c_uc ); } );

		if ( isOnlyWhitespace )
		{
			errorBuilder.addError( LocationValidationResult::NullOrWhiteSpace,
				"Invalid location: contains only whitespace" );

			return false;
		}

		std::string result;
		LocationCharDict charDict{};

		int digitStartIndex = -1;
		int prevDigitIndex = -1;
		int charsStartIndex = -1;

		for ( size_t i = 0; i < span.length(); ++i )
		{
			char ch = span[i];

			if ( std::isdigit( ch ) )
			{
				if ( prevDigitIndex != -1 && prevDigitIndex != static_cast<int>( i ) - 1 )
				{
					errorBuilder.addError( LocationValidationResult::Invalid,
						"Invalid location: cannot have multiple separated digits in location: '" + displayString() + "'" );

					return false;
				}

				if ( charsStartIndex != -1 )
				{
					errorBuilder.addError( LocationValidationResult::InvalidOrder,
						"Invalid location: numeric location should start before location code(s) in location: '" + displayString() + "'" );

					return false;
				}

				if ( digitStartIndex == -1 )
				{
					digitStartIndex = static_cast<int>( i );
				}

				prevDigitIndex = static_cast<int>( i );

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
				std::string invalidChars;
				std::string source = displayString();
				bool first = true;

				for ( char c : source )
				{
					if ( !std::isdigit( c ) && ( c == 'N' || std::find( m_locationCodes.begin(), m_locationCodes.end(), c ) == m_locationCodes.end() ) )
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

			if ( m_reversedGroups.find( ch ) != m_reversedGroups.end() )
			{
				LocationGroup group = m_reversedGroups.at( ch );
				std::optional<char> existingValue;

				if ( !charDict.tryAdd( group, ch, existingValue ) )
				{
					std::string groupName = groupNameToString( group );
					errorBuilder.addError( LocationValidationResult::Invalid,
						"Invalid location: Multiple '" + groupName + "' values. Got both '" +
							std::string( 1, existingValue.value() ) + "' and '" + std::string( 1, ch ) + "' in '" + displayString() + "'" );

					return false;
				}
			}

			if ( i > 0 && charsStartIndex != static_cast<int>( i ) )
			{
				char prevCh = span[i - 1];
				if ( !std::isdigit( prevCh ) && ch < prevCh )
				{
					errorBuilder.addError( LocationValidationResult::InvalidOrder,
						"Invalid location: '" + displayString() + "' not alphabetically sorted" );

					return false;
				}
			}

			result.push_back( ch );
		}

		location = Location( originalStr.has_value() ? *originalStr : result );

		return true;
	}
}
