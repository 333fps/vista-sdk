/**
 * @file VIS.h
 * @brief Vessel Information Structure (VIS) interface and implementation
 *
 * Provides access to VIS data like GMOD, Codebooks, and Locations,
 * along with version conversion utilities and ISO string validation.
 */

#pragma once

#include "CodebooksDto.h"
#include "GmodVersioning.h"
#include "Codebooks.h"
#include "Gmod.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Forward declarations
	//-------------------------------------------------------------------------

	enum class VisVersion;
	class GmodNode;
	class GmodPath;
	class LocalId;
	class LocalIdBuilder;
	class Locations;
	class GmodDto;
	class GmodVersioningDto;
	class LocationsDto;

	/**
	 * @brief Interface for VIS (Vessel Information Structure) operations
	 *
	 * This interface defines the core operations for accessing and manipulating
	 * Vessel Information Structure (VIS) data across different versions.
	 */
	class IVIS
	{
	public:
		/**
		 * @brief Virtual destructor
		 */
		virtual ~IVIS() = default;

		//-------------------------------------------------------------------------
		// Core Data Access Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the GMOD for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The GMOD (Generic Product Model) for the specified version
		 * @throws std::invalid_argument If the VIS version is invalid
		 * @note This method is const as it only provides read access to data
		 */
		virtual const Gmod& gmod( VisVersion visVersion ) const = 0;

		/**
		 * @brief Get the codebooks for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The codebooks for the specified version
		 * @throws std::invalid_argument If the VIS version is invalid
		 * @note This method is non-const because it may update the internal cache
		 */
		virtual const Codebooks& codebooks( VisVersion visVersion ) = 0;

		/**
		 * @brief Get the locations for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The locations for the specified version
		 * @throws std::invalid_argument If the VIS version is invalid
		 * @note This method is non-const because it may update the internal cache
		 */
		virtual const Locations& locations( VisVersion visVersion ) = 0;

		//-------------------------------------------------------------------------
		// Batch Data Access Methods
		//-------------------------------------------------------------------------

		virtual std::unordered_map<VisVersion, const Codebooks*> codebooksMap( const std::vector<VisVersion>& visVersions ) = 0;

		virtual std::unordered_map<VisVersion, const Gmod*> gmodsMap( const std::vector<VisVersion>& visVersions ) const = 0;

		virtual std::unordered_map<VisVersion, const Locations*> locationsMap( const std::vector<VisVersion>& visVersions ) = 0;

		/**
		 * @brief Get all available VIS versions
		 * @return Vector of all supported VIS versions
		 * @note This method is non-const because it may update the internal cache
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
		 * @note This method is non-const because it may update internal caches when loading required data
		 */
		virtual std::optional<GmodNode> convertNode( VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) = 0;

		/**
		 * @brief Convert a GMOD path from one VIS version to another
		 * @param sourceVersion The source VIS version
		 * @param sourcePath The path to convert
		 * @param targetVersion The target VIS version
		 * @return The converted path, or none if conversion is not possible
		 * @note This method is non-const because it may update internal caches when loading required data
		 */
		virtual std::optional<GmodPath> convertPath( VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) = 0;
	};

	/**
	 * @brief Implementation of the IVIS interface for Vessel Information Structure operations
	 *
	 * This class implements the IVIS interface and provides efficient access to VIS data
	 * using thread-safe caching mechanisms. It follows the Singleton pattern to ensure
	 * consistent data access throughout the application.
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
		// Singleton Access
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the singleton instance of VIS
		 * @return Reference to the singleton instance
		 */
		static VIS& instance();

		//-------------------------------------------------------------------------
		// IVIS Interface Implementation
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the GMOD for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The GMOD for the specified version
		 * @throws std::invalid_argument If the VIS version is invalid
		 */
		virtual const Gmod& gmod( VisVersion visVersion ) const override;

		/**
		 * @brief Get the codebooks for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The codebooks for the specified version
		 * @throws std::invalid_argument If the VIS version is invalid
		 */
		virtual const Codebooks& codebooks( VisVersion visVersion ) override;

		/**
		 * @brief Get the locations for a specific VIS version
		 * @param visVersion The VIS version
		 * @return The locations for the specified version
		 * @throws std::invalid_argument If the VIS version is invalid
		 */
		virtual const Locations& locations( VisVersion visVersion ) override;

		virtual std::unordered_map<VisVersion, const Codebooks*> codebooksMap( const std::vector<VisVersion>& visVersions ) override;

		virtual std::unordered_map<VisVersion, const Gmod*> gmodsMap( const std::vector<VisVersion>& visVersions ) const override;

		virtual std::unordered_map<VisVersion, const Locations*> locationsMap( const std::vector<VisVersion>& visVersions ) override;

		/**
		 * @brief Get all available VIS versions
		 * @return Vector of all supported VIS versions
		 */
		virtual std::vector<VisVersion> visVersions() override;

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
		 *
		 * This overload assumes the source node's VIS version is known from the node itself.
		 *
		 * @param sourceNode The node to convert
		 * @param targetVersion The target VIS version
		 * @param sourceParent Optional parent node for context
		 * @return The converted node, or none if conversion is not possible
		 */
		std::optional<GmodNode> convertNode( const GmodNode& sourceNode, VisVersion targetVersion, const GmodNode* sourceParent = nullptr );

		/**
		 * @brief Convert a GMOD path to a different VIS version
		 *
		 * This overload assumes the source path's VIS version is known from the path itself.
		 *
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
		 * @throws std::runtime_error If the versioning data cannot be loaded
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
		static bool isISOString( std::string_view value );

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
		// Constructors & Assignment Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Constructor
		 * @details Initializes the VIS singleton with empty caches
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
		 * @brief Move constructor (deleted)
		 * @details VIS is a singleton and shouldn't be moved
		 */
		VIS( VIS&& ) = delete;

		/**
		 * @brief Move assignment operator (deleted)
		 * @details VIS is a singleton and shouldn't be moved
		 */
		VIS& operator=( VIS&& ) = delete;

		/**
		 * @brief Virtual destructor
		 */
		~VIS() override = default;

		//-------------------------------------------------------------------------
		// Private Inner Types
		//-------------------------------------------------------------------------

		/**
		 * @brief Simple template-based cache implementation
		 *
		 * This class provides thread-safe caching for VIS data, with automatic
		 * cleanup of expired entries and LRU-based eviction when the cache grows too large.
		 *
		 * @tparam K The key type
		 * @tparam V The value type
		 */
		template <typename K, typename V>
		class Cache final
		{
		public:
			/**
			 * @brief Constructor
			 */
			Cache() : m_lastCleanup{ std::chrono::steady_clock::now() } {}

			/**
			 * @brief Copy constructor (deleted)
			 * @details Cache contains non-copyable mutex
			 */
			Cache( const Cache& ) = delete;

			/**
			 * @brief Copy assignment operator (deleted)
			 * @details Cache contains non-copyable mutex
			 */
			Cache& operator=( const Cache& ) = delete;

			/**
			 * @brief Move constructor (deleted)
			 * @details Cache contains non-movable mutex
			 */
			Cache( Cache&& ) = delete;

			/**
			 * @brief Move assignment operator (deleted)
			 * @details Cache contains non-movable mutex
			 */
			Cache& operator=( Cache&& ) = delete;

			/**
			 * @brief Get an item from the cache or create it if not found
			 *
			 * This method is thread-safe and manages cache expiration and eviction.
			 *
			 * @param key The cache key
			 * @param factory Function to create the value if not found in cache
			 * @return Reference to the cached value
			 */
			V& getOrCreate( const K& key, std::function<V()> factory ) const
			{
				const auto now = std::chrono::steady_clock::now();

				std::scoped_lock lock( m_mutex );

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
					auto [inserted_it, success] = m_cache.emplace( key, CacheItem{ std::move( result ), now } );
					SPDLOG_TRACE( "Cache miss for key. Created and inserted." );

					return inserted_it->second.value;
				}

				it->second.lastAccess = now;

				SPDLOG_TRACE( "Cache hit for key." );

				return it->second.value;
			}

		private:
			/**
			 * @brief Cache item containing value and last access time
			 */
			struct CacheItem final
			{
				V value;
				std::chrono::steady_clock::time_point lastAccess;
			};

			/**
			 * @brief Remove expired cache entries
			 * @details Assumes the mutex is locked by the caller
			 */
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

				SPDLOG_TRACE( "Cache cleanup performed." );
			}

			/**
			 * @brief Remove the least recently used cache entry
			 * @details Assumes the mutex is locked by the caller
			 */
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

				SPDLOG_TRACE( "Cache eviction performed (removed oldest)." );
			}

			mutable std::mutex m_mutex;
			mutable std::unordered_map<K, CacheItem> m_cache;
			mutable std::chrono::steady_clock::time_point m_lastCleanup;
		};

		//-------------------------------------------------------------------------
		// Private Static Members
		//-------------------------------------------------------------------------

		/**
		 * @brief GMOD versioning data path
		 */
		static const std::string m_versioning;

		//-------------------------------------------------------------------------
		// Private Member Variables - Caches
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
