#include "pch.h"

#include "dnv/vista/sdk/UniversalId.h"

#include "dnv/vista/sdk/IUniversalIdBuilder.h"
#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/UniversalIdBuilder.h"

namespace dnv::vista::sdk
{
	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	UniversalId::UniversalId( const UniversalIdBuilder& builder )
		: m_imoNumber( *builder.imoNumber() ),
		  m_localId( *builder.localId() )
	{
		if ( !builder.isValid() )
		{
			throw std::invalid_argument( "Invalid UniversalIdBuilder state" );
		}

		if ( !builder.imoNumber().has_value() )
		{
			throw std::invalid_argument( "UniversalIdBuilder missing IMO number" );
		}

		if ( !builder.localId().has_value() )
		{
			throw std::invalid_argument( "UniversalIdBuilder missing Local ID" );
		}
	}

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	bool UniversalId::operator==( const UniversalId& other ) const
	{
		return equals( other );
	}

	bool UniversalId::operator!=( const UniversalId& other ) const
	{
		return !equals( other );
	}
	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const ImoNumber& UniversalId::imoNumber() const
	{
		if ( !m_builder->imoNumber().has_value() )
		{
			throw std::runtime_error( "Invalid ImoNumber" );
		}
		return m_imoNumber;
	}

	const LocalId& UniversalId::localId() const
	{
		return m_localId;
	}

	size_t UniversalId::hashCode() const noexcept
	{
		return std::hash<std::string>{}( m_builder->toString() );
	}

	//----------------------------------------------
	// Conversion and comparison
	//----------------------------------------------

	std::string UniversalId::toString() const
	{
		return m_builder->toString();
	}

	bool UniversalId::equals( const UniversalId& other ) const
	{
		return m_builder->toString() == other.m_builder->toString();
	}

	//----------------------------------------------
	// Static parsing methods
	//----------------------------------------------

	UniversalId UniversalId::parse( const std::string& universalIdStr )
	{
		auto builder = UniversalIdBuilder::parse( universalIdStr );

		return builder.build();
	}

	bool UniversalId::tryParse( const std::string& universalIdStr, ParsingErrors& errors, std::optional<UniversalId>& universalId )
	{
		std::optional<UniversalIdBuilder> builder;

		if ( !UniversalIdBuilder::tryParse( universalIdStr, errors, builder ) )
		{
			universalId = std::nullopt;
			return false;
		}

		universalId = builder->build();

		return true;
	}
}
