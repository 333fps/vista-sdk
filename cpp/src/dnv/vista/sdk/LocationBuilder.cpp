/**
 * @file LocationBuilder.cpp
 * @brief Implementation of the LocationBuilder class
 */

#include "pch.h"

#include "dnv/vista/sdk/LocationBuilder.h"

#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/VISVersion.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Static helper methods
		//=====================================================================

		template <typename T>
		static LocationBuilder withValueInternal( const LocationBuilder& builder, LocationGroup group, T value )
		{
			if constexpr ( std::is_same_v<T, int> )
			{
				if ( group != LocationGroup::Number )
				{
					throw std::invalid_argument( "Value should be number" );
				}
				if ( value < 1 )
				{
					throw std::invalid_argument( "Value should be greater than 0" );
				}

				LocationBuilder result = builder;
				result.m_number = value;
				return result;
			}
			else if constexpr ( std::is_same_v<T, char> )
			{
				if ( group == LocationGroup::Number )
				{
					throw std::invalid_argument( "Value should be a character" );
				}

				auto it = builder.m_reversedGroups.find( value );
				if ( it == builder.m_reversedGroups.end() || it->second != group )
				{
					std::string groupName;
					switch ( group )
					{
						case LocationGroup::Side:
							groupName = "Side";
							break;
						case LocationGroup::Vertical:
							groupName = "Vertical";
							break;
						case LocationGroup::Transverse:
							groupName = "Transverse";
							break;
						case LocationGroup::Longitudinal:
							groupName = "Longitudinal";
							break;
						default:
							groupName = "Unknown";
							break;
					}
					throw std::invalid_argument(
						"The value '" + std::string( 1, value ) + "' is an invalid " + groupName + " value" );
				}

				LocationBuilder result = builder;
				switch ( group )
				{
					case LocationGroup::Side:
						result.m_side = value;
						break;
					case LocationGroup::Vertical:
						result.m_vertical = value;
						break;
					case LocationGroup::Transverse:
						result.m_transverse = value;
						break;
					case LocationGroup::Longitudinal:
						result.m_longitudinal = value;
						break;
					default:
						throw std::invalid_argument( "Unsupported LocationGroup" );
				}
				return result;
			}
			else
			{
				static_assert( std::is_same_v<T, int> || std::is_same_v<T, char>,
					"withValueInternal only supports int and char types" );
			}
		}
	}

	//=====================================================================
	// LocationBuilder class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	LocationBuilder::LocationBuilder( VisVersion visVersion, const std::map<char, LocationGroup>& reversedGroups )
		: m_visVersion{ visVersion },
		  m_reversedGroups{ reversedGroups }
	{
	}

	//----------------------------------------------
	// Conversion and comparison
	//----------------------------------------------

	std::string LocationBuilder::toString() const
	{
		std::vector<char> items;

		if ( m_side.has_value() )
		{
			items.push_back( m_side.value() );
		}
		if ( m_vertical.has_value() )
		{
			items.push_back( m_vertical.value() );
		}
		if ( m_transverse.has_value() )
		{
			items.push_back( m_transverse.value() );
		}
		if ( m_longitudinal.has_value() )
		{
			items.push_back( m_longitudinal.value() );
		}

		std::string result;

		if ( m_number.has_value() )
		{
			result += std::to_string( m_number.value() );
		}

		for ( char item : items )
		{
			result += item;
		}

		std::sort( result.begin(), result.end() );

		return result;
	}

	//----------------------------------------------
	// Static factory methods
	//----------------------------------------------

	LocationBuilder LocationBuilder::create( const Locations& locations )
	{
		return LocationBuilder( locations.visVersion(), locations.reversedGroups() );
	}

	//----------------------------------------------
	// Build methods (Immutable fluent interface)
	//----------------------------------------------

	//----------------------------
	// Build
	//----------------------------

	Location LocationBuilder::build() const
	{
		return Location( toString() );
	}

	//----------------------------
	// Location
	//----------------------------

	LocationBuilder LocationBuilder::withLocation( const Location& location ) const
	{
		LocationBuilder builder = *this;

		std::string locationStr = location.toString();
		std::string_view span = locationStr;
		std::optional<int> number;

		for ( size_t i = 0; i < span.length(); ++i )
		{
			char ch = span[i];

			if ( std::isdigit( ch ) )
			{
				if ( !number.has_value() )
				{
					number = ch - '0';
				}
				else
				{
					int parsedNumber;
					if ( !Locations::tryParseInt( span, 0, static_cast<int>( i + 1 ), parsedNumber ) )
					{
						throw std::invalid_argument( "Should include a valid number" );
					}
					number = parsedNumber;
				}
				continue;
			}

			builder = builder.withValue( ch );
		}

		if ( number.has_value() )
		{
			builder = builder.withNumber( number.value() );
		}

		return builder;
	}

	//----------------------------
	// Number
	//----------------------------

	LocationBuilder LocationBuilder::withNumber( int number ) const
	{
		if ( number < 1 )
		{
			throw std::invalid_argument( "Value should be greater than 0" );
		}

		LocationBuilder result = *this;
		result.m_number = number;

		return result;
	}

	LocationBuilder LocationBuilder::withoutNumber() const
	{
		LocationBuilder result = *this;
		result.m_number = std::nullopt;

		return result;
	}

	//----------------------------
	// Side
	//----------------------------

	LocationBuilder LocationBuilder::withSide( char side ) const
	{
		auto it = m_reversedGroups.find( side );
		if ( it == m_reversedGroups.end() || it->second != LocationGroup::Side )
		{
			throw std::invalid_argument(
				"The value '" + std::string( 1, side ) + "' is an invalid Side value" );
		}

		LocationBuilder result = *this;
		result.m_side = side;

		return result;
	}

	LocationBuilder LocationBuilder::withoutSide() const
	{
		LocationBuilder result = *this;
		result.m_side = std::nullopt;

		return result;
	}

	//----------------------------
	// Vertical
	//----------------------------

	LocationBuilder LocationBuilder::withVertical( char vertical ) const
	{
		auto it = m_reversedGroups.find( vertical );
		if ( it == m_reversedGroups.end() || it->second != LocationGroup::Vertical )
		{
			throw std::invalid_argument(
				"The value '" + std::string( 1, vertical ) + "' is an invalid Vertical value" );
		}

		LocationBuilder result = *this;
		result.m_vertical = vertical;

		return result;
	}

	LocationBuilder LocationBuilder::withoutVertical() const
	{
		LocationBuilder result = *this;
		result.m_vertical = std::nullopt;

		return result;
	}

	//----------------------------
	// Transverse
	//----------------------------

	LocationBuilder LocationBuilder::withTransverse( char transverse ) const
	{
		auto it = m_reversedGroups.find( transverse );
		if ( it == m_reversedGroups.end() || it->second != LocationGroup::Transverse )
		{
			throw std::invalid_argument(
				"The value '" + std::string( 1, transverse ) + "' is an invalid Transverse value" );
		}

		LocationBuilder result = *this;
		result.m_transverse = transverse;

		return result;
	}

	LocationBuilder LocationBuilder::withoutTransverse() const
	{
		LocationBuilder result = *this;
		result.m_transverse = std::nullopt;

		return result;
	}

	//----------------------------
	// Longitudinal
	//----------------------------

	LocationBuilder LocationBuilder::withLongitudinal( char longitudinal ) const
	{
		auto it = m_reversedGroups.find( longitudinal );
		if ( it == m_reversedGroups.end() || it->second != LocationGroup::Longitudinal )
		{
			throw std::invalid_argument(
				"The value '" + std::string( 1, longitudinal ) + "' is an invalid Longitudinal value" );
		}

		LocationBuilder result = *this;
		result.m_longitudinal = longitudinal;

		return result;
	}

	LocationBuilder LocationBuilder::withoutLongitudinal() const
	{
		LocationBuilder result = *this;
		result.m_longitudinal = std::nullopt;

		return result;
	}

	//----------------------------
	// Value
	//----------------------------

	LocationBuilder LocationBuilder::withValue( int value ) const
	{
		return withNumber( value );
	}

	LocationBuilder LocationBuilder::withValue( char value ) const
	{
		auto it = m_reversedGroups.find( value );
		if ( it == m_reversedGroups.end() )
		{
			throw std::invalid_argument( "The value '" + std::string( 1, value ) + "' is an invalid Locations value" );
		}

		LocationGroup group = it->second;

		switch ( group )
		{
			case LocationGroup::Side:
				return withSide( value );
			case LocationGroup::Vertical:
				return withVertical( value );
			case LocationGroup::Transverse:
				return withTransverse( value );
			case LocationGroup::Longitudinal:
				return withLongitudinal( value );
			case LocationGroup::Number:
				throw std::invalid_argument( "Number group should not contain character values" );
			default:
				throw std::invalid_argument( "Unsupported LocationGroup" );
		}
	}

	LocationBuilder LocationBuilder::withoutValue( LocationGroup group ) const
	{
		switch ( group )
		{
			case LocationGroup::Number:
				return withoutNumber();
			case LocationGroup::Side:
				return withoutSide();
			case LocationGroup::Vertical:
				return withoutVertical();
			case LocationGroup::Transverse:
				return withoutTransverse();
			case LocationGroup::Longitudinal:
				return withoutLongitudinal();
			default:
				throw std::invalid_argument( "Unsupported LocationGroup" );
		}
	}
}
