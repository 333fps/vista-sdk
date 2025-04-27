#pragma once

#include "CodebooksDto.h"
#include "GmodVersioning.h"
#include "Codebooks.h"
#include "Gmod.h"

namespace dnv::vista::sdk
{
	class GmodNode;
	class GmodPath;
	class LocalId;
	class LocalIdBuilder;
	class Locations;
	enum class VisVersion;
	class GmodDto;
	class GmodVersioningDto;
	class LocationsDto;

	/**
	 * @brief Interface for VIS (Vessel Information Structure) operations
	 */
	class IVIS
	{
	public:
		//-------------------------------------------------------------------------
		// Destructor
		//-------------------------------------------------------------------------
		virtual ~IVIS() = default;

		//-------------------------------------------------------------------------
		// Core Data Access Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the GMOD for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The GMOD (Generic Product Model) for the specified version
		 */
		virtual Gmod gmod( VisVersion visVersion ) const = 0;

		/**
		 * @brief Get the codebooks for a specific VIS version
		 * @param visversion The VIS version
		 * @return The codebooks for the specified version
		 */
		virtual Codebooks codebooks( VisVersion visversion ) = 0;

		/**
		 * @brief Get the locations for a specific VIS version
		 * @param visversion The VIS version
		 * @return The locations for the specified version
		 */
		virtual Locations locations( VisVersion visversion ) = 0;

		//-------------------------------------------------------------------------
		// Batch Data Access Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get a map of codebooks for multiple VIS versions
		 * @param visVersions The VIS versions to retrieve codebooks for
		 * @return Map of VIS versions to their corresponding codebooks
		 */
		virtual std::unordered_map<VisVersion, Codebooks> codebooksMap( const std::vector<VisVersion>& visVersions ) = 0;

		/**
		 * @brief Get a map of GMODs for multiple VIS versions
		 * @param visVersions The VIS versions to retrieve GMODs for
		 * @return Map of VIS versions to their corresponding GMODs
		 */
		virtual std::unordered_map<VisVersion, Gmod> gmodsMap( const std::vector<VisVersion>& visVersions ) = 0;

		/**
		 * @brief Get a map of locations for multiple VIS versions
		 * @param visVersions The VIS versions to retrieve locations for
		 * @return Map of VIS versions to their corresponding locations
		 */
		virtual std::unordered_map<VisVersion, Locations> locationsMap( const std::vector<VisVersion>& visVersions ) = 0;

		/**
		 * @brief Get all available VIS versions
		 * @return Vector of all supported VIS versions
		 */
		virtual std::vector<VisVersion> visVersions() = 0;

		//-------------------------------------------------------------------------
		// Conversion Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Convert a GMOD node from one VIS version to another
		 * @param sourceVersion The source VIS version
		 * @param sourceNode The node to convert
		 * @param targetVersion The target VIS version
		 * @return The converted node, or none if conversion is not possible
		 */
		virtual std::optional<GmodNode> convertNode( VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) = 0;

		/**
		 * @brief Convert a GMOD path from one VIS version to another
		 * @param sourceVersion The source VIS version
		 * @param sourcePath The path to convert
		 * @param targetVersion The target VIS version
		 * @return The converted path, or none if conversion is not possible
		 */
		virtual std::optional<GmodPath> convertPath( VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) = 0;
	};

	/**
	 * @brief Implementation of the IVIS interface for Vessel Information Structure operations
	 */
	class VIS final : public IVIS
	{
	public:
		//-------------------------------------------------------------------------
		// Constants
		//-------------------------------------------------------------------------

		/**
		 * @brief The latest supported VIS version
		 */
		static const VisVersion LatestVisVersion;

		//-------------------------------------------------------------------------
		// Constructors & Destructors
		//-------------------------------------------------------------------------

		/**
		 * @brief Constructor
		 */
		VIS();

		/**
		 * @brief Copy constructor (deleted)
		 * @details VIS is a singleton and shouldn't be copied
		 */
		VIS( const VIS& ) = delete;

		/**
		 * @brief Copy assignment operator (deleted)
		 * @details VIS is a singleton and shouldn't be assigned
		 */
		VIS& operator=( const VIS& ) = delete;

		/**
		 * @brief Virtual destructor
		 */
		virtual ~VIS() = default;

		//-------------------------------------------------------------------------
		// Singleton Access
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the singleton instance of VIS
		 * @return Reference to the singleton instance
		 */
		static VIS& instance();

		//-------------------------------------------------------------------------
		// Core Data Access Methods (IVIS implementation)
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the GMOD for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The GMOD for the specified version
		 * @throws std::invalid_argument If the VIS version is invalid
		 */
		virtual Gmod gmod( VisVersion visVersion ) const override;

		/**
		 * @brief Get the codebooks for a specific VIS version
		 * @param visversion The VIS version
		 * @return The codebooks for the specified version
		 */
		virtual Codebooks codebooks( VisVersion visversion ) override;

