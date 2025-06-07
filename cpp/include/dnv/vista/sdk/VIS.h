/**
 * @file VIS.h
 * @brief Vessel Information Structure (VIS) interface and implementation
 *
 * Provides access to VIS data like GMOD, Codebooks, and Locations,
 * along with version conversion utilities and ISO string validation.
 */

#pragma once

#include "VISVersion.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class Codebooks;
	class CodebooksDto;
	class Gmod;
	class GmodDto;
	class GmodNode;
	class GmodPath;
	class GmodVersioningDto;
	class GmodVersioning;
	class LocalId;
	class LocalIdBuilder;
	class Locations;
	class LocationsDto;

	//=====================================================================
	// IVIS interface
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
		// Construction / destruction
		//----------------------------------------------

		/** @brief Default constructor. */
		IVIS() = default;

		/** @brief Copy constructor (deleted) */
		IVIS( const IVIS& ) = delete;

		/** @brief Move constructor (deleted) */
		IVIS( IVIS&& ) noexcept = delete;

	public:
		/** @brief Virtual destructor */
		virtual ~IVIS() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator (deleted) */
		IVIS& operator=( const IVIS& ) = delete;

		/** @brief Move assignment operator (deleted) */
		IVIS& operator=( IVIS&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the latest known and supported VIS version.
		 * @return The latest VisVersion.
		 */
		[[nodiscard]] VisVersion latestVisVersion() const noexcept;

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
		 * @return An unordered_map where keys are VisVersion and values are Codebooks objects (copies).
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, Codebooks> codebooksMap( const std::vector<VisVersion>& visVersions ) = 0;

		/**
		 * @brief Get GMODs for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map where keys are VisVersion and values are Gmod objects (copies).
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, Gmod> gmodsMap( const std::vector<VisVersion>& visVersions ) const = 0;

		/**
		 * @brief Get Locations for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map where keys are VisVersion and values are Locations objects (copies).
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, Locations> locationsMap( const std::vector<VisVersion>& visVersions ) = 0;

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
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::optional<GmodNode> convertNode( VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) = 0;

		/**
		 * @brief Convert a GMOD path from one VIS version to another.
		 * @param sourceVersion The VIS version of the source GMOD path.
		 * @param sourcePath The GMOD path to convert.
		 * @param targetVersion The target VIS version for the conversion.
		 * @return An std::optional containing the converted GmodPath if successful, or std::nullopt if conversion is not possible or fails.
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::optional<GmodPath> convertPath( VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) = 0;
	};

	//=====================================================================
	// VIS singleton
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
		// Construction / destruction
		//----------------------------------------------

		/** @brief Private constructor (singleton pattern) */
		VIS();

		/** @brief Copy constructor (deleted) */
		VIS( const VIS& ) = delete;

		/** @brief Move constructor (deleted) */
		VIS( VIS&& ) noexcept = delete;

	public:
		/** @brief Virtual destructor */
		virtual ~VIS() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator (deleted) */
		VIS& operator=( const VIS& ) = delete;

		/** @brief Move assignment operator (deleted) */
		VIS& operator=( VIS&& ) noexcept = delete;

		//----------------------------------------------
		// Singleton access
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
		 * @brief Get all available and supported VIS versions.
		 * @return A vector containing all supported VisVersion enumerators.
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
		 * @brief Get the GMOD (Generic Product Model) for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the GMOD.
		 * @return A constant reference to the GMOD object.
		 * @throws std::invalid_argument If the provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual const Gmod& gmod( VisVersion visVersion ) const override;

		/**
		 * @brief Get the Codebooks for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the Codebooks.
		 * @return A constant reference to the Codebooks object.
		 * @throws std::invalid_argument If the provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual const Codebooks& codebooks( VisVersion visVersion ) override;

		/**
		 * @brief Get the Locations for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the Locations.
		 * @return A constant reference to the Locations object.
		 * @throws std::invalid_argument If the provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual const Locations& locations( VisVersion visVersion ) override;

		/**
		 * @brief Get Codebooks for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map where keys are VisVersion and values are Codebooks objects (copies).
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, Codebooks> codebooksMap( const std::vector<VisVersion>& visVersions ) override;

		/**
		 * @brief Get GMODs for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map where keys are VisVersion and values are Gmod objects (copies).
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, Gmod> gmodsMap( const std::vector<VisVersion>& visVersions ) const override;

		/**
		 * @brief Get Locations for multiple VIS versions.
		 * @param visVersions A vector of VIS versions.
		 * @return An unordered_map where keys are VisVersion and values are Locations objects (copies).
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::unordered_map<VisVersion, Locations> locationsMap( const std::vector<VisVersion>& visVersions ) override;

		//----------------------------------------------
		// DTO accessors
		//----------------------------------------------

		/**
		 * @brief Retrieves the GMOD Data Transfer Object (DTO) for a specific VIS version.
		 * This method utilizes caching for performance.
		 * @param visVersion The VIS version for which to retrieve the GMOD DTO.
		 * @return The GmodDto object.
		 * @throws std::invalid_argument If the provided VIS version is invalid or not supported.
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
		 * @return An unordered_map where keys are version transition strings or identifiers,
		 *         and values are GmodVersioningDto objects.
		 * @throws std::runtime_error If the versioning DTOs cannot be loaded.
		 */
		[[nodiscard]] std::unordered_map<std::string, GmodVersioningDto> gmodVersioningDto();

		/**
		 * @brief Retrieves the Codebooks Data Transfer Object (DTO) for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the Codebooks DTO.
		 * @return The CodebooksDto object.
		 * @throws std::invalid_argument If the provided VIS version is invalid or not supported.
		 * @throws std::runtime_error If the Codebooks DTO cannot be loaded or found.
		 */
		[[nodiscard]] CodebooksDto codebooksDto( VisVersion visVersion );

		/**
		 * @brief Retrieves the Locations Data Transfer Object (DTO) for a specific VIS version.
		 * @param visVersion The VIS version for which to retrieve the Locations DTO.
		 * @return The LocationsDto object.
		 * @throws std::invalid_argument If the provided VIS version is invalid or not supported.
		 * @throws std::runtime_error If the Locations DTO cannot be loaded or found.
		 */
		[[nodiscard]] LocationsDto locationsDto( VisVersion visVersion );

		//----------------------------------------------
		// Conversion
		//----------------------------------------------

		/**
		 * @brief Convert a GMOD node from one VIS version to another.
		 * @param sourceVersion The VIS version of the source GMOD node.
		 * @param sourceNode The GMOD node to convert.
		 * @param targetVersion The target VIS version for the conversion.
		 * @return An std::optional containing the converted GmodNode if successful, or std::nullopt if conversion is not possible or fails.
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::optional<GmodNode> convertNode( VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) override;

		/**
		 * @brief Convert a GMOD path from one VIS version to another.
		 * @param sourceVersion The VIS version of the source GMOD path.
		 * @param sourcePath The GMOD path to convert.
		 * @param targetVersion The target VIS version for the conversion.
		 * @return An std::optional containing the converted GmodPath if successful, or std::nullopt if conversion is not possible or fails.
		 * @throws std::invalid_argument If any provided VIS version is invalid or not supported.
		 */
		[[nodiscard]] virtual std::optional<GmodPath> convertPath( VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) override;

		/**
		 * @brief Convert a GMOD node to a different VIS version, inferring source version from the node.
		 * @param sourceNode The GMOD node to convert (must have its VIS version embedded or retrievable).
		 * @param targetVersion The target VIS version for the conversion.
		 * @param sourceParent Optional parent GMOD node for context during conversion, can be nullptr.
		 * @return An std::optional containing the converted GmodNode if successful, or std::nullopt if conversion is not possible or fails.
		 * @throws std::invalid_argument If the target VIS version is invalid or not supported.
		 */
		[[nodiscard]] std::optional<GmodNode> convertNode( const GmodNode& sourceNode, VisVersion targetVersion, const GmodNode* sourceParent = nullptr );

		/**
		 * @brief Convert a GMOD path to a different VIS version, inferring source version from the path.
		 * @param sourcePath The GMOD path to convert (must have its VIS version embedded or retrievable).
		 * @param targetVersion The target VIS version for the conversion.
		 * @return An std::optional containing the converted GmodPath if successful, or std::nullopt if conversion is not possible or fails.
		 * @throws std::invalid_argument If the target VIS version is invalid or not supported.
		 */
		[[nodiscard]] std::optional<GmodPath> convertPath( const GmodPath& sourcePath, VisVersion targetVersion );

		/**
		 * @brief Convert a LocalIdBuilder instance to a different VIS version.
		 * @param sourceLocalId The LocalIdBuilder to convert.
		 * @param targetVersion The target VIS version for the conversion.
		 * @return An std::optional containing the converted LocalIdBuilder if successful, or std::nullopt if conversion is not possible or fails.
		 * @throws std::invalid_argument If the target VIS version is invalid or not supported.
		 */
		[[nodiscard]] std::optional<LocalIdBuilder> convertLocalId( const LocalIdBuilder& sourceLocalId, VisVersion targetVersion );

		/**
		 * @brief Convert a LocalId instance to a different VIS version.
		 * @param sourceLocalId The LocalId to convert.
		 * @param targetVersion The target VIS version for the conversion.
		 * @return An std::optional containing the converted LocalId if successful, or std::nullopt if conversion is not possible or fails.
		 * @throws std::invalid_argument If the target VIS version is invalid or not supported.
		 */
		[[nodiscard]] std::optional<LocalId> convertLocalId( const LocalId& sourceLocalId, VisVersion targetVersion );

		//----------------------------------------------
		// ISO string validation methods
		//----------------------------------------------

		/**
		 * @brief Checks if the content of a std::stringstream matches the ISO Local ID format.
		 * @param builder The std::stringstream whose content is to be validated.
		 * @return True if the content matches the ISO Local ID format, false otherwise.
		 */
		[[nodiscard]] static bool matchISOLocalIdString( const std::stringstream& builder ) noexcept;

		/**
		 * @brief Checks if a string_view matches the expected format for an ISO Local ID string.
		 * @param value The std::string_view to validate.
		 * @return True if the string_view matches the ISO Local ID format, false otherwise.
		 */
		[[nodiscard]] static bool matchISOLocalIdString( std::string_view value ) noexcept;

		/**
		 * @brief Checks if all characters in a string_view are valid ISO characters.
		 * @param value The std::string_view to validate.
		 * @return True if all characters are valid ISO characters, false otherwise.
		 */
		[[nodiscard]] static bool isISOString( std::string_view value ) noexcept;

		/**
		 * @brief Checks if all characters in a std::string are valid ISO characters.
		 * @param value The std::string to validate.
		 * @return True if all characters are valid ISO characters, false otherwise.
		 */
		[[nodiscard]] static bool isISOString( const std::string& value ) noexcept;

		/**
		 * @brief Checks if all characters in the content of a std::stringstream are valid ISO characters.
		 * @param builder The std::stringstream whose content is to be validated.
		 * @return True if all characters in the content are valid ISO characters, false otherwise.
		 */
		[[nodiscard]] static bool isISOString( const std::stringstream& builder ) noexcept;

		/**
		 * @brief A comprehensive check if a string is a valid ISO Local ID string.
		 * This may involve format and character set validation.
		 * @param value The std::string to validate.
		 * @return True if the string is a valid ISO Local ID string, false otherwise.
		 */
		[[nodiscard]] static bool isISOLocalIdString( const std::string& value ) noexcept;

		/**
		 * @brief Checks if a character is a valid character within an ISO-compliant string.
		 * @param c The character to check.
		 * @return True if the character is valid for ISO strings, false otherwise.
		 */
		[[nodiscard]] static bool isISOString( char c ) noexcept;

		/**
		 * @brief Checks if an ASCII decimal code corresponds to a valid ISO character.
		 * @param code The integer ASCII code to check.
		 * @return True if the code represents a valid ISO character, false otherwise.
		 */
		[[nodiscard]] static bool matchAsciiDecimal( int code ) noexcept;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		mutable std::shared_mutex m_cacheMutex;
		mutable std::unordered_map<VisVersion, Codebooks> m_codebooksCache;
		mutable std::unordered_map<VisVersion, Gmod> m_gmodCache;
		mutable std::unordered_map<VisVersion, Locations> m_locationsCache;
		mutable std::unordered_map<VisVersion, CodebooksDto> m_codebooksDtoCache;
		mutable std::unordered_map<VisVersion, GmodDto> m_gmodDtoCache;
		mutable std::unordered_map<VisVersion, LocationsDto> m_locationsDtoCache;
		mutable std::unordered_map<std::string, GmodVersioning> m_gmodVersioningCache;
		mutable std::unordered_map<std::string, std::unordered_map<std::string, GmodVersioningDto>> m_gmodVersioningDtoCache;
	};
}
