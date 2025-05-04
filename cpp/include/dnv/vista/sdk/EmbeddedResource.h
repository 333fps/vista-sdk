/**
 * @file EmbeddedResource.h
 * @brief Resource loading and caching utilities for Vista SDK
 * @copyright Copyright @kzu, License MIT
 * @see https://github.com/devlooped/ThisAssembly
 */

#pragma once

namespace dnv::vista::sdk
{
	//--------------------------------------------------------------------------
	// Forward declarations
	//--------------------------------------------------------------------------

	class GmodDto;
	class GmodVersioningDto;
	class CodebooksDto;
	class DataChannelTypeNamesDto;
	class LocationsDto;
	class FormatDataTypesDto;

	//--------------------------------------------------------------------------
	// Resource utilities
	//--------------------------------------------------------------------------

	/**
	 * @brief Utility class for accessing and managing embedded resources
	 *
	 * This class provides access to gzipped JSON resource files containing configuration
	 * data for the Vista SDK. It implements thread-safe caching mechanisms for efficient
	 * resource loading and provides type-specific access methods for each resource type.
	 */
	class EmbeddedResource final
	{
	public:
		//----------------------------------------------------------------------
		// Public interface
		//----------------------------------------------------------------------

		/**
		 * @brief Get all available VIS versions
		 *
		 * Scans all resources to extract the VIS versions available in the current
		 * resource collection.
		 *
		 * @return Vector of VIS version strings
		 */
		static std::vector<std::string> visVersions();

		/**
		 * @brief Get GMOD for specific VIS version
		 *
		 * Loads and caches the Global Model DTO for the specified VIS version.
		 *
		 * @param visVersion VIS version string
		 * @return GMOD DTO if found, std::nullopt otherwise
		 */
		static std::optional<GmodDto> gmod( const std::string& visVersion );

		/**
		 * @brief Get all GMOD versioning data
		 *
		 * Loads and caches all Global Model versioning information.
		 *
		 * @return Dictionary of versioning DTOs by version string if found, std::nullopt otherwise
		 */
		static const std::optional<std::unordered_map<std::string, GmodVersioningDto>>& gmodVersioning();

		/**
		 * @brief Get codebooks for specific VIS version
		 *
		 * Loads and caches the codebook collection for the specified VIS version.
		 *
		 * @param visVersion VIS version string
		 * @return Codebooks DTO if found, std::nullopt otherwise
		 */
		static std::optional<CodebooksDto> codebooks( const std::string& visVersion );

		/**
		 * @brief Get locations for specific VIS version
		 *
		 * Loads and caches the location definitions for the specified VIS version.
		 *
		 * @param visVersion VIS version string
		 * @return Locations DTO if found, std::nullopt otherwise
		 */
		static std::optional<LocationsDto> locations( const std::string& visVersion );

		/**
		 * @brief Get data channel type names for specific version
		 *
		 * Loads and caches the ISO 19848 data channel type names for the specified version.
		 *
		 * @param version ISO 19848 version string
		 * @return DataChannelTypeNamesDto if found, std::nullopt otherwise
		 */
		static std::optional<DataChannelTypeNamesDto> dataChannelTypeNames( const std::string& version );

		/**
		 * @brief Get format data types for specific version
		 *
		 * Loads and caches the ISO 19848 format data types for the specified version.
		 *
		 * @param version ISO 19848 version string
		 * @return FormatDataTypesDto if found, std::nullopt otherwise
		 */
		static std::optional<FormatDataTypesDto> formatDataTypes( const std::string& version );

	private:
		//----------------------------------------------------------------------
		// Resource access implementation
		//----------------------------------------------------------------------

		/**
		 * @brief Get all embedded resource names
		 *
		 * Scans predefined directories for resource files with .json.gz extension.
		 * Results are cached for subsequent calls.
		 *
		 * @return Vector of resource names
		 */
		static std::vector<std::string> resourceNames();

		/**
		 * @brief Get resource stream for specific resource name
		 *
		 * Searches for the resource file in multiple possible directories and
		 * returns a stream for reading the resource data.
		 *
		 * @param resourceName Resource name
		 * @return Shared pointer to input stream containing the resource data
		 * @throws std::runtime_error if resource cannot be found
		 */
		static std::shared_ptr<std::istream> stream( const std::string& resourceName );

		/**
		 * @brief Get decompressed stream for gzipped resource
		 *
		 * Loads a gzipped resource and decompresses it using zlib.
		 *
		 * @param resourceName Compressed resource name
		 * @return Shared pointer to input stream containing the decompressed data
		 * @throws std::runtime_error if decompression fails
		 */
		static std::shared_ptr<std::istream> decompressedStream( const std::string& resourceName );
	};
}