		/**
		 * @brief Get the locations for a specific VIS version
		 * @param visversion The VIS version
		 * @return The locations for the specified version
		 */
		virtual Locations locations( VisVersion visversion ) override;

		//-------------------------------------------------------------------------
		// Batch Data Access Methods (IVIS implementation)
		//-------------------------------------------------------------------------

		/**
		 * @brief Get a map of codebooks for multiple VIS versions
		 * @param visVersions The VIS versions to retrieve codebooks for
		 * @return Map of VIS versions to their corresponding codebooks
		 * @throws std::invalid_argument If any VIS version is invalid
		 */
		virtual std::unordered_map<VisVersion, Codebooks> codebooksMap( const std::vector<VisVersion>& visVersions ) override;

		/**
		 * @brief Get a map of GMODs for multiple VIS versions
		 * @param visVersions The VIS versions to retrieve GMODs for
		 * @return Map of VIS versions to their corresponding GMODs
		 * @throws std::invalid_argument If any VIS version is invalid
		 */
		virtual std::unordered_map<VisVersion, Gmod> gmodsMap( const std::vector<VisVersion>& visVersions ) override;

		/**
		 * @brief Get a map of locations for multiple VIS versions
		 * @param visVersions The VIS versions to retrieve locations for
		 * @return Map of VIS versions to their corresponding locations
		 * @throws std::invalid_argument If any VIS version is invalid
		 */
		virtual std::unordered_map<VisVersion, Locations> locationsMap( const std::vector<VisVersion>& visVersions ) override;

		/**
		 * @brief Get all available VIS versions
		 * @return Vector of all supported VIS versions
		 */
		virtual std::vector<VisVersion> visVersions() override;

		//-------------------------------------------------------------------------
		// Conversion Methods (IVIS implementation)
		//-------------------------------------------------------------------------

		/**
		 * @brief Convert a GMOD node from one VIS version to another
		 * @param sourceVersion The source VIS version
		 * @param sourceNode The node to convert
		 * @param targetVersion The target VIS version
		 * @return The converted node, or none if conversion is not possible
		 */
		virtual std::optional<GmodNode> convertNode( VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) override;

		/**
		 * @brief Convert a GMOD path from one VIS version to another
		 * @param sourceVersion The source VIS version
		 * @param sourcePath The path to convert
		 * @param targetVersion The target VIS version
		 * @return The converted path, or none if conversion is not possible
		 */
		virtual std::optional<GmodPath> convertPath( VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) override;

		//-------------------------------------------------------------------------
		// Extended Conversion Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Convert a GMOD node to a different VIS version
		 * @param sourceNode The node to convert
		 * @param targetVersion The target VIS version
		 * @param sourceParent Optional parent node for context
		 * @return The converted node, or none if conversion is not possible
		 */
		std::optional<GmodNode> convertNode( const GmodNode& sourceNode, VisVersion targetVersion, const GmodNode* sourceParent = nullptr );

		/**
		 * @brief Convert a GMOD path to a different VIS version
		 * @param sourcePath The path to convert
		 * @param targetVersion The target VIS version
		 * @return The converted path, or none if conversion is not possible
		 */
		std::optional<GmodPath> convertPath( const GmodPath& sourcePath, VisVersion targetVersion );

		/**
		 * @brief Convert a LocalIdBuilder to a different VIS version
		 * @param sourceLocalId The LocalIdBuilder to convert
		 * @param targetVersion The target VIS version
		 * @return The converted LocalIdBuilder, or none if conversion is not possible
		 */
		std::optional<LocalIdBuilder> convertLocalId( const LocalIdBuilder& sourceLocalId, VisVersion targetVersion );

		/**
		 * @brief Convert a LocalId to a different VIS version
		 * @param sourceLocalId The LocalId to convert
		 * @param targetVersion The target VIS version
		 * @return The converted LocalId, or none if conversion is not possible
		 */
		std::optional<LocalId> convertLocalId( const LocalId& sourceLocalId, VisVersion targetVersion );

		//-------------------------------------------------------------------------
		// DTO Access Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the GMOD DTO for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The GMOD DTO
		 * @throws std::runtime_error If the GMOD cannot be loaded
		 */
		GmodDto gmodDto( VisVersion visVersion ) const;

		/**
		 * @brief Load the GMOD DTO for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The GMOD DTO, or none if it cannot be loaded
		 */
		static std::optional<GmodDto> loadGmodDto( VisVersion visVersion );

		/**
		 * @brief Get the GMOD versioning DTOs
		 * @return Map of versioning DTOs
		 * @throws std::invalid_argument If the versioning data cannot be loaded
		 */
		std::unordered_map<std::string, GmodVersioningDto> gmodVersioningDto();

		/**
		 * @brief Get the GMOD versioning
		 * @return The GMOD versioning object
		 */
		GmodVersioning gmodVersioning();

		/**
		 * @brief Get the codebooks DTO for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The codebooks DTO
		 * @throws std::runtime_error If the codebooks cannot be loaded
		 */
		CodebooksDto codebooksDto( VisVersion visVersion );

