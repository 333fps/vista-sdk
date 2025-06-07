/**
 * @file LocalId.cpp
 * @brief Implementation of the LocalId class
 */

#include "pch.h"

#include "dnv/vista/sdk/LocalId.h"

#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VISVersion.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	static const std::string namingRule = "dnv-v2";

	//=====================================================================
	// Construction / destruction
	//=====================================================================

	LocalId::LocalId( LocalIdBuilder builder )
		: m_builder{ std::make_shared<LocalIdBuilder>( std::move( builder ) ) }
	{
		if ( m_builder->isEmpty() )
		{
			throw std::invalid_argument( "LocalId cannot be constructed from empty LocalIdBuilder" );
		}

		if ( !m_builder->isValid() )
		{
			throw std::invalid_argument( "LocalId cannot be constructed from invalid LocalIdBuilder" );
		}
	}

	//=====================================================================
	// Operators
	//=====================================================================

	bool LocalId::operator==( const LocalId& other ) const
	{
		return equals( other );
	}

	bool LocalId::operator!=( const LocalId& other ) const
	{
		return !equals( other );
	}

	//=====================================================================
	// Hashing
	//=====================================================================

	size_t LocalId::hashCode() const noexcept
	{
		return m_builder->hashCode();
	}

	//=====================================================================
	// Builder Accessor
	//=====================================================================

	const LocalIdBuilder& LocalId::builder() const
	{
		return *m_builder;
	}

	//=====================================================================
	// Core Properties
	//=====================================================================

	VisVersion LocalId::visVersion() const
	{
		return *m_builder->visVersion();
	}

	bool LocalId::isVerboseMode() const noexcept
	{
		return m_builder->isVerboseMode();
	}

	const std::optional<GmodPath>& LocalId::primaryItem() const
	{
		return m_builder->primaryItem();
	}

	const std::optional<GmodPath>& LocalId::secondaryItem() const
	{
		return m_builder->secondaryItem();
	}

	bool LocalId::hasCustomTag() const noexcept
	{
		return m_builder->hasCustomTag();
	}

	std::vector<MetadataTag> LocalId::metadataTags() const
	{
		return m_builder->metadataTags();
	}

	//=====================================================================
	// Metadata Accessors
	//=====================================================================

	const std::optional<MetadataTag>& LocalId::quantity() const
	{
		return m_builder->quantity();
	}

	const std::optional<MetadataTag>& LocalId::content() const
	{
		return m_builder->content();
	}

	const std::optional<MetadataTag>& LocalId::calculation() const
	{
		return m_builder->calculation();
	}

	const std::optional<MetadataTag>& LocalId::state() const
	{
		return m_builder->state();
	}

	const std::optional<MetadataTag>& LocalId::command() const
	{
		return m_builder->command();
	}

	const std::optional<MetadataTag>& LocalId::type() const
	{
		return m_builder->type();
	}

	const std::optional<MetadataTag>& LocalId::position() const
	{
		return m_builder->position();
	}

	const std::optional<MetadataTag>& LocalId::detail() const
	{
		return m_builder->detail();
	}

	//=====================================================================
	// Conversion and comparison
	//=====================================================================

	std::string LocalId::toString() const
	{
		return m_builder->toString();
	}

	bool LocalId::equals( const LocalId& other ) const
	{
		return *m_builder == *other.m_builder;
	}
}
