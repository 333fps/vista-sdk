/**
 * @file MetadataTag.cpp
 * @brief Implementation of the MetadataTag class
 */

#include "pch.h"

#include "dnv/vista/sdk/MetadataTag.h"

#include "dnv/vista/sdk/CodebookName.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// MetadataTag class
	//=====================================================================

	//-------------------------------------------------------------------------
	// Construction / destruction
	//-------------------------------------------------------------------------

	MetadataTag::MetadataTag( CodebookName name, const std::string& value, bool isCustom )
		: m_name{ name },
		  m_custom{ isCustom },
		  m_value{ value }
	{
	}

	//-------------------------------------------------------------------------
	// Operators
	//-------------------------------------------------------------------------

	bool MetadataTag::operator==( const MetadataTag& other ) const
	{
		if ( m_value != other.m_value ) [[likely]]
		{
			return false;
		}

		if ( m_name != other.m_name ) [[unlikely]]
		{
			static constexpr std::string_view errorPrefix = "Can't compare tags with different names";
			throw std::invalid_argument( std::string( errorPrefix ) );
		}

		return true;
	}

	bool MetadataTag::operator!=( const MetadataTag& other ) const
	{
		return !( *this == other );
	}

	MetadataTag::operator std::string() const
	{
		return m_value;
	}

	//-------------------------------------------------------------------------
	// Accessors
	//-------------------------------------------------------------------------

	CodebookName MetadataTag::name() const noexcept
	{
		return m_name;
	}

	std::string_view MetadataTag::value() const noexcept
	{
		return m_value;
	}

	char MetadataTag::prefix() const noexcept
	{
		return m_custom ? '~' : '-';
	}

	size_t MetadataTag::hashCode() const noexcept
	{
		return std::hash<std::string_view>{}( m_value );
	}

	//-------------------------------------------------------------------------
	// State inspection methods
	//-------------------------------------------------------------------------

	bool MetadataTag::isCustom() const noexcept
	{
		return m_custom;
	}

	//-------------------------------------------------------------------------
	// String conversion methods
	//-------------------------------------------------------------------------

	std::string MetadataTag::toString() const noexcept
	{
		return m_value;
	}

	void MetadataTag::toString( std::string& builder, char separator ) const
	{
		const auto prefixView = CodebookNames::toPrefix( m_name );

		const auto currentSize = builder.size();
		const auto requiredSize = prefixView.size() + 1 + m_value.size() + 1;
		builder.reserve( currentSize + requiredSize );

		builder.append( prefixView );
		builder.push_back( m_custom ? '~' : '-' );
		builder.append( m_value );
		builder.push_back( separator );
	}
}
