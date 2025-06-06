/**
 * @file LocalIdBuilder.cpp
 * @brief Implementation of the LocalIdBuilder class
 */

#include "pch.h"

#include "dnv/vista/sdk/LocalIdBuilder.h"

#include "dnv/vista/sdk/CodebookName.h"
#include "dnv/vista/sdk/LocalIdParsingErrorBuilder.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VISVersion.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Static helper functions
		//=====================================================================

		std::pair<size_t, size_t> nextStateIndexes( std::string_view span, LocalIdParsingState state )
		{
			size_t customIndex = span.find( "~" );
			size_t endOfCustomIndex = ( customIndex != std::string_view::npos ) ? ( customIndex + 1 + 1 ) : std::string_view::npos;

			size_t metaIndex = span.find( "/meta" );
			size_t endOfMetaIndex = ( metaIndex != std::string_view::npos ) ? ( metaIndex + 5 + 1 ) : std::string_view::npos;

			bool isVerbose = ( customIndex != std::string_view::npos ) &&
							 ( metaIndex != std::string_view::npos ) &&
							 ( customIndex < metaIndex );

			switch ( state )
			{
				case LocalIdParsingState::PrimaryItem:
				{
					size_t secIndex = span.find( "/sec" );
					size_t endOfSecIndex = ( secIndex != std::string_view::npos ) ? ( secIndex + 4 + 1 ) : std::string_view::npos;

					if ( secIndex != std::string_view::npos )
						return { secIndex, endOfSecIndex };

					if ( isVerbose && customIndex != std::string_view::npos )
						return { customIndex, endOfCustomIndex };

					return { metaIndex, endOfMetaIndex };
				}

				case LocalIdParsingState::SecondaryItem:
					if ( isVerbose && customIndex != std::string_view::npos )
						return { customIndex, endOfCustomIndex };
					return { metaIndex, endOfMetaIndex };

				case LocalIdParsingState::NamingRule:
				case LocalIdParsingState::VisVersion:
				case LocalIdParsingState::ItemDescription:
				case LocalIdParsingState::MetaQuantity:
				case LocalIdParsingState::MetaContent:
				case LocalIdParsingState::MetaCalculation:
				case LocalIdParsingState::MetaState:
				case LocalIdParsingState::MetaCommand:
				case LocalIdParsingState::MetaType:
				case LocalIdParsingState::MetaPosition:
				case LocalIdParsingState::MetaDetail:
				case LocalIdParsingState::EmptyState:
				case LocalIdParsingState::Formatting:
				case LocalIdParsingState::Completeness:
				case LocalIdParsingState::NamingEntity:
				case LocalIdParsingState::IMONumber:
				default:
					return { metaIndex, endOfMetaIndex };
			}
		}

		static std::string codebookNametoString( CodebookName name )
		{
			switch ( name )
			{
				case CodebookName::Position:
					return "Position";
				case CodebookName::Quantity:
					return "Quantity";
				case CodebookName::Calculation:
					return "Calculation";
				case CodebookName::State:
					return "State";
				case CodebookName::Content:
					return "Content";
				case CodebookName::Command:
					return "Command";
				case CodebookName::Type:
					return "Type";
				case CodebookName::FunctionalServices:
					return "FunctionalServices";
				case CodebookName::MaintenanceCategory:
					return "MaintenanceCategory";
				case CodebookName::ActivityType:
					return "ActivityType";
				case CodebookName::Detail:
					return "Detail";
				default:
					throw std::invalid_argument( "Unknown codebook: " + std::to_string( static_cast<int>( name ) ) );
			}
		}
	}

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
	// LocalIdBuilder class
	//=====================================================================

	//----------------------------------------------
	// Assignment operators
	//----------------------------------------------

	LocalIdBuilder& LocalIdBuilder::operator=( LocalIdBuilder&& other ) noexcept
	{
		if ( this != &other )
		{
			m_visVersion = std::move( other.m_visVersion );
			m_verboseMode = other.m_verboseMode;
			m_items = std::move( other.m_items );
			m_quantity = std::move( other.m_quantity );
			m_content = std::move( other.m_content );
			m_calculation = std::move( other.m_calculation );
			m_state = std::move( other.m_state );
			m_command = std::move( other.m_command );
			m_type = std::move( other.m_type );
			m_position = std::move( other.m_position );
			m_detail = std::move( other.m_detail );
		}
		return *this;
	}

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	bool LocalIdBuilder::operator==( const LocalIdBuilder& other ) const
	{
		return equals( other );
	}

	bool LocalIdBuilder::operator!=( const LocalIdBuilder& other ) const
	{
		return !equals( other );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	std::optional<VisVersion> LocalIdBuilder::visVersion() const
	{
		return m_visVersion;
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
		if ( m_calculation.has_value() )
			tags.push_back( *m_calculation );
		if ( m_content.has_value() )
			tags.push_back( *m_content );
		if ( m_position.has_value() )
			tags.push_back( *m_position );
		if ( m_state.has_value() )
			tags.push_back( *m_state );
		if ( m_command.has_value() )
			tags.push_back( *m_command );
		if ( m_type.has_value() )
			tags.push_back( *m_type );
		if ( m_detail.has_value() )
			tags.push_back( *m_detail );

		return tags;
	}

	size_t LocalIdBuilder::hashCode() const noexcept
	{
		size_t hash = 0;

		if ( m_items.primaryItem().has_value() )
			hash ^= m_items.primaryItem()->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		if ( m_items.secondaryItem().has_value() )
			hash ^= m_items.secondaryItem()->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		if ( m_quantity.has_value() )
			hash ^= m_quantity->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		if ( m_calculation.has_value() )
			hash ^= m_calculation->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		if ( m_content.has_value() )
			hash ^= m_content->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		if ( m_position.has_value() )
			hash ^= m_position->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		if ( m_state.has_value() )
			hash ^= m_state->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		if ( m_command.has_value() )
			hash ^= m_command->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		if ( m_type.has_value() )
			hash ^= m_type->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		if ( m_detail.has_value() )
			hash ^= m_detail->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );

		return hash;
	}

	//----------------------------------------------
	// State inspection methods
	//----------------------------------------------

	bool LocalIdBuilder::isValid() const noexcept
	{
		if ( !m_visVersion.has_value() )
		{
			return false;
		}

		if ( !m_items.primaryItem().has_value() )
		{
			return false;
		}

		if ( !m_quantity.has_value() &&
			 !m_calculation.has_value() &&
			 !m_content.has_value() &&
			 !m_position.has_value() &&
			 !m_state.has_value() &&
			 !m_command.has_value() &&
			 !m_type.has_value() &&
			 !m_detail.has_value() )
		{
			return false;
		}

		return true;
	}

	bool LocalIdBuilder::isEmpty() const noexcept
	{
		bool itemsEmpty = !m_items.primaryItem().has_value() &&
						  !m_items.secondaryItem().has_value();

		bool metadataEmpty = !m_quantity.has_value() &&
							 !m_content.has_value() &&
							 !m_calculation.has_value() &&
							 !m_state.has_value() &&
							 !m_command.has_value() &&
							 !m_type.has_value() &&
							 !m_position.has_value() &&
							 !m_detail.has_value();

		return itemsEmpty && metadataEmpty;
	}

	bool LocalIdBuilder::isVerboseMode() const noexcept
	{
		return m_verboseMode;
	}

	//----------------------------------------------
	// Metadata inspection methods
	//----------------------------------------------

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

	//----------------------------------------------
	// Conversion and comparison
	//----------------------------------------------

	std::string LocalIdBuilder::toString() const
	{
		std::stringstream ss;
		toString( ss );

		return ss.str();
	}

	void LocalIdBuilder::toString( std::stringstream& builder ) const
	{
		if ( !m_visVersion.has_value() )
		{
			throw std::invalid_argument( "No VisVersion configured on LocalId" );
		}

		builder << "/" << namingRule << "/";
		builder << VisVersionExtensions::toVersionString( *m_visVersion );
		builder << '/';

		m_items.append( builder, m_verboseMode );
		builder << "meta/";

		auto appendMeta = [&builder]( const std::optional<MetadataTag>& tag ) {
			if ( !tag.has_value() )
			{
				return;
			}

			builder << CodebookNames::toPrefix( tag->name() ) << tag->prefix() << tag->value() << "/";
		};

		appendMeta( m_quantity );
		appendMeta( m_content );
		appendMeta( m_calculation );
		appendMeta( m_state );
		appendMeta( m_command );
		appendMeta( m_type );
		appendMeta( m_position );
		appendMeta( m_detail );

		std::string result = builder.str();
		if ( !result.empty() && result.back() == '/' )
		{
			result.pop_back();
			builder.str( result );
			builder.clear();
		}
	}

	bool LocalIdBuilder::equals( const LocalIdBuilder& other ) const
	{
		if ( m_visVersion != other.m_visVersion )
			throw std::invalid_argument( "Cant compare local IDs from different VIS versions" );

		return m_items.primaryItem() == other.m_items.primaryItem() &&
			   m_items.secondaryItem() == other.m_items.secondaryItem() &&
			   m_quantity == other.m_quantity &&
			   m_calculation == other.m_calculation &&
			   m_content == other.m_content &&
			   m_position == other.m_position &&
			   m_state == other.m_state &&
			   m_command == other.m_command &&
			   m_type == other.m_type &&
			   m_detail == other.m_detail;
	}

	//----------------------------------------------
	// Static factory methods
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::create( VisVersion version )
	{
		return LocalIdBuilder().withVisVersion( version );
	}

	//----------------------------------------------
	// Build methods (Immutable fluent interface)
	//----------------------------------------------

	//----------------------------
	// Build
	//----------------------------

	LocalId LocalIdBuilder::build()
	{
		if ( isEmpty() )
		{
			throw std::invalid_argument( "Cannot build LocalId: builder is empty." );
		}
		if ( !isValid() )
		{
			throw std::invalid_argument( "Cannot build LocalId: builder state is invalid." );
		}

		return LocalId( std::move( *this ) );
	}

	//----------------------------------------------
	// Verbose Mode
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withVerboseMode( bool verboseMode )
	{
		LocalIdBuilder result = std::move( *this );
		result.m_verboseMode = verboseMode;

		return result;
	}

	//----------------------------------------------
	// VIS Version
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withVisVersion( const std::string& visVersionStr )
	{
		bool succeeded;
		auto localIdBuilder = tryWithVisVersion( visVersionStr, succeeded );

		if ( !succeeded )
		{
			throw std::invalid_argument( "Failed to parse VIS version" );
		}

		return localIdBuilder;
	}

	LocalIdBuilder LocalIdBuilder::withVisVersion( VisVersion version )
	{
		bool succeeded;
		auto localIdBuilder = tryWithVisVersion( version, succeeded );

		if ( !succeeded )
		{
			throw std::invalid_argument( "withVisVersion" );
		}

		return localIdBuilder;
	}

	LocalIdBuilder LocalIdBuilder::tryWithVisVersion( const std::optional<VisVersion>& version )
	{
		bool succeeded;
		return tryWithVisVersion( version, succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithVisVersion( const std::optional<VisVersion>& version, bool& succeeded )
	{
		succeeded = true;
		LocalIdBuilder result = std::move( *this );
		result.m_visVersion = version;

		return result;
	}

	LocalIdBuilder LocalIdBuilder::tryWithVisVersion( const std::optional<std::string>& visVersionStr, bool& succeeded )
	{
		if ( visVersionStr.has_value() )
		{
			VisVersion v;
			if ( VisVersionExtensions::tryParse( *visVersionStr, v ) )
			{
				auto localIdBuilder = tryWithVisVersion( v, succeeded );
				return localIdBuilder;
			}
		}

		succeeded = false;
		return std::move( *this );
	}

	LocalIdBuilder LocalIdBuilder::withoutVisVersion()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_visVersion = std::nullopt;

		return result;
	}

	//----------------------------------------------
	// Primary Item
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withPrimaryItem( GmodPath&& item )
	{
		bool succeeded;
		auto localIdBuilder = tryWithPrimaryItem( std::move( item ), succeeded );

		if ( !succeeded )
		{
			throw std::invalid_argument( "Failed to set primary item: invalid or empty GmodPath." );
		}

		return localIdBuilder;
	}

	LocalIdBuilder LocalIdBuilder::tryWithPrimaryItem( GmodPath&& item )
	{
		bool succeeded;
		return tryWithPrimaryItem( std::move( item ), succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithPrimaryItem( GmodPath&& item, bool& succeeded )
	{
		if ( item.length() == 0 )
		{
			succeeded = false;
			return std::move( *this );
		}

		succeeded = true;
		LocalIdBuilder result = std::move( *this );
		result.m_items = LocalIdItems( std::move( result.m_items ), std::move( item ) );

		return result;
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
			succeeded = false;
			return std::move( *this );
		}

		succeeded = true;
		LocalIdBuilder result = std::move( *this );
		result.m_items = LocalIdItems( std::move( result.m_items ), std::move( *item ) );

		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutPrimaryItem()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_items = LocalIdItems{};

		return result;
	}

	//----------------------------------------------
	// Secondary Item
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withSecondaryItem( GmodPath&& item )
	{
		bool succeeded;
		auto localIdBuilder = tryWithSecondaryItem( std::move( item ), succeeded );

		if ( !succeeded )
		{
			throw std::invalid_argument( "Failed to set secondary item: invalid or empty GmodPath." );
		}

		return localIdBuilder;
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( GmodPath&& item )
	{
		bool succeeded;
		return tryWithSecondaryItem( std::move( item ), succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( GmodPath&& item, bool& succeeded )
	{
		if ( item.length() == 0 )
		{
			succeeded = false;
			return std::move( *this );
		}

		succeeded = true;
		LocalIdBuilder result = std::move( *this );
		result.m_items = LocalIdItems( std::move( result.m_items ), std::make_optional( std::move( item ) ) );

		return result;
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( std::optional<GmodPath>&& item )
	{
		bool succeeded;
		return tryWithSecondaryItem( std::move( item ), succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( std::optional<GmodPath>&& item, bool& succeeded )
	{
		if ( !item.has_value() )
		{
			succeeded = false;
			return std::move( *this );
		}

		succeeded = true;
		LocalIdBuilder result = std::move( *this );
		result.m_items = LocalIdItems( std::move( result.m_items ), std::move( item ) );

		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutSecondaryItem()
	{
		LocalIdBuilder result( std::move( *this ) );

		result.m_items = LocalIdItems( std::move( result.m_items ), std::nullopt );
		return result;
	}

	//----------------------------------------------
	// Metadata Tags
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::withMetadataTag( const MetadataTag& metadataTag )
	{
		bool succeeded;
		auto localIdBuilder = tryWithMetadataTag( metadataTag, succeeded );

		if ( !succeeded )
		{
			throw std::invalid_argument( "invalid metadata codebook name: " + std::string( CodebookNames::toPrefix( metadataTag.name() ) ) );
		}

		return localIdBuilder;
	}

	LocalIdBuilder LocalIdBuilder::tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag )
	{
		bool succeeded;
		return tryWithMetadataTag( metadataTag, succeeded );
	}

	LocalIdBuilder LocalIdBuilder::tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag, bool& succeeded )
	{
		if ( !metadataTag.has_value() )
		{
			succeeded = false;
			return std::move( *this );
		}

		switch ( metadataTag->name() )
		{
			case CodebookName::Quantity:
				succeeded = true;
				return withQuantity( *metadataTag );
			case CodebookName::Content:
				succeeded = true;
				return withContent( *metadataTag );
			case CodebookName::Calculation:
				succeeded = true;
				return withCalculation( *metadataTag );
			case CodebookName::State:
				succeeded = true;
				return withState( *metadataTag );
			case CodebookName::Command:
				succeeded = true;
				return withCommand( *metadataTag );
			case CodebookName::Type:
				succeeded = true;
				return withType( *metadataTag );
			case CodebookName::Position:
				succeeded = true;
				return withPosition( *metadataTag );
			case CodebookName::Detail:
				succeeded = true;
				return withDetail( *metadataTag );
			case CodebookName::FunctionalServices:
			case CodebookName::MaintenanceCategory:
			case CodebookName::ActivityType:
			default:
				succeeded = false;
				return std::move( *this );
		}
	}

	LocalIdBuilder LocalIdBuilder::withoutMetadataTag( CodebookName name )
	{
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
				return std::move( *this );
		}
	}

	//----------------------------------------------
	// Specific metadata tag builder methods
	//----------------------------------------------

	//----------------------------
	// Quantity
	//----------------------------

	LocalIdBuilder LocalIdBuilder::withQuantity( const MetadataTag& quantity )
	{
		LocalIdBuilder result = std::move( *this );
		result.m_quantity = quantity;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutQuantity()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_quantity = std::nullopt;
		return result;
	}

	//----------------------------
	// Content
	//----------------------------

	LocalIdBuilder LocalIdBuilder::withContent( const MetadataTag& content )
	{
		LocalIdBuilder result = std::move( *this );
		result.m_content = content;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutContent()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_content = std::nullopt;
		return result;
	}

	//----------------------------
	// Calculation
	//----------------------------

	LocalIdBuilder LocalIdBuilder::withCalculation( const MetadataTag& calculation )
	{
		LocalIdBuilder result = std::move( *this );
		result.m_calculation = calculation;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutCalculation()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_calculation = std::nullopt;
		return result;
	}

	//----------------------------
	// State
	//----------------------------

	LocalIdBuilder LocalIdBuilder::withState( const MetadataTag& state )
	{
		LocalIdBuilder result = std::move( *this );
		result.m_state = state;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutState()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_state = std::nullopt;
		return result;
	}

	//----------------------------
	// Command
	//----------------------------

	LocalIdBuilder LocalIdBuilder::withCommand( const MetadataTag& command )
	{
		LocalIdBuilder result = std::move( *this );
		result.m_command = command;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutCommand()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_command = std::nullopt;
		return result;
	}

	//----------------------------
	// Type
	//----------------------------

	LocalIdBuilder LocalIdBuilder::withType( const MetadataTag& type )
	{
		LocalIdBuilder result = std::move( *this );
		result.m_type = type;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutType()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_type = std::nullopt;
		return result;
	}

	//----------------------------
	// Position
	//----------------------------

	LocalIdBuilder LocalIdBuilder::withPosition( const MetadataTag& position )
	{
		LocalIdBuilder result = std::move( *this );
		result.m_position = position;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutPosition()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_position = std::nullopt;
		return result;
	}

	//----------------------------
	// Detail
	//----------------------------

	LocalIdBuilder LocalIdBuilder::withDetail( const MetadataTag& detail )
	{
		LocalIdBuilder result = std::move( *this );
		result.m_detail = detail;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutDetail()
	{
		LocalIdBuilder result = std::move( *this );
		result.m_detail = std::nullopt;
		return result;
	}

	//----------------------------------------------
	// Static parsing methods
	//----------------------------------------------

	LocalIdBuilder LocalIdBuilder::parse( const std::string& localIdStr )
	{
		std::optional<LocalIdBuilder> localId;
		ParsingErrors errors;

		if ( !tryParse( localIdStr, errors, localId ) )
		{
			throw std::invalid_argument( "Couldn't parse local ID from: '" + localIdStr + "'. " + errors.toString() );
		}

		return std::move( *localId );
	}

	bool LocalIdBuilder::tryParse( const std::string& localIdStr, std::optional<LocalIdBuilder>& localId )
	{
		ParsingErrors dummyErrors;

		return tryParse( localIdStr, dummyErrors, localId );
	}

	bool LocalIdBuilder::tryParse( const std::string& localIdStr, ParsingErrors& errors, std::optional<LocalIdBuilder>& localId )
	{
		localId = std::nullopt;

		LocalIdParsingErrorBuilder errorBuilder = LocalIdParsingErrorBuilder::create();

		bool success = tryParseInternal( localIdStr, errorBuilder, localId );

		if ( errorBuilder.hasError() )
		{
			SPDLOG_ERROR( "Parsing encountered errors." );
		}

		errors = errorBuilder.build();
		return success;
	}

	//----------------------------------------------
	// Private static helper parsing methods
	//----------------------------------------------

	bool LocalIdBuilder::tryParseInternal( const std::string& localIdStr,
		LocalIdParsingErrorBuilder& errorBuilder,
		std::optional<LocalIdBuilder>& localIdBuilder )
	{
		localIdBuilder = std::nullopt;

		if ( localIdStr.empty() )
		{
			return false;
		}

		if ( localIdStr[0] != '/' )
		{
			errorBuilder.addError( LocalIdParsingState::Formatting, "Invalid format: missing '/' as first character" );
			return false;
		}

		std::string_view span = localIdStr;

		std::optional<GmodPath> primaryItem = std::nullopt;
		std::optional<GmodPath> secondaryItem = std::nullopt;
		std::optional<MetadataTag> qty = std::nullopt;
		std::optional<MetadataTag> cnt = std::nullopt;
		std::optional<MetadataTag> calc = std::nullopt;
		std::optional<MetadataTag> stateTag = std::nullopt;
		std::optional<MetadataTag> cmd = std::nullopt;
		std::optional<MetadataTag> type = std::nullopt;
		std::optional<MetadataTag> pos = std::nullopt;
		std::optional<MetadataTag> detail = std::nullopt;
		bool verbose = false;
		std::string predefinedMessage;
		bool invalidSecondaryItem = false;

		size_t primaryItemStart = std::numeric_limits<size_t>::max();
		size_t secondaryItemStart = std::numeric_limits<size_t>::max();

		LocalIdParsingState state = LocalIdParsingState::NamingRule;
		size_t i = 1;

		VIS& vis = VIS::instance();
		auto visVersion = VisVersion::Unknown;
		const Gmod* gmod = nullptr;
		const Codebooks* codebooks = nullptr;

		while ( state <= LocalIdParsingState::MetaDetail )
		{
			size_t nextStart = std::min( span.length(), i );
			size_t nextSlashPos = span.substr( nextStart ).find( '/' );
			std::string_view segment = ( nextSlashPos == std::string_view::npos )
										   ? span.substr( nextStart )
										   : span.substr( nextStart, nextSlashPos );

			switch ( state )
			{
				case LocalIdParsingState::NamingRule:
				{
					if ( segment.empty() )
					{
						errorBuilder.addError( LocalIdParsingState::NamingRule, predefinedMessage );
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( segment != namingRule )
					{
						errorBuilder.addError( LocalIdParsingState::NamingRule, predefinedMessage );
						return false;
					}
					advanceParser( i, segment, state );
					break;
				}

				case LocalIdParsingState::VisVersion:
				{
					if ( segment.empty() )
					{
						errorBuilder.addError( LocalIdParsingState::VisVersion, predefinedMessage );
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !segment.starts_with( "vis-" ) )
					{
						errorBuilder.addError( LocalIdParsingState::VisVersion, predefinedMessage );
						return false;
					}

					std::string versionStr( segment.substr( 4 ) );
					if ( !VisVersionExtensions::tryParse( versionStr, visVersion ) )
					{
						errorBuilder.addError( LocalIdParsingState::VisVersion, predefinedMessage );
						return false;
					}

					gmod = &vis.gmod( visVersion );
					codebooks = &vis.codebooks( visVersion );

					if ( !gmod || !codebooks )
					{
						return false;
					}

					advanceParser( i, segment, state );
					break;
				}

				case LocalIdParsingState::PrimaryItem:
				{
					if ( segment.empty() )
					{
						if ( primaryItemStart != std::numeric_limits<size_t>::max() )
						{
							if ( !gmod )
							{
								return false;
							}

							std::string_view path = span.substr( primaryItemStart, i - 1 - primaryItemStart );
							std::optional<GmodPath> parsedPath;
							if ( !gmod->tryParsePath( std::string( path ), parsedPath ) )
							{
								parsedPath = std::nullopt;
							}
							if ( !parsedPath )
							{
								errorBuilder.addError( LocalIdParsingState::PrimaryItem,
									"Invalid GmodPath in Primary item: " + std::string( path ) );
							}
							else
							{
								primaryItem = std::move( *parsedPath );
							}
						}
						else
						{
							errorBuilder.addError( LocalIdParsingState::PrimaryItem );
						}
						errorBuilder.addError( LocalIdParsingState::PrimaryItem,
							"Invalid or missing '/meta' prefix after Primary item" );
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					size_t dashIndex = segment.find( '-' );
					std::string_view code = ( dashIndex == std::string_view::npos ) ? segment : segment.substr( 0, dashIndex );

					if ( !gmod )
						return false;

					if ( primaryItemStart == std::numeric_limits<size_t>::max() )
					{
						const GmodNode* nodePtr = nullptr;
						if ( !gmod->tryGetNode( std::string( code ), nodePtr ) )
						{
							errorBuilder.addError( LocalIdParsingState::PrimaryItem,
								"Invalid start GmodNode in Primary item: " + std::string( code ) );
						}
						primaryItemStart = i;
						advanceParser( i, segment );
					}
					else
					{
						LocalIdParsingState nextState = state;

						if ( segment.starts_with( "sec" ) )
							nextState = LocalIdParsingState::SecondaryItem;
						else if ( segment.starts_with( "meta" ) )
							nextState = LocalIdParsingState::MetaQuantity;
						else if ( !segment.empty() && segment[0] == '~' )
							nextState = LocalIdParsingState::ItemDescription;

						if ( nextState != state )
						{
							std::string_view path = span.substr( primaryItemStart, i - 1 - primaryItemStart );
							std::optional<GmodPath> parsedPath;
							if ( !gmod->tryParsePath( std::string( path ), parsedPath ) )
							{
								parsedPath = std::nullopt;
							}
							if ( !parsedPath )
							{
								errorBuilder.addError( LocalIdParsingState::PrimaryItem,
									"Invalid GmodPath in Primary item: " + std::string( path ) );

								auto [_, endOfNextStateIndex] = nextStateIndexes( span, state );
								i = endOfNextStateIndex;
								advanceParser( state, nextState );
								break;
							}
							else
							{
								primaryItem = std::move( *parsedPath );
							}

							if ( !segment.empty() && segment[0] == '~' )
								advanceParser( state, nextState );
							else
								advanceParser( i, segment, state, nextState );
							break;
						}

						const GmodNode* nodePtr = nullptr;
						if ( !gmod->tryGetNode( std::string( code ), nodePtr ) )
						{
							errorBuilder.addError( LocalIdParsingState::PrimaryItem,
								"Invalid GmodNode in Primary item: " + std::string( code ) );

							auto [nextStateIndex, endOfNextStateIndex] = nextStateIndexes( span, state );

							if ( nextStateIndex == std::numeric_limits<size_t>::max() )
							{
								errorBuilder.addError( LocalIdParsingState::PrimaryItem,
									"Invalid or missing '/meta' prefix after Primary item" );
								return false;
							}

							std::string_view nextSegment = span.substr( nextStateIndex + 1 );

							if ( nextSegment.starts_with( "sec" ) )
								nextState = LocalIdParsingState::SecondaryItem;
							else if ( nextSegment.starts_with( "meta" ) )
								nextState = LocalIdParsingState::MetaQuantity;
							else if ( !nextSegment.empty() && nextSegment[0] == '~' )
								nextState = LocalIdParsingState::ItemDescription;

							std::string_view invalidPrimaryItemPath = span.substr( i, nextStateIndex - i );
							errorBuilder.addError( LocalIdParsingState::PrimaryItem,
								"Invalid GmodPath: Last part in Primary item: " + std::string( invalidPrimaryItemPath ) );

							i = endOfNextStateIndex;
							advanceParser( state, nextState );
							break;
						}

						advanceParser( i, segment );
					}
					break;
				}

				case LocalIdParsingState::SecondaryItem:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					size_t dashIndex = segment.find( '-' );
					std::string_view code = ( dashIndex == std::string_view::npos ) ? segment : segment.substr( 0, dashIndex );

					if ( !gmod )
						return false;

					if ( secondaryItemStart == std::numeric_limits<size_t>::max() )
					{
						const GmodNode* nodePtr = nullptr;
						if ( !gmod->tryGetNode( std::string( code ), nodePtr ) )
						{
							errorBuilder.addError( LocalIdParsingState::SecondaryItem,
								"Invalid start GmodNode in Secondary item: " + std::string( code ) );
						}

						secondaryItemStart = i;
						advanceParser( i, segment );
					}
					else
					{
						LocalIdParsingState nextState = state;

						if ( segment.starts_with( "meta" ) )
							nextState = LocalIdParsingState::MetaQuantity;
						else if ( !segment.empty() && segment[0] == '~' )
							nextState = LocalIdParsingState::ItemDescription;

						if ( nextState != state )
						{
							std::string_view path = span.substr( secondaryItemStart, i - 1 - secondaryItemStart );
							std::optional<GmodPath> parsedPath;
							if ( !gmod->tryParsePath( std::string( path ), parsedPath ) )
							{
								parsedPath = std::nullopt;
							}
							if ( !parsedPath )
							{
								invalidSecondaryItem = true;
								errorBuilder.addError( LocalIdParsingState::SecondaryItem,
									"Invalid GmodPath in Secondary item: " + std::string( path ) );

								auto [_, endOfNextStateIndex] = nextStateIndexes( span, state );
								i = endOfNextStateIndex;
								advanceParser( state, nextState );
								break;
							}
							else
							{
								secondaryItem = std::move( *parsedPath );
							}

							if ( !segment.empty() && segment[0] == '~' )
								advanceParser( state, nextState );
							else
								advanceParser( i, segment, state, nextState );
							break;
						}

						const GmodNode* nodePtr = nullptr;
						if ( !gmod->tryGetNode( std::string( code ), nodePtr ) )
						{
							invalidSecondaryItem = true;
							errorBuilder.addError( LocalIdParsingState::SecondaryItem,
								"Invalid GmodNode in Secondary item: " + std::string( code ) );

							auto [nextStateIndex, endOfNextStateIndex] = nextStateIndexes( span, state );
							if ( nextStateIndex == std::numeric_limits<size_t>::max() )
							{
								errorBuilder.addError( LocalIdParsingState::SecondaryItem,
									"Invalid or missing '/meta' prefix after Secondary item" );
								return false;
							}

							std::string_view nextSegment = span.substr( nextStateIndex + 1 );

							if ( nextSegment.starts_with( "meta" ) )
								nextState = LocalIdParsingState::MetaQuantity;
							else if ( !nextSegment.empty() && nextSegment[0] == '~' )
								nextState = LocalIdParsingState::ItemDescription;

							std::string_view invalidSecondaryItemPath = span.substr( i, nextStateIndex - i );
							errorBuilder.addError( LocalIdParsingState::SecondaryItem,
								"Invalid GmodPath: Last part in Secondary item: " + std::string( invalidSecondaryItemPath ) );

							i = endOfNextStateIndex;
							advanceParser( state, nextState );
							break;
						}
						advanceParser( i, segment );
					}
					break;
				}

				case LocalIdParsingState::ItemDescription:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					verbose = true;

					size_t metaIndex = span.find( "/meta" );
					if ( metaIndex == std::string_view::npos )
					{
						errorBuilder.addError( LocalIdParsingState::ItemDescription, predefinedMessage );
						return false;
					}

					segment = span.substr( i, ( metaIndex + 5 ) - i ); /* "/meta".length() = 5 */

					advanceParser( i, segment, state );
					break;
				}

				case LocalIdParsingState::MetaQuantity:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					bool result = parseMetaTag( CodebookName::Quantity, state, i, segment, qty, codebooks, errorBuilder );
					if ( !result )
						return false;
					break;
				}

				case LocalIdParsingState::MetaContent:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					bool result = parseMetaTag( CodebookName::Content, state, i, segment, cnt, codebooks, errorBuilder );
					if ( !result )
						return false;
					break;
				}

				case LocalIdParsingState::MetaCalculation:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					bool result = parseMetaTag( CodebookName::Calculation, state, i, segment, calc, codebooks, errorBuilder );
					if ( !result )
						return false;
					break;
				}

				case LocalIdParsingState::MetaState:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					bool result = parseMetaTag( CodebookName::State, state, i, segment, stateTag, codebooks, errorBuilder );
					if ( !result )
						return false;
					break;
				}

				case LocalIdParsingState::MetaCommand:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					bool result = parseMetaTag( CodebookName::Command, state, i, segment, cmd, codebooks, errorBuilder );
					if ( !result )
						return false;
					break;
				}

				case LocalIdParsingState::MetaType:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					bool result = parseMetaTag( CodebookName::Type, state, i, segment, type, codebooks, errorBuilder );
					if ( !result )
						return false;
					break;
				}

				case LocalIdParsingState::MetaPosition:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					bool result = parseMetaTag( CodebookName::Position, state, i, segment, pos, codebooks, errorBuilder );
					if ( !result )
						return false;
					break;
				}

				case LocalIdParsingState::MetaDetail:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					bool result = parseMetaTag( CodebookName::Detail, state, i, segment, detail, codebooks, errorBuilder );
					if ( !result )
						return false;
					break;
				}
				case LocalIdParsingState::EmptyState:
				case LocalIdParsingState::Formatting:
				case LocalIdParsingState::Completeness:
				case LocalIdParsingState::NamingEntity:
				case LocalIdParsingState::IMONumber:
				default:
					advanceParser( i, segment, state );
					break;
			}
		}

		LocalIdBuilder builder = LocalIdBuilder::create( visVersion );

		if ( primaryItem.has_value() )
			builder = builder.tryWithPrimaryItem( primaryItem.value() );

		if ( secondaryItem.has_value() )
			builder = builder.tryWithSecondaryItem( secondaryItem.value() );

		if ( verbose )
			builder = builder.withVerboseMode( verbose );

		if ( qty.has_value() )
			builder = builder.tryWithMetadataTag( qty.value() );

		if ( cnt.has_value() )
			builder = builder.tryWithMetadataTag( cnt.value() );

		if ( calc.has_value() )
			builder = builder.tryWithMetadataTag( calc.value() );

		if ( stateTag.has_value() )
			builder = builder.tryWithMetadataTag( stateTag.value() );

		if ( cmd.has_value() )
			builder = builder.tryWithMetadataTag( cmd.value() );

		if ( type.has_value() )
			builder = builder.tryWithMetadataTag( type.value() );

		if ( pos.has_value() )
			builder = builder.tryWithMetadataTag( pos.value() );

		if ( detail.has_value() )
			builder = builder.tryWithMetadataTag( detail.value() );

		if ( !qty.has_value() && !cnt.has_value() && !calc.has_value() &&
			 !stateTag.has_value() && !cmd.has_value() && !type.has_value() &&
			 !pos.has_value() && !detail.has_value() )
		{
			errorBuilder.addError( LocalIdParsingState::Completeness,
				"No metadata tags specified. Local IDs require atleast 1 metadata tag." );
		}

		localIdBuilder = std::move( builder );
		return ( !errorBuilder.hasError() && !invalidSecondaryItem );
	}

	void LocalIdBuilder::advanceParser( size_t& i, std::string_view segment, LocalIdParsingState& state )
	{
		state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
		i += segment.length() + 1;
	}

	void LocalIdBuilder::advanceParser( size_t& i, std::string_view segment )
	{
		i += segment.length() + 1;
	}

	void LocalIdBuilder::advanceParser( LocalIdParsingState& state, LocalIdParsingState to )
	{
		state = to;
	}

	void LocalIdBuilder::advanceParser( size_t& i, std::string_view segment, LocalIdParsingState& state, LocalIdParsingState to )
	{
		i += segment.length() + 1;
		state = to;
	}

	std::optional<LocalIdParsingState> LocalIdBuilder::metaPrefixToState( std::string_view prefix )
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
		size_t& i, std::string_view segment, std::optional<MetadataTag>& tag,
		const Codebooks* codebooks,
		LocalIdParsingErrorBuilder& errorBuilder )
	{
		if ( !codebooks )
			return false;

		auto dashIndex = segment.find( '-' );
		auto tildeIndex = segment.find( '~' );
		auto prefixIndex = ( dashIndex == std::string_view::npos ) ? tildeIndex : dashIndex;

		if ( prefixIndex == std::string_view::npos )
		{
			errorBuilder.addError( state,
				"Invalid metadata tag: missing prefix '-' or '~' in " + std::string( segment ) );
			advanceParser( i, segment, state );
			return true;
		}

		auto actualPrefix = segment.substr( 0, prefixIndex );

		auto actualState = metaPrefixToState( actualPrefix );
		if ( !actualState.has_value() || actualState.value() < state )
		{
			errorBuilder.addError( state,
				"Invalid metadata tag: unknown prefix " + std::string( actualPrefix ) );
			return false;
		}

		if ( actualState.value() > state )
		{
			advanceParser( state, actualState.value() );
			return true;
		}

		auto nextState = nextParsingState( actualState.value() );

		auto value = segment.substr( prefixIndex + 1 );
		if ( value.empty() )
		{
			auto codebookStr = codebookNametoString( codebookName );
			errorBuilder.addError( state,
				"Invalid " + codebookStr + " metadata tag: missing value" );
			return false;
		}

		tag = codebooks->tryCreateTag( codebookName, std::string( value ) );
		if ( !tag.has_value() )
		{
			auto codebookStr = codebookNametoString( codebookName );

			if ( prefixIndex == tildeIndex )
			{
				errorBuilder.addError( state,
					"Invalid custom " + codebookStr + " metadata tag: failed to create " + std::string( value ) );
			}
			else
			{
				errorBuilder.addError( state,
					"Invalid " + codebookStr + " metadata tag: failed to create " + std::string( value ) );
			}

			advanceParser( i, segment, state );
			return true;
		}

		if ( prefixIndex == dashIndex && tag.value().prefix() == '~' )
		{
			auto codebookStr = codebookNametoString( codebookName );
			errorBuilder.addError( state,
				"Invalid " + codebookStr + " metadata tag: '" + std::string( value ) + "'. Use prefix '~' for custom values" );
		}

		if ( !nextState.has_value() )
			advanceParser( i, segment, state );
		else
			advanceParser( i, segment, state, nextState.value() );

		return true;
	}
}
