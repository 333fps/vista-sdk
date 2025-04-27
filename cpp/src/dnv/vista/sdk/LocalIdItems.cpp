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
		std::optional<GmodPath> secondaryItem )
	{
		if ( primaryItem.length() > 0 )
		{
			SPDLOG_INFO( "LocalIdItems: primaryItem: {}", primaryItem.toString() ); // Use . not ->
			m_primaryItem = primaryItem;
		}
		else
		{
			SPDLOG_WARN( "LocalIdItems: primaryItem is empty" );
		}

		if ( secondaryItem.has_value() )
		{
			SPDLOG_INFO( "LocalIdItems: secondaryItem: {}", secondaryItem->toString() );
			m_secondaryItem = secondaryItem;
		}
		else
		{
			SPDLOG_WARN( "LocalIdItems: secondaryItem is empty" );
		}

		SPDLOG_INFO( "LocalIdItems: Created successfully" );
	}

	LocalIdItems::LocalIdItems( const LocalIdItems& other )
	{
		m_primaryItem = other.m_primaryItem;
		m_secondaryItem = other.m_secondaryItem;
	}

	LocalIdItems& LocalIdItems::operator=( const LocalIdItems& other )
	{
		if ( this != &other )
		{
			m_primaryItem = other.m_primaryItem;
			m_secondaryItem = other.m_secondaryItem;
		}
		return *this;
	}

	//-------------------------------------------------------------------------
	// Core Properties
	//-------------------------------------------------------------------------

	const GmodPath& LocalIdItems::primaryItem() const
	{
		return m_primaryItem;
	}

	std::optional<GmodPath> LocalIdItems::secondaryItem() const
	{
		return m_secondaryItem;
	}

	//-------------------------------------------------------------------------
	// String Generation
	//-------------------------------------------------------------------------

	void LocalIdItems::append( std::stringstream& builder, bool verboseMode ) const
	{
		if ( m_primaryItem.length() == 0 && !m_secondaryItem.has_value() )
		{
			return;
		}

		if ( m_primaryItem.length() > 0 )
		{
			m_primaryItem.toString( builder );
			builder << '/';
		}

		if ( m_secondaryItem.has_value() )
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

			if ( m_secondaryItem.has_value() )
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

	void LocalIdItems::appendCommonName(
		std::stringstream& builder,
		const std::string& commonName,
		std::optional<std::string> location )
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

	//-------------------------------------------------------------------------
	// Comparison Operators
	//-------------------------------------------------------------------------

	bool LocalIdItems::operator==( const LocalIdItems& other ) const
	{
		if ( m_primaryItem.length() == 0 && !m_secondaryItem.has_value() &&
			 other.m_primaryItem.length() == 0 && !other.m_secondaryItem.has_value() )
		{
			return true;
		}

		if ( ( m_primaryItem.length() > 0 ) != ( other.m_primaryItem.length() > 0 ) )
			return false;

		if ( m_primaryItem.length() > 0 && other.m_primaryItem.length() > 0 &&
			 !( m_primaryItem == other.m_primaryItem ) )
		{
			return false;
		}

		if ( m_secondaryItem.has_value() != other.m_secondaryItem.has_value() )
			return false;

		if ( m_secondaryItem.has_value() && other.m_secondaryItem.has_value() &&
			 !( *m_secondaryItem == *other.m_secondaryItem ) )
		{
			return false;
		}

		return true;
	}

	bool LocalIdItems::operator!=( const LocalIdItems& other ) const
	{
		return !( *this == other );
	}
}