		/**
		 * @brief Get the locations DTO for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The locations DTO
		 * @throws std::runtime_error If the locations cannot be loaded
		 */
		LocationsDto locationsDto( VisVersion visVersion );

		//-------------------------------------------------------------------------
		// ISO String Validation Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Check if a string matches ISO local ID format
		 * @param value The string to check
		 * @return True if the string matches ISO local ID format
		 */
		static bool matchISOLocalIdString( const std::string& value );

		/**
		 * @brief Check if a string stream is a valid ISO local ID string
		 * @param builder The string stream to check
		 * @return True if the string stream is a valid ISO local ID string
		 */
		static bool matchISOLocalIdString( const std::stringstream& builder );

		/**
		 * @brief Check if a string view matches ISO local ID format
		 * @param value The string view to check
		 * @return True if the string view matches ISO local ID format
		 */
		static bool matchISOLocalIdString( std::string_view value );

		/**
		 * @brief Check if a character is valid for ISO strings
		 * @param c The character to check
		 * @return True if the character is valid for ISO strings
		 */
		static bool isISOString( char c );

		/**
		 * @brief Check if a string view contains only valid ISO characters
		 * @param value The string view to check
		 * @return True if the string view contains only valid ISO characters
		 */
		static bool isISOString( const std::string_view value );

		/**
		 * @brief Check if a string contains only valid ISO characters
		 * @param value The string to check
		 * @return True if the string contains only valid ISO characters
		 */
		static bool isISOString( const std::string& value );

		/**
		 * @brief Check if a string stream contains only valid ISO characters
		 * @param builder The string stream to check
		 * @return True if the string stream contains only valid ISO characters
		 */
		static bool isISOString( const std::stringstream& builder );

		/**
		 * @brief Check if a string is a valid ISO local ID string
		 * @param value The string to check
		 * @return True if the string is a valid ISO local ID string
		 */
		static bool isISOLocalIdString( const std::string& value );

		/**
		 * @brief Check if a character code is valid for ISO strings
		 * @param code The ASCII code to check
		 * @return True if the code is valid for ISO strings
		 */
		static bool matchAsciiDecimal( int code );

	private:
		//-------------------------------------------------------------------------
		// Private Inner Types
		//-------------------------------------------------------------------------

		/**
		 * @brief Simple template-based cache implementation
		 * @tparam K The key type
		 * @tparam V The value type
		 */
		template <typename K, typename V>
		class Cache final
		{
		public:
			Cache() : m_lastCleanup( std::chrono::steady_clock::now() ) {}

			V& getOrCreate( const K& key, std::function<V()> factory ) const
			{
				const auto now = std::chrono::steady_clock::now();
				if ( now - m_lastCleanup > std::chrono::hours( 1 ) )
				{
					cleanupCache();
					m_lastCleanup = now;
				}

				auto it = m_cache.find( key );
				if ( it == m_cache.end() )
				{
					if ( m_cache.size() >= 10 )
					{
						removeOldestEntry();
					}

					auto result = factory();
					auto [inserted_it, success] = m_cache.emplace(
						key, CacheItem{ std::move( result ), now } );
					return inserted_it->second.value;
				}

				it->second.lastAccess = now;
				return it->second.value;
			}

		private:
			struct CacheItem final
			{
				V value;
				std::chrono::steady_clock::time_point lastAccess;
			};

			void cleanupCache() const
			{
				const auto now = std::chrono::steady_clock::now();
				for ( auto it = m_cache.begin(); it != m_cache.end(); )
				{
					if ( now - it->second.lastAccess > std::chrono::hours( 1 ) )
						it = m_cache.erase( it );
					else
						++it;
				}
			}

			void removeOldestEntry() const
			{
				if ( m_cache.empty() )
					return;
				auto oldest = m_cache.begin();
				for ( auto it = std::next( m_cache.begin() ); it != m_cache.end(); ++it )
				{
					if ( it->second.lastAccess < oldest->second.lastAccess )
					{
						oldest = it;
					}
				}
				m_cache.erase( oldest );
			}

			mutable std::unordered_map<K, CacheItem> m_cache;
			mutable std::chrono::steady_clock::time_point m_lastCleanup;
		};

		//-------------------------------------------------------------------------
		// Private Static Members
		//-------------------------------------------------------------------------

		static const std::string m_versioning;

		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		Cache<VisVersion, GmodDto> m_gmodDtoCache;
		Cache<VisVersion, Gmod> m_gmodCache;
		Cache<VisVersion, CodebooksDto> m_codebooksDtoCache;
		Cache<VisVersion, Codebooks> m_codebooksCache;
		Cache<VisVersion, LocationsDto> m_locationsDtoCache;
		Cache<VisVersion, Locations> m_locationsCache;
		Cache<std::string, std::unordered_map<std::string, GmodVersioningDto>> m_gmodVersioningDtoCache;
		Cache<std::string, GmodVersioning> m_gmodVersioningCache;
	};
}
