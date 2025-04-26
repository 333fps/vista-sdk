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
			try
			{
				auto val{ std::stoi( tagValue ) };
				(void)val;
				return true;
			}
			catch ( const std::invalid_argument& )
			{
				// Fall through to standard lookup
			}
			catch ( const std::out_of_range& )
			{
				// Fall through to standard lookup
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

	bool Codebook::hasGroup( std::string_view group ) const
	{
		return m_groups.contains( std::string( group ) );
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
		if ( valueView.empty() || std::all_of( valueView.begin(), valueView.end(),
									  []( unsigned char c ) { return std::isspace( c ); } ) )
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
				SPDLOG_INFO( "Position validation failed with result: {}",
					static_cast<int>( positionValidity ) );
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
				isCustom = true;
		}

		SPDLOG_INFO( "Creating tag with value: {}, custom: {}", value, isCustom );

		return MetadataTag{ m_name, value, isCustom };
	}

	MetadataTag Codebook::createTag( const std::string& value ) const
	{
		auto tag{ tryCreateTag( value ) };
		if ( !tag.has_value() )
		{
			SPDLOG_ERROR( "Invalid value for metadata tag: codebook={}, value={}",
				static_cast<int>( m_name ), value );
			throw std::invalid_argument( "Invalid value for metadata tag: codebook=" +
										 std::to_string( static_cast<int>( m_name ) ) + ", value=" + value );
		}

		return tag.value();
	}

	PositionValidationResult Codebook::validatePosition( const std::string& position ) const
	{
		SPDLOG_INFO( "Validating position: {}", position );
		if ( position.empty() ||
			 std::all_of( position.begin(), position.end(), []( unsigned char c ) { return std::isspace( c ); } ) ||
			 !VIS::isISOString( position ) )
		{
			SPDLOG_WARN( "Position is empty or whitespace-only or not an ISO string: {}", position );
			return PositionValidationResult::Invalid;
		}

		std::string trimmedPosition{ position };
		trimmedPosition.erase( 0, trimmedPosition.find_first_not_of( " \t\n\r\f\v" ) );
		trimmedPosition.erase( trimmedPosition.find_last_not_of( " \t\n\r\f\v" ) + 1 );
		if ( trimmedPosition.length() != position.length() )
		{
			SPDLOG_WARN( "Position has leading or trailing whitespace: {}", position );
			return PositionValidationResult::Invalid;
		}

		if ( m_standardValues.contains( position ) )
		{
			SPDLOG_INFO( "Position is a standard value: {}", position );
			return PositionValidationResult::Valid;
		}

		try
		{
			auto val{ std::stoi( position ) };
			(void)val;

			SPDLOG_INFO( "Position is a number: {}", position );
			return PositionValidationResult::Valid;
		}
		catch ( const std::invalid_argument& )
		{
			SPDLOG_DEBUG( "Position is not a number: {}", position );
		}
		catch ( const std::out_of_range& )
		{
			SPDLOG_INFO( "Position is out of range: {}", position );
		}

		if ( position.find( '-' ) == std::string::npos )
		{
			SPDLOG_INFO( "Position is not compound: {}", position );
			return PositionValidationResult::Custom;
		}

		std::vector<std::string> positions{};
		std::string temp{};
		for ( char c : position )
		{
			if ( c == '-' )
			{
				SPDLOG_DEBUG( "Position is compound: {}", position );
				positions.push_back( temp );
				temp.clear();
			}
			else
			{
				temp.append( { c } );
			}
		}
		positions.push_back( temp );

		std::vector<PositionValidationResult> validations{};
		for ( const auto& positionStr : positions )
		{
			validations.push_back( validatePosition( positionStr ) );
		}

		if ( std::any_of( validations.begin(), validations.end(),
				 []( PositionValidationResult v ) { return static_cast<int>( v ) < 100; } ) )
		{
			return *std::max_element( validations.begin(), validations.end() );
		}

		bool numberNotAtEnd{ false };
		for ( size_t i{ 0 }; i < positions.size() - 1; ++i )
		{
			try
			{
				auto val{ std::stoi( positions[i] ) };
				(void)val;

				numberNotAtEnd = true;
				break;
			}
			catch ( const std::invalid_argument& )
			{
				SPDLOG_DEBUG( "Position is not a number: {}", positions[i] );
			}
			catch ( const std::out_of_range& )
			{
				SPDLOG_INFO( "Position is out of range: {}", positions[i] );
			}
		}

		std::vector<std::string> positionsWithoutNumber{};
		for ( const auto& p : positions )
		{
			try
			{
				auto val{ std::stoi( p ) };
				(void)val;
			}
			catch ( const std::invalid_argument& )
			{
				SPDLOG_DEBUG( "Position is not a number: {}", p );
				positionsWithoutNumber.push_back( p );
			}
			catch ( const std::out_of_range& )
			{
				SPDLOG_INFO( "Position is out of range: {}", p );
				positionsWithoutNumber.push_back( p );
			}
		}

		std::vector<std::string> alphabeticallySorted{ positionsWithoutNumber };
		std::sort( alphabeticallySorted.begin(), alphabeticallySorted.end() );
		bool notAlphabeticallySorted{ positionsWithoutNumber != alphabeticallySorted };

		if ( numberNotAtEnd || notAlphabeticallySorted )
		{
			return PositionValidationResult::InvalidOrder;
		}

		bool allValid{ std::all_of( validations.begin(), validations.end(),
			[]( PositionValidationResult v ) {
				return static_cast<int>( v ) == static_cast<int>( PositionValidationResult::Valid );
			} ) };

		if ( allValid )
		{
			std::vector<std::string> groups{};
			for ( const auto& p : positions )
			{
				try
				{
					auto val{ std::stoi( p ) };
					(void)val;

					groups.push_back( "<number>" );
				}
				catch ( const std::invalid_argument& )
				{
					auto it{ m_groupMap.find( p ) };
					if ( it != m_groupMap.end() )
					{
						groups.push_back( it->second );
					}
					else
					{
						groups.push_back( "UNKNOWN" );
					}
				}
				catch ( const std::out_of_range& )
				{
					auto it{ m_groupMap.find( p ) };
					if ( it != m_groupMap.end() )
					{
						groups.push_back( it->second );
					}
					else
					{
						groups.push_back( "UNKNOWN" );
					}
				}
			}

			auto begin = groups.begin();
			auto end = groups.end();
			std::unordered_set<std::string> groupsSet( begin, end );

			auto defaultGroupIt{ std::find( groups.begin(), groups.end(), "DEFAULT_GROUP" ) };
			if ( defaultGroupIt == groups.end() && groupsSet.size() != groups.size() )
			{
				SPDLOG_INFO( "Position has invalid grouping: {}", position );
				return PositionValidationResult::InvalidGrouping;
			}
		}

		auto result = *std::max_element( validations.begin(), validations.end() );
		SPDLOG_INFO( "Position validation result for '{}': {}", position, static_cast<int>( result ) );
		return result;
	}
}
