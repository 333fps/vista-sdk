/**
 * @file Codebook.cpp
 * @brief Implementation of codebook-related components
 */

#include "pch.h"

#include "dnv/vista/sdk/Codebook.h"

#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	//----------------------------------------------
	// Codebooks names
	//----------------------------------------------

	namespace
	{
		static constexpr std::string_view POSITION_NAME = "positions";
		static constexpr std::string_view CALCULATION_NAME = "calculations";
		static constexpr std::string_view QUANTITY_NAME = "quantities";
		static constexpr std::string_view STATE_NAME = "states";
		static constexpr std::string_view CONTENT_NAME = "contents";
		static constexpr std::string_view COMMAND_NAME = "commands";
		static constexpr std::string_view TYPE_NAME = "types";
		static constexpr std::string_view FUNCTIONAL_SERVICES_NAME = "functional_services";
		static constexpr std::string_view MAINTENANCE_CATEGORY_NAME = "maintenance_category";
		static constexpr std::string_view ACTIVITY_TYPE_NAME = "activity_type";
		static constexpr std::string_view DETAIL_NAME = "detail";
	}

	//----------------------------------------------
	// Position validation constants
	//----------------------------------------------

	namespace
	{
		/** @brief Special group identifier for numeric-only values in VISTA codebooks. */
		static constexpr std::string_view NUMBER_GROUP = "<number>";

		/** @brief Default group name for ungrouped position components. */
		static constexpr std::string_view DEFAULT_GROUP_NAME = "DEFAULT_GROUP";

		/** @brief Fallback group identifier for unrecognized position components. */
		static constexpr std::string_view UNKNOWN_GROUP = "UNKNOWN";

		/** @brief Standard whitespace characters for string trimming operations. */
		static constexpr std::string_view WHITESPACE = " \t\n\r\f\v";
	}

	//=====================================================================
	// Magic numbers
	//=====================================================================

	namespace
	{
		/** @brief Hash table load factor multiplier to minimize rehashing during Codebook construction. */
		static constexpr size_t LOAD_FACTOR = 2;

		/** @brief Stack allocation limit for position parsing arrays to avoid heap allocation during position validation. */
		static constexpr size_t MAX_POSITIONS = 16;

		/** @brief Stack allocation limit for group tracking arrays to avoid heap allocation during position grouping validation. */
		static constexpr size_t MAX_GROUPS = 16;

		/** @brief Stack allocation limit for non-numeric position tracking to avoid heap allocation during order validation. */
		static constexpr size_t MAX_NON_NUMERIC = 8;
	}

	//=====================================================================
	// Enum mapping tables
	//=====================================================================

	namespace
	{
		static constexpr std::array s_codebookNameMap{ std::to_array<std::pair<std::string_view, CodebookName>>(
			{ { POSITION_NAME, CodebookName::Position },
				{ CALCULATION_NAME, CodebookName::Calculation },
				{ QUANTITY_NAME, CodebookName::Quantity },
				{ STATE_NAME, CodebookName::State },
				{ CONTENT_NAME, CodebookName::Content },
				{ COMMAND_NAME, CodebookName::Command },
				{ TYPE_NAME, CodebookName::Type },
				{ FUNCTIONAL_SERVICES_NAME, CodebookName::FunctionalServices },
				{ MAINTENANCE_CATEGORY_NAME, CodebookName::MaintenanceCategory },
				{ ACTIVITY_TYPE_NAME, CodebookName::ActivityType },
				{ DETAIL_NAME, CodebookName::Detail } } ) };

		static constexpr CodebookName codebookNameFromString( std::string_view name )
		{
			for ( const auto& [nameStr, enumValue] : s_codebookNameMap )
			{
				if ( nameStr == name )
					return enumValue;
			}
			throw std::invalid_argument( "Unknown codebook name: " + std::string( name ) );
		}
	}

	//=====================================================================
	// Character lookup tables
	//=====================================================================

	namespace
	{
		alignas( 64 ) static constexpr std::array<bool, 256> s_digitLookup = []() constexpr {
			std::array<bool, 256> lookup{};
			for ( unsigned char i = '0'; i <= '9'; ++i )
			{
				lookup[i] = true;
			}

			return lookup;
		}();

		alignas( 64 ) static constexpr std::array<bool, 256> s_whitespaceLookup = []() constexpr {
			std::array<bool, 256> lookup{};
			constexpr std::string_view whitespace = WHITESPACE;
			for ( char c : whitespace )
			{
				lookup[static_cast<unsigned char>( c )] = true;
			}

			return lookup;
		}();

		constexpr bool isDigit( char c ) noexcept
		{
			return s_digitLookup[static_cast<unsigned char>( c )];
		}

		constexpr bool isWhitespace( char c ) noexcept
		{
			return s_whitespaceLookup[static_cast<unsigned char>( c )];
		}
	}

	//=====================================================================
	// PositionValidationResult string conversion
	//=====================================================================

	namespace
	{
		static constexpr std::array<std::pair<std::string_view, PositionValidationResult>, 5> s_validationResultMap =
			{ { { "invalid", PositionValidationResult::Invalid },
				{ "invalidorder", PositionValidationResult::InvalidOrder },
				{ "invalidgrouping", PositionValidationResult::InvalidGrouping },
				{ "valid", PositionValidationResult::Valid },
				{ "custom", PositionValidationResult::Custom } } };

		[[nodiscard]] static std::string toLower( std::string_view input ) noexcept
		{
			std::string result;
			result.reserve( input.size() );

			for ( char c : input )
			{
				result.push_back( ( c >= 'A' && c <= 'Z' ) ? ( c + 32 ) : c );
			}

			return result;
		}
	}

	//=====================================================================
	// Heterogeneous lookup
	//=====================================================================

	//----------------------------------------------
	// StringHash
	//----------------------------------------------

	size_t StringHash::operator()( std::string_view sv ) const noexcept
	{
		return std::hash<std::string_view>{}( sv );
	}

	size_t StringHash::operator()( const std::string& s ) const noexcept
	{
		return std::hash<std::string>{}( s );
	}

	//----------------------------------------------
	// StringEqual
	//----------------------------------------------

	bool StringEqual::operator()( const std::string& lhs, const std::string& rhs ) const noexcept
	{
		return lhs == rhs;
	}

	bool StringEqual::operator()( const std::string& lhs, std::string_view rhs ) const noexcept
	{
		return lhs == rhs;
	}

	bool StringEqual::operator()( std::string_view lhs, const std::string& rhs ) const noexcept
	{
		return lhs == rhs;
	}

	bool StringEqual::operator()( std::string_view lhs, std::string_view rhs ) const noexcept
	{
		return lhs == rhs;
	}

	//=====================================================================
	// PositionValidationResults class
	//=====================================================================

	//----------------------------------------------
	// Static utility methods
	//----------------------------------------------

	PositionValidationResult PositionValidationResults::fromString( std::string_view name )
	{
		if ( name.empty() )
		{
			throw std::invalid_argument( "PositionValidationResult name cannot be empty" );
		}

		const std::string lowerName = toLower( name );

		for ( const auto& [key, value] : s_validationResultMap )
		{
			if ( key == lowerName )
			{
				SPDLOG_TRACE( "Converted PositionValidationResult: '{}' -> {}",
					name, static_cast<int>( value ) );
				return value;
			}
		}

		throw std::invalid_argument( "Unknown PositionValidationResult name: '" + std::string( name ) +
									 "'. Valid values are: Invalid, InvalidOrder, InvalidGrouping, Valid, Custom" );
	}

	//=====================================================================
	// CodebookStandardValues class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	CodebookStandardValues::CodebookStandardValues( CodebookName name, std::unordered_set<std::string, StringHash, StringEqual>&& standardValues )
		: m_name{ name },
		  m_standardValues{ std::move( standardValues ) }
	{
	}

	//----------------------------------------------
	// Public methods
	//----------------------------------------------

	bool CodebookStandardValues::contains( std::string_view tagValue ) const noexcept
	{
		if ( m_standardValues.contains( tagValue ) )
		{
			return true;
		}

		if ( m_name == CodebookName::Position )
		{
			return !tagValue.empty() && std::all_of( tagValue.begin(), tagValue.end(), isDigit );
		}

		return false;
	}

	//=====================================================================
	// CodebookGroups class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	CodebookGroups::CodebookGroups( std::unordered_set<std::string, StringHash, StringEqual>&& groups )
		: m_groups{ std::move( groups ) }
	{
	}

	//=====================================================================
	// Codebook class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	Codebook::Codebook( const CodebookDto& dto )
		: m_name{ codebookNameFromString( dto.name() ) },
		  m_groupMap{},
		  m_standardValues{},
		  m_groups{},
		  m_rawData{}
	{
		auto trimString = []( std::string_view str ) -> std::string_view {
			if ( str.empty() )
			{
				return {};
			}

			const char* data = str.data();
			const size_t size = str.size();

			size_t first = 0;
			while ( first < size && isWhitespace( data[first] ) )
			{
				++first;
			}

			if ( first == size )
			{
				return {};
			}

			size_t last = size - 1;
			while ( last > first && isWhitespace( data[last] ) )
			{
				--last;
			}

			return str.substr( first, last - first + 1 );
		};

		size_t totalEstimate = 0;
		for ( const auto& [_, values] : dto.values() )
		{
			totalEstimate += values.size();
		}

		m_groupMap.reserve( totalEstimate * LOAD_FACTOR );
		m_rawData.reserve( dto.values().size() * LOAD_FACTOR );

		std::unordered_set<std::string, StringHash, StringEqual> valueSet;
		std::unordered_set<std::string, StringHash, StringEqual> groupSet;
		valueSet.reserve( totalEstimate * LOAD_FACTOR );
		groupSet.reserve( dto.values().size() * LOAD_FACTOR );

		for ( const auto& [groupKey, values] : dto.values() )
		{
			std::string_view groupTrimmed = trimString( groupKey );
			std::string groupStr{ groupTrimmed };

			std::vector<std::string> trimmedValues;
			trimmedValues.reserve( values.size() );

			for ( const auto& value : values )
			{
				std::string_view valueTrimmed = trimString( value );
				std::string valueStr{ valueTrimmed };
				trimmedValues.push_back( valueStr );

				if ( valueStr != NUMBER_GROUP )
				{
					m_groupMap.try_emplace( valueStr, groupStr );
					valueSet.insert( valueStr );
					groupSet.insert( groupStr );
				}
			}

			m_rawData.emplace( std::move( groupStr ), std::move( trimmedValues ) );
		}

		m_standardValues = CodebookStandardValues{ m_name, std::move( valueSet ) };
		m_groups = CodebookGroups{ std::move( groupSet ) };

		SPDLOG_DEBUG( "Codebook '{}' constructed: {} groups, {} values, {} raw entries",
			CodebookNames::toPrefix( m_name ), m_groups.count(),
			m_standardValues.count(), m_rawData.size() );
	}

	//----------------------------------------------
	// Metadata tag creation
	//----------------------------------------------

	std::optional<MetadataTag> Codebook::tryCreateTag( std::string_view valueView ) const
	{
		if ( valueView.empty() )
		{
			return std::nullopt;
		}

		bool isCustom = false;

		if ( m_name == CodebookName::Position )
		{
			PositionValidationResult positionValidity = validatePosition( valueView );

			if ( positionValidity < PositionValidationResult::Valid )
			{
				return std::nullopt;
			}

			if ( positionValidity == PositionValidationResult::Custom )
			{
				isCustom = true;
			}
		}
		else
		{
			if ( !VIS::isISOString( valueView ) )
			{
				return std::nullopt;
			}

			if ( m_name != CodebookName::Detail && !m_standardValues.contains( valueView ) )
			{
				isCustom = true;
			}
		}

		std::string value{ valueView };
		return MetadataTag( m_name, value, isCustom );
	}

	MetadataTag Codebook::createTag( std::string_view value ) const
	{
		auto tagOpt = tryCreateTag( value );
		if ( !tagOpt )
		{
			throw std::invalid_argument( "Invalid value for metadata tag: codebook=" + std::to_string( static_cast<int>( m_name ) ) + ", value=" + std::string( value ) );
		}

		return tagOpt.value();
	}

	//----------------------------------------------
	// Position validation
	//----------------------------------------------

	PositionValidationResult Codebook::validatePosition( std::string_view position ) const
	{
		if ( position.empty() )
		{
			return PositionValidationResult::Invalid;
		}

		if ( position.find_first_of( WHITESPACE ) != std::string_view::npos )
		{
			return PositionValidationResult::Invalid;
		}

		if ( !VIS::isISOString( position ) )
		{
			return PositionValidationResult::Invalid;
		}

		if ( m_standardValues.contains( position ) )
		{
			return PositionValidationResult::Valid;
		}

		if ( !position.empty() && std::all_of( position.begin(), position.end(), isDigit ) )
		{
			return PositionValidationResult::Valid;
		}

		const auto hyphenPos = position.find( '-' );
		if ( hyphenPos == std::string_view::npos )
		{
			return PositionValidationResult::Custom;
		}

		std::array<std::string_view, MAX_POSITIONS> positions;
		std::array<PositionValidationResult, MAX_POSITIONS> results;
		size_t positionCount = 0;

		size_t start = 0;
		size_t pos = 0;
		while ( ( pos = position.find( '-', start ) ) != std::string_view::npos && positionCount < MAX_POSITIONS )
		{
			positions[positionCount] = position.substr( start, pos - start );
			start = pos + 1;
			++positionCount;
		}

		if ( positionCount < MAX_POSITIONS )
		{
			positions[positionCount] = position.substr( start );
			++positionCount;
		}
		else
		{
			return PositionValidationResult::Invalid;
		}

		auto validateSinglePosition = [this]( std::string_view pos ) noexcept -> PositionValidationResult {
			if ( m_standardValues.contains( pos ) )
				return PositionValidationResult::Valid;

			if ( !pos.empty() && std::all_of( pos.begin(), pos.end(), isDigit ) )
				return PositionValidationResult::Valid;

			return PositionValidationResult::Custom;
		};

		PositionValidationResult worstResult = PositionValidationResult::Valid;

		for ( size_t i = 0; i < positionCount; ++i )
		{
			results[i] = validateSinglePosition( positions[i] );

			if ( results[i] < PositionValidationResult::Valid )
			{
				return results[i];
			}

			if ( results[i] > worstResult )
				worstResult = results[i];
		}

		std::array<std::string_view, MAX_NON_NUMERIC> nonNumericPositions;
		size_t nonNumericCount = 0;
		bool hasNumberNotAtEnd = false;

		for ( size_t i = 0; i < positionCount; ++i )
		{
			const bool isNumber = !positions[i].empty() && std::all_of( positions[i].begin(), positions[i].end(), isDigit );

			if ( isNumber )
			{
				if ( i < positionCount - 1 )
				{
					hasNumberNotAtEnd = true;
				}
			}
			else
			{
				if ( nonNumericCount < MAX_NON_NUMERIC )
				{
					nonNumericPositions[nonNumericCount] = positions[i];
					++nonNumericCount;
				}
			}
		}

		bool isNotSorted = false;
		if ( nonNumericCount > 1 )
		{
			for ( size_t i = 1; i < nonNumericCount; ++i )
			{
				if ( nonNumericPositions[i] < nonNumericPositions[i - 1] )
				{
					isNotSorted = true;
					break;
				}
			}
		}

		if ( hasNumberNotAtEnd || isNotSorted )
		{
			return PositionValidationResult::InvalidOrder;
		}

		if ( worstResult == PositionValidationResult::Valid )
		{
			std::array<std::string_view, MAX_GROUPS> groups;
			std::array<bool, MAX_GROUPS> groupSeen;
			groupSeen.fill( false );

			size_t groupCount = 0;
			size_t uniqueGroupCount = 0;
			bool hasDefaultGroup = false;

			for ( size_t i = 0; i < positionCount; ++i )
			{
				std::string_view group;

				if ( !positions[i].empty() && std::all_of( positions[i].begin(), positions[i].end(), isDigit ) )
				{
					group = NUMBER_GROUP;
				}
				else
				{
					auto it = m_groupMap.find( positions[i] );
					group = ( it != m_groupMap.end() ) ? std::string_view{ it->second } : UNKNOWN_GROUP;
				}

				if ( groupCount < MAX_GROUPS )
				{
					groups[groupCount] = group;
					++groupCount;
				}

				bool alreadySeen = false;
				for ( size_t j = 0; j < uniqueGroupCount; ++j )
				{
					if ( groups[j] == group )
					{
						alreadySeen = true;
						break;
					}
				}

				if ( !alreadySeen && uniqueGroupCount < MAX_GROUPS )
				{
					groups[uniqueGroupCount] = group;
					++uniqueGroupCount;
				}

				if ( group == DEFAULT_GROUP_NAME )
				{
					hasDefaultGroup = true;
				}
			}

			if ( !hasDefaultGroup && uniqueGroupCount != groupCount )
			{
				return PositionValidationResult::InvalidGrouping;
			}
		}

		return worstResult;
	}
}
