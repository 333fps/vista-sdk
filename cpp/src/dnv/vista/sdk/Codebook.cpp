/**
 * @file Codebook.cpp
 * @brief Implementation of codebook-related components
 */

#include "pch.h"

#include "dnv/vista/sdk/Codebook.h"

#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------

	namespace
	{
		constexpr const char* NUMBER_GROUP = "<number>";
		constexpr const char* DEFAULT_GROUP_NAME = "DEFAULT_GROUP";
		constexpr const char* UNKNOWN_GROUP = "UNKNOWN";

		constexpr const char* WHITESPACE = " \t\n\r\f\v";
	}

	//-------------------------------------------------------------------
	// PositionValidationResults Implementation
	//-------------------------------------------------------------------

	PositionValidationResult PositionValidationResults::fromString( const std::string& name )
	{
		static const std::unordered_map<std::string, PositionValidationResult> nameMap{
			{ "Valid", PositionValidationResult::Valid },
			{ "Invalid", PositionValidationResult::Invalid },
			{ "InvalidOrder", PositionValidationResult::InvalidOrder },
			{ "InvalidGrouping", PositionValidationResult::InvalidGrouping },
			{ "Custom", PositionValidationResult::Custom } };

		auto it{ nameMap.find( name ) };
		if ( it != nameMap.end() )
		{
			return it->second;
		}

		SPDLOG_INFO( "Unknown position validation result: {}", name );
		throw std::invalid_argument( "Unknown position validation result: " + name );
	}

	//-------------------------------------------------------------------
	// CodebookStandardValues Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	CodebookStandardValues::CodebookStandardValues( CodebookName name, const std::unordered_set<std::string>& standardValues )
		: m_name{ name }, m_standardValues{ standardValues }
	{
		SPDLOG_INFO( "CodebookStandardValues constructed for '{}' with {} values",
			static_cast<int>( m_name ), m_standardValues.size() );
	}

	//-------------------------------------------------------------------
	// Capacity
	//-------------------------------------------------------------------

	size_t CodebookStandardValues::count() const
	{
		return m_standardValues.size();
	}

	//-------------------------------------------------------------------
	// Element Access
	//-------------------------------------------------------------------

	bool CodebookStandardValues::contains( const std::string& tagValue ) const
	{
		if ( m_name == CodebookName::Position )
		{
			int parsedValue;
			auto result = std::from_chars( tagValue.data(), tagValue.data() + tagValue.size(), parsedValue );
			if ( result.ec == std::errc() && result.ptr == tagValue.data() + tagValue.size() )
			{
				return true;
			}
		}
		return m_standardValues.find( tagValue ) != m_standardValues.end();
	}
	//-------------------------------------------------------------------
	// Iterators
	//-------------------------------------------------------------------

	CodebookStandardValues::iterator CodebookStandardValues::begin() const
	{
		return m_standardValues.begin();
	}

	CodebookStandardValues::iterator CodebookStandardValues::end() const
	{
		return m_standardValues.end();
	}

	//-------------------------------------------------------------------
	// CodebookGroups Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	CodebookGroups::CodebookGroups( const std::unordered_set<std::string>& groups )
		: m_groups{ groups }
	{
		SPDLOG_INFO( "CodebookGroups constructed with {} groups", m_groups.size() );
	}

	//-------------------------------------------------------------------
	// Capacity
	//-------------------------------------------------------------------

	size_t CodebookGroups::count() const
	{
		return m_groups.size();
	}

	//-------------------------------------------------------------------
	// Element Access
	//-------------------------------------------------------------------

	bool CodebookGroups::contains( const std::string& group ) const
	{
		return m_groups.find( group ) != m_groups.end();
	}

	//-------------------------------------------------------------------
	// Iterators
	//-------------------------------------------------------------------

	CodebookGroups::Iterator CodebookGroups::begin() const
	{
		return m_groups.begin();
	}

	CodebookGroups::Iterator CodebookGroups::end() const
	{
		return m_groups.end();
	}

	//-------------------------------------------------------------------
	// Codebook Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	Codebook::Codebook( const CodebookDto& dto )
		: m_name{},
		  m_groupMap{},
		  m_standardValues{},
		  m_groups{},
		  m_rawData{}
	{
		static const std::unordered_map<std::string, CodebookName> nameMap{
			{ "positions", CodebookName::Position },
			{ "calculations", CodebookName::Calculation },
			{ "quantities", CodebookName::Quantity },
			{ "states", CodebookName::State },
			{ "contents", CodebookName::Content },
			{ "commands", CodebookName::Command },
			{ "types", CodebookName::Type },
			{ "functional_services", CodebookName::FunctionalServices },
			{ "maintenance_category", CodebookName::MaintenanceCategory },
			{ "activity_type", CodebookName::ActivityType },
			{ "detail", CodebookName::Detail } };

		auto it{ nameMap.find( dto.name() ) };
		if ( it == nameMap.end() )
		{
			const std::string errorMsg = "Unknown codebook name: " + dto.name();
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		m_name = it->second;

		SPDLOG_INFO( "Constructing Codebook '{}'", dto.name() );

		std::unordered_set<std::string> valueSet{};
		std::unordered_set<std::string> groupSet{};

		m_rawData.reserve( dto.values().size() );

		size_t totalValueCount = std::accumulate(
			dto.values().begin(), dto.values().end(), static_cast<size_t>( 0 ),
			[]( size_t sum, const auto& pair ) {
				return sum + std::count_if( pair.second.begin(), pair.second.end(),
								 []( const std::string& v ) { return v != NUMBER_GROUP; } );
			} );
		m_groupMap.reserve( totalValueCount );

		valueSet.reserve( totalValueCount );
		groupSet.reserve( dto.values().size() );

		for ( const auto& [groupKey, values] : dto.values() )
		{
			std::string_view groupKeyView( groupKey );
			const auto groupFirst = groupKeyView.find_first_not_of( WHITESPACE );
			std::string trimmedGroup;
			if ( groupFirst != std::string_view::npos )
			{
				const auto groupLast = groupKeyView.find_last_not_of( WHITESPACE );
				trimmedGroup = std::string( groupKeyView.substr( groupFirst, groupLast - groupFirst + 1 ) );
			}

			std::vector<std::string> trimmedValues;
			trimmedValues.reserve( values.size() );
			bool groupHasValidValue = false;

			for ( const auto& value : values )
			{
				std::string_view valueView( value );
				const auto valueFirst = valueView.find_first_not_of( WHITESPACE );
				std::string trimmedValue;
				if ( valueFirst != std::string_view::npos )
				{
					const auto valueLast = valueView.find_last_not_of( WHITESPACE );
					trimmedValue = std::string( valueView.substr( valueFirst, valueLast - valueFirst + 1 ) );
				}

				trimmedValues.push_back( trimmedValue );

				if ( trimmedValue != NUMBER_GROUP )
				{
					groupHasValidValue = true;
					if ( auto [mapIt, inserted] = m_groupMap.try_emplace( trimmedValue, trimmedGroup ); !inserted )
					{
						SPDLOG_WARN( "Duplicate value '{}' found. Keeping group '{}', ignoring group '{}'.", trimmedValue,
							mapIt->second, trimmedGroup );
					}
					valueSet.insert( trimmedValue );
				}
			}

			m_rawData.emplace( trimmedGroup, std::move( trimmedValues ) );

			if ( groupHasValidValue )
			{
				groupSet.insert( trimmedGroup );
			}
		}

		m_standardValues = CodebookStandardValues{ m_name, valueSet };
		m_groups = CodebookGroups{ groupSet };

		SPDLOG_INFO( "Codebook '{}' constructed. Groups: {}, Standard Values: {}, Raw Entries: {}", dto.name(),
			m_groups.count(), m_standardValues.count(), m_rawData.size() );
	}

	//-------------------------------------------------------------------
	// Accessors
	//-------------------------------------------------------------------

	CodebookName Codebook::name() const
	{
		return m_name;
	}

	const CodebookGroups& Codebook::groups() const
	{
		return m_groups;
	}

	const CodebookStandardValues& Codebook::standardValues() const
	{
		return m_standardValues;
	}

	const std::unordered_map<std::string, std::vector<std::string>>& Codebook::rawData() const
	{
		return m_rawData;
	}

	//-------------------------------------------------------------------
	// Queries
	//-------------------------------------------------------------------

	bool Codebook::hasGroup( const std::string& group ) const
	{
		return m_groups.contains( group );
	}

	bool Codebook::hasStandardValue( const std::string& value ) const
	{
		return m_standardValues.contains( value );
	}

	//-------------------------------------------------------------------
	// Operations
	//-------------------------------------------------------------------

	std::optional<MetadataTag> Codebook::tryCreateTag( const std::string_view valueView ) const
	{
		if ( valueView.empty() || std::all_of( valueView.begin(), valueView.end(), []( unsigned char c ) { return std::isspace( c ); } ) )
		{
			SPDLOG_INFO( "Rejecting empty or whitespace-only value" );
			return std::nullopt;
		}

		std::string value{ valueView };
		bool isCustom{ false };

		if ( m_name == CodebookName::Position )
		{
			auto positionValidity{ validatePosition( value ) };
			if ( static_cast<int>( positionValidity ) < 100 )
			{
				SPDLOG_INFO( "Position validation failed with result: {}", static_cast<int>( positionValidity ) );
				return std::nullopt;
			}

			isCustom = ( positionValidity == PositionValidationResult::Custom );
		}
		else
		{
			if ( !VIS::isISOString( value ) )
			{
				SPDLOG_INFO( "Value is not an ISO string: {}", value );
				return std::nullopt;
			}

			if ( m_name != CodebookName::Detail && !m_standardValues.contains( value ) )
			{
				isCustom = true;
			}
		}

		SPDLOG_INFO( "Creating tag with value: {}, custom: {}", value, isCustom );

		return MetadataTag{ m_name, value, isCustom };
	}

	MetadataTag Codebook::createTag( const std::string& value ) const
	{
		auto tag{ tryCreateTag( value ) };
		if ( !tag.has_value() )
		{
			SPDLOG_ERROR( "Invalid value for metadata tag: codebook={}, value={}", static_cast<int>( m_name ), value );
			throw std::invalid_argument( "Invalid value for metadata tag: codebook=" + std::to_string( static_cast<int>( m_name ) ) + ", value=" + value );
		}

		return tag.value();
	}

	PositionValidationResult Codebook::validatePosition( const std::string& position ) const
	{
		if ( position.empty() ||
			 std::all_of( position.begin(), position.end(), []( unsigned char c ) { return std::isspace( c ); } ) ||
			 !VIS::isISOString( position ) )
		{
			SPDLOG_TRACE( "validatePosition('{}'): Failed initial check (empty, whitespace, or not ISO)", position );
			return PositionValidationResult::Invalid;
		}

		std::string_view positionView( position );
		size_t first_char = positionView.find_first_not_of( " \t\n\r\f\v" );
		size_t last_char = positionView.find_last_not_of( " \t\n\r\f\v" );

		if ( first_char == std::string_view::npos )
		{
			SPDLOG_TRACE( "validatePosition('{}'): Failed trim check (all whitespace?)", position );
			return PositionValidationResult::Invalid;
		}
		std::string_view trimmedView = positionView.substr( first_char, last_char - first_char + 1 );

		if ( trimmedView.length() != position.length() )
		{
			SPDLOG_TRACE( "validatePosition('{}'): Failed trim check (had leading/trailing whitespace)", position );
			return PositionValidationResult::Invalid;
		}

		std::string currentPositionStr( trimmedView );
		if ( m_standardValues.contains( currentPositionStr ) )
		{
			SPDLOG_TRACE( "validatePosition('{}'): Matched standard value", currentPositionStr );
			return PositionValidationResult::Valid;
		}

		int parsedValue;
		auto result = std::from_chars( trimmedView.data(), trimmedView.data() + trimmedView.size(), parsedValue );
		if ( result.ec == std::errc() && result.ptr == trimmedView.data() + trimmedView.size() )
		{
			SPDLOG_TRACE( "validatePosition('{}'): Matched numeric value", currentPositionStr );
			return PositionValidationResult::Valid;
		}

		size_t hyphenPos = trimmedView.find( '-' );
		if ( hyphenPos == std::string_view::npos )
		{
			SPDLOG_TRACE( "validatePosition('{}'): No hyphen, not standard/numeric -> Custom", currentPositionStr );
			return PositionValidationResult::Custom;
		}

		std::vector<std::string_view> parts{};
		size_t start = 0;
		while ( hyphenPos != std::string_view::npos )
		{
			parts.push_back( trimmedView.substr( start, hyphenPos - start ) );
			start = hyphenPos + 1;
			hyphenPos = trimmedView.find( '-', start );
		}
		parts.push_back( trimmedView.substr( start ) );

		std::vector<PositionValidationResult> validations{};
		validations.reserve( parts.size() );
		PositionValidationResult worstResult = PositionValidationResult::Valid;

		for ( const auto& partView : parts )
		{
			std::string partStr( partView );
			PositionValidationResult partValidation = validatePosition( partStr );
			validations.push_back( partValidation );

			if ( partValidation == PositionValidationResult::Invalid )
			{
				worstResult = PositionValidationResult::Invalid;
			}
			else if ( partValidation == PositionValidationResult::InvalidOrder && worstResult != PositionValidationResult::Invalid )
			{
				worstResult = PositionValidationResult::InvalidOrder;
			}
			else if ( partValidation == PositionValidationResult::InvalidGrouping &&
					  worstResult != PositionValidationResult::Invalid &&
					  worstResult != PositionValidationResult::InvalidOrder )
			{
				worstResult = PositionValidationResult::InvalidGrouping;
			}
			else if ( partValidation == PositionValidationResult::Custom &&
					  worstResult != PositionValidationResult::Invalid &&
					  worstResult != PositionValidationResult::InvalidOrder &&
					  worstResult != PositionValidationResult::InvalidGrouping )
			{
				worstResult = PositionValidationResult::Custom;
			}
		}
		SPDLOG_TRACE( "validatePosition('{}'): Recursive validation results worst: {}", currentPositionStr, static_cast<int>( worstResult ) );

		if ( worstResult == PositionValidationResult::Invalid ||
			 worstResult == PositionValidationResult::InvalidOrder ||
			 worstResult == PositionValidationResult::InvalidGrouping )
		{
			SPDLOG_TRACE( "validatePosition('{}'): Returning early due to invalid sub-part: {}", currentPositionStr, static_cast<int>( worstResult ) );
			return worstResult;
		}

		bool numberNotAtEnd{ false };
		std::vector<std::string_view> nonNumericParts{};
		nonNumericParts.reserve( parts.size() );

		for ( size_t i = 0; i < parts.size(); ++i )
		{
			int checkVal;
			auto checkResult = std::from_chars( parts[i].data(), parts[i].data() + parts[i].size(), checkVal );
			bool isNumber = ( checkResult.ec == std::errc() && checkResult.ptr == parts[i].data() + parts[i].size() );

			if ( isNumber )
			{
				if ( i < parts.size() - 1 )
				{
					numberNotAtEnd = true;
				}
			}
			else
			{
				nonNumericParts.push_back( parts[i] );
			}
		}

		bool notAlphabeticallySorted = false;
		if ( nonNumericParts.size() > 1 )
		{
			if ( !std::is_sorted( nonNumericParts.begin(), nonNumericParts.end() ) )
			{
				notAlphabeticallySorted = true;
			}
		}
		SPDLOG_TRACE( "validatePosition('{}'): Order check: numberNotAtEnd={}, notAlphabetical={}", currentPositionStr, numberNotAtEnd, notAlphabeticallySorted );

		if ( numberNotAtEnd || notAlphabeticallySorted )
		{
			return PositionValidationResult::InvalidOrder;
		}

		bool allSubPartsValid = std::all_of( validations.begin(), validations.end(),
			[]( PositionValidationResult v ) {
				return v == PositionValidationResult::Valid;
			} );

		if ( allSubPartsValid )
		{
			std::vector<std::string> groups{};
			groups.reserve( parts.size() );
			std::unordered_set<std::string> uniqueGroups{};
			bool hasDefaultGroup = false;

			for ( const auto& partView : parts )
			{
				int checkVal;
				auto checkResult = std::from_chars( partView.data(), partView.data() + partView.size(), checkVal );
				bool isNumber = ( checkResult.ec == std::errc() && checkResult.ptr == partView.data() + partView.size() );

				std::string groupName;
				if ( isNumber )
				{
					groupName = NUMBER_GROUP;
				}
				else
				{
					std::string partStr( partView );
					auto it = m_groupMap.find( partStr );
					groupName = ( it != m_groupMap.end() ) ? it->second : UNKNOWN_GROUP;
				}

				groups.push_back( groupName );
				uniqueGroups.insert( groupName );
				if ( groupName == DEFAULT_GROUP_NAME )
				{
					hasDefaultGroup = true;
				}
			}
			SPDLOG_TRACE( "validatePosition('{}'): Grouping check: hasDefault={}, uniqueGroups={}, totalGroups={}", currentPositionStr, hasDefaultGroup, uniqueGroups.size(), groups.size() );

			if ( !hasDefaultGroup && uniqueGroups.size() != groups.size() )
			{
				return PositionValidationResult::InvalidGrouping;
			}
		}

		SPDLOG_DEBUG( "validatePosition('{}'): Passed all checks, returning worst recursive result: {}", currentPositionStr, static_cast<int>( worstResult ) );
		return worstResult;
	}
}
