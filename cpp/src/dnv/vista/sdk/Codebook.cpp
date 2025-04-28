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

	CodebookGroups::iterator CodebookGroups::begin() const
	{
		return m_groups.begin();
	}

	CodebookGroups::iterator CodebookGroups::end() const
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
			SPDLOG_ERROR( "Unknown metadata tag: {}", dto.name() );
			throw std::invalid_argument( "Unknown metadata tag: " + dto.name() );
		}
		m_name = it->second;

		m_rawData = dto.values();

		std::vector<std::pair<std::string, std::string>> data{};
		std::unordered_set<std::string> valueSet{};
		std::unordered_set<std::string> groupSet{};

		for ( const auto& [group, values] : dto.values() )
		{
			std::string trimmedGroup{ group };
			trimmedGroup.erase( 0, trimmedGroup.find_first_not_of( " \t\n\r\f\v" ) );
			trimmedGroup.erase( trimmedGroup.find_last_not_of( " \t\n\r\f\v" ) + 1 );

			for ( const auto& value : values )
			{
				std::string trimmedValue{ value };
				trimmedValue.erase( 0, trimmedValue.find_first_not_of( " \t\n\r\f\v" ) );
				trimmedValue.erase( trimmedValue.find_last_not_of( " \t\n\r\f\v" ) + 1 );

				if ( trimmedValue != "<number>" )
				{
					data.push_back( { trimmedGroup, trimmedValue } );
					m_groupMap[trimmedValue] = trimmedGroup;
					valueSet.insert( trimmedValue );
					groupSet.insert( trimmedGroup );
				}
			}
		}

		m_standardValues = CodebookStandardValues{ m_name, valueSet };
		m_groups = CodebookGroups{ groupSet };

		SPDLOG_INFO( "Codebook created with {} standard values across {} groups", valueSet.size(), groupSet.size() );
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
			return PositionValidationResult::Invalid;
		}

		std::string_view positionView( position );
		size_t first_char = positionView.find_first_not_of( " \t\n\r\f\v" );
		size_t last_char = positionView.find_last_not_of( " \t\n\r\f\v" );
		if ( first_char == std::string_view::npos )
		{
			return PositionValidationResult::Invalid;
		}
		std::string_view trimmedView = positionView.substr( first_char, last_char - first_char + 1 );

		if ( trimmedView.length() != position.length() )
		{
			return PositionValidationResult::Invalid;
		}

		std::string currentPosition( trimmedView );

		if ( m_standardValues.contains( currentPosition ) )
		{
			return PositionValidationResult::Valid;
		}

		int parsedValue;
		auto result = std::from_chars( currentPosition.data(), currentPosition.data() + currentPosition.size(), parsedValue );
		if ( result.ec == std::errc() && result.ptr == currentPosition.data() + currentPosition.size() )
		{
			return PositionValidationResult::Valid;
		}

		size_t hyphenPos = currentPosition.find( '-' );
		if ( hyphenPos == std::string::npos )
		{
			return PositionValidationResult::Custom;
		}

		std::vector<std::string> parts{};
		size_t start = 0;
		size_t end = currentPosition.find( '-' );
		while ( end != std::string::npos )
		{
			parts.push_back( currentPosition.substr( start, end - start ) );
			start = end + 1;
			end = currentPosition.find( '-', start );
		}
		parts.push_back( currentPosition.substr( start ) );

		std::vector<PositionValidationResult> validations{};
		validations.reserve( parts.size() );
		PositionValidationResult worstResult = PositionValidationResult::Valid;
		for ( const auto& partStr : parts )
		{
			PositionValidationResult partValidation = validatePosition( partStr );
			validations.push_back( partValidation );
			if ( static_cast<int>( partValidation ) < static_cast<int>( worstResult ) )
			{
				worstResult = partValidation;
			}
		}

		if ( static_cast<int>( worstResult ) < 100 )
		{
			return worstResult;
		}

		bool numberNotAtEnd{ false };
		std::vector<std::string> nonNumericParts{};
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

			for ( const auto& p : parts )
			{
				int checkVal;
				auto checkResult = std::from_chars( p.data(), p.data() + p.size(), checkVal );
				bool isNumber = ( checkResult.ec == std::errc() && checkResult.ptr == p.data() + p.size() );

				std::string groupName;
				if ( isNumber )
				{
					groupName = "<number>";
				}
				else
				{
					auto it = m_groupMap.find( p );
					groupName = ( it != m_groupMap.end() ) ? it->second : "UNKNOWN";
				}

				groups.push_back( groupName );
				uniqueGroups.insert( groupName );
				if ( groupName == "DEFAULT_GROUP" )
				{
					hasDefaultGroup = true;
				}
			}

			if ( !hasDefaultGroup && uniqueGroups.size() != groups.size() )
			{
				return PositionValidationResult::InvalidGrouping;
			}
		}

		return worstResult;
	}
}
