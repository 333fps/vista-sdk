/**
 * @file VIS.cpp
 * @brief Implementation of Vessel Information Structure (VIS) interface
 */

#include "pch.h"

#include "dnv/vista/sdk/VIS.h"

#include "dnv/vista/sdk/Codebooks.h"
#include "dnv/vista/sdk/CodebooksDto.h"
#include "dnv/vista/sdk/EmbeddedResource.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodDto.h"
#include "dnv/vista/sdk/GmodVersioning.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	namespace
	{
		constexpr const char* VERSIONING = "<versioning>";
	}

	//=====================================================================
	// VIS singleton
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	VIS::VIS() : IVIS{}
	{
	}

	//----------------------------------------------
	// Singleton access
	//----------------------------------------------

	VIS& VIS::instance()
	{
		static VIS instance;
		return instance;
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	std::vector<VisVersion> VIS::visVersions()
	{
		return VisVersionExtensions::allVersions();
	}

	GmodVersioning VIS::gmodVersioning()
	{
		{
			std::shared_lock lock( m_cacheMutex );
			if ( auto it = m_gmodVersioningCache.find( VERSIONING ); it != m_gmodVersioningCache.end() )
			{
				return it->second;
			}
		}

		std::unique_lock lock( m_cacheMutex );

		if ( auto it = m_gmodVersioningCache.find( VERSIONING ); it != m_gmodVersioningCache.end() )
		{
			return it->second;
		}

		auto dto = EmbeddedResource::gmodVersioning();
		if ( !dto )
		{
			throw std::runtime_error( "Failed to load GMOD versioning data" );
		}

		auto [inserted_it, success] = m_gmodVersioningCache.emplace( VERSIONING, GmodVersioning( *dto ) );
		return inserted_it->second;
	}

	VisVersion IVIS::latestVisVersion() const noexcept
	{
		return VisVersionExtensions::latestVersion();
	}

	const Gmod& VIS::gmod( VisVersion visVersion ) const
	{
		if ( !VisVersionExtensions::isValid( visVersion ) )
		{
			throw std::invalid_argument( "Invalid VIS version: " + std::to_string( static_cast<int>( visVersion ) ) );
		}

		{ /* Hot path: Try shared lock first */
			std::shared_lock lock( m_cacheMutex );
			if ( auto it = m_gmodCache.find( visVersion ); it != m_gmodCache.end() )
			{
				return it->second;
			}
		}

		std::unique_lock lock( m_cacheMutex );

		if ( auto it = m_gmodCache.find( visVersion ); it != m_gmodCache.end() )
		{
			return it->second;
		}

		auto dto = loadGmodDto( visVersion );
		if ( !dto )
		{
			throw std::runtime_error( "Failed to load GMOD DTO for version: " + VisVersionExtensions::toVersionString( visVersion ) );
		}

		auto [inserted_it, success] = m_gmodCache.emplace( visVersion, Gmod( visVersion, std::move( *dto ) ) );
		return inserted_it->second;
	}

	const Codebooks& VIS::codebooks( VisVersion visVersion )
	{
		if ( !VisVersionExtensions::isValid( visVersion ) )
		{
			throw std::invalid_argument( "Invalid VIS version: " + std::to_string( static_cast<int>( visVersion ) ) );
		}

		{
			std::shared_lock lock( m_cacheMutex );
			if ( auto it = m_codebooksCache.find( visVersion ); it != m_codebooksCache.end() )
			{
				return it->second;
			}
		}

		std::unique_lock lock( m_cacheMutex );

		if ( auto it = m_codebooksCache.find( visVersion ); it != m_codebooksCache.end() )
		{
			return it->second;
		}

		auto dto = EmbeddedResource::codebooks( VisVersionExtensions::toVersionString( visVersion ) );
		if ( !dto )
		{
			throw std::runtime_error( "Failed to load codebooks DTO for version: " + VisVersionExtensions::toVersionString( visVersion ) );
		}

		auto [inserted_it, success] = m_codebooksCache.emplace( visVersion, Codebooks( visVersion, *dto ) );
		return inserted_it->second;
	}

	const Locations& VIS::locations( VisVersion visVersion )
	{
		if ( !VisVersionExtensions::isValid( visVersion ) )
		{
			throw std::invalid_argument( "Invalid VIS version: " + std::to_string( static_cast<int>( visVersion ) ) );
		}

		{
			std::shared_lock lock( m_cacheMutex );
			if ( auto it = m_locationsCache.find( visVersion ); it != m_locationsCache.end() )
			{
				return it->second;
			}
		}

		std::unique_lock lock( m_cacheMutex );

		if ( auto it = m_locationsCache.find( visVersion ); it != m_locationsCache.end() )
		{
			return it->second;
		}

		auto dto = EmbeddedResource::locations( VisVersionExtensions::toVersionString( visVersion ) );
		if ( !dto )
		{
			throw std::runtime_error( "Failed to load locations DTO for version: " + VisVersionExtensions::toVersionString( visVersion ) );
		}

		auto [inserted_it, success] = m_locationsCache.emplace( visVersion, Locations( visVersion, *dto ) );
		return inserted_it->second;
	}

	std::unordered_map<VisVersion, Codebooks> VIS::codebooksMap( const std::vector<VisVersion>& visVersions )
	{
		for ( auto version : visVersions )
		{
			if ( !VisVersionExtensions::isValid( version ) )
			{
				throw std::invalid_argument( "Invalid VIS version provided: " + VisVersionExtensions::toVersionString( version ) );
			}
		}

		std::unordered_map<VisVersion, Codebooks> result;
		result.reserve( visVersions.size() );

		for ( auto version : visVersions )
		{
			auto dto = EmbeddedResource::codebooks( VisVersionExtensions::toVersionString( version ) );
			if ( !dto )
			{
				throw std::runtime_error( "Failed to load codebooks DTO for version: " + VisVersionExtensions::toVersionString( version ) );
			}

			result.emplace( std::piecewise_construct,
				std::forward_as_tuple( version ),
				std::forward_as_tuple( version, std::move( *dto ) ) );
		}

		return result;
	}

	std::unordered_map<VisVersion, Locations> VIS::locationsMap( const std::vector<VisVersion>& visVersions )
	{
		for ( auto version : visVersions )
		{
			if ( !VisVersionExtensions::isValid( version ) )
			{
				throw std::invalid_argument( "Invalid VIS version provided: " + VisVersionExtensions::toVersionString( version ) );
			}
		}

		std::unordered_map<VisVersion, Locations> result;
		result.reserve( visVersions.size() );

		for ( auto version : visVersions )
		{
			auto dto = EmbeddedResource::locations( VisVersionExtensions::toVersionString( version ) );
			if ( !dto )
			{
				throw std::runtime_error( "Failed to load locations DTO for version: " + VisVersionExtensions::toVersionString( version ) );
			}

			result.emplace( std::piecewise_construct,
				std::forward_as_tuple( version ),
				std::forward_as_tuple( version, std::move( *dto ) ) );
		}

		return result;
	}

	std::unordered_map<VisVersion, Gmod> VIS::gmodsMap( const std::vector<VisVersion>& visVersions ) const
	{
		for ( auto version : visVersions )
		{
			if ( !VisVersionExtensions::isValid( version ) )
			{
				throw std::invalid_argument( "Invalid VIS version provided: " + VisVersionExtensions::toVersionString( version ) );
			}
		}

		std::unordered_map<VisVersion, Gmod> result;
		result.reserve( visVersions.size() );

		for ( auto version : visVersions )
		{
			auto dto = loadGmodDto( version );
			if ( !dto )
			{
				throw std::runtime_error( "Failed to load GMOD DTO for version: " + VisVersionExtensions::toVersionString( version ) );
			}

			result.emplace( std::piecewise_construct,
				std::forward_as_tuple( version ),
				std::forward_as_tuple( version, std::move( *dto ) ) );
		}

		return result;
	}

	//----------------------------------------------
	// DTO accessors
	//----------------------------------------------

	GmodDto VIS::gmodDto( VisVersion visVersion ) const
	{
		{
			std::shared_lock lock( m_cacheMutex );
			if ( auto it = m_gmodDtoCache.find( visVersion ); it != m_gmodDtoCache.end() )
			{
				return it->second;
			}
		}

		std::unique_lock lock( m_cacheMutex );

		if ( auto it = m_gmodDtoCache.find( visVersion ); it != m_gmodDtoCache.end() )
		{
			return it->second;
		}

		auto dto = loadGmodDto( visVersion );
		if ( !dto )
		{
			throw std::runtime_error( "Failed to load GMOD DTO for version: " + VisVersionExtensions::toVersionString( visVersion ) );
		}

		auto [inserted_it, success] = m_gmodDtoCache.emplace( visVersion, *dto );
		return inserted_it->second;
	}

	std::optional<GmodDto> VIS::loadGmodDto( VisVersion visVersion )
	{
		return EmbeddedResource::gmod( VisVersionExtensions::toVersionString( visVersion ) );
	}

	std::unordered_map<std::string, GmodVersioningDto> VIS::gmodVersioningDto()
	{
		{
			std::shared_lock lock( m_cacheMutex );
			if ( auto it = m_gmodVersioningDtoCache.find( VERSIONING ); it != m_gmodVersioningDtoCache.end() )
			{
				return it->second;
			}
		}

		std::unique_lock lock( m_cacheMutex );

		if ( auto it = m_gmodVersioningDtoCache.find( VERSIONING ); it != m_gmodVersioningDtoCache.end() )
		{
			return it->second;
		}

		auto dto = EmbeddedResource::gmodVersioning();
		if ( !dto )
		{
			throw std::runtime_error( "Failed to load GMOD versioning data" );
		}

		auto [inserted_it, success] = m_gmodVersioningDtoCache.emplace( VERSIONING, *dto );
		return inserted_it->second;
	}

	CodebooksDto VIS::codebooksDto( VisVersion visVersion )
	{
		{
			std::shared_lock lock( m_cacheMutex );
			if ( auto it = m_codebooksDtoCache.find( visVersion ); it != m_codebooksDtoCache.end() )
			{
				return it->second;
			}
		}

		std::unique_lock lock( m_cacheMutex );

		if ( auto it = m_codebooksDtoCache.find( visVersion ); it != m_codebooksDtoCache.end() )
		{
			return it->second;
		}

		auto dto = EmbeddedResource::codebooks( VisVersionExtensions::toVersionString( visVersion ) );
		if ( !dto )
		{
			throw std::runtime_error( "Failed to load codebooks DTO for version: " + VisVersionExtensions::toVersionString( visVersion ) );
		}

		auto [inserted_it, success] = m_codebooksDtoCache.emplace( visVersion, *dto );
		return inserted_it->second;
	}

	LocationsDto VIS::locationsDto( VisVersion visVersion )
	{
		{
			std::shared_lock lock( m_cacheMutex );
			if ( auto it = m_locationsDtoCache.find( visVersion ); it != m_locationsDtoCache.end() )
			{
				return it->second;
			}
		}

		std::unique_lock lock( m_cacheMutex );

		if ( auto it = m_locationsDtoCache.find( visVersion ); it != m_locationsDtoCache.end() )
		{
			return it->second;
		}

		auto dto = EmbeddedResource::locations( VisVersionExtensions::toVersionString( visVersion ) );
		if ( !dto )
		{
			throw std::runtime_error( "Failed to load locations DTO for version: " + VisVersionExtensions::toVersionString( visVersion ) );
		}

		auto [inserted_it, success] = m_locationsDtoCache.emplace( visVersion, *dto );
		return inserted_it->second;
	}

	//----------------------------------------------
	// Conversion
	//----------------------------------------------

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

	//----------------------------------------------
	// ISO string validation methods
	//----------------------------------------------

	bool VIS::matchISOLocalIdString( const std::stringstream& builder ) noexcept
	{
		return matchISOLocalIdString( std::string_view( builder.str() ) );
	}

	bool VIS::matchISOLocalIdString( std::string_view value ) noexcept
	{
		const char* data = value.data();
		const char* end = data + value.size();

		for ( const char* charPtr = data; charPtr != end; ++charPtr )
		{
			if ( *charPtr == '/' )
			{
				continue;
			}

			if ( !matchAsciiDecimal( static_cast<int>( *charPtr ) ) )
			{
				SPDLOG_ERROR( "Character '{}' is not ISO compliant", *charPtr );

				return false;
			}
		}

		return true;
	}

	bool VIS::isISOString( std::string_view value ) noexcept
	{
		if ( value.empty() )
		{
			return true;
		}

		const char* data = value.data();
		const char* end = data + value.size();

		for ( const char* charPtr = data; charPtr != end; ++charPtr )
		{
			if ( !matchAsciiDecimal( static_cast<int>( *charPtr ) ) )
			{
				SPDLOG_ERROR( "Character '{}' (code {}) makes string not ISO compliant", *charPtr, static_cast<int>( *charPtr ) );

				return false;
			}
		}

		return true;
	}

	bool VIS::isISOString( const std::string& value ) noexcept
	{
		return isISOString( std::string_view( value ) );
	}

	bool VIS::isISOString( const std::stringstream& builder ) noexcept
	{
		std::string str = builder.str();

		return isISOString( std::string_view( str ) );
	}

	bool VIS::isISOLocalIdString( const std::string& value ) noexcept
	{
		return !value.empty() && matchISOLocalIdString( value );
	}

	bool VIS::isISOString( char c ) noexcept
	{
		return matchAsciiDecimal( static_cast<int>( c ) );
	}

	bool VIS::matchAsciiDecimal( int code ) noexcept
	{
		/* Numbers (48-57) */
		if ( code >= 48 && code <= 57 )
		{
			return true;
		}

		/* Uppercase A-Z (65-90) */
		if ( code >= 65 && code <= 90 )
		{
			return true;
		}

		/* Lowercase a-z (97-122) */
		if ( code >= 97 && code <= 122 )
		{
			return true;
		}

		/* Special chars: "-", ".", "_", "~" */
		return ( code == 45 || code == 46 || code == 95 || code == 126 );
	}
}
