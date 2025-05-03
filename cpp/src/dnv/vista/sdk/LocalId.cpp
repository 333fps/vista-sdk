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
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	static const std::string namingRule = "dnv-v2";

	//=====================================================================
	// Construction / Destruction
	//=====================================================================

	LocalId::LocalId( LocalIdBuilder builder )
		: m_builder{ std::make_shared<LocalIdBuilder>( std::move( builder ) ) }
	{
		SPDLOG_INFO( "Constructing LocalId from builder" );

		if ( m_builder->isEmpty() )
		{
			SPDLOG_ERROR( "LocalId cannot be constructed from empty LocalIdBuilder" );
			throw std::invalid_argument( "LocalId cannot be constructed from empty LocalIdBuilder" );
		}

		if ( !m_builder->isValid() )
		{
			SPDLOG_ERROR( "LocalId cannot be constructed from invalid LocalIdBuilder" );
			throw std::invalid_argument( "LocalId cannot be constructed from invalid LocalIdBuilder" );
		}

		SPDLOG_INFO( "LocalId constructed successfully: primaryItem={}",
			m_builder->primaryItem().toString() );
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

	size_t LocalId::hashCode() const
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

	bool LocalId::isVerboseMode() const
	{
		return m_builder->isVerboseMode();
	}

	const GmodPath& LocalId::primaryItem() const
	{
		return m_builder->primaryItem();
	}

	const std::optional<GmodPath>& LocalId::secondaryItem() const
	{
		return m_builder->secondaryItem();
	}

	bool LocalId::hasCustomTag() const
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
	// Conversion and Comparison
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
