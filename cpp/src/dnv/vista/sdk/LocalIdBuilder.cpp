/**
 * @file LocalIdBuilder.cpp
 * @brief Implementation of the LocalIdBuilder class
 */

#include "pch.h"

#include "dnv/vista/sdk/LocalIdBuilder.h"

#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/CodebookName.h"
#include "dnv/vista/sdk/LocalIdParsingErrorBuilder.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	const std::string LocalIdBuilder::namingRule = "dnv-v2";

	const std::vector<CodebookName> LocalIdBuilder::usedCodebooks = {
		CodebookName::Quantity,
		CodebookName::Content,
		CodebookName::State,
		CodebookName::Command,
		CodebookName::FunctionalServices,
		CodebookName::MaintenanceCategory,
		CodebookName::ActivityType,
		CodebookName::Position,
		CodebookName::Detail };

	//=====================================================================
	// Operators
	//=====================================================================

	bool LocalIdBuilder::operator==( const LocalIdBuilder& other ) const
	{
		return equals( other );
	}

	bool LocalIdBuilder::operator!=( const LocalIdBuilder& other ) const
	{
		return !equals( other );
	}

	//=====================================================================
	// Hashing
	//=====================================================================

	size_t LocalIdBuilder::hashCode() const
	{
		SPDLOG_TRACE( "Calculating hash code for LocalIdBuilder" );

		size_t hash = 17;
		constexpr size_t prime = 31;

		hash = hash * prime + m_items.primaryItem()->hashCode() + m_items.secondaryItem()->hashCode();

		if ( m_quantity.has_value() )
			hash = hash * prime + m_quantity->hashCode();
		if ( m_content.has_value() )
			hash = hash * prime + m_content->hashCode();
		if ( m_calculation.has_value() )
			hash = hash * prime + m_calculation->hashCode();
		if ( m_state.has_value() )
			hash = hash * prime + m_state->hashCode();
		if ( m_command.has_value() )
			hash = hash * prime + m_command->hashCode();
		if ( m_type.has_value() )
			hash = hash * prime + m_type->hashCode();
		if ( m_position.has_value() )
			hash = hash * prime + m_position->hashCode();
		if ( m_detail.has_value() )
			hash = hash * prime + m_detail->hashCode();

		hash = hash * prime + std::hash<bool>{}( m_verboseMode );

		hash = hash * prime + ( m_visVersion.has_value() ? std::hash<int>{}( static_cast<int>( *m_visVersion ) ) : 0 );

		return hash;
	}

	//=====================================================================
	// Core Build Method
	//=====================================================================

	LocalId LocalIdBuilder::build()
	{
		if ( !isValid() )
		{
			SPDLOG_ERROR( "Cannot build LocalId: builder state is invalid." );
			throw std::invalid_argument( "Cannot build LocalId: builder state is invalid." );
		}
		SPDLOG_DEBUG( "Building LocalId from builder state." );
		return LocalId( std::move( *this ) );
	}

	//=====================================================================
	// State Inspection Methods
	//=====================================================================

	bool LocalIdBuilder::isValid() const noexcept
	{
		if ( !m_visVersion.has_value() )
		{
			SPDLOG_TRACE( "isValid() -> false: Missing VIS version." );
			return false;
		}

		if ( m_items.primaryItem()->length() == 0 )
		{
			SPDLOG_TRACE( "isValid() -> false: Primary item path is empty." );
			return false;
		}

		if ( isEmptyMetadata() )
		{
			SPDLOG_TRACE( "isValid() -> false: No metadata tags are set." );
			return false;
		}

		SPDLOG_TRACE( "isValid() -> true" );
		return true;
	}

	bool LocalIdBuilder::isEmpty() const noexcept
	{
		bool empty = !m_visVersion.has_value() &&
					 m_items.primaryItem()->length() == 0 &&
					 !m_items.secondaryItem().has_value() &&
					 isEmptyMetadata();
		SPDLOG_TRACE( "isEmpty() -> {}", empty );
		return empty;
	}

	std::optional<VisVersion> LocalIdBuilder::visVersion() const
	{
		return m_visVersion;
	}

	bool LocalIdBuilder::isVerboseMode() const noexcept
	{
		return m_verboseMode;
	}

	const std::optional<GmodPath>& LocalIdBuilder::primaryItem() const
	{
		return m_items.primaryItem();
	}

	const std::optional<GmodPath>& LocalIdBuilder::secondaryItem() const
	{
		return m_items.secondaryItem();
	}

	std::vector<MetadataTag> LocalIdBuilder::metadataTags() const
	{
		std::vector<MetadataTag> tags;
		tags.reserve( 8 );

		if ( m_quantity.has_value() )
			tags.push_back( *m_quantity );
		if ( m_content.has_value() )
			tags.push_back( *m_content );
		if ( m_calculation.has_value() )
			tags.push_back( *m_calculation );
		if ( m_state.has_value() )
			tags.push_back( *m_state );
		if ( m_command.has_value() )
			tags.push_back( *m_command );
		if ( m_type.has_value() )
			tags.push_back( *m_type );
		if ( m_position.has_value() )
			tags.push_back( *m_position );
		if ( m_detail.has_value() )
			tags.push_back( *m_detail );

		return tags;
	}

	//=====================================================================
	// Metadata Inspection Methods
	//=====================================================================

	bool LocalIdBuilder::isEmptyMetadata() const
	{
		return !m_quantity.has_value() &&
			   !m_content.has_value() &&
			   !m_calculation.has_value() &&
			   !m_state.has_value() &&
			   !m_command.has_value() &&
			   !m_type.has_value() &&
			   !m_position.has_value() &&
			   !m_detail.has_value();
	}

	bool LocalIdBuilder::hasCustomTag() const noexcept
	{
		return ( m_quantity.has_value() && m_quantity->isCustom() ) ||
			   ( m_content.has_value() && m_content->isCustom() ) ||
			   ( m_calculation.has_value() && m_calculation->isCustom() ) ||
			   ( m_state.has_value() && m_state->isCustom() ) ||
			   ( m_command.has_value() && m_command->isCustom() ) ||
			   ( m_type.has_value() && m_type->isCustom() ) ||
			   ( m_position.has_value() && m_position->isCustom() ) ||
			   ( m_detail.has_value() && m_detail->isCustom() );
	}

	const LocalIdItems& LocalIdBuilder::items() const
	{
		return m_items;
	}

	const std::optional<MetadataTag>& LocalIdBuilder::quantity() const
	{
		return m_quantity;
	}

	const std::optional<MetadataTag>& LocalIdBuilder::content() const
	{
		return m_content;
	}

	const std::optional<MetadataTag>& LocalIdBuilder::calculation() const
	{
		return m_calculation;
	}

	const std::optional<MetadataTag>& LocalIdBuilder::state() const
	{
		return m_state;
	}

	const std::optional<MetadataTag>& LocalIdBuilder::command() const
	{
		return m_command;
	}

	const std::optional<MetadataTag>& LocalIdBuilder::type() const
	{
		return m_type;
	}

	const std::optional<MetadataTag>& LocalIdBuilder::position() const
	{
		return m_position;
	}

	const std::optional<MetadataTag>& LocalIdBuilder::detail() const
	{
		return m_detail;
	}

	//=====================================================================
	// Conversion and Comparison
	//=====================================================================

	std::string LocalIdBuilder::toString() const
	{
		std::stringstream ss;
		toString( ss );
		return ss.str();
	}

	void LocalIdBuilder::toString( std::stringstream& builder ) const
	{
		SPDLOG_TRACE( "Converting LocalIdBuilder to string" );

		if ( !m_visVersion.has_value() )
		{
			SPDLOG_WARN( "toString() called on builder without VIS version set." );
			return;
		}

		builder << '/' << namingRule << '/' << VisVersionExtensions::toVersionString( *m_visVersion ) << '/';

		m_items.append( builder, m_verboseMode );

		if ( !isEmptyMetadata() )
		{
			builder << "meta/";

			if ( m_quantity.has_value() )
				builder << CodebookNames::toPrefix( CodebookName::Quantity ) << m_quantity->prefix() << m_quantity->value() << '/';
			if ( m_content.has_value() )
				builder << CodebookNames::toPrefix( CodebookName::Content ) << m_content->prefix() << m_content->value() << '/';
			if ( m_calculation.has_value() )
				builder << CodebookNames::toPrefix( CodebookName::Calculation ) << m_calculation->prefix() << m_calculation->value() << '/';
			if ( m_state.has_value() )
				builder << CodebookNames::toPrefix( CodebookName::State ) << m_state->prefix() << m_state->value() << '/';
			if ( m_command.has_value() )
				builder << CodebookNames::toPrefix( CodebookName::Command ) << m_command->prefix() << m_command->value() << '/';
			if ( m_type.has_value() )
				builder << CodebookNames::toPrefix( CodebookName::Type ) << m_type->prefix() << m_type->value() << '/';
			if ( m_position.has_value() )
				builder << CodebookNames::toPrefix( CodebookName::Position ) << m_position->prefix() << m_position->value() << '/';
			if ( m_detail.has_value() )
				builder << CodebookNames::toPrefix( CodebookName::Detail ) << m_detail->prefix() << m_detail->value() << '/';
		}
		std::string temp = builder.str();
		if ( temp.length() > 1 && temp.back() == '/' )
		{
			builder.str( temp.substr( 0, temp.length() - 1 ) );
		}
	}

	bool LocalIdBuilder::equals( const LocalIdBuilder& other ) const
	{
		if ( m_visVersion.has_value() != other.m_visVersion.has_value() )
		{
			if ( m_visVersion.has_value() || other.m_visVersion.has_value() )
			{
				SPDLOG_ERROR( "Cannot compare LocalIdBuilders: one has a VIS version while the other does not." );
				throw std::invalid_argument( "Cannot compare LocalIdBuilders: one has a VIS version while the other does not." );
			}
		}
		else if ( m_visVersion.has_value() )
		{
			if ( *m_visVersion != *other.m_visVersion )
			{
				std::string errorMsg = "Cannot compare LocalIdBuilders with different VIS versions (" +
									   VisVersionExtensions::toVersionString( *m_visVersion ) + " vs " +
									   VisVersionExtensions::toVersionString( *other.m_visVersion ) + ")";
				SPDLOG_ERROR( errorMsg );
				throw std::invalid_argument( errorMsg );
			}
		}

		return m_items == other.m_items &&
			   m_verboseMode == other.m_verboseMode &&
			   m_quantity == other.m_quantity &&
			   m_content == other.m_content &&
			   m_calculation == other.m_calculation &&
			   m_state == other.m_state &&
			   m_command == other.m_command &&
			   m_type == other.m_type &&
			   m_position == other.m_position &&
			   m_detail == other.m_detail;
	}

	//=====================================================================
	// Builder Methods (Immutable Fluent Interface)
	//=====================================================================

	//----------------------------------------------
	// Creation
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::create( VisVersion version )
	{
		SPDLOG_DEBUG( "Creating LocalIdBuilder with VisVersion: {}", VisVersionExtensions::toString( version ) );
		return LocalIdBuilder().withVisVersion( version );
	}

	//----------------------------------------------
	// Verbose Mode
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withVerboseMode( bool verboseMode )
	{
		SPDLOG_DEBUG( "Setting verbose mode: {}", verboseMode );
		LocalIdBuilder result = std::move( *this );
		result.m_verboseMode = verboseMode;
		return result;
	}

	//----------------------------------------------
	// VIS Version
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withVisVersion( const std::string& visVersionStr )
	{
		SPDLOG_DEBUG( "Setting VisVersion from string: {}", visVersionStr );
		try
		{
			VisVersion version = VisVersionExtensions::parse( visVersionStr );
			return withVisVersion( version );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Invalid VisVersion string '{}': {}", visVersionStr, ex.what() );
			throw std::invalid_argument( "Invalid VisVersion format: " + visVersionStr );
		}
	}

	LocalIdBuilder LocalIdBuilder::withVisVersion( VisVersion version )
	{
		SPDLOG_DEBUG( "Setting VisVersion: {}", VisVersionExtensions::toString( version ) );
		LocalIdBuilder result = std::move( *this );
		result.m_visVersion = version;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::tryWithVisVersion( const std::optional<VisVersion>& version )
	{
		bool succeeded;
		return tryWithVisVersion( version, succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithVisVersion( const std::optional<VisVersion>& version, bool& succeeded )
	{
		succeeded = false;
		if ( !version.has_value() )
		{
			SPDLOG_TRACE( "tryWithVisVersion(optional<VisVersion>) called with empty optional." );
			return std::move( *this );
		}

		SPDLOG_TRACE( "Trying to set VisVersion: {}", VisVersionExtensions::toString( *version ) );
		LocalIdBuilder result = std::move( *this );
		result.m_visVersion = version;
		succeeded = true;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::tryWithVisVersion( const std::optional<std::string>& visVersionStr, bool& succeeded )
	{
		succeeded = false;
		if ( !visVersionStr.has_value() )
		{
			SPDLOG_TRACE( "tryWithVisVersion(optional<string>) called with empty optional." );
			return std::move( *this );
		}

		SPDLOG_TRACE( "Trying to set VisVersion from string: {}", *visVersionStr );
		try
		{
			VisVersion version = VisVersionExtensions::parse( *visVersionStr );

			return tryWithVisVersion( std::make_optional( version ), succeeded );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_WARN( "Failed to parse VisVersion string '{}' in tryWithVisVersion: {}", *visVersionStr, ex.what() );
			succeeded = false;
			return std::move( *this );
		}
	}

	LocalIdBuilder LocalIdBuilder::withoutVisVersion()
	{
		SPDLOG_DEBUG( "Removing VisVersion" );
		LocalIdBuilder result = std::move( *this );
		result.m_visVersion = std::nullopt;
		return result;
	}

	//----------------------------------------------
	// Primary Item
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withPrimaryItem( GmodPath&& item )
	{
		SPDLOG_DEBUG( "Setting primary item..." );
		bool succeeded;

		LocalIdBuilder result = tryWithPrimaryItem( std::move( item ), succeeded );
		if ( !succeeded )
		{
			throw std::invalid_argument( "Failed to set primary item (invalid path?)" );
		}
		return result;
	}

	LocalIdBuilder LocalIdBuilder::tryWithPrimaryItem( GmodPath&& item )
	{
		bool succeeded;
		return tryWithPrimaryItem( std::move( item ), succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithPrimaryItem( GmodPath&& item, bool& succeeded )
	{
		succeeded = false;

		if ( item.length() == 0 )
		{
			SPDLOG_WARN( "Attempted to set an empty GmodPath as primary item." );
			return std::move( *this );
		}

		SPDLOG_TRACE( "Trying to set primary item." );
		try
		{
			LocalIdBuilder result = std::move( *this );

			result.m_items = LocalIdItems( std::move( result.m_items ), std::move( item ) );
			succeeded = true;
			return result;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Failed during tryWithPrimaryItem: {}", ex.what() );
			return std::move( *this );
		}
	}

	LocalIdBuilder LocalIdBuilder::tryWithPrimaryItem( std::optional<GmodPath>&& item )
	{
		bool succeeded;
		return tryWithPrimaryItem( std::move( item ), succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithPrimaryItem( std::optional<GmodPath>&& item, bool& succeeded )
	{
		if ( !item.has_value() )
		{
			SPDLOG_TRACE( "tryWithPrimaryItem(optional): Optional has no value." );
			succeeded = false;
			return std::move( *this );
		}

		return tryWithPrimaryItem( std::move( *item ), succeeded );
	}

	LocalIdBuilder LocalIdBuilder::withoutPrimaryItem()
	{
		SPDLOG_DEBUG( "Removing primary item (resetting to empty path)." );
		LocalIdBuilder result( std::move( *this ) );

		result.m_items = LocalIdItems( std::move( result.m_items ), GmodPath{} );
		return result;
	}

	//----------------------------------------------
	// Secondary Item
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withSecondaryItem( GmodPath&& item )
	{
		SPDLOG_DEBUG( "Setting secondary item..." );
		bool succeeded;
		LocalIdBuilder result = tryWithSecondaryItem( std::move( item ), succeeded );
		if ( !succeeded )
		{
			throw std::invalid_argument( "Failed to set secondary item (invalid path?)" );
		}
		return result;
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( GmodPath&& item )
	{
		bool succeeded;
		return tryWithSecondaryItem( std::move( item ), succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( GmodPath&& item, bool& succeeded )
	{
		succeeded = false;

		if ( item.length() == 0 )
		{
			SPDLOG_WARN( "Attempted to set an empty GmodPath as secondary item." );
			return std::move( *this );
		}

		SPDLOG_TRACE( "Trying to set secondary item." );
		try
		{
			LocalIdBuilder result = std::move( *this );

			result.m_items = LocalIdItems( std::move( result.m_items ), std::make_optional( std::move( item ) ) );
			succeeded = true;
			return result;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Failed during tryWithSecondaryItem(GmodPath&&): {}", ex.what() );
			return std::move( *this );
		}
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( std::optional<GmodPath>&& item, bool& succeeded )
	{
		if ( !item.has_value() )
		{
			SPDLOG_TRACE( "tryWithSecondaryItem(optional): Optional has no value." );
			succeeded = false;
			return std::move( *this );
		}

		return tryWithSecondaryItem( std::move( *item ), succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( std::optional<GmodPath>&& item )
	{
		bool succeeded;
		return tryWithSecondaryItem( std::move( item ), succeeded );
	}

	LocalIdBuilder LocalIdBuilder::withoutSecondaryItem()
	{
		SPDLOG_DEBUG( "Removing secondary item." );
		LocalIdBuilder result( std::move( *this ) );

		result.m_items = LocalIdItems( std::move( result.m_items ), std::nullopt );
		return result;
	}

	//----------------------------------------------
	// Metadata Tags
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withMetadataTag( const MetadataTag& metadataTag )
	{
		SPDLOG_DEBUG( "Adding/replacing metadata tag: {} ('{}')",
			CodebookNames::toString( metadataTag.name() ),
			metadataTag.value() );

		switch ( metadataTag.name() )
		{
			case CodebookName::Quantity:
				return withQuantity( metadataTag );
			case CodebookName::Content:
				return withContent( metadataTag );
			case CodebookName::Calculation:
				return withCalculation( metadataTag );
			case CodebookName::State:
				return withState( metadataTag );
			case CodebookName::Command:
				return withCommand( metadataTag );
			case CodebookName::Type:
				return withType( metadataTag );
			case CodebookName::Position:
				return withPosition( metadataTag );
			case CodebookName::Detail:
				return withDetail( metadataTag );

			case CodebookName::FunctionalServices:
			case CodebookName::MaintenanceCategory:
			case CodebookName::ActivityType:
			default:
				SPDLOG_ERROR( "Unsupported codebook for direct metadata tag: {}", CodebookNames::toPrefix( metadataTag.name() ) );
				throw std::invalid_argument( "Unsupported codebook for direct metadata tag: " + std::string( CodebookNames::toPrefix( metadataTag.name() ) ) );
		}
	}

	LocalIdBuilder LocalIdBuilder::tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag )
	{
		bool succeeded;
		return tryWithMetadataTag( metadataTag, succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag, bool& succeeded )
	{
		succeeded = false;
		if ( !metadataTag.has_value() )
		{
			SPDLOG_TRACE( "tryWithMetadataTag called with empty optional." );
			return std::move( *this );
		}

		SPDLOG_TRACE( "Trying to add/replace metadata tag: {} ('{}')",
			CodebookNames::toString( metadataTag->name() ), metadataTag->value() );
		try
		{
			auto result = withMetadataTag( *metadataTag );
			succeeded = true;
			return result;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_WARN( "Failed to set metadata tag in tryWithMetadataTag: {}", ex.what() );
			return std::move( *this );
		}
	}

	LocalIdBuilder LocalIdBuilder::withoutMetadataTag( CodebookName name )
	{
		SPDLOG_DEBUG( "Removing metadata tag: {}", CodebookNames::toString( name ) );

		switch ( name )
		{
			case CodebookName::Quantity:
				return withoutQuantity();
			case CodebookName::Content:
				return withoutContent();
			case CodebookName::Calculation:
				return withoutCalculation();
			case CodebookName::State:
				return withoutState();
			case CodebookName::Command:
				return withoutCommand();
			case CodebookName::Type:
				return withoutType();
			case CodebookName::Position:
				return withoutPosition();
			case CodebookName::Detail:
				return withoutDetail();

			case CodebookName::FunctionalServices:
			case CodebookName::MaintenanceCategory:
			case CodebookName::ActivityType:
			default:
				SPDLOG_ERROR( "Cannot remove metadata tag for unsupported codebook: {}", CodebookNames::toPrefix( name ) );
				throw std::invalid_argument( "Cannot remove metadata tag for unsupported codebook: " + std::string( CodebookNames::toPrefix( name ) ) );
		}
	}

	//=====================================================================
	// Specific Metadata Tag Builder Methods
	//=====================================================================

	//----------------------------------------------
	// Quantity
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withQuantity( const MetadataTag& quantity )
	{
		if ( quantity.name() != CodebookName::Quantity )
		{
			SPDLOG_ERROR( "Invalid tag passed to withQuantity: expected Quantity, got {}", CodebookNames::toPrefix( quantity.name() ) );
			throw std::invalid_argument( "MetadataTag must be of type Quantity" );
		}
		SPDLOG_DEBUG( "Setting quantity tag: '{}'", quantity.value() );
		LocalIdBuilder result = std::move( *this );
		result.m_quantity = quantity;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutQuantity()
	{
		SPDLOG_DEBUG( "Removing quantity tag" );
		LocalIdBuilder result = std::move( *this );
		result.m_quantity = std::nullopt;
		return result;
	}

	//----------------------------------------------
	// Content
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withContent( const MetadataTag& content )
	{
		if ( content.name() != CodebookName::Content )
		{
			SPDLOG_ERROR( "Invalid tag passed to withContent: expected Content, got {}", CodebookNames::toPrefix( content.name() ) );
			throw std::invalid_argument( "MetadataTag must be of type Content" );
		}
		SPDLOG_DEBUG( "Setting content tag: '{}'", content.value() );
		LocalIdBuilder result = std::move( *this );
		result.m_content = content;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutContent()
	{
		SPDLOG_DEBUG( "Removing content tag" );
		LocalIdBuilder result = std::move( *this );
		result.m_content = std::nullopt;
		return result;
	}

	//----------------------------------------------
	// Calculation
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withCalculation( const MetadataTag& calculation )
	{
		if ( calculation.name() != CodebookName::Calculation )
		{
			SPDLOG_ERROR( "Invalid tag passed to withCalculation: expected Calculation, got {}", CodebookNames::toPrefix( calculation.name() ) );
			throw std::invalid_argument( "MetadataTag must be of type Calculation" );
		}
		SPDLOG_DEBUG( "Setting calculation tag: '{}'", calculation.value() );
		LocalIdBuilder result = std::move( *this );
		result.m_calculation = calculation;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutCalculation()
	{
		SPDLOG_DEBUG( "Removing calculation tag" );
		LocalIdBuilder result = std::move( *this );
		result.m_calculation = std::nullopt;
		return result;
	}

	//----------------------------------------------
	// State
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withState( const MetadataTag& state )
	{
		if ( state.name() != CodebookName::State )
		{
			SPDLOG_ERROR( "Invalid tag passed to withState: expected State, got {}", CodebookNames::toPrefix( state.name() ) );
			throw std::invalid_argument( "MetadataTag must be of type State" );
		}
		SPDLOG_DEBUG( "Setting state tag: '{}'", state.value() );
		LocalIdBuilder result = std::move( *this );
		result.m_state = state;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutState()
	{
		SPDLOG_DEBUG( "Removing state tag" );
		LocalIdBuilder result = std::move( *this );
		result.m_state = std::nullopt;
		return result;
	}

	//----------------------------------------------
	// Command
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withCommand( const MetadataTag& command )
	{
		if ( command.name() != CodebookName::Command )
		{
			SPDLOG_ERROR( "Invalid tag passed to withCommand: expected Command, got {}", CodebookNames::toPrefix( command.name() ) );
			throw std::invalid_argument( "MetadataTag must be of type Command" );
		}
		SPDLOG_DEBUG( "Setting command tag: '{}'", command.value() );
		LocalIdBuilder result = std::move( *this );
		result.m_command = command;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutCommand()
	{
		SPDLOG_DEBUG( "Removing command tag" );
		LocalIdBuilder result = std::move( *this );
		result.m_command = std::nullopt;
		return result;
	}

	//----------------------------------------------
	// Type
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withType( const MetadataTag& type )
	{
		if ( type.name() != CodebookName::Type )
		{
			SPDLOG_ERROR( "Invalid tag passed to withType: expected Type, got {}", CodebookNames::toPrefix( type.name() ) );
			throw std::invalid_argument( "MetadataTag must be of type Type" );
		}
		SPDLOG_DEBUG( "Setting type tag: '{}'", type.value() );
		LocalIdBuilder result = std::move( *this );
		result.m_type = type;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutType()
	{
		SPDLOG_DEBUG( "Removing type tag" );
		LocalIdBuilder result = std::move( *this );
		result.m_type = std::nullopt;
		return result;
	}

	//----------------------------------------------
	// Position
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withPosition( const MetadataTag& position )
	{
		if ( position.name() != CodebookName::Position )
		{
			SPDLOG_ERROR( "Invalid tag passed to withPosition: expected Position, got {}", CodebookNames::toPrefix( position.name() ) );
			throw std::invalid_argument( "MetadataTag must be of type Position" );
		}
		SPDLOG_DEBUG( "Setting position tag: '{}'", position.value() );
		LocalIdBuilder result = std::move( *this );
		result.m_position = position;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutPosition()
	{
		SPDLOG_DEBUG( "Removing position tag" );
		LocalIdBuilder result = std::move( *this );
		result.m_position = std::nullopt;
		return result;
	}

	//----------------------------------------------
	// Detail
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withDetail( const MetadataTag& detail )
	{
		if ( detail.name() != CodebookName::Detail )
		{
			SPDLOG_ERROR( "Invalid tag passed to withDetail: expected Detail, got {}", CodebookNames::toPrefix( detail.name() ) );
			throw std::invalid_argument( "MetadataTag must be of type Detail" );
		}
		SPDLOG_DEBUG( "Setting detail tag: '{}'", detail.value() );
		LocalIdBuilder result = std::move( *this );
		result.m_detail = detail;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutDetail()
	{
		SPDLOG_DEBUG( "Removing detail tag" );
		LocalIdBuilder result = std::move( *this );
		result.m_detail = std::nullopt;
		return result;
	}

	//=====================================================================
	// Static Parsing Methods
	//=====================================================================

	LocalIdBuilder LocalIdBuilder::parse( const std::string& localIdStr )
	{
		SPDLOG_DEBUG( "Parsing LocalId string: '{}'", localIdStr );

		ParsingErrors errors;
		std::optional<LocalIdBuilder> resultBuilder;

		if ( !tryParse( localIdStr, errors, resultBuilder ) || !resultBuilder.has_value() )
		{
			SPDLOG_ERROR( "Failed to parse LocalId string: '{}'", localIdStr );

			std::string errorMsg = "Failed to parse LocalId string: " + localIdStr;
			if ( !errors.isEmpty() )
			{
				errorMsg += ". Errors:";
				for ( const auto& err : errors )
				{
					errorMsg += "\n - " + std::get<0>( err ) + ": " + std::get<1>( err );
				}
			}
			throw std::invalid_argument( errorMsg );
		}

		SPDLOG_DEBUG( "Successfully parsed LocalId string." );
		return std::move( *resultBuilder );
	}

	bool LocalIdBuilder::tryParse( const std::string& localIdStr, std::optional<LocalIdBuilder>& localId )
	{
		ParsingErrors dummyErrors;

		return tryParse( localIdStr, dummyErrors, localId );
	}

	bool LocalIdBuilder::tryParse( const std::string& localIdStr, ParsingErrors& errors, std::optional<LocalIdBuilder>& localId )
	{
		SPDLOG_TRACE( "Attempting to parse LocalId string: '{}'", localIdStr );
		localId = std::nullopt;

		LocalIdParsingErrorBuilder errorBuilder = LocalIdParsingErrorBuilder::create();

		bool success = tryParseInternal( localIdStr, errorBuilder, localId );

		if ( errorBuilder.hasError() )
		{
			SPDLOG_DEBUG( "Parsing encountered errors." );
			errors = errorBuilder.build();
		}

		bool finalResult = success && localId.has_value();
		SPDLOG_TRACE( "tryParse result: {}", finalResult );
		return finalResult;
	}

	//=====================================================================
	// Private Static Helper Methods for Parsing
	//=====================================================================

	bool LocalIdBuilder::tryParseInternal( const std::string& localIdStr,
		LocalIdParsingErrorBuilder& errorBuilder,
		std::optional<LocalIdBuilder>& localIdBuilder )
	{
		SPDLOG_TRACE( "Internal parsing LocalId: '{}'", localIdStr );
		localIdBuilder = std::nullopt;

		if ( localIdStr.empty() )
		{
			SPDLOG_DEBUG( "Parsing failed: Input string is empty." );
			addError( errorBuilder, LocalIdParsingState::EmptyState, "Input string is empty" );
			return false;
		}

		if ( localIdStr[0] != '/' )
		{
			SPDLOG_DEBUG( "Parsing failed: String must start with '/'." );
			addError( errorBuilder, LocalIdParsingState::Formatting, "String must start with '/'" );
			return false;
		}

		std::string_view localIdView{ localIdStr };

		std::optional<VisVersion> parsedVisVersion{};
		std::shared_ptr<const Gmod> gmod = nullptr;
		std::shared_ptr<const Codebooks> codebooks = nullptr;

		std::optional<GmodPath> parsedPrimaryItem = GmodPath{};
		std::optional<GmodPath> parsedSecondaryItem = std::nullopt;

		std::optional<MetadataTag> qty{}, cnt{}, calc{}, stateTag{}, cmd{}, type{}, pos{}, detail{};

		bool parsedVerbose{ false };
		bool invalidSecondaryItemEncountered{ false };

		size_t primaryItemParseStart{ std::string::npos };
		size_t secondaryItemParseStart{ std::string::npos };

		LocalIdParsingState currentState{ LocalIdParsingState::NamingRule };
		size_t currentIndex{ 1 };

		while ( currentIndex < localIdView.length() && currentState <= LocalIdParsingState::MetaDetail )
		{
			size_t nextSlash = localIdView.find( '/', currentIndex );
			size_t segmentEnd = ( nextSlash == std::string_view::npos ) ? localIdView.length() : nextSlash;
			std::string_view segment = localIdView.substr( currentIndex, segmentEnd - currentIndex );

			SPDLOG_TRACE( "State: {}, Index: {}, Segment: '{}'", LocalIdParsingStateExtensions::toString( currentState ), currentIndex, segment );

			switch ( currentState )
			{
				case LocalIdParsingState::NamingRule:
					if ( segment != namingRule )
					{
						SPDLOG_DEBUG( "Parsing failed: Invalid naming rule '{}', expected '{}'", segment, namingRule );
						addError( errorBuilder, currentState, "Invalid naming rule: " + std::string( segment ) );
						return false;
					}
					advanceParser( currentIndex, segment, currentState, LocalIdParsingState::VisVersion );
					break;

				case LocalIdParsingState::VisVersion:
					try
					{
						parsedVisVersion = VisVersionExtensions::parse( std::string( segment ) );

						const Gmod& gmod_ref = VIS::instance().gmod( *parsedVisVersion );
						const Codebooks& codebooks_ref = VIS::instance().codebooks( *parsedVisVersion );

						gmod = std::shared_ptr<const Gmod>( &gmod_ref, []( const Gmod* ) {} );
						codebooks = std::shared_ptr<const Codebooks>( &codebooks_ref, []( const Codebooks* ) {} );

						if ( !gmod || !codebooks )
						{
							SPDLOG_ERROR( "Failed to create shared_ptr for GMOD/Codebooks for version {}", VisVersionExtensions::toVersionString( *parsedVisVersion ) );
							addError( errorBuilder, currentState, "Failed to initialize resources for VIS version" );
							return false;
						}
						advanceParser( currentIndex, segment, currentState, LocalIdParsingState::PrimaryItem );
						primaryItemParseStart = currentIndex;
					}
					catch ( [[maybe_unused]] const std::exception& ex )
					{
						SPDLOG_ERROR( "Parsing failed: Error processing VIS version '{}': {}", segment, ex.what() );
						addError( errorBuilder, currentState, "Error processing VIS version: " + std::string( segment ) );
						return false;
					}
					break;

				case LocalIdParsingState::PrimaryItem:
				{
					bool isSec = ( segment == "sec" );
					bool isMeta = ( segment == "meta" );
					bool isDesc = !segment.empty() && segment[0] == '~';

					if ( isSec || isMeta || isDesc )
					{
						std::string_view pathView = localIdView.substr( primaryItemParseStart, currentIndex - 1 - primaryItemParseStart );
						GmodPath tempPath;
						if ( gmod->tryParsePath( std::string( pathView ), tempPath ) )
						{
							parsedPrimaryItem = std::move( tempPath );
						}
						else
						{
							SPDLOG_WARN( "Invalid GMOD path in Primary item: '{}'", pathView );
							addError( errorBuilder, currentState, "Invalid GMOD path in Primary item: " + std::string( pathView ) );
						}

						if ( isSec )
							advanceParser( currentIndex, segment, currentState, LocalIdParsingState::SecondaryItem );
						else if ( isMeta )
							advanceParser( currentIndex, segment, currentState, LocalIdParsingState::MetaQuantity );
						else
						{
							parsedVerbose = true;

							size_t metaPos = localIdView.find( "/meta/", currentIndex - 1 );
							if ( metaPos == std::string::npos )
							{
								SPDLOG_DEBUG( "Parsing failed: Missing '/meta/' after item description." );
								addError( errorBuilder, LocalIdParsingState::ItemDescription, "Missing '/meta/' after item description" );
								return false;
							}

							currentIndex = metaPos + 6;
							advanceParser( currentState, LocalIdParsingState::MetaQuantity );
						}

						if ( currentState == LocalIdParsingState::SecondaryItem )
							secondaryItemParseStart = currentIndex;
					}
					else
					{
						advanceParser( currentIndex, segment );
					}
					break;
				}

				case LocalIdParsingState::SecondaryItem:
				{
					bool isMeta = ( segment == "meta" );
					bool isDesc = !segment.empty() && segment[0] == '~';

					if ( isMeta || isDesc )
					{
						std::string_view pathView = localIdView.substr( secondaryItemParseStart, currentIndex - 1 - secondaryItemParseStart );
						GmodPath tempPath;
						if ( gmod->tryParsePath( std::string( pathView ), tempPath ) )
						{
							parsedSecondaryItem = std::move( tempPath );
							SPDLOG_TRACE( "Successfully parsed secondary item path: '{}'", pathView );
						}
						else
						{
							SPDLOG_WARN( "Invalid GMOD path in Secondary item: '{}'", pathView );
							addError( errorBuilder, currentState, "Invalid GMOD path in Secondary item: " + std::string( pathView ) );
							invalidSecondaryItemEncountered = true;
						}

						if ( isMeta )
						{
							advanceParser( currentIndex, segment, currentState, LocalIdParsingState::MetaQuantity );
						}
						else
						{
							parsedVerbose = true;
							size_t metaPos = localIdView.find( "/meta/", currentIndex - 1 );
							if ( metaPos == std::string::npos )
							{
								SPDLOG_DEBUG( "Parsing failed: Missing '/meta/' after item description." );
								addError( errorBuilder, LocalIdParsingState::ItemDescription, "Missing '/meta/' after item description" );
								return false;
							}
							currentIndex = metaPos + 6;
							advanceParser( currentState, LocalIdParsingState::MetaQuantity );
						}
					}
					else
					{
						const GmodNode* tempNodePtr = nullptr;
						if ( !gmod->tryGetNode( std::string( segment ), tempNodePtr ) )
						{
							SPDLOG_WARN( "Invalid intermediate GMOD node in Secondary item path: '{}'", segment );
							addError( errorBuilder, currentState, "Invalid intermediate GMOD node in Secondary item path: " + std::string( segment ) );
							invalidSecondaryItemEncountered = true;
						}

						advanceParser( currentIndex, segment );
					}
					break;
				}

				case LocalIdParsingState::MetaQuantity:
					if ( !parseMetaTag( CodebookName::Quantity, currentState, currentIndex, segment, qty, codebooks, errorBuilder ) )
						return false;
					break;
				case LocalIdParsingState::MetaContent:
					if ( !parseMetaTag( CodebookName::Content, currentState, currentIndex, segment, cnt, codebooks, errorBuilder ) )
						return false;
					break;
				case LocalIdParsingState::MetaCalculation:
					if ( !parseMetaTag( CodebookName::Calculation, currentState, currentIndex, segment, calc, codebooks, errorBuilder ) )
						return false;
					break;
				case LocalIdParsingState::MetaState:
					if ( !parseMetaTag( CodebookName::State, currentState, currentIndex, segment, stateTag, codebooks, errorBuilder ) )
						return false;
					break;
				case LocalIdParsingState::MetaCommand:
					if ( !parseMetaTag( CodebookName::Command, currentState, currentIndex, segment, cmd, codebooks, errorBuilder ) )
						return false;
					break;
				case LocalIdParsingState::MetaType:
					if ( !parseMetaTag( CodebookName::Type, currentState, currentIndex, segment, type, codebooks, errorBuilder ) )
						return false;
					break;
				case LocalIdParsingState::MetaPosition:
					if ( !parseMetaTag( CodebookName::Position, currentState, currentIndex, segment, pos, codebooks, errorBuilder ) )
						return false;
					break;
				case LocalIdParsingState::MetaDetail:
					if ( !parseMetaTag( CodebookName::Detail, currentState, currentIndex, segment, detail, codebooks, errorBuilder ) )
						return false;
					break;

				case LocalIdParsingState::ItemDescription:
				case LocalIdParsingState::EmptyState:
				case LocalIdParsingState::Formatting:
				case LocalIdParsingState::Completeness:
				case LocalIdParsingState::NamingEntity:
				case LocalIdParsingState::IMONumber:
				default:
					SPDLOG_ERROR( "Unexpected state encountered during parsing: {}", static_cast<int>( currentState ) );
					addError( errorBuilder, currentState, "Unexpected internal parser state" );
					return false;
			}

			if ( nextSlash == std::string_view::npos && currentIndex < localIdView.length() )
			{
				SPDLOG_TRACE( "End of string reached at index {}, current state {}", currentIndex, static_cast<int>( currentState ) );
			}
		}

		if ( !parsedVisVersion.has_value() )
		{
			SPDLOG_DEBUG( "Parsing finished but VIS version was not parsed." );
			addError( errorBuilder, LocalIdParsingState::VisVersion, "VIS version missing or invalid" );

			return false;
		}
		if ( parsedPrimaryItem->length() == 0 && primaryItemParseStart != std::string::npos )
		{
			SPDLOG_DEBUG( "Parsing finished but Primary Item path is invalid/empty." );
			addError( errorBuilder, LocalIdParsingState::PrimaryItem, "Primary item path missing or invalid" );
		}
		else if ( primaryItemParseStart == std::string::npos )
		{
			SPDLOG_DEBUG( "Parsing finished but Primary Item section was never reached/parsed." );
			addError( errorBuilder, LocalIdParsingState::PrimaryItem, "Primary item section missing" );
			return false;
		}

		bool hasAnyMetadata = qty || cnt || calc || stateTag || cmd || type || pos || detail;
		if ( !hasAnyMetadata )
		{
			SPDLOG_DEBUG( "Parsing finished but no metadata tags were found." );
			addError( errorBuilder, LocalIdParsingState::Completeness, "At least one metadata tag is required" );
		}

		if ( currentState < LocalIdParsingState::MetaQuantity && !hasAnyMetadata )
		{
			SPDLOG_DEBUG( "Parsing finished prematurely before metadata section." );
			addError( errorBuilder, LocalIdParsingState::Completeness, "Metadata section is missing" );
		}

		if ( invalidSecondaryItemEncountered )
		{
			SPDLOG_DEBUG( "Internal parsing finished with errors (invalid secondary item), builder not constructed." );

			return false;
		}

		if ( !errorBuilder.hasError() )
		{
			LocalIdBuilder builder;
			builder.m_visVersion = parsedVisVersion;
			builder.m_verboseMode = parsedVerbose;

			if ( parsedPrimaryItem.has_value() )
			{
				builder.m_items = LocalIdItems( std::move( builder.m_items ), std::move( parsedPrimaryItem.value() ) );
			}
			else
			{
				builder.m_items = LocalIdItems( std::move( builder.m_items ), GmodPath{} );
			}

			if ( parsedSecondaryItem.has_value() )
			{
				builder.m_items = LocalIdItems( std::move( builder.m_items ), std::move( parsedSecondaryItem ) );
			}
			builder.m_quantity = qty;
			builder.m_content = cnt;
			builder.m_calculation = calc;
			builder.m_state = stateTag;
			builder.m_command = cmd;
			builder.m_type = type;
			builder.m_position = pos;
			builder.m_detail = detail;

			localIdBuilder = std::move( builder );
			SPDLOG_TRACE( "Internal parsing succeeded (no errors detected)." );
			return true;
		}
		else
		{
			SPDLOG_DEBUG( "Internal parsing finished with errors, builder not constructed." );
			return false;
		}
	}

	void LocalIdBuilder::addError( LocalIdParsingErrorBuilder& errorBuilder, LocalIdParsingState state, const std::string& message )
	{
		errorBuilder.addError( state, message );
	}

	void LocalIdBuilder::advanceParser( size_t& i, const std::string_view& segment )
	{
		i += segment.length() + 1;
	}

	void LocalIdBuilder::advanceParser( size_t& i, const std::string_view& segment, LocalIdParsingState& state )
	{
		state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
		i += segment.length() + 1;
	}

	void LocalIdBuilder::advanceParser( size_t& i, const std::string_view& segment, LocalIdParsingState& state, LocalIdParsingState to )
	{
		state = to;
		i += segment.length() + 1;
	}

	void LocalIdBuilder::advanceParser( LocalIdParsingState& state, LocalIdParsingState to )
	{
		state = to;
	}

	std::pair<size_t, size_t> LocalIdBuilder::nextStateIndexes( const std::string& str, LocalIdParsingState state )
	{
		size_t secIndex = str.find( "/sec/" );
		size_t metaIndex = str.find( "/meta/" );
		size_t descIndex = str.find( "/~" );

		size_t nextIndex = std::string::npos;
		size_t endIndex = std::string::npos;
		size_t markerLength = 0;

		if ( state == LocalIdParsingState::PrimaryItem )
		{
			nextIndex = secIndex;
			markerLength = 5;

			if ( metaIndex != std::string::npos && ( nextIndex == std::string::npos || metaIndex < nextIndex ) )
			{
				nextIndex = metaIndex;
				markerLength = 6;
			}
			if ( descIndex != std::string::npos && ( nextIndex == std::string::npos || descIndex < nextIndex ) )
			{
				nextIndex = descIndex;
				markerLength = 2;
			}
		}
		else if ( state == LocalIdParsingState::SecondaryItem )
		{
			nextIndex = metaIndex;
			markerLength = 6;

			if ( descIndex != std::string::npos && ( nextIndex == std::string::npos || descIndex < nextIndex ) )
			{
				nextIndex = descIndex;
				markerLength = 2;
			}
		}
		else
		{
			nextIndex = metaIndex;
			markerLength = 6;
		}

		if ( nextIndex != std::string::npos )
		{
			endIndex = nextIndex + markerLength;
		}

		return { nextIndex, endIndex };
	}

	std::optional<LocalIdParsingState> LocalIdBuilder::metaPrefixToState( const std::string_view& prefix )
	{
		if ( prefix == "q" || prefix == "qty" )
			return LocalIdParsingState::MetaQuantity;
		if ( prefix == "c" || prefix == "cnt" )
			return LocalIdParsingState::MetaContent;
		if ( prefix == "cal" || prefix == "calc" )
			return LocalIdParsingState::MetaCalculation;
		if ( prefix == "s" || prefix == "state" )
			return LocalIdParsingState::MetaState;
		if ( prefix == "cmd" )
			return LocalIdParsingState::MetaCommand;
		if ( prefix == "t" || prefix == "type" )
			return LocalIdParsingState::MetaType;
		if ( prefix == "pos" )
			return LocalIdParsingState::MetaPosition;
		if ( prefix == "d" || prefix == "detail" )
			return LocalIdParsingState::MetaDetail;

		return std::nullopt;
	}

	std::optional<LocalIdParsingState> LocalIdBuilder::nextParsingState( LocalIdParsingState prev )
	{
		switch ( prev )
		{
			case LocalIdParsingState::MetaQuantity:
				return LocalIdParsingState::MetaContent;
			case LocalIdParsingState::MetaContent:
				return LocalIdParsingState::MetaCalculation;
			case LocalIdParsingState::MetaCalculation:
				return LocalIdParsingState::MetaState;
			case LocalIdParsingState::MetaState:
				return LocalIdParsingState::MetaCommand;
			case LocalIdParsingState::MetaCommand:
				return LocalIdParsingState::MetaType;
			case LocalIdParsingState::MetaType:
				return LocalIdParsingState::MetaPosition;
			case LocalIdParsingState::MetaPosition:
				return LocalIdParsingState::MetaDetail;
			case LocalIdParsingState::MetaDetail:
				return std::nullopt;

			case LocalIdParsingState::NamingRule:
			case LocalIdParsingState::VisVersion:
			case LocalIdParsingState::PrimaryItem:
			case LocalIdParsingState::SecondaryItem:
			case LocalIdParsingState::ItemDescription:
			case LocalIdParsingState::EmptyState:
			case LocalIdParsingState::Formatting:
			case LocalIdParsingState::Completeness:
			case LocalIdParsingState::NamingEntity:
			case LocalIdParsingState::IMONumber:
			default:
				return std::nullopt;
		}
	}

	bool LocalIdBuilder::parseMetaTag( CodebookName codebookName, LocalIdParsingState& state,
		size_t& i, const std::string_view& segment, std::optional<MetadataTag>& tag,
		const std::shared_ptr<const Codebooks>& codebooks,
		LocalIdParsingErrorBuilder& errorBuilder )
	{
		if ( !codebooks )
		{
			addError( errorBuilder, state, "Codebooks not available for validation" );
			return false;
		}

		size_t dashIndex = segment.find( '-' );
		size_t tildeIndex = segment.find( '~' );
		size_t separatorIndex = std::min( dashIndex, tildeIndex );

		if ( separatorIndex == std::string_view::npos )
		{
			auto actualState = metaPrefixToState( segment );
			if ( actualState.has_value() && actualState.value() > state )
			{
				SPDLOG_TRACE( "Metadata state transition from {} to {} based on prefix '{}'",
					LocalIdParsingStateExtensions::toString( state ), LocalIdParsingStateExtensions::toString( *actualState ), segment );
				advanceParser( state, *actualState );
				return true;
			}
			else
			{
				SPDLOG_DEBUG( "Invalid metadata segment format: '{}'. Missing separator '-' or '~'.", segment );
				addError( errorBuilder, state, "Invalid metadata format: missing separator in '" + std::string( segment ) + "'" );
				auto nextExpected = nextParsingState( state );
				if ( nextExpected )
				{
					advanceParser( state, *nextExpected );

					return true;
				}
				return false;
			}
		}

		std::string_view actualPrefix = segment.substr( 0, separatorIndex );
		std::string_view value = segment.substr( separatorIndex + 1 );
		char separator = segment[separatorIndex];

		auto actualState = metaPrefixToState( actualPrefix );
		if ( !actualState.has_value() )
		{
			SPDLOG_DEBUG( "Invalid metadata prefix: '{}'", actualPrefix );
			addError( errorBuilder, state, "Invalid metadata prefix: " + std::string( actualPrefix ) );
			return false;
		}

		if ( actualState.value() != state )
		{
			if ( actualState.value() > state )
			{
				SPDLOG_TRACE( "Metadata state transition from {} to {} based on prefix '{}'",
					LocalIdParsingStateExtensions::toString( state ), LocalIdParsingStateExtensions::toString( *actualState ), actualPrefix );
				advanceParser( state, *actualState );
				return true;
			}
			else
			{
				SPDLOG_DEBUG( "Out-of-order metadata tag: prefix '{}' found in state {}", actualPrefix, LocalIdParsingStateExtensions::toString( state ) );
				addError( errorBuilder, state, "Out-of-order metadata tag: " + std::string( actualPrefix ) );
				auto nextExpected = nextParsingState( state );
				if ( nextExpected )
					advanceParser( state, *nextExpected );
				else
					state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
				advanceParser( i, segment );

				return true;
			}
		}

		if ( value.empty() )
		{
			SPDLOG_DEBUG( "Missing value for metadata tag with prefix '{}'", actualPrefix );
			addError( errorBuilder, state, "Missing value for metadata tag " + std::string( actualPrefix ) );

			auto nextExpected = nextParsingState( state );
			if ( nextExpected )
				advanceParser( state, *nextExpected );
			else
				state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
			advanceParser( i, segment );
			return true;
		}

		std::optional<MetadataTag> createdTag = codebooks->tryCreateTag( codebookName, std::string( value ) );

		if ( !createdTag.has_value() )
		{
			SPDLOG_DEBUG( "Invalid value '{}' for metadata tag '{}'", value, actualPrefix );
			addError( errorBuilder, state, "Invalid value '" + std::string( value ) + "' for metadata tag " + std::string( actualPrefix ) );

			auto nextExpected = nextParsingState( state );
			if ( nextExpected )
				advanceParser( state, *nextExpected );
			else
				state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
			advanceParser( i, segment );
			return true;
		}

		if ( separator == '~' && !createdTag->isCustom() )
		{
			SPDLOG_DEBUG( "Used custom prefix '~' for standard value '{}' in tag '{}'", value, actualPrefix );
			addError( errorBuilder, state, "Used custom prefix '~' for standard value '" + std::string( value ) + "' in tag " + std::string( actualPrefix ) );
		}
		else if ( separator == '-' && createdTag->isCustom() )
		{
			SPDLOG_DEBUG( "Used standard prefix '-' for custom value '{}' in tag '{}'", value, actualPrefix );
			addError( errorBuilder, state, "Used standard prefix '-' for custom value '" + std::string( value ) + "' in tag " + std::string( actualPrefix ) );
		}

		tag = createdTag;
		SPDLOG_TRACE( "Parsed metadata tag: {} = '{}'", actualPrefix, value );

		auto nextExpected = nextParsingState( state );
		if ( nextExpected )
			advanceParser( state, *nextExpected );
		else
			state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );

		advanceParser( i, segment );
		return true;
	}
}
