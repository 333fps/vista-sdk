#include "pch.h"

#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------------

	const std::string LocalId::namingRule = "dnv-v2";

	//-------------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------------

	LocalId::LocalId( const LocalIdBuilder& builder )
		: m_builder( builder )
	{
		if ( m_builder.isEmpty() )
		{
			SPDLOG_ERROR( "LocalId cannot be constructed from empty LocalIdBuilder" );
			throw std::invalid_argument( "LocalId cannot be constructed from empty LocalIdBuilder" );
		}
		if ( !m_builder.isValid() )
		{
			SPDLOG_ERROR( "LocalId cannot be constructed from invalid LocalIdBuilder" );
			throw std::invalid_argument( "LocalId cannot be constructed from invalid LocalIdBuilder" );
		}
	}

	LocalId::LocalId( LocalId&& other ) noexcept
		: m_builder( std::move( other.m_builder ) )
	{
	}

	LocalId& LocalId::operator=( LocalId&& other ) noexcept
	{
		if ( this != &other )
		{
			m_builder = std::move( other.m_builder );
		}
		return *this;
	}

	//-------------------------------------------------------------------------
	// ILocalId Interface Implementation
	//-------------------------------------------------------------------------

	VisVersion LocalId::visVersion() const
	{
		return *m_builder.visVersion();
	}

	bool LocalId::isVerboseMode() const
	{
		return m_builder.isVerboseMode();
	}

	const GmodPath& LocalId::primaryItem() const
	{
		if ( m_builder.primaryItem().length() == 0 )
		{
			SPDLOG_ERROR( "Attempted to access unset primary item" );
			throw std::runtime_error( "Primary item is not set" );
		}
		return m_builder.primaryItem();
	}

	std::optional<GmodPath> LocalId::secondaryItem() const
	{
		return m_builder.secondaryItem();
	}

	bool LocalId::hasCustomTag() const
	{
		return m_builder.hasCustomTag();
	}

	std::vector<MetadataTag> LocalId::metadataTags() const
	{
		return m_builder.metadataTags();
	}

	std::string LocalId::toString() const
	{
		SPDLOG_INFO( "Converting LocalId to string: primaryItem={}, hasSecondary={}",
			m_builder.primaryItem().toString(),
			m_builder.secondaryItem().has_value() );

		return m_builder.toString();
	}

	bool LocalId::equals( const LocalId& other ) const
	{
		return m_builder == other.m_builder;
	}

	//-------------------------------------------------------------------------
	// Metadata Tag Accessors
	//-------------------------------------------------------------------------

	std::optional<MetadataTag> LocalId::quantity() const
	{
		return m_builder.quantity();
	}

	std::optional<MetadataTag> LocalId::content() const
	{
		return m_builder.content();
	}

	std::optional<MetadataTag> LocalId::calculation() const
	{
		return m_builder.calculation();
	}

	std::optional<MetadataTag> LocalId::state() const
	{
		return m_builder.state();
	}

	std::optional<MetadataTag> LocalId::command() const
	{
		return m_builder.command();
	}

	std::optional<MetadataTag> LocalId::type() const
	{
		return m_builder.type();
	}

	std::optional<MetadataTag> LocalId::position() const
	{
		return m_builder.position();
	}

	std::optional<MetadataTag> LocalId::detail() const
	{
		return m_builder.detail();
	}

	//-------------------------------------------------------------------------
	// Builder Access
	//-------------------------------------------------------------------------

	const LocalIdBuilder& LocalId::builder() const
	{
		return m_builder;
	}

	//-------------------------------------------------------------------------
	// Static Factory Methods
	//-------------------------------------------------------------------------

	LocalId LocalId::parse( const std::string& localIdStr )
	{
		SPDLOG_INFO( "Parsing LocalId from string: {}", localIdStr );
		return LocalIdBuilder::parse( localIdStr ).build();
	}

	bool LocalId::tryParse( const std::string& localIdStr, ParsingErrors& errors, std::optional<LocalId>& localId )
	{
		SPDLOG_TRACE( "Attempting to parse LocalId from: {}", localIdStr );

		std::optional<LocalIdBuilder> localIdBuilder;
		if ( !LocalIdBuilder::tryParse( localIdStr, errors, localIdBuilder ) )
		{
			SPDLOG_ERROR( "LocalId parsing failed" );
			localId = std::nullopt;
			return false;
		}

		SPDLOG_DEBUG( "LocalId parsing succeeded, building final LocalId" );
		try
		{
			localId.emplace( std::move( localIdBuilder->build() ) );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Failed to build LocalId after parsing: {}", e.what() );
			errors.addError( "BuildError", e.what() );
			localId = std::nullopt;
			return false;
		}
	}

	//-------------------------------------------------------------------------
	// Operators and Hash Code
	//-------------------------------------------------------------------------

	bool LocalId::operator==( const LocalId& other ) const noexcept
	{
		return equals( other );
	}

	bool LocalId::operator!=( const LocalId& other ) const noexcept
	{
		return !equals( other );
	}

	size_t LocalId::hashCode() const
	{
		return m_builder.hashCode();
	}

	//-------------------------------------------------------------------------
	// LocalIdParsingErrorBuilder Implementation
	//-------------------------------------------------------------------------

	const std::unordered_map<LocalIdParsingState, std::string> LocalIdParsingErrorBuilder::m_predefinedErrorMessages = {
		{ LocalIdParsingState::NamingRule, "Invalid naming rule prefix" },
		{ LocalIdParsingState::VisVersion, "Invalid VIS version" },
		{ LocalIdParsingState::PrimaryItem, "Invalid primary item path" },
		{ LocalIdParsingState::SecondaryItem, "Invalid secondary item path" },
		{ LocalIdParsingState::ItemDescription, "Invalid item description" },
		{ LocalIdParsingState::MetaQuantity, "Invalid quantity metadata tag" },
		{ LocalIdParsingState::MetaContent, "Invalid content metadata tag" },
		{ LocalIdParsingState::MetaCalculation, "Invalid calculation metadata tag" },
		{ LocalIdParsingState::MetaState, "Invalid state metadata tag" },
		{ LocalIdParsingState::MetaCommand, "Invalid command metadata tag" },
		{ LocalIdParsingState::MetaType, "Invalid type metadata tag" },
		{ LocalIdParsingState::MetaPosition, "Invalid position metadata tag" },
		{ LocalIdParsingState::MetaDetail, "Invalid detail metadata tag" },
		{ LocalIdParsingState::EmptyState, "Empty state value provided" },
		{ LocalIdParsingState::Formatting, "Formatting error in Local ID string" },
		{ LocalIdParsingState::Completeness, "Incomplete Local ID structure" },
		{ LocalIdParsingState::NamingEntity, "Invalid naming entity" },
		{ LocalIdParsingState::IMONumber, "Invalid IMO number" } };

	LocalIdParsingErrorBuilder& LocalIdParsingErrorBuilder::addError( LocalIdParsingState state )
	{
		auto it = m_predefinedErrorMessages.find( state );
		if ( it != m_predefinedErrorMessages.end() )
		{
			m_errors.emplace_back( state, it->second );
			SPDLOG_INFO( "Added predefined parsing error: {}", it->second );
		}
		return *this;
	}

	LocalIdParsingErrorBuilder& LocalIdParsingErrorBuilder::addError( LocalIdParsingState state, const std::string& message )
	{
		m_errors.emplace_back( state, message );
		SPDLOG_INFO( "Added custom parsing error for state {}: {}", static_cast<int>( state ), message );
		return *this;
	}

	bool LocalIdParsingErrorBuilder::hasError() const
	{
		return !m_errors.empty();
	}

	ParsingErrors LocalIdParsingErrorBuilder::build() const
	{
		ParsingErrors errors;
		for ( const auto& [state, message] : m_errors )
		{
			std::string stateStr;
			switch ( state )
			{
				case LocalIdParsingState::NamingRule:
					stateStr = "NamingRule";
					break;
				case LocalIdParsingState::VisVersion:
					stateStr = "VisVersion";
					break;
				case LocalIdParsingState::PrimaryItem:
					stateStr = "PrimaryItem";
					break;
				case LocalIdParsingState::SecondaryItem:
					stateStr = "SecondaryItem";
					break;
				case LocalIdParsingState::ItemDescription:
					stateStr = "ItemDescription";
					break;
				case LocalIdParsingState::MetaQuantity:
					stateStr = "MetaQuantity";
					break;
				case LocalIdParsingState::MetaContent:
					stateStr = "MetaContent";
					break;
				case LocalIdParsingState::MetaCalculation:
					stateStr = "MetaCalculation";
					break;
				case LocalIdParsingState::MetaState:
					stateStr = "MetaState";
					break;
				case LocalIdParsingState::MetaCommand:
					stateStr = "MetaCommand";
					break;
				case LocalIdParsingState::MetaType:
					stateStr = "MetaType";
					break;
				case LocalIdParsingState::MetaPosition:
					stateStr = "MetaPosition";
					break;
				case LocalIdParsingState::MetaDetail:
					stateStr = "MetaDetail";
					break;
				case LocalIdParsingState::EmptyState:
					stateStr = "EmptyState";
					break;
				case LocalIdParsingState::Formatting:
					stateStr = "Formatting";
					break;
				case LocalIdParsingState::Completeness:
					stateStr = "Completeness";
					break;
				case LocalIdParsingState::NamingEntity:
					stateStr = "NamingEntity";
					break;
				case LocalIdParsingState::IMONumber:
					stateStr = "IMONumber";
					break;
				default:
					stateStr = "State" + std::to_string( static_cast<int>( state ) );
					break;
			}

			errors.addError( stateStr, message );
		}
		return errors;
	}

	LocalIdParsingErrorBuilder LocalIdParsingErrorBuilder::create()
	{
		return LocalIdParsingErrorBuilder();
	}
}
