/**
 * @file LocalIdItems.cpp
 * @brief Implementation of the LocalIdItems class.
 */
#include "pch.h"

#include "dnv/vista/sdk/LocalIdItems.h"
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
		SPDLOG_DEBUG( "LocalIdItems created via base move constructor." );
		if ( m_primaryItem )
		{
			SPDLOG_TRACE( "  Primary Item: {}", m_primaryItem->toString() );
		}
		if ( m_secondaryItem )
		{
			SPDLOG_TRACE( "  Secondary Item: {}", m_secondaryItem->toString() );
		}
	}

	LocalIdItems::LocalIdItems( LocalIdItems&& other, GmodPath&& newPrimaryItem )
		: m_primaryItem( std::move( newPrimaryItem ) ),
		  m_secondaryItem( std::move( other.m_secondaryItem ) )
	{
		SPDLOG_TRACE( "LocalIdItems created by replacing primary via move." );
	}

	LocalIdItems::LocalIdItems( LocalIdItems&& other, std::optional<GmodPath>&& newSecondaryItem )
		: m_primaryItem( std::move( other.m_primaryItem ) ),
		  m_secondaryItem( std::move( newSecondaryItem ) )
	{
		SPDLOG_TRACE( "LocalIdItems created by replacing secondary via move." );
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
	// Public Methods
	//-------------------------------------------------------------------------

	void LocalIdItems::append( std::stringstream& builder, bool verboseMode ) const
	{
		if ( m_primaryItem && m_primaryItem->length() > 0 )
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
			SPDLOG_TRACE( "Appending verbose information for LocalIdItems" );

			if ( m_primaryItem && m_primaryItem->length() > 0 )
			{
				for ( const auto& [depth, name] : m_primaryItem.value().commonNames() )
				{
					builder << '~';
					std::optional<std::string> locationStr;
					const GmodNode* nodePtr = m_primaryItem.value()[depth];

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
				for ( const auto& [depth, name] : m_secondaryItem.value().commonNames() )
				{
					builder << prefix;
					if ( prefix != "~" )
						prefix = "~";

					std::optional<std::string> locationStr;
					const GmodNode* nodePtr = m_secondaryItem.value()[depth];

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

	std::string LocalIdItems::toString( bool verboseMode ) const
	{
		std::stringstream builder;
		append( builder, verboseMode );
		std::string result = builder.str();

		if ( !result.empty() && result.back() == '/' && result.length() > 1 )
		{
			result.pop_back();
		}
		return result;
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
	// Private Helper Methods
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
