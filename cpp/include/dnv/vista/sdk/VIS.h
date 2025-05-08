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
	//=====================================================================
	// Forward declarations
	//=====================================================================

	enum class VisVersion;
	class GmodNode;
	class GmodPath;
	class LocalId;
	class LocalIdBuilder;
	class Locations;
	class GmodDto;
	class GmodVersioningDto;
	class LocationsDto;

	//=====================================================================
	// IVIS Interface
	//=====================================================================

	/**
	 * @brief Interface for VIS (Vessel Information Structure) operations.
	 *
	 * This interface defines the core operations for accessing and manipulating
	 * Vessel Information Structure (VIS) data across different versions.
	 * Implementations are expected to handle data retrieval, caching, and version conversion.
	 */
	class IVIS
	{
	protected:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/** @brief Default constructor. */
		IVIS() = default;

		/** @brief Copy constructor */
		IVIS( const IVIS& ) = delete;

		/** @brief Move constructor */
		IVIS( IVIS&& ) noexcept = delete;

	public:
		/** @brief Destructor */
		virtual ~IVIS() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		IVIS& operator=( const IVIS& ) = delete;

		/** @brief Move assignment operator */
		IVIS& operator=( IVIS&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the latest known and supported VIS version.
		 * @return The latest VisVersion.
		 */
		[[nodiscard]] VisVersion latestVisVersion() const;

		/**
		 * @brief Get the GMOD (Generic Product Model) for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the GMOD.
		 * @return A constant reference to the GMOD object.
		 * @throws std::invalid_argument If the provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual const Gmod& gmod( VisVersion visVersion ) const = 0;

		/**
		 * @brief Get the Codebooks for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the Codebooks.
		 * @return A constant reference to the Codebooks object.
		 * @throws std::invalid_argument If the provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual const Codebooks& codebooks( VisVersion visVersion ) = 0;

		/**
		 * @brief Get the Locations for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the Locations.
		 * @return A constant reference to the Locations object.
		 * @throws std::invalid_argument If the provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual const Locations& locations( VisVersion visVersion ) = 0;

		/**
		 * @brief Get Codebooks for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map where keys are VisVersion and values are pointers to const Codebooks objects.
		 *         The lifetime of the pointed-to objects is managed by the IVIS implementation.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, const Codebooks*> codebooksMap( const std::vector<VisVersion>& visVersions ) = 0;

		/**
		 * @brief Get GMODs for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map where keys are VisVersion and values are pointers to const Gmod objects.
		 *         The lifetime of the pointed-to objects is managed by the IVIS implementation.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, const Gmod*> gmodsMap( const std::vector<VisVersion>& visVersions ) const = 0;

		/**
		 * @brief Get Locations for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map where keys are VisVersion and values are pointers to const Locations objects.
		 *         The lifetime of the pointed-to objects is managed by the IVIS implementation.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, const Locations*> locationsMap( const std::vector<VisVersion>& visVersions ) = 0;

		/**
		 * @brief Get all available and supported VIS versions.
		 * @return A vector containing all supported VisVersion enumerators.
		 */
		[[nodiscard]] virtual std::vector<VisVersion> visVersions() = 0;

		//----------------------------------------------
		// Conversion
		//----------------------------------------------

		/**
		 * @brief Convert a GMOD node from one VIS version to another.
		 * @param sourceVersion The VIS version of the source GMOD node.
		 * @param sourceNode The GMOD node to convert.
		 * @param targetVersion The target VIS version for the conversion.
		 * @return An std::optional containing the converted GmodNode if successful, or std::nullopt if conversion is not possible or fails.
		 */
		[[nodiscard]] virtual std::optional<GmodNode> convertNode( VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) = 0;

		/**
		 * @brief Convert a GMOD path from one VIS version to another.
		 * @param sourceVersion The VIS version of the source GMOD path.
		 * @param sourcePath The GMOD path to convert.
		 * @param targetVersion The target VIS version for the conversion.
		 * @return An std::optional containing the converted GmodPath if successful, or std::nullopt if conversion is not possible or fails.
		 */
		[[nodiscard]] virtual std::optional<GmodPath> convertPath( VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) = 0;
	};

	//=====================================================================
	// VIS Singleton
	//=====================================================================

	/**
	 * @brief Singleton implementation of the IVIS interface for Vessel Information Structure operations.
	 *
	 * This class implements the IVIS interface and provides efficient access to VIS data
	 * (GMOD, Codebooks, Locations) using thread-safe caching mechanisms. It follows the
	 * Singleton pattern to ensure a single point of access and consistent data management
	 * throughout the application.
	 */
	class VIS final : public IVIS
	{
	private:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/** @brief Constructor */
		VIS();

		/** @brief Copy constructor */
		VIS( const VIS& ) = delete;

		/** @brief Move constructor */
		VIS( VIS&& ) noexcept = delete;

	public:
		/** @brief Destructor */
		virtual ~VIS() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		VIS& operator=( const VIS& ) = delete;

		/** @brief Move assignment operator */
		VIS& operator=( VIS&& ) noexcept = delete;

		//----------------------------------------------
		// Singleton Access
		//----------------------------------------------

		/**
		 * @brief Provides access to the singleton instance of the VIS class.
		 * @return A reference to the unique VIS instance.
		 */
		[[nodiscard]] static VIS& instance();

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get all available VIS versions.
		 * @return Vector of all supported VIS versions.
		 */
		[[nodiscard]] virtual std::vector<VisVersion> visVersions() override;

		/**
		 * @brief Retrieves the GMOD versioning information processed into a GmodVersioning object.
		 * This object provides higher-level access to version conversion logic.
		 * @return The GmodVersioning object.
		 * @throws std::runtime_error If the GMOD versioning data cannot be loaded or processed.
		 */
		[[nodiscard]] GmodVersioning gmodVersioning();

		/**
		 * @brief Get the GMOD for a specific VIS version.
		 * @param visVersion The VIS version.
		 * @return Constant reference to the GMOD.
		 * @throws std::invalid_argument If the VIS version is invalid.
		 */
		[[nodiscard]] virtual const Gmod& gmod( VisVersion visVersion ) const override;

		/**
		 * @brief Get the Codebooks for a specific VIS version.
		 * @param visVersion The VIS version.
		 * @return Constant reference to the Codebooks.
		 * @throws std::invalid_argument If the VIS version is invalid.
		 */
		[[nodiscard]] virtual const Codebooks& codebooks( VisVersion visVersion ) override;

		/**
		 * @brief Get the Locations for a specific VIS version.
		 * @param visVersion The VIS version.
		 * @return Constant reference to the Locations.
		 * @throws std::invalid_argument If the VIS version is invalid.
		 */
		[[nodiscard]] virtual const Locations& locations( VisVersion visVersion ) override;

		/**
		 * @brief Get Codebooks for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map of VisVersion to const Codebooks pointers.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, const Codebooks*> codebooksMap( const std::vector<VisVersion>& visVersions ) override;

		/**
		 * @brief Get GMODs for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map of VisVersion to const Gmod pointers.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, const Gmod*> gmodsMap( const std::vector<VisVersion>& visVersions ) const override;

		/**
		 * @brief Get Locations for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map of VisVersion to const Locations pointers.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, const Locations*> locationsMap( const std::vector<VisVersion>& visVersions ) override;

		//----------------------------------------------
		// DTO Accessors
		//----------------------------------------------
		/**
		 * @brief Retrieves the GMOD Data Transfer Object (DTO) for a specific VIS version.
		 * This method may utilize caching.
		 * @param visVersion The VIS version for which to retrieve the GMOD DTO.
		 * @return The GmodDto object.
		 * @throws std::runtime_error If the GMOD DTO cannot be loaded or found for the specified version.
		 */
		[[nodiscard]] GmodDto gmodDto( VisVersion visVersion ) const;

		/**
		 * @brief Statically loads the GMOD Data Transfer Object (DTO) for a specific VIS version.
		 * This method performs direct loading (e.g., from a file or resource) without instance caching.
		 * @param visVersion The VIS version for which to load the GMOD DTO.
		 * @return An std::optional<GmodDto> containing the DTO if loading is successful, or std::nullopt otherwise.
		 */
		[[nodiscard]] static std::optional<GmodDto> loadGmodDto( VisVersion visVersion );

		/**
		 * @brief Retrieves the GMOD versioning DTOs.
		 * This method provides access to the data structures defining how GMOD nodes convert between versions.
		 * @return An unordered_map where keys are typically version transition strings or identifiers,
		 *         and values are GmodVersioningDto objects.
		 * @throws std::runtime_error If the versioning DTOs cannot be loaded.
		 */
		[[nodiscard]] std::unordered_map<std::string, GmodVersioningDto> gmodVersioningDto();

		/**
		 * @brief Retrieves the Codebooks Data Transfer Object (DTO) for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the Codebooks DTO.
		 * @return The CodebooksDto object.
		 * @throws std::runtime_error If the Codebooks DTO cannot be loaded or found.
		 */
		[[nodiscard]] CodebooksDto codebooksDto( VisVersion visVersion );

		/**
		 * @brief Retrieves the Locations Data Transfer Object (DTO) for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the Locations DTO.
		 * @return The LocationsDto object.
		 * @throws std::runtime_error If the Locations DTO cannot be loaded or found.
		 */
		[[nodiscard]] LocationsDto locationsDto( VisVersion visVersion );

		//----------------------------------------------
		// Conversion
		//----------------------------------------------

		/**
		 * @brief Convert a GMOD node from one VIS version to another.
		 * @param sourceVersion The source VIS version.
		 * @param sourceNode The GMOD node to convert.
		 * @param targetVersion The target VIS version.
		 * @return std::optional<GmodNode> containing the converted node or std::nullopt.
		 */
		[[nodiscard]] virtual std::optional<GmodNode> convertNode( VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) override;

		/**
		 * @brief Convert a GMOD path from one VIS version to another.
		 * @param sourceVersion The source VIS version.
		 * @param sourcePath The GMOD path to convert.
		 * @param targetVersion The target VIS version.
		 * @return std::optional<GmodPath> containing the converted path or std::nullopt.
		 */
		[[nodiscard]] virtual std::optional<GmodPath> convertPath( VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) override;

		/**
		 * @brief Convert a GMOD node to a different VIS version, inferring source version from the node.
		 * @param sourceNode The GMOD node to convert (must have its VIS version embedded or retrievable).
		 * @param targetVersion The target VIS version.
		 * @param sourceParent Optional parent GMOD node for context during conversion, can be nullptr.
		 * @return std::optional<GmodNode> containing the converted node or std::nullopt.
		 */
		[[nodiscard]] std::optional<GmodNode> convertNode( const GmodNode& sourceNode, VisVersion targetVersion, const GmodNode* sourceParent = nullptr );

		/**
		 * @brief Convert a GMOD path to a different VIS version, inferring source version from the path.
		 * @param sourcePath The GMOD path to convert (must have its VIS version embedded or retrievable).
		 * @param targetVersion The target VIS version.
		 * @return std::optional<GmodPath> containing the converted path or std::nullopt.
		 */
		[[nodiscard]] std::optional<GmodPath> convertPath( const GmodPath& sourcePath, VisVersion targetVersion );

		/**
		 * @brief Convert a LocalIdBuilder instance to a different VIS version.
		 * @param sourceLocalId The LocalIdBuilder to convert.
		 * @param targetVersion The target VIS version.
		 * @return std::optional<LocalIdBuilder> containing the converted LocalIdBuilder or std::nullopt.
		 */
		[[nodiscard]] std::optional<LocalIdBuilder> convertLocalId( const LocalIdBuilder& sourceLocalId, VisVersion targetVersion );

		/**
		 * @brief Convert a LocalId instance to a different VIS version.
		 * @param sourceLocalId The LocalId to convert.
		 * @param targetVersion The target VIS version.
		 * @return std::optional<LocalId> containing the converted LocalId or std::nullopt.
		 */
		[[nodiscard]] std::optional<LocalId> convertLocalId( const LocalId& sourceLocalId, VisVersion targetVersion );

		//----------------------------------------------
		// ISO String Validation Methods
		//----------------------------------------------

		/**
		 * @brief Checks if a string matches the expected format for an ISO Local ID string.
		 * @param value The std::string to validate.
		 * @return True if the string matches the ISO Local ID format, false otherwise.
		 */
		[[nodiscard]] static bool matchISOLocalIdString( const std::string& value );

		/**
		 * @brief Checks if the content of a std::stringstream matches the ISO Local ID format.
		 * @param builder The std::stringstream whose content is to be validated.
		 * @return True if the content matches the ISO Local ID format, false otherwise.
		 */
		[[nodiscard]] static bool matchISOLocalIdString( const std::stringstream& builder );

		/**
		 * @brief Checks if a string_view matches the expected format for an ISO Local ID string.
		 * @param value The std::string_view to validate.
		 * @return True if the string_view matches the ISO Local ID format, false otherwise.
		 */
		[[nodiscard]] static bool matchISOLocalIdString( std::string_view value );

		/**
		 * @brief Checks if all characters in a string_view are valid ISO characters.
		 * @param value The std::string_view to validate.
		 * @return True if all characters are valid ISO characters, false otherwise.
		 */
		[[nodiscard]] static bool isISOString( std::string_view value );

		/**
		 * @brief Checks if all characters in a std::string are valid ISO characters.
		 * @param value The std::string to validate.
		 * @return True if all characters are valid ISO characters, false otherwise.
		 */
		[[nodiscard]] static bool isISOString( const std::string& value );

		/**
		 * @brief Checks if all characters in the content of a std::stringstream are valid ISO characters.
		 * @param builder The std::stringstream whose content is to be validated.
		 * @return True if all characters in the content are valid ISO characters, false otherwise.
		 */
		[[nodiscard]] static bool isISOString( const std::stringstream& builder );

		/**
		 * @brief A comprehensive check if a string is a valid ISO Local ID string.
		 * This may involve format and character set validation.
		 * @param value The std::string to validate.
		 * @return True if the string is a valid ISO Local ID string, false otherwise.
		 */
		[[nodiscard]] static bool isISOLocalIdString( const std::string& value );

		/**
		 * @brief Checks if a character is a valid character within an ISO-compliant string.
		 * @param c The character to check.
		 * @return True if the character is valid for ISO strings, false otherwise.
		 */
		[[nodiscard]] static bool isISOString( char c );

		/**
		 * @brief Checks if an ASCII decimal code corresponds to a valid ISO character.
		 * @param code The integer ASCII code to check.
		 * @return True if the code represents a valid ISO character, false otherwise.
		 */
		[[nodiscard]] static bool matchAsciiDecimal( int code );

	private:
		//=====================================================================
		// Caching
		//=====================================================================

		/**
		 * @brief A thread-safe, template-based cache with LRU-like eviction and time-based cleanup.
		 *
		 * This class provides a generic caching mechanism used internally by VIS
		 * to store frequently accessed data, such as DTOs or processed VIS objects.
		 * It ensures thread safety for concurrent access and manages cache size
		 * and entry lifetime.
		 *
		 * @tparam K The key type for cache lookups.
		 * @tparam V The value type stored in the cache.
		 */
		template <typename K, typename V>
		class Cache final
		{
		public:
			//----------------------------------------------
			// Construction / Destruction
			//----------------------------------------------

			/** @brief constructor. */
			Cache();

			/** @brief Copy constructor */
			Cache( const Cache& ) = delete;

			/** @brief Move constructor */
			Cache( Cache&& ) = delete;

			/** @brief Destructor */
			~Cache() = default;

			//----------------------------------------------
			// Assignment Operators
			//----------------------------------------------

			/** @brief Copy assignment operator */
			Cache& operator=( const Cache& ) = delete;

			/** @brief Move assignment operator */
			Cache& operator=( Cache&& ) = delete;

			//------------------------------------------
			// Public Methods
			//------------------------------------------

			/**
			 * @brief Retrieves an item from the cache or creates it using the provided factory if not found.
			 *
			 * This method handles thread-safe access, cache cleanup (time-based),
			 * and cache eviction (LRU-like when size limit is reached).
			 *
			 * @param key The key to look up in the cache.
			 * @param factory A function (std::function<V()>) that will be called to create the value if it's not found in the cache.
			 * @return A reference to the cached value (either existing or newly created).
			 */
			V& getOrCreate( const K& key, std::function<V()> factory ) const;

		private:
			//------------------------------------------
			// Private Inner Types
			//------------------------------------------
			/**
			 * @brief Represents an item stored in the cache.
			 * Contains the actual value and metadata for cache management (last access time).
			 */
			struct CacheItem final
			{
				V value;
				std::chrono::steady_clock::time_point lastAccess;
			};

			//------------------------------------------
			// Private Methods
			//------------------------------------------

			/**
			 * @brief Performs cleanup of expired cache entries.
			 * Entries older than a predefined duration (e.g., 1 hour) are removed.
			 */
			void cleanupCache() const;

			/**
			 * @brief Removes the least recently used (LRU) entry from the cache.
			 * This is typically called when the cache reaches its maximum size limit.
			 */
			void removeOldestEntry() const;

			//----------------------------------------------
			// Private Member Variables
			//----------------------------------------------

			mutable std::mutex m_mutex;
			mutable std::unordered_map<K, CacheItem> m_cache;
			mutable std::chrono::steady_clock::time_point m_lastCleanup;
		};

		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief Cache for GMOD Data Transfer Objects, keyed by VisVersion. */
		Cache<VisVersion, GmodDto> m_gmodDtoCache;

		/** @brief Cache for processed Gmod objects, keyed by VisVersion. */
		Cache<VisVersion, Gmod> m_gmodCache;

		/** @brief Cache for Codebooks Data Transfer Objects, keyed by VisVersion. */
		Cache<VisVersion, CodebooksDto> m_codebooksDtoCache;

		/** @brief Cache for processed Codebooks objects, keyed by VisVersion. */
		Cache<VisVersion, Codebooks> m_codebooksCache;

		/** @brief Cache for Locations Data Transfer Objects, keyed by VisVersion. */
		Cache<VisVersion, LocationsDto> m_locationsDtoCache;

		/** @brief Cache for processed Locations objects, keyed by VisVersion. */
		Cache<VisVersion, Locations> m_locationsCache;

		/** @brief Cache for GMOD Versioning DTOs, keyed by a string (e.g., "versioning"). */
		Cache<std::string, std::unordered_map<std::string, GmodVersioningDto>> m_gmodVersioningDtoCache;

		/** @brief Cache for processed GmodVersioning objects, keyed by a string (e.g., "versioning"). */
		Cache<std::string, GmodVersioning> m_gmodVersioningCache;
	};
}

#include "VIS.inl"
