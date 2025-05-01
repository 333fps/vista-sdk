/**
 * @file LocalIdItems.cpp
 * @brief Implementation of LocalIdItems class for primary and secondary item handling
 */
#include "pch.h"

#include "dnv/vista/sdk/LocalIdItems.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Constructors and Assignment
	//-------------------------------------------------------------------------

	LocalIdItems::LocalIdItems(
		const GmodPath& primaryItem,
		const std::optional<GmodPath>& secondaryItem )
		: m_primaryItem( primaryItem ),
		  m_secondaryItem( secondaryItem )
	{
		if ( m_primaryItem.length() > 0 )
		{
			SPDLOG_INFO( "LocalIdItems: primaryItem: {}", m_primaryItem.toString() );
		}
		else
		{
			SPDLOG_WARN( "LocalIdItems: primaryItem is empty" );
		}

		if ( m_secondaryItem )
		{
			SPDLOG_INFO( "LocalIdItems: secondaryItem: {}", m_secondaryItem->toString() );
		}
		else
		{
			SPDLOG_DEBUG( "LocalIdItems: no secondary item provided" );
		}

		SPDLOG_INFO( "LocalIdItems: Created successfully" );
	}

	//-------------------------------------------------------------------------
	// Core Properties
	//-------------------------------------------------------------------------

	const GmodPath& LocalIdItems::primaryItem() const noexcept
	{
		return m_primaryItem;
	}

	const std::optional<GmodPath>& LocalIdItems::secondaryItem() const noexcept
	{
		return m_secondaryItem;
	}

	bool LocalIdItems::isEmpty() const noexcept
	{
		return m_primaryItem.length() == 0 && !m_secondaryItem;
	}

	//-------------------------------------------------------------------------
	// String Generation
	//-------------------------------------------------------------------------

	void LocalIdItems::append( std::stringstream& builder, bool verboseMode ) const
	{
		if ( m_primaryItem.length() == 0 && !m_secondaryItem )
		{
			return;
		}

		if ( m_primaryItem.length() > 0 )
		{
			m_primaryItem.toString( builder );
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
			SPDLOG_INFO( "Appending verbose information for LocalIdItems" );

			if ( m_primaryItem.length() > 0 )
			{
				for ( const auto& [depth, name] : m_primaryItem.commonNames() )
				{
					builder << '~';
					std::optional<std::string> location;

					if ( m_primaryItem[depth].location().has_value() )
						location = m_primaryItem[depth].location()->toString();

					appendCommonName( builder, name, location );
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

					std::optional<std::string> location;

					if ( ( *m_secondaryItem )[depth].location().has_value() )
						location = ( *m_secondaryItem )[depth].location()->toString();

					appendCommonName( builder, name, location );
					builder << '/';
				}
			}
		}
	}

	std::string LocalIdItems::toString( bool verboseMode ) const
	{
		std::stringstream builder;
		append( builder, verboseMode );
		return builder.str();
	}

	//-------------------------------------------------------------------------
	// Comparison Operators
	//-------------------------------------------------------------------------

	bool LocalIdItems::operator==( const LocalIdItems& other ) const noexcept
	{
		if ( m_primaryItem != other.m_primaryItem )
		{
			return false;
		}

		if ( m_secondaryItem && other.m_secondaryItem )
		{
			return *m_secondaryItem == *other.m_secondaryItem;
		}
		else if ( !m_secondaryItem && !other.m_secondaryItem )
		{
			return true;
		}
		else
		{
			return false;
		}
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
					bool match = VIS::isISOString( ch );
					if ( !match )
					{
						current = '.';
						break;
					}
					current = static_cast<char>( std::tolower( ch ) );
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
