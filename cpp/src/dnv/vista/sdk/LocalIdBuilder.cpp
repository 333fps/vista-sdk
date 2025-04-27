#include "pch.h"

#include "dnv/vista/sdk/LocalIdBuilder.h"

#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/CodebookName.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/CodeBooks.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------------

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

	//-------------------------------------------------------------------------
	// Constructors and Static Factories
	//-------------------------------------------------------------------------

	LocalIdBuilder::LocalIdBuilder()
	{
		SPDLOG_INFO( "Created empty LocalIdBuilder" );
	}

	LocalIdBuilder::LocalIdBuilder( const LocalIdBuilder& other )
	{
		m_verboseMode = other.m_verboseMode;
		m_items = other.m_items;

		if ( other.m_visVersion.has_value() )
		{
			m_visVersion.emplace( other.m_visVersion.value() );
		}

		if ( other.m_quantity.has_value() )
		{
			m_quantity.emplace( other.m_quantity.value() );
		}

		if ( other.m_content.has_value() )
		{
			m_content.emplace( other.m_content.value() );
		}

		if ( other.m_calculation.has_value() )
		{
			m_calculation.emplace( other.m_calculation.value() );
		}

		if ( other.m_state.has_value() )
		{
			m_state.emplace( other.m_state.value() );
		}

		if ( other.m_command.has_value() )
		{
			m_command.emplace( other.m_command.value() );
		}

		if ( other.m_type.has_value() )
		{
			m_type.emplace( other.m_type.value() );
		}

		if ( other.m_position.has_value() )
		{
			m_position.emplace( other.m_position.value() );
		}

		if ( other.m_detail.has_value() )
		{
			m_detail.emplace( other.m_detail.value() );
		}

		SPDLOG_INFO( "Created LocalIdBuilder via copy constructor" );
	}

	LocalIdBuilder& LocalIdBuilder::operator=( const LocalIdBuilder& other )
	{
		if ( this != &other )
		{
			m_visVersion = other.m_visVersion;
			m_verboseMode = other.m_verboseMode;
			m_items = other.m_items;
			m_quantity = other.m_quantity;
			m_content = other.m_content;
			m_calculation = other.m_calculation;
			m_state = other.m_state;
			m_command = other.m_command;
			m_type = other.m_type;
			m_position = other.m_position;
			m_detail = other.m_detail;
		}
		return *this;
	}

	LocalIdBuilder LocalIdBuilder::create( VisVersion version )
	{
		SPDLOG_INFO( "Creating LocalIdBuilder with VisVersion: {}", static_cast<int>( version ) );
		LocalIdBuilder builder;
		builder.m_visVersion = version;
		return builder;
	}

	LocalId LocalIdBuilder::build() const
	{
		try
		{
			SPDLOG_INFO( "Building LocalId from builder" );
			return LocalId( *this );
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Failed to build LocalId: {}", ex.what() );
			throw;
		}
	}

	//-------------------------------------------------------------------------
	// State Inspection Methods
	//-------------------------------------------------------------------------

	bool LocalIdBuilder::isValid() const
	{
		if ( !m_visVersion.has_value() )
			return false;

		if ( m_items.primaryItem().length() == 0 )
		{
			return false;
		}

		return !isEmptyMetadata();
	}

	bool LocalIdBuilder::isEmpty() const
	{
		return !m_visVersion.has_value() &&
			   m_items.primaryItem().length() == 0 && //
			   !m_items.secondaryItem().has_value() &&
			   isEmptyMetadata();
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

	bool LocalIdBuilder::hasCustomTag() const
	{
		return ( m_quantity.has_value() && m_quantity->isCustom() ) ||
			   ( m_calculation.has_value() && m_calculation->isCustom() ) ||
			   ( m_content.has_value() && m_content->isCustom() ) ||
			   ( m_position.has_value() && m_position->isCustom() ) ||
			   ( m_state.has_value() && m_state->isCustom() ) ||
			   ( m_command.has_value() && m_command->isCustom() ) ||
			   ( m_type.has_value() && m_type->isCustom() ) ||
			   ( m_detail.has_value() && m_detail->isCustom() );
	}

	//-------------------------------------------------------------------------
	// Core Property Getters
	//-------------------------------------------------------------------------

	std::optional<VisVersion> LocalIdBuilder::visVersion() const
	{
		return m_visVersion;
	}

	bool LocalIdBuilder::isVerboseMode() const
	{
		return m_verboseMode;
	}

	//-------------------------------------------------------------------------
	// Item Getters
	//-------------------------------------------------------------------------

	const LocalIdItems& LocalIdBuilder::items() const
	{
		return m_items;
	}

	const GmodPath& LocalIdBuilder::primaryItem() const
	{
		SPDLOG_INFO( "Getting primary item" );
		return m_items.primaryItem();
	}

	std::optional<GmodPath> LocalIdBuilder::secondaryItem() const
	{
		SPDLOG_INFO( "Getting secondary item" );
		return m_items.secondaryItem();
	}

	//-------------------------------------------------------------------------
	// Metadata Tag Getters
	//-------------------------------------------------------------------------

	const std::vector<MetadataTag> LocalIdBuilder::metadataTags() const
	{
		static thread_local std::vector<MetadataTag> tags;
		tags.clear();

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

	std::optional<MetadataTag> LocalIdBuilder::quantity() const
	{
		return m_quantity;
	}

	std::optional<MetadataTag> LocalIdBuilder::content() const
	{
		return m_content;
	}

	std::optional<MetadataTag> LocalIdBuilder::calculation() const
	{
		return m_calculation;
	}

	std::optional<MetadataTag> LocalIdBuilder::state() const
	{
		return m_state;
	}

	std::optional<MetadataTag> LocalIdBuilder::command() const
	{
		return m_command;
	}

	std::optional<MetadataTag> LocalIdBuilder::type() const
	{
		return m_type;
	}

	std::optional<MetadataTag> LocalIdBuilder::position() const
	{
		return m_position;
	}

	std::optional<MetadataTag> LocalIdBuilder::detail() const
	{
		return m_detail;
	}

	//-------------------------------------------------------------------------
	// Conversion Methods
	//-------------------------------------------------------------------------

	std::string LocalIdBuilder::toString() const
	{
		std::stringstream builder;
		toString( builder );
		return builder.str();
	}

	void LocalIdBuilder::toString( std::stringstream& builder ) const
	{
		SPDLOG_INFO( "Converting LocalIdBuilder to string" );

		if ( !m_visVersion.has_value() )
		{
			SPDLOG_ERROR( "Cannot convert LocalIdBuilder to string: missing VisVersion" );
			return;
		}

		builder << namingRule << "/v" << static_cast<int>( *m_visVersion ) << '/';
		m_items.append( builder, m_verboseMode );

		if ( !isEmptyMetadata() )
		{
			builder << "meta/";

			if ( m_quantity.has_value() )
			{
				builder << "q/";
				builder << m_quantity->value() << '/';
			}

			if ( m_content.has_value() )
			{
				builder << "c/";
				builder << m_content->value() << '/';
			}

			if ( m_calculation.has_value() )
			{
				builder << "cal/";
				builder << m_calculation->value() << '/';
			}

			if ( m_state.has_value() )
			{
				builder << "s/";
				builder << m_state->value() << '/';
			}

			if ( m_command.has_value() )
			{
				builder << "cmd/";
				builder << m_command->value() << '/';
			}

			if ( m_type.has_value() )
			{
				builder << "t/";
				builder << m_type->value() << '/';
			}

			if ( m_position.has_value() )
			{
				builder << "pos/";
				builder << m_position->value() << '/';
			}

			if ( m_detail.has_value() )
			{
				builder << "d/";
				builder << m_detail->value() << '/';
			}
		}
	}

	//-------------------------------------------------------------------------
	// Builder Methods - Core Properties
	//-------------------------------------------------------------------------

	LocalIdBuilder LocalIdBuilder::withVisVersion( const std::string& visVersion )
	{
		try
		{
			SPDLOG_INFO( "Setting VisVersion from string: {}", visVersion );

			VisVersion version = VisVersionExtensions::parse( visVersion );

			return withVisVersion( version );
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Invalid VisVersion string '{}': {}", visVersion, ex.what() );
			throw std::invalid_argument( "Invalid VisVersion: " + visVersion );
		}
	}

	LocalIdBuilder LocalIdBuilder::withVisVersion( VisVersion version )
	{
		SPDLOG_INFO( "Setting VisVersion: {}", static_cast<int>( version ) );
		LocalIdBuilder result( *this );
		result.m_visVersion = version;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::tryWithVisVersion( const std::optional<VisVersion>& version )
	{
		if ( !version.has_value() )
		{
			SPDLOG_INFO( "Skipping VisVersion setting: value is empty" );
			return *this;
		}

		return withVisVersion( *version );
	}

	LocalIdBuilder LocalIdBuilder::tryWithVisVersion( const std::optional<std::string>& visVersionStr, bool& succeeded )
	{
		succeeded = false;

		if ( !visVersionStr.has_value() || visVersionStr->empty() )
		{
			SPDLOG_INFO( "Skipping VisVersion setting: string is empty" );
			return *this;
		}

		try
		{
			auto result = withVisVersion( *visVersionStr );
			succeeded = true;
			return result;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Failed to set VisVersion: {}", ex.what() );
			return *this;
		}
	}

	LocalIdBuilder LocalIdBuilder::withoutVisVersion()
	{
		SPDLOG_INFO( "Removing VisVersion" );
		LocalIdBuilder result( *this );
		result.m_visVersion = std::nullopt;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withVerboseMode( bool verboseMode )
	{
		SPDLOG_INFO( "Setting verbose mode: {}", verboseMode );
		LocalIdBuilder result( *this );
		result.m_verboseMode = verboseMode;
		return result;
	}

	//-------------------------------------------------------------------------
	// Builder Methods - Items
	//-------------------------------------------------------------------------

	LocalIdBuilder LocalIdBuilder::withPrimaryItem( const GmodPath& item )
	{
		SPDLOG_INFO( "Setting primary item: {}", item.toString() );
		LocalIdBuilder result( *this );
		result.m_items = LocalIdItems( item, m_items.secondaryItem() );
		return result;
	}

	LocalIdBuilder LocalIdBuilder::tryWithPrimaryItem( const std::optional<GmodPath>& item )
	{
		if ( !item.has_value() )
		{
			SPDLOG_INFO( "Skipping primary item: value is empty" );
			return *this;
		}

		return withPrimaryItem( *item );
	}

	LocalIdBuilder LocalIdBuilder::tryWithPrimaryItem( const std::optional<GmodPath>& item, bool& succeeded )
	{
		succeeded = false;

		if ( !item.has_value() )
		{
			SPDLOG_INFO( "Skipping primary item: value is empty" );
			return *this;
		}

		try
		{
			auto result = withPrimaryItem( *item );
			succeeded = true;
			return result;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Failed to set primary item: {}", ex.what() );
			return *this;
		}
	}

	LocalIdBuilder LocalIdBuilder::withoutPrimaryItem()
	{
		SPDLOG_INFO( "Removing primary item - primary item will be reset to default" );
		LocalIdBuilder result( *this );

		result.m_items = LocalIdItems();

		return result;
	}

	LocalIdBuilder LocalIdBuilder::withSecondaryItem( const GmodPath& item )
	{
		SPDLOG_INFO( "Setting secondary item: {}", item.toString() );
		LocalIdBuilder result( *this );
		result.m_items = LocalIdItems( m_items.primaryItem(), item );
		return result;
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( const std::optional<GmodPath>& item )
	{
		if ( !item.has_value() )
		{
			SPDLOG_INFO( "Skipping secondary item: value is empty" );
			return *this;
		}

		return withSecondaryItem( *item );
	}

	LocalIdBuilder LocalIdBuilder::tryWithSecondaryItem( const std::optional<GmodPath>& item, bool& succeeded )
	{
		succeeded = false;

		if ( !item.has_value() )
		{
			SPDLOG_INFO( "Skipping secondary item: value is empty" );
			return *this;
		}

		try
		{
			auto result = withSecondaryItem( *item );
			succeeded = true;
			return result;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Failed to set secondary item: {}", ex.what() );
			return *this;
		}
	}

	LocalIdBuilder LocalIdBuilder::withoutSecondaryItem()
	{
		SPDLOG_INFO( "Removing secondary item" );
		LocalIdBuilder result( *this );
		result.m_items = LocalIdItems( m_items.primaryItem(), std::nullopt );
		return result;
	}

	//-------------------------------------------------------------------------
	// Builder Methods - Metadata Tags
	//-------------------------------------------------------------------------

	LocalIdBuilder LocalIdBuilder::withMetadataTag( const MetadataTag& metadataTag )
	{
		SPDLOG_INFO( "Adding metadata tag: {} ({})",
			static_cast<int>( metadataTag.name() ),
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
				SPDLOG_ERROR( "FunctionalServices is not supported as a direct metadata tag" );
				throw std::invalid_argument( "FunctionalServices is not supported as a metadata tag" );
			case CodebookName::MaintenanceCategory:
				SPDLOG_ERROR( "MaintenanceCategory is not supported as a direct metadata tag" );
				throw std::invalid_argument( "MaintenanceCategory is not supported as a metadata tag" );
			case CodebookName::ActivityType:
				SPDLOG_ERROR( "ActivityType is not supported as a direct metadata tag" );
				throw std::invalid_argument( "ActivityType is not supported as a metadata tag" );
			default:
				SPDLOG_ERROR( "Unsupported codebook for metadata tag: {}", static_cast<int>( metadataTag.name() ) );
				throw std::invalid_argument( "Unsupported codebook for metadata tag" );
		}
	}

	LocalIdBuilder LocalIdBuilder::tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag )
	{
		if ( !metadataTag.has_value() )
		{
			SPDLOG_INFO( "Skipping metadata tag: value is empty" );
			return *this;
		}

		return withMetadataTag( *metadataTag );
	}

	LocalIdBuilder LocalIdBuilder::tryWithMetadataTag( const std::optional<MetadataTag>& metadataTag, bool& succeeded )
	{
		succeeded = false;

		if ( !metadataTag.has_value() )
		{
			SPDLOG_INFO( "Skipping metadata tag: value is empty" );
			return *this;
		}

		try
		{
			auto result = withMetadataTag( *metadataTag );
			succeeded = true;
			return result;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Failed to set metadata tag: {}", ex.what() );
			return *this;
		}
	}

	LocalIdBuilder LocalIdBuilder::withoutMetadataTag( CodebookName name )
	{
		SPDLOG_INFO( "Removing metadata tag: {}", static_cast<int>( name ) );

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
				SPDLOG_ERROR( "Cannot remove {} metadata tag - not supported as direct metadata", static_cast<int>( name ) );
				throw std::invalid_argument( "Cannot remove unsupported metadata tag type" );
			default:
				SPDLOG_ERROR( "Unsupported codebook for metadata tag removal: {}", static_cast<int>( name ) );
				throw std::invalid_argument( "Unsupported codebook for metadata tag removal" );
		}
	}

	LocalIdBuilder LocalIdBuilder::withQuantity( const MetadataTag& quantity )
	{
		SPDLOG_INFO( "Setting quantity tag: {}", quantity.value() );
		LocalIdBuilder result( *this );
		result.m_quantity = quantity;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withContent( const MetadataTag& content )
	{
		SPDLOG_INFO( "Setting content tag: {}", content.value() );
		LocalIdBuilder result( *this );
		result.m_content = content;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withCalculation( const MetadataTag& calculation )
	{
		SPDLOG_INFO( "Setting calculation tag: {}", calculation.value() );
		LocalIdBuilder result( *this );
		result.m_calculation = calculation;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withState( const MetadataTag& state )
	{
		SPDLOG_INFO( "Setting state tag: {}", state.value() );
		LocalIdBuilder result( *this );
		result.m_state = state;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withCommand( const MetadataTag& command )
	{
		SPDLOG_INFO( "Setting command tag: {}", command.value() );
		LocalIdBuilder result( *this );
		result.m_command = command;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withType( const MetadataTag& type )
	{
		SPDLOG_INFO( "Setting type tag: {}", type.value() );
		LocalIdBuilder result( *this );
		result.m_type = type;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withPosition( const MetadataTag& position )
	{
		SPDLOG_INFO( "Setting position tag: {}", position.value() );
		LocalIdBuilder result( *this );
		result.m_position = position;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withDetail( const MetadataTag& detail )
	{
		SPDLOG_INFO( "Setting detail tag: {}", detail.value() );
		LocalIdBuilder result( *this );
		result.m_detail = detail;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutQuantity()
	{
		SPDLOG_INFO( "Removing quantity tag" );
		LocalIdBuilder result( *this );
		result.m_quantity = std::nullopt;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutContent()
	{
		SPDLOG_INFO( "Removing content tag" );
		LocalIdBuilder result( *this );
		result.m_content = std::nullopt;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutCalculation()
	{
		SPDLOG_INFO( "Removing calculation tag" );
		LocalIdBuilder result( *this );
		result.m_calculation = std::nullopt;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutState()
	{
		SPDLOG_INFO( "Removing state tag" );
		LocalIdBuilder result( *this );
		result.m_state = std::nullopt;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutCommand()
	{
		SPDLOG_INFO( "Removing command tag" );
		LocalIdBuilder result( *this );
		result.m_command = std::nullopt;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutType()
	{
		SPDLOG_INFO( "Removing type tag" );
		LocalIdBuilder result( *this );
		result.m_type = std::nullopt;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutPosition()
	{
		SPDLOG_INFO( "Removing position tag" );
		LocalIdBuilder result( *this );
		result.m_position = std::nullopt;
		return result;
	}

	LocalIdBuilder LocalIdBuilder::withoutDetail()
	{
		SPDLOG_INFO( "Removing detail tag" );
		LocalIdBuilder result( *this );
		result.m_detail = std::nullopt;
		return result;
	}

	//-------------------------------------------------------------------------
	// Static Parsing Methods
	//-------------------------------------------------------------------------

	LocalIdBuilder LocalIdBuilder::parse( const std::string& localIdStr )
	{
		SPDLOG_INFO( "Parsing LocalId from string: {}", localIdStr );

		ParsingErrors errors;
		std::optional<LocalIdBuilder> result;
		if ( !tryParse( localIdStr, errors, result ) || !result.has_value() )
		{
			SPDLOG_ERROR( "Failed to parse LocalId: {}", localIdStr );
			std::string errorMsg = "Failed to parse LocalId: " + localIdStr;
			if ( !errors.isEmpty() )
			{
				errorMsg += "\nErrors: ";
				for ( const auto& err : errors )
				{
					errorMsg += "\n  - " + std::get<0>( err ) + ": " + std::get<1>( err );
				}
			}
			throw std::invalid_argument( errorMsg );
		}

		return *result;
	}

	bool LocalIdBuilder::tryParse( const std::string& localIdStr, ParsingErrors& errors, std::optional<LocalIdBuilder>& localId )
	{
		SPDLOG_INFO( "Attempting to parse LocalId: {}", localIdStr );

		LocalIdParsingErrorBuilder errorBuilder = LocalIdParsingErrorBuilder::create();

		bool result = tryParseInternal( localIdStr, errorBuilder, localId );

		if ( errorBuilder.hasError() )
		{
			SPDLOG_ERROR( "Parsing encountered errors" );
			errors = errorBuilder.build();
		}

		return result && localId.has_value();
	}

	bool LocalIdBuilder::tryParseInternal( const std::string& localIdStr,
		LocalIdParsingErrorBuilder& errorBuilder,
		std::optional<LocalIdBuilder>& localIdBuilder )
	{
		SPDLOG_INFO( "Parsing LocalId: {}", localIdStr );
		localIdBuilder = std::nullopt;

		if ( localIdStr.empty() )
		{
			SPDLOG_INFO( "LocalId string is empty" );
			errorBuilder.addError( LocalIdParsingState::EmptyState );
			return false;
		}

		if ( localIdStr[0] != '/' )
		{
			SPDLOG_WARN( "Invalid format: missing '/' as first character" );
			errorBuilder.addError( LocalIdParsingState::Formatting,
				"Invalid format: missing '/' as first character" );
			return false;
		}

		std::string_view localIdView{ localIdStr };

		std::optional<VisVersion> visVersion{};
		std::shared_ptr<Gmod> gmod{ nullptr };
		std::shared_ptr<Codebooks> codebooks{ nullptr };

		std::optional<GmodPath> primaryItem{};
		std::optional<GmodPath> secondaryItem{};

		std::optional<MetadataTag> qty{};
		std::optional<MetadataTag> cnt{};
		std::optional<MetadataTag> calc{};
		std::optional<MetadataTag> stateTag{};
		std::optional<MetadataTag> cmd{};
		std::optional<MetadataTag> type{};
		std::optional<MetadataTag> pos{};
		std::optional<MetadataTag> detail{};

		bool verbose{ false };
		bool invalidSecondaryItem{ false };
		const char* predefinedMessage{ nullptr };

		size_t primaryItemStart{ static_cast<size_t>( -1 ) };
		size_t secondaryItemStart{ static_cast<size_t>( -1 ) };

		LocalIdParsingState state{ LocalIdParsingState::NamingRule };
		size_t i{ 1 };

		while ( state <= LocalIdParsingState::MetaDetail )
		{
			size_t nextStart{ std::min( localIdView.size(), i ) };
			size_t nextSlash{ localIdView.find( '/', nextStart ) };
			std::string_view segment;

			if ( nextSlash == std::string_view::npos )
				segment = localIdView.substr( nextStart );
			else
				segment = localIdView.substr( nextStart, nextSlash - nextStart );

			SPDLOG_INFO( "Processing segment: '{}', state: {}", segment, static_cast<int>( state ) );

			switch ( state )
			{
				case LocalIdParsingState::NamingRule:
					if ( segment.empty() )
					{
						SPDLOG_ERROR( "Invalid naming rule format: {}", segment );
						addError( errorBuilder, state, predefinedMessage );
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( segment != namingRule )
					{
						SPDLOG_WARN( "Invalid naming rule: {}", segment );
						addError( errorBuilder, state, predefinedMessage );
						return false;
					}

					advanceParser( i, segment, state );
					break;

				case LocalIdParsingState::VisVersion:
					if ( segment.empty() )
					{
						SPDLOG_ERROR( "Invalid VisVersion format: {}", segment );
						addError( errorBuilder, state, predefinedMessage );
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !segment.starts_with( "vis-" ) )
					{
						SPDLOG_WARN( "Invalid VisVersion format: {}", segment );
						addError( errorBuilder, state, predefinedMessage );
						return false;
					}

					try
					{
						std::string_view versionPart{ segment.substr( 4 ) }; // TODO Extract version part after "vis-"
						visVersion = VisVersionExtensions::parse( std::string( versionPart ) );
					}
					catch ( const std::exception& e )
					{
						SPDLOG_WARN( "Failed to parse VisVersion: {}", e.what() );
						addError( errorBuilder, state, predefinedMessage );
						return false;
					}

					gmod = std::make_shared<Gmod>( VIS::instance().gmod( *visVersion ) );
					codebooks = std::make_shared<Codebooks>( VIS::instance().codebooks( *visVersion ) );

					if ( !gmod || !codebooks )
					{
						SPDLOG_ERROR( "Failed to get Gmod or Codebooks for version {}",
							static_cast<int>( *visVersion ) );
						return false;
					}

					advanceParser( i, segment, state );
					break;

				case LocalIdParsingState::PrimaryItem:
				{
					if ( segment.empty() )
					{
						if ( primaryItemStart != static_cast<size_t>( -1 ) )
						{
							if ( !gmod )
								return false;

							std::string_view pathView{ localIdView.substr( primaryItemStart,
								i - 1 - primaryItemStart ) };
							std::string path{ pathView };
							if ( !gmod->tryParsePath( path, primaryItem ) )
							{
								SPDLOG_WARN( "Invalid GmodPath in Primary item: {}", path );
								addError( errorBuilder, state, "Invalid GmodPath in Primary item: " + path );
							}
						}
						else
						{
							SPDLOG_ERROR( "Invalid or missing '/meta' prefix after Primary item" );
							addError( errorBuilder, state, predefinedMessage );
						}

						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					size_t dashIndex{ segment.find( '-' ) };
					std::string_view codeView{ ( dashIndex == std::string_view::npos ) ? segment : segment.substr( 0, dashIndex ) };
					std::string code{ codeView };

					if ( !gmod )
					{
						SPDLOG_ERROR( "Gmod is not initialized" );
						return false;
					}

					if ( primaryItemStart == static_cast<size_t>( -1 ) )
					{
						GmodNode tempNode{};
						if ( !gmod->tryGetNode( code, tempNode ) )
						{
							SPDLOG_WARN( "Invalid start GmodNode in Primary item: {}", code );
							addError( errorBuilder, state,
								"Invalid start GmodNode in Primary item: " + code );
						}

						primaryItemStart = i;
						advanceParser( i, segment );
					}
					else
					{
						bool isSecPrefix{ segment == "sec" };
						bool isMetaPrefix{ segment == "meta" };
						bool isTildePrefix{ !segment.empty() && segment[0] == '~' };

						LocalIdParsingState nextState{ state };

						if ( isSecPrefix )
							nextState = LocalIdParsingState::SecondaryItem;
						else if ( isMetaPrefix )
							nextState = LocalIdParsingState::MetaQuantity;
						else if ( isTildePrefix )
							nextState = LocalIdParsingState::ItemDescription;

						if ( nextState != state )
						{
							std::string_view pathView{ localIdView.substr( primaryItemStart,
								i - 1 - primaryItemStart ) };
							std::string path{ pathView };
							if ( !gmod->tryParsePath( path, primaryItem ) )
							{
								SPDLOG_WARN( "Invalid GmodPath in Primary item: {}", path );
								addError( errorBuilder, state, "Invalid GmodPath in Primary item: " + path );

								auto [_, endOfNextStateIndex] = nextStateIndexes( localIdStr, state );
								i = endOfNextStateIndex;
								advanceParser( state, nextState );
								break;
							}

							if ( isTildePrefix )
								advanceParser( state, nextState );
							else
								advanceParser( i, segment, state, nextState );
							break;
						}

						GmodNode tempNode2{};
						if ( !gmod->tryGetNode( code, tempNode2 ) )
						{
							SPDLOG_WARN( "Invalid GmodNode in Primary item: {}", code );
							addError( errorBuilder, state, "Invalid GmodNode in Primary item: " + code );

							auto [nextStateIndex, endOfNextStateIndex] = nextStateIndexes( localIdStr, state );

							if ( nextStateIndex == static_cast<size_t>( -1 ) )
							{
								SPDLOG_ERROR( "Invalid or missing '/meta' prefix after Primary item" );
								addError( errorBuilder, state,
									"Invalid or missing '/meta' prefix after Primary item" );
								return false;
							}

							std::string_view nextSegmentView{ localIdView.substr( nextStateIndex + 1 ) };
							std::string nextSegment{};

							if ( !nextSegmentView.empty() )
							{
								size_t nextSegmentSlash{ nextSegmentView.find( '/' ) };
								if ( nextSegmentSlash != std::string_view::npos )
									nextSegment = std::string( nextSegmentView.substr( 0, nextSegmentSlash ) );
								else
									nextSegment = std::string( nextSegmentView );
							}

							bool nextIsSecPrefix{ nextSegment.length() >= 3 &&
												  nextSegment.substr( 0, 3 ) == "sec" };
							bool nextIsMetaPrefix{ nextSegment.length() >= 4 &&
												   nextSegment.substr( 0, 4 ) == "meta" };
							bool nextIsTildePrefix{ !nextSegment.empty() && nextSegment[0] == '~' };

							if ( nextIsSecPrefix )
								nextState = LocalIdParsingState::SecondaryItem;
							else if ( nextIsMetaPrefix )
								nextState = LocalIdParsingState::MetaQuantity;
							else if ( nextIsTildePrefix )
								nextState = LocalIdParsingState::ItemDescription;

							std::string_view invalidPathView{ localIdView.substr( i, nextStateIndex - i ) };
							std::string invalidPrimaryItemPath{ invalidPathView };

							addError( errorBuilder, state,
								"Invalid GmodPath: Last part in Primary item: " + invalidPrimaryItemPath );

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

					if ( secondaryItemStart == static_cast<size_t>( -1 ) )
					{
						GmodNode tempNode;
						if ( !gmod->tryGetNode( std::string( code ), tempNode ) )
						{
							SPDLOG_WARN( "Invalid start GmodNode in Secondary item: {}", code );
							addError( errorBuilder, state,
								"Invalid start GmodNode in Secondary item: " + std::string( code ) );
						}

						secondaryItemStart = i;
						advanceParser( i, segment );
					}
					else
					{
						bool isMetaPrefix = segment == "meta";
						bool isTildePrefix = !segment.empty() && segment[0] == '~';

						LocalIdParsingState nextState = state;

						if ( isMetaPrefix )
							nextState = LocalIdParsingState::MetaQuantity;
						else if ( isTildePrefix )
							nextState = LocalIdParsingState::ItemDescription;

						if ( nextState != state )
						{
							std::string_view pathView = localIdView.substr( secondaryItemStart,
								i - 1 - secondaryItemStart );
							std::string path( pathView );

							if ( !gmod->tryParsePath( path, secondaryItem ) )
							{
								SPDLOG_WARN( "Invalid GmodPath in Secondary item: {}", path );
								addError( errorBuilder, state,
									"Invalid GmodPath in Secondary item: " + path );
								invalidSecondaryItem = true;

								auto [_, endOfNextStateIndex] = nextStateIndexes( localIdStr, state );
								i = endOfNextStateIndex;
								advanceParser( state, nextState );
								break;
							}

							if ( isTildePrefix )
								advanceParser( state, nextState );
							else
								advanceParser( i, segment, state, nextState );
							break;
						}

						GmodNode tempNode;
						if ( !gmod->tryGetNode( std::string( code ), tempNode ) )
						{
							SPDLOG_WARN( "Invalid GmodNode in Secondary item: {}", code );
							addError( errorBuilder, state, "Invalid GmodNode in Secondary item: " + std::string( code ) );

							auto [nextStateIndex, endOfNextStateIndex] = nextStateIndexes( localIdStr, state );

							if ( nextStateIndex == static_cast<size_t>( -1 ) )
							{
								SPDLOG_ERROR( "Invalid or missing '/meta' prefix after Secondary item" );
								addError( errorBuilder, state,
									"Invalid or missing '/meta' prefix after Secondary item" );
								return false;
							}

							std::string_view nextSegmentView{ localIdView.substr( nextStateIndex + 1 ) };
							std::string nextSegment{};

							if ( !nextSegmentView.empty() )
							{
								size_t nextSegmentSlash{ nextSegmentView.find( '/' ) };
								if ( nextSegmentSlash != std::string_view::npos )
									nextSegment = std::string( nextSegmentView.substr( 0, nextSegmentSlash ) );
								else
									nextSegment = std::string( nextSegmentView );
							}

							bool nextIsMetaPrefix{ nextSegment.length() >= 4 &&
												   nextSegment.substr( 0, 4 ) == "meta" };
							bool nextIsTildePrefix{ !nextSegment.empty() && nextSegment[0] == '~' };

							if ( nextIsMetaPrefix )
								nextState = LocalIdParsingState::MetaQuantity;
							else if ( nextIsTildePrefix )
								nextState = LocalIdParsingState::ItemDescription;

							std::string_view invalidPathView{ localIdView.substr( i, nextStateIndex - i ) };
							std::string invalidSecondaryItemPath{ invalidPathView };

							addError( errorBuilder, state, "Invalid GmodPath: Last part in Secondary item: " + invalidSecondaryItemPath );

							invalidSecondaryItem = true;
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

					size_t metaIndex = localIdView.find( "/meta", i - 1 );
					if ( metaIndex == std::string_view::npos )
					{
						SPDLOG_ERROR( "Missing required '/meta' marker after item description" );
						addError( errorBuilder, state, predefinedMessage ? predefinedMessage : "Invalid item description: missing required '/meta' marker" );
						return false;
					}

					segment = localIdView.substr( i - 1, ( metaIndex + 5 ) - ( i - 1 ) );

					i += segment.length();
					state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
					break;
				}
				case LocalIdParsingState::MetaQuantity:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !parseMetaTag( CodebookName::Quantity, state, i, segment, qty, codebooks, errorBuilder ) )
					{
						return false;
					}
					break;
				}

				case LocalIdParsingState::MetaContent:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !parseMetaTag( CodebookName::Content, state, i, segment, cnt, codebooks, errorBuilder ) )
					{
						return false;
					}
					break;
				}
				case LocalIdParsingState::MetaCalculation:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !parseMetaTag( CodebookName::Calculation, state, i, segment, calc, codebooks, errorBuilder ) )
					{
						return false;
					}
					break;
				}

				case LocalIdParsingState::MetaState:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !parseMetaTag( CodebookName::State, state, i, segment, stateTag, codebooks, errorBuilder ) )
					{
						return false;
					}
					break;
				}

				case LocalIdParsingState::MetaCommand:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !parseMetaTag( CodebookName::Command, state, i, segment, cmd, codebooks, errorBuilder ) )
					{
						return false;
					}
					break;
				}

				case LocalIdParsingState::MetaType:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !parseMetaTag( CodebookName::Type, state, i, segment, type, codebooks, errorBuilder ) )
					{
						return false;
					}
					break;
				}

				case LocalIdParsingState::MetaPosition:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !parseMetaTag( CodebookName::Position, state, i, segment, pos, codebooks, errorBuilder ) )
					{
						return false;
					}
					break;
				}

				case LocalIdParsingState::MetaDetail:
				{
					if ( segment.empty() )
					{
						state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
						break;
					}

					if ( !parseMetaTag( CodebookName::Detail, state, i, segment, detail, codebooks, errorBuilder ) )
					{
						return false;
					}
					break;
				}
				case LocalIdParsingState::EmptyState:
				case LocalIdParsingState::Formatting:
				case LocalIdParsingState::Completeness:
				case LocalIdParsingState::NamingEntity:
				case LocalIdParsingState::IMONumber:
					SPDLOG_WARN( "Unexpected state in parsing: {}", static_cast<int>( state ) );
					advanceParser( state, static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 ) );
					break;

				default:
					SPDLOG_WARN( "Unknown parsing state: {}", static_cast<int>( state ) );
					addError( errorBuilder, LocalIdParsingState::Formatting );
					return false;
			}
		}

		localIdBuilder = create( visVersion.value_or( VisVersion::Unknown ) )
							 .tryWithPrimaryItem( primaryItem )
							 .tryWithSecondaryItem( secondaryItem )
							 .withVerboseMode( verbose )
							 .tryWithMetadataTag( qty )
							 .tryWithMetadataTag( cnt )
							 .tryWithMetadataTag( calc )
							 .tryWithMetadataTag( stateTag )
							 .tryWithMetadataTag( cmd )
							 .tryWithMetadataTag( type )
							 .tryWithMetadataTag( pos )
							 .tryWithMetadataTag( detail );

		if ( localIdBuilder->isEmptyMetadata() )
		{
			addError( errorBuilder, LocalIdParsingState::Completeness,
				"No metadata tags specified. Local IDs require at least 1 metadata tag." );
		}

		return ( !errorBuilder.hasError() && !invalidSecondaryItem );
	}

	//-------------------------------------------------------------------------
	// Operators and Utilities
	//-------------------------------------------------------------------------

	bool LocalIdBuilder::operator==( const LocalIdBuilder& other ) const
	{
		SPDLOG_INFO( "Comparing LocalIdBuilders for equality" );

		if ( !m_visVersion.has_value() && !other.m_visVersion.has_value() )
			return true;

		if ( m_visVersion.has_value() != other.m_visVersion.has_value() )
			return false;

		if ( *m_visVersion != *other.m_visVersion )
			throw std::invalid_argument( "Can't compare local IDs from different VIS versions" );

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

	bool LocalIdBuilder::operator!=( const LocalIdBuilder& other ) const
	{
		SPDLOG_INFO( "Comparing LocalIdBuilders for inequality" );
		return !( *this == other );
	}

	size_t LocalIdBuilder::hashCode() const
	{
		SPDLOG_INFO( "Calculating hash code for LocalIdBuilder" );

		size_t hash = 17;

		if ( m_items.primaryItem().length() > 0 )
			hash = hash * 31 + m_items.primaryItem().hashCode();

		if ( m_items.secondaryItem().has_value() )
			hash = hash * 31 + m_items.secondaryItem()->hashCode();

		if ( m_quantity.has_value() )
			hash = hash * 31 + m_quantity->hashCode();

		if ( m_calculation.has_value() )
			hash = hash * 31 + m_calculation->hashCode();

		if ( m_content.has_value() )
			hash = hash * 31 + m_content->hashCode();

		if ( m_position.has_value() )
			hash = hash * 31 + m_position->hashCode();

		if ( m_state.has_value() )
			hash = hash * 31 + m_state->hashCode();

		if ( m_command.has_value() )
			hash = hash * 31 + m_command->hashCode();

		if ( m_type.has_value() )
			hash = hash * 31 + m_type->hashCode();

		if ( m_detail.has_value() )
			hash = hash * 31 + m_detail->hashCode();

		return hash;
	}

	//-------------------------------------------------------------------------
	// Private Helper Methods
	//-------------------------------------------------------------------------

	void LocalIdBuilder::addError( LocalIdParsingErrorBuilder& errorBuilder, LocalIdParsingState state, const std::string& message )
	{
		errorBuilder.addError( state, message );
	}

	void LocalIdBuilder::advanceParser( size_t& i, const std::string& segment, LocalIdParsingState& state )
	{
		state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
		i += segment.length() + 1;
	}

	void LocalIdBuilder::advanceParser( size_t& i, const std::string& segment )
	{
		i += segment.length() + 1;
	}

	void LocalIdBuilder::advanceParser( LocalIdParsingState& state, LocalIdParsingState to )
	{
		state = to;
	}

	void LocalIdBuilder::advanceParser( size_t& i, const std::string& segment, LocalIdParsingState& state, LocalIdParsingState to )
	{
		i += segment.length() + 1;
		state = to;
	}

	void LocalIdBuilder::advanceParser( size_t& i, const std::string_view& segment, LocalIdParsingState& state )
	{
		state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
		i += segment.length() + 1;
	}

	void LocalIdBuilder::advanceParser( size_t& i, const std::string_view& segment )
	{
		i += segment.length() + 1;
	}

	void LocalIdBuilder::advanceParser( size_t& i, const std::string_view& segment, LocalIdParsingState& state, LocalIdParsingState to )
	{
		i += segment.length() + 1;
		state = to;
	}

	std::pair<size_t, size_t> LocalIdBuilder::nextStateIndexes( const std::string& str, LocalIdParsingState state )
	{
		auto customIndex{ size_t{ str.find( "~" ) } };
		auto endOfCustomIndex{ size_t{ customIndex + 1 + 1 } };
		auto metaIndex{ size_t{ str.find( "/meta" ) } };
		auto endOfMetaIndex{ size_t{ metaIndex + 5 + 1 } };
		auto isVerbose{ bool{ customIndex < metaIndex } };

		switch ( state )
		{
			case LocalIdParsingState::PrimaryItem:
			{
				auto secIndex{ size_t{ str.find( "/sec" ) } };
				auto endOfSecIndex{ size_t{ secIndex + 4 + 1 } };

				if ( secIndex != std::string::npos )
				{
					return { secIndex, endOfSecIndex };
				}
				if ( isVerbose && customIndex != std::string::npos )
				{
					return { customIndex, endOfCustomIndex };
				}

				return { metaIndex, endOfMetaIndex };
			}
			case LocalIdParsingState::SecondaryItem:
			{
				if ( isVerbose && customIndex != std::string::npos )
				{
					return { customIndex, endOfCustomIndex };
				}
				return { metaIndex, endOfMetaIndex };
			}
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
			case LocalIdParsingState::NamingRule:
			case LocalIdParsingState::VisVersion:
			case LocalIdParsingState::ItemDescription:
			default:
				return { metaIndex, endOfMetaIndex };
		}
	}

	std::optional<LocalIdParsingState> LocalIdBuilder::metaPrefixToState( const std::string_view& prefix )
	{
		SPDLOG_INFO( "Converting meta prefix to state: {}", prefix );

		if ( prefix == "qty" || prefix == "q" )
			return LocalIdParsingState::MetaQuantity;
		if ( prefix == "cnt" || prefix == "c" )
			return LocalIdParsingState::MetaContent;
		if ( prefix == "calc" || prefix == "cal" )
			return LocalIdParsingState::MetaCalculation;
		if ( prefix == "state" || prefix == "s" )
			return LocalIdParsingState::MetaState;
		if ( prefix == "cmd" )
			return LocalIdParsingState::MetaCommand;
		if ( prefix == "type" || prefix == "t" )
			return LocalIdParsingState::MetaType;
		if ( prefix == "pos" )
			return LocalIdParsingState::MetaPosition;
		if ( prefix == "detail" || prefix == "d" )
			return LocalIdParsingState::MetaDetail;

		return std::nullopt;
	}

	std::optional<LocalIdParsingState> LocalIdBuilder::nextParsingState( LocalIdParsingState prev )
	{
		SPDLOG_INFO( "Getting next parsing state from: {}", static_cast<int>( prev ) );

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
				return LocalIdParsingState::NamingEntity;
			case LocalIdParsingState::NamingEntity:
				return LocalIdParsingState::IMONumber;
			case LocalIdParsingState::IMONumber:
			case LocalIdParsingState::EmptyState:
			case LocalIdParsingState::Formatting:
			case LocalIdParsingState::Completeness:
			case LocalIdParsingState::NamingRule:
			case LocalIdParsingState::VisVersion:
			case LocalIdParsingState::PrimaryItem:
			case LocalIdParsingState::SecondaryItem:
			case LocalIdParsingState::ItemDescription:
				return std::nullopt;

			default:
				return std::nullopt;
		}
	}

	bool LocalIdBuilder::parseMetaTag( CodebookName codebookName, LocalIdParsingState& state,
		size_t& i, const std::string_view& segment, std::optional<MetadataTag>& tag,
		const std::shared_ptr<Codebooks>& codebooks, LocalIdParsingErrorBuilder& errorBuilder )
	{
		if ( !codebooks )
			return false;

		auto dashIndex{ size_t{ segment.find( '-' ) } };
		auto tildeIndex{ size_t{ segment.find( '~' ) } };
		auto prefixIndex{ size_t{ dashIndex == std::string::npos ? tildeIndex : dashIndex } };

		if ( prefixIndex == std::string::npos )
		{
			SPDLOG_WARN( "Invalid metadata tag: missing prefix" );
			addError( errorBuilder, state, "Invalid metadata tag: missing prefix '-' or '~' in " + std::string( segment ) );
			advanceParser( i, segment, state );
			return true;
		}

		std::string_view actualPrefix = segment.substr( 0, prefixIndex );

		std::optional<LocalIdParsingState> actualState = metaPrefixToState( actualPrefix );
		if ( !actualState.has_value() || actualState.value() < state )
		{
			SPDLOG_WARN( "Invalid metadata tag prefix: {}", actualPrefix );
			addError( errorBuilder, state, "Invalid metadata tag: unknown prefix " + std::string( actualPrefix ) );
			return false;
		}

		if ( actualState > state )
		{
			SPDLOG_INFO( "State transition: {} -> {}",
				static_cast<int>( state ),
				static_cast<int>( actualState.value() ) );

			advanceParser( state, *actualState );
			return true;
		}

		auto nextState = nextParsingState( actualState.value() );

		std::string_view value = segment.substr( prefixIndex + 1 );
		if ( value.empty() )
		{
			SPDLOG_WARN( "Invalid {} metadata tag: missing value", CodebookNames::toPrefix( codebookName ) );
			addError( errorBuilder, state,
				"Invalid " + CodebookNames::toPrefix( codebookName ) + " metadata tag: missing value" );
			return false;
		}

		tag = codebooks->tryCreateTag( codebookName, std::string( value ) );
		if ( !tag.has_value() )
		{
			if ( prefixIndex == tildeIndex )
			{
				SPDLOG_WARN( "Invalid custom {} metadata tag: {}", CodebookNames::toPrefix( codebookName ), value );
				addError( errorBuilder, state, "Invalid custom " + CodebookNames::toPrefix( codebookName ) + " metadata tag: failed to create " + std::string( value ) );
			}
			else
			{
				SPDLOG_WARN( "Invalid {} metadata tag: {}", CodebookNames::toPrefix( codebookName ), value );
				addError( errorBuilder, state, "Invalid " + CodebookNames::toPrefix( codebookName ) + " metadata tag: failed to create " + std::string( value ) );
			}

			advanceParser( i, segment, state );
			return true;
		}

		if ( prefixIndex == dashIndex && tag->prefix() == '~' )
		{
			SPDLOG_WARN( "Invalid prefix for custom value: {}", value );
			addError( errorBuilder, state, "Invalid " + CodebookNames::toPrefix( codebookName ) + " metadata tag: '" + std::string( value ) + "'. Use prefix '~' for custom values" );

			return false;
		}

		SPDLOG_INFO( "Validate tag: {}", tag->value() );

		if ( !nextState.has_value() )
		{
			advanceParser( i, segment, state );
		}
		else
		{
			advanceParser( i, segment, state, nextState.value() );
		}

		SPDLOG_INFO( "Parsed metadata tag: {} - {}", CodebookNames::toPrefix( codebookName ), tag->value() );

		return true;
	}
}
