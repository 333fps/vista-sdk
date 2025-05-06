/**
 * @file VIS.cpp
 * @brief Implementation of Vessel Information Structure (VIS) interface
 */

#include "pch.h"

#include "dnv/vista/sdk/VIS.h"

#include "dnv/vista/sdk/Codebook.h"
#include "dnv/vista/sdk/EmbeddedResource.h"
#include "dnv/vista/sdk/GmodDto.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/LocationsDto.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/Gmod.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------------

	const VisVersion VIS::LatestVisVersion = VisVersionExtensions::latestVersion();
	const std::string VIS::m_versioning = "versioning";

	//-------------------------------------------------------------------------
	// Constructors & Assignment Operators
	//-------------------------------------------------------------------------

	VIS::VIS() : IVIS{}
	{
		SPDLOG_INFO( "Initializing VIS singleton with empty caches" );
	}

	//-------------------------------------------------------------------------
	// Singleton Access
	//-------------------------------------------------------------------------

	VIS& VIS::instance()
	{
		static VIS instance;
		return instance;
	}

	//-------------------------------------------------------------------------
	// IVIS Interface Implementation
	//-------------------------------------------------------------------------

	const Gmod& VIS::gmod( VisVersion visVersion ) const
	{
		if ( !VisVersionExtensions::isValid( visVersion ) )
		{
			SPDLOG_ERROR( "Invalid VIS version: {}", static_cast<int>( visVersion ) );
			throw std::invalid_argument( "Invalid VIS version: " +
										 std::to_string( static_cast<int>( visVersion ) ) );
		}

		SPDLOG_INFO( "Attempting to load GMOD for version: {}",
			VisVersionExtensions::toVersionString( visVersion ) );

		return m_gmodCache.getOrCreate( visVersion, [this, visVersion]() {
			auto dto = gmodDto( visVersion );
			SPDLOG_INFO( "Successfully loaded GMOD DTO for version: {}",
				VisVersionExtensions::toVersionString( visVersion ) );

			Gmod gmod( visVersion, dto );

			SPDLOG_INFO( "Created Gmod with {} nodes in dictionary",
				!gmod.isEmpty() ? "non-empty" : "EMPTY" );

			return gmod;
		} );
	}

	const Codebooks& VIS::codebooks( VisVersion visVersion )
	{
		if ( !VisVersionExtensions::isValid( visVersion ) )
		{
			SPDLOG_ERROR( "Invalid VIS version: {}", static_cast<int>( visVersion ) );
			throw std::invalid_argument( "Invalid VIS version" );
		}

		SPDLOG_INFO( "Getting codebooks for version: {}",
			VisVersionExtensions::toVersionString( visVersion ) );

		return m_codebooksCache.getOrCreate( visVersion, [this, visVersion]() {
			auto dto = codebooksDto( visVersion );
			SPDLOG_INFO( "Successfully loaded codebooks for version: {}",
				VisVersionExtensions::toVersionString( visVersion ) );
			return Codebooks( visVersion, dto );
		} );
	}

	const Locations& VIS::locations( VisVersion visVersion )
	{
		if ( !VisVersionExtensions::isValid( visVersion ) )
		{
			SPDLOG_ERROR( "Invalid VIS version: {}", static_cast<int>( visVersion ) );
			throw std::invalid_argument( "Invalid VIS version" );
		}

		SPDLOG_INFO( "Getting locations for version: {}",
			VisVersionExtensions::toVersionString( visVersion ) );

		return m_locationsCache.getOrCreate( visVersion, [this, visVersion]() {
			auto dto = locationsDto( visVersion );
			SPDLOG_INFO( "Successfully loaded locations for version: {}",
				VisVersionExtensions::toVersionString( visVersion ) );
			return Locations( visVersion, dto );
		} );
	}

	std::unordered_map<VisVersion, const Codebooks*> VIS::codebooksMap(
		const std::vector<VisVersion>& visVersions )
	{
		SPDLOG_INFO( "Getting codebooks map for {} versions", visVersions.size() );

		std::unordered_map<VisVersion, const Codebooks*> result;
		result.reserve( visVersions.size() );
		for ( auto version : visVersions )
		{
			try
			{
				result.emplace( version, &codebooks( version ) );
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "Failed to get codebooks for version {}: {}",
					VisVersionExtensions::toVersionString( version ), ex.what() );
			}
		}
		return result;
	}

	std::unordered_map<VisVersion, const Gmod*> VIS::gmodsMap(
		const std::vector<VisVersion>& visVersions ) const
	{
		SPDLOG_INFO( "Getting GMOD map for {} versions", visVersions.size() );

		std::unordered_map<VisVersion, const Gmod*> result;
		result.reserve( visVersions.size() );
		for ( auto version : visVersions )
		{
			try
			{
				result.emplace( version, &gmod( version ) );
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "Failed to get GMOD for version {}: {}",
					VisVersionExtensions::toVersionString( version ), ex.what() );
			}
		}
		return result;
	}

	std::unordered_map<VisVersion, const Locations*> VIS::locationsMap(
		const std::vector<VisVersion>& visVersions )
	{
		SPDLOG_INFO( "Getting locations map for {} versions", visVersions.size() );

		std::unordered_map<VisVersion, const Locations*> result;
		result.reserve( visVersions.size() );
		for ( auto version : visVersions )
		{
			try
			{
				result.emplace( version, &locations( version ) );
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "Failed to get locations for version {}: {}",
					VisVersionExtensions::toVersionString( version ), ex.what() );
			}
		}
		return result;
	}

	std::vector<VisVersion> VIS::visVersions()
	{
		SPDLOG_INFO( "Getting all VIS versions" );
		return VisVersionExtensions::allVersions();
	}

	std::optional<GmodNode> VIS::convertNode( VisVersion sourceVersion, const GmodNode& sourceNode,
		VisVersion targetVersion )
	{
		return gmodVersioning().convertNode( sourceVersion, sourceNode, targetVersion );
	}

	std::optional<GmodPath> VIS::convertPath( VisVersion sourceVersion, const GmodPath& sourcePath,
		VisVersion targetVersion )
	{
		return gmodVersioning().convertPath( sourceVersion, sourcePath, targetVersion );
	}

	//-------------------------------------------------------------------------
	// Extended Conversion Methods
	//-------------------------------------------------------------------------

	std::optional<GmodNode> VIS::convertNode( const GmodNode& sourceNode, VisVersion targetVersion,
		[[maybe_unused]] const GmodNode* sourceParent )
	{
		return convertNode( sourceNode.visVersion(), sourceNode, targetVersion );
	}

	std::optional<GmodPath> VIS::convertPath( const GmodPath& sourcePath, VisVersion targetVersion )
	{
		return convertPath( sourcePath.visVersion(), sourcePath, targetVersion );
	}

	std::optional<LocalIdBuilder> VIS::convertLocalId( const LocalIdBuilder& sourceLocalId,
		VisVersion targetVersion )
	{
		return gmodVersioning().convertLocalId( sourceLocalId, targetVersion );
	}

	std::optional<LocalId> VIS::convertLocalId( const LocalId& sourceLocalId, VisVersion targetVersion )
	{
		return gmodVersioning().convertLocalId( sourceLocalId, targetVersion );
	}

	//-------------------------------------------------------------------------
	// DTO Access Methods
	//-------------------------------------------------------------------------

	GmodDto VIS::gmodDto( VisVersion visVersion ) const
	{
		SPDLOG_INFO( "Getting GMOD DTO for version: {}",
			VisVersionExtensions::toVersionString( visVersion ) );

		return m_gmodDtoCache.getOrCreate( visVersion, [visVersion]() {
			auto dto = loadGmodDto( visVersion );
			if ( !dto )
			{
				SPDLOG_ERROR( "Failed to load GMOD DTO for version: {}",
					VisVersionExtensions::toVersionString( visVersion ) );
				throw std::runtime_error( "Failed to load GMOD DTO for version" );
			}
			return *dto;
		} );
	}

	std::optional<GmodDto> VIS::loadGmodDto( VisVersion visVersion )
	{
		SPDLOG_INFO( "Loading GMOD DTO from resources for version: {}",
			VisVersionExtensions::toVersionString( visVersion ) );

		return EmbeddedResource::gmod( VisVersionExtensions::toVersionString( visVersion ) );
	}

	std::unordered_map<std::string, GmodVersioningDto> VIS::gmodVersioningDto()
	{
		SPDLOG_INFO( "Getting GMOD versioning DTO" );

		return m_gmodVersioningDtoCache.getOrCreate( m_versioning, []() {
			auto dto = EmbeddedResource::gmodVersioning();
			if ( !dto )
			{
				SPDLOG_ERROR( "Failed to load GMOD versioning data" );
				throw std::runtime_error( "Failed to load GMOD versioning data" );
			}
			return *dto;
		} );
	}

	GmodVersioning VIS::gmodVersioning()
	{
		SPDLOG_INFO( "Getting GMOD versioning" );

		return m_gmodVersioningCache.getOrCreate( m_versioning, [this]() {
			auto dto = gmodVersioningDto();
			SPDLOG_INFO( "Successfully loaded GMOD versioning data with {} entries", dto.size() );
			return GmodVersioning( dto );
		} );
	}

	CodebooksDto VIS::codebooksDto( VisVersion visVersion )
	{
		SPDLOG_INFO( "Getting codebooks DTO for version: {}",
			VisVersionExtensions::toVersionString( visVersion ) );

		return m_codebooksDtoCache.getOrCreate( visVersion, [visVersion]() {
			auto dto = EmbeddedResource::codebooks( VisVersionExtensions::toVersionString( visVersion ) );
			if ( !dto )
			{
				SPDLOG_ERROR( "Failed to load codebooks DTO for version: {}",
					VisVersionExtensions::toVersionString( visVersion ) );
				throw std::runtime_error( "Failed to load codebooks DTO" );
			}
			return *dto;
		} );
	}

	LocationsDto VIS::locationsDto( VisVersion visVersion )
	{
		SPDLOG_INFO( "Getting locations DTO for version: {}",
			VisVersionExtensions::toVersionString( visVersion ) );

		return m_locationsDtoCache.getOrCreate( visVersion, [visVersion]() {
			auto dto = EmbeddedResource::locations( VisVersionExtensions::toVersionString( visVersion ) );
			if ( !dto )
			{
				SPDLOG_ERROR( "Failed to load locations DTO for version: {}",
					VisVersionExtensions::toVersionString( visVersion ) );
				throw std::runtime_error( "Failed to load locations DTO" );
			}
			return *dto;
		} );
	}

	//-------------------------------------------------------------------------
	// ISO String Validation Methods
	//-------------------------------------------------------------------------

	bool VIS::matchISOLocalIdString( const std::string& value )
	{
		SPDLOG_DEBUG( "Checking if string matches ISO local ID format: '{}'", value );

		for ( char ch : value )
		{
			if ( ch == '/' )
				continue;

			if ( !isISOString( ch ) )
			{
				SPDLOG_ERROR( "Character '{}' is not ISO compliant", ch );
				return false;
			}
		}

		SPDLOG_DEBUG( "String is ISO local ID compliant" );
		return true;
	}

	bool VIS::matchISOLocalIdString( const std::stringstream& builder )
	{
		return matchISOLocalIdString( std::string_view( builder.str() ) );
	}

	bool VIS::matchISOLocalIdString( std::string_view value )
	{
		SPDLOG_DEBUG( "Checking if string matches ISO local ID format: '{}'", value );

		for ( char ch : value )
		{
			if ( ch == '/' )
				continue;

			if ( !isISOString( ch ) )
			{
				SPDLOG_ERROR( "Character '{}' is not ISO compliant", ch );
				return false;
			}
		}

		SPDLOG_DEBUG( "String is ISO local ID compliant" );
		return true;
	}

	bool VIS::isISOString( char c )
	{
		// ISO 10303-21 syntax requires specific character codes
		return matchAsciiDecimal( static_cast<int>( c ) ) &&
			   c != '/' && c != '*' && c != '\\' && c != '!' &&
			   c != '\'' && c != '"' && c != ';' && c != '(' && c != ')';
	}

	bool VIS::isISOString( std::string_view value )
	{
		SPDLOG_DEBUG( "Checking if string_view is ISO compliant: '{}'", value );

		if ( value.empty() )
		{
			SPDLOG_INFO( "Empty string_view is ISO compliant" );
			return true;
		}

		for ( char ch : value )
		{
			if ( !isISOString( ch ) )
			{
				SPDLOG_ERROR( "Character '{}' (code {}) makes string not ISO compliant",
					ch, static_cast<int>( ch ) );
				return false;
			}
		}

		SPDLOG_DEBUG( "String_view is ISO compliant" );
		return true;
	}

	bool VIS::isISOString( const std::string& value )
	{
		return isISOString( std::string_view( value ) );
	}

	bool VIS::isISOString( const std::stringstream& builder )
	{
		std::string str = builder.str();
		return isISOString( std::string_view( str ) );
	}

	bool VIS::isISOLocalIdString( const std::string& value )
	{
		SPDLOG_DEBUG( "Checking if string is ISO local ID compliant: '{}'", value );
		return !value.empty() && matchISOLocalIdString( value );
	}

	bool VIS::matchAsciiDecimal( int code )
	{
		// Numbers (48-57)
		if ( code >= 48 && code <= 57 )
			return true;
		// Uppercase A-Z (65-90)
		if ( code >= 65 && code <= 90 )
			return true;
		// Lowercase a-z (97-122)
		if ( code >= 97 && code <= 122 )
			return true;
		// Special chars: "-", ".", "_", "~"
		return ( code == 45 || code == 46 || code == 95 || code == 126 );
	}
}
