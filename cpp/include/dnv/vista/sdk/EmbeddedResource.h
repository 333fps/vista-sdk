/*
	Copyright @kzu
	License MIT
	https://github.com/devlooped/ThisAssembly
*/

#pragma once

namespace dnv::vista::sdk
{
	struct GmodDto;
	struct GmodVersioningDto;
	class CodebooksDto;
	struct DataChannelTypeNamesDto;
	struct LocationsDto;
	struct FormatDataTypesDto;

	/**
	 * @brief Utility class for accessing embedded resources
	 */
	class EmbeddedResource final
	{
	public:
		/**
		 * @brief Get all available VIS versions
		 * @return List of VIS version strings
		 */
		static std::vector<std::string> visVersions();

		/**
		 * @brief Get all GMOD versioning data
		 * @return Dictionary of versioning DTOs by version string
		 */
		static std::optional<std::unordered_map<std::string, GmodVersioningDto>> gmodVersioning();

		/**
		 * @brief Get GMOD for specific VIS version
		 * @param visVersion VIS version string
		 * @return GMOD DTO, or nullptr if not found
		 */
		static std::optional<GmodDto> gmod( const std::string& visVersion );

		/**
		 * @brief Get codebooks for specific VIS version
		 * @param visVersion VIS version string
		 * @return Codebooks DTO, or nullptr if not found
		 */
		static std::optional<CodebooksDto> codebooks( const std::string& visVersion );

		/**
		 * @brief Get locations for specific VIS version
		 * @param visVersion VIS version string
		 * @return Locations DTO, or nullptr if not found
		 */
		static std::optional<LocationsDto> locations( const std::string& visVersion );

		/**
		 * @brief Get data channel type names for specific version
		 * @param version Version string
		 * @return DataChannelTypeNamesDto, or nullptr if not found
		 */
		static std::optional<DataChannelTypeNamesDto> dataChannelTypeNames( const std::string& version );

		/**
		 * @brief Get format data types for specific version
		 * @param version Version string
		 * @return FormatDataTypesDto, or nullptr if not found
		 */
		static std::optional<FormatDataTypesDto> formatDataTypes( const std::string& version );

	private:
		/**
		 * @brief Get all embedded resource names
		 * @return Array of resource names
		 */
		static std::vector<std::string> resourceNames();

		/**
		 * @brief Get resource stream for specific resource name
		 * @param resourceName Resource name
		 * @return Memory stream containing resource data
		 */
		static std::shared_ptr<std::istream> stream( const std::string& resourceName );

		/**
		 * @brief Get decompressed stream for gzipped resource
		 * @param resourceName Compressed resource name
		 * @return Decompressed stream
		 */
		static std::shared_ptr<std::istream> decompressedStream( const std::string& resourceName );
	};
}
