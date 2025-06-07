/**
 * @file LocalIdItems.cpp
 * @brief Implementation of the LocalIdItems class.
 */
#include "pch.h"

#include "dnv/vista/sdk/LocalIdItems.h"

#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Constructors, Destructor, Assignment Operators
	//-------------------------------------------------------------------------

	LocalIdItems::LocalIdItems( GmodPath&& primaryItem, std::optional<GmodPath>&& secondaryItem )
		: m_primaryItem( std::move( primaryItem ) ),
		  m_secondaryItem( std::move( secondaryItem ) )
	{
	}

	LocalIdItems::LocalIdItems( LocalIdItems&& other, GmodPath&& newPrimaryItem )
		: m_primaryItem( std::move( newPrimaryItem ) ),
		  m_secondaryItem( std::move( other.m_secondaryItem ) )
	{
	}

	LocalIdItems::LocalIdItems( LocalIdItems&& other, std::optional<GmodPath>&& newSecondaryItem )
		: m_primaryItem( std::move( other.m_primaryItem ) ),
		  m_secondaryItem( std::move( newSecondaryItem ) )
	{
	}

	//-------------------------------------------------------------------------
	// Public Accessors (Read-only)
	//-------------------------------------------------------------------------

	const std::optional<GmodPath>& LocalIdItems::primaryItem() const noexcept
	{
		return m_primaryItem;
	}

	const std::optional<GmodPath>& LocalIdItems::secondaryItem() const noexcept
	{
		return m_secondaryItem;
	}

	bool LocalIdItems::isEmpty() const noexcept
	{
		return !m_primaryItem.has_value() && !m_secondaryItem.has_value();
	}

	//-------------------------------------------------------------------------
	// Public methods
	//-------------------------------------------------------------------------

	void LocalIdItems::append( std::stringstream& builder, bool verboseMode ) const
	{
		if ( !m_primaryItem && !m_secondaryItem )
			return;

		if ( m_primaryItem )
		{
			m_primaryItem->toString( builder );
			builder << '/';
		}

		if ( m_secondaryItem )
		{
			builder << "sec/";
			m_secondaryItem->toString( builder );
			builder << '/';
		}

		if ( verboseMode )
		{
			if ( m_primaryItem )
			{
				for ( const auto& [depth, name] : m_primaryItem->commonNames() )
				{
					builder << '~';
					auto nodePtr = ( *m_primaryItem )[depth];
					std::optional<std::string> locationStr;
					if ( nodePtr && nodePtr->location().has_value() )
					{
						locationStr = nodePtr->location()->toString();
					}
					appendCommonName( builder, name, locationStr );
					builder << '/';
				}
			}

			if ( m_secondaryItem )
			{
				std::string prefix = "~for.";
				for ( const auto& [depth, name] : m_secondaryItem->commonNames() )
				{
					builder << prefix;
					if ( prefix != "~" )
						prefix = "~";

					auto nodePtr = ( *m_secondaryItem )[depth];
					std::optional<std::string> locationStr;
					if ( nodePtr && nodePtr->location().has_value() )
					{
						locationStr = nodePtr->location()->toString();
					}
					appendCommonName( builder, name, locationStr );
					builder << '/';
				}
			}
		}
	}

	//-------------------------------------------------------------------------
	// Comparison Operators
	//-------------------------------------------------------------------------

	bool LocalIdItems::operator==( const LocalIdItems& other ) const noexcept
	{
		return m_primaryItem == other.m_primaryItem && m_secondaryItem == other.m_secondaryItem;
	}

	bool LocalIdItems::operator!=( const LocalIdItems& other ) const noexcept
	{
		return !( *this == other );
	}

	//-------------------------------------------------------------------------
	// Private helper methods
	//-------------------------------------------------------------------------

	void LocalIdItems::appendCommonName(
		std::stringstream& builder,
		std::string_view commonName,
		const std::optional<std::string>& location )
	{
		char prev = '\0';

		for ( const char ch : commonName )
		{
			if ( ch == '/' )
				continue;

			if ( prev == ' ' && ch == ' ' )
				continue;

			char current = ch;
			switch ( ch )
			{
				case ' ':
					current = '.';
					break;
				default:
					if ( !VIS::isISOString( ch ) )
					{
						current = '.';
					}
					else
					{
						current = static_cast<char>( std::tolower( static_cast<unsigned char>( ch ) ) );
					}
					break;
			}

			if ( current == '.' && prev == '.' )
				continue;

			builder << current;
			prev = current;
		}

		if ( location.has_value() && !location->empty() )
		{
			builder << '.';
			builder << *location;
		}
	}
}
