#include "pch.h"

#include "dnv/vista/sdk/UniversalId.h"

#include "dnv/vista/sdk/ImoNumber.h"
#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/UniversalIdBuilder.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// UniversalId class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	UniversalId::UniversalId( const UniversalIdBuilder& builder )
		: m_imoNumber( *builder.imoNumber() ),
		  m_localId( builder.localId()->build() )
	{
		if ( !builder.isValid() )
		{
			throw std::invalid_argument( "Invalid UniversalIdBuilder state" );
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

	bool UniversalId::equals( const UniversalId& other ) const
	{
		return m_imoNumber == other.m_imoNumber &&
			   m_localId.equals( other.m_localId );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const ImoNumber& UniversalId::imoNumber() const
	{
		return m_imoNumber;
	}

	const LocalId& UniversalId::localId() const noexcept
	{
		return m_localId;
	}

	size_t UniversalId::hashCode() const noexcept
	{
		size_t hash = 0;

		hash ^= m_imoNumber.hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		hash ^= m_localId.hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		return hash;
	}

	//----------------------------------------------
	// String conversion
	//----------------------------------------------

	std::string UniversalId::toString() const
	{
		std::ostringstream oss;

		oss << UniversalIdBuilder::namingEntity;
		oss << "/" << m_imoNumber.toString();
		oss << m_localId.toString();

		return oss.str();
	}

	//----------------------------------------------
	// Static parsing methods
	//----------------------------------------------

	UniversalId UniversalId::parse( std::string_view universalIdStr )
	{
		auto builder = UniversalIdBuilder::parse( universalIdStr );

		return builder.build();
	}

	bool UniversalId::tryParse( std::string_view universalIdStr, ParsingErrors& errors, std::optional<UniversalId>& universalId )
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
