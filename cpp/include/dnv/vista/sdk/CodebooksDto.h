/**
 * @file CodebooksDto.h
 * @brief Data transfer objects for ISO 19848 codebook serialization
 *
 * This file defines the data transfer objects used for serializing and deserializing
 * codebook information according to the ISO 19848 standard. These DTOs are used
 * as an intermediate representation when loading or saving codebook data to JSON.
 */

#pragma once

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Codebook Data Transfer Objects
	//-------------------------------------------------------------------

	/**
	 * @brief Data transfer object for a single codebook
	 *
	 * Represents serialized information about a codebook from the ISO 19848 standard.
	 * Each codebook contains a name identifier and a collection of values organized by groups.
	 */
	class CodebookDto final
	{
	public:
		//-------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		CodebookDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param name The codebook name
		 * @param values The map of group names to values
		 */
		CodebookDto( std::string name, std::unordered_map<std::string, std::vector<std::string>> values );

		CodebookDto( const CodebookDto& ) = default;
		CodebookDto( CodebookDto&& ) noexcept = default;
		CodebookDto& operator=( const CodebookDto& ) = default;
		CodebookDto& operator=( CodebookDto&& ) noexcept = default;
		~CodebookDto() = default;

		//-------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Get the name of this codebook
		 * @return The codebook name
		 */
		const std::string& name() const;

		/**
		 * @brief Get the values map of this codebook
		 * @return The map of group names to their corresponding values
		 */
		const std::unordered_map<std::string, std::vector<std::string>>& values() const;

		//-------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Deserialize a CodebookDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized CodebookDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		static CodebookDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize a CodebookDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, CodebookDto& dto );

		/**
		 * @brief Serialize this CodebookDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief Name identifier of the codebook (e.g., "positions", "quantities") */
		std::string m_name;

		/** @brief Map of group names to their corresponding values */
		std::unordered_map<std::string, std::vector<std::string>> m_values;
	};

	/**
	 * @brief Data transfer object for a collection of codebooks
	 *
	 * Represents a complete set of codebooks for a specific VIS version,
	 * used for serialization to and from JSON format.
	 */
	class CodebooksDto final
	{
	public:
		//-------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		CodebooksDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param visVersion The VIS version
		 * @param items The collection of codebook DTOs
		 */
		CodebooksDto( std::string visVersion, std::vector<CodebookDto> items );

		CodebooksDto( const CodebooksDto& ) = default;
		CodebooksDto( CodebooksDto&& ) noexcept = default;
		CodebooksDto& operator=( const CodebooksDto& ) = default;
		CodebooksDto& operator=( CodebooksDto&& ) noexcept = default;
		~CodebooksDto() = default;

		//-------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Get the VIS version string
		 * @return The VIS version string
		 */
		const std::string& visVersion() const;

		/**
		 * @brief Get the collection of codebooks
		 * @return The vector of codebook DTOs
		 */
		const std::vector<CodebookDto>& items() const;

		//-------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Deserialize a CodebooksDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized CodebooksDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		static CodebooksDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize a CodebooksDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, CodebooksDto& dto );

		/**
		 * @brief Serialize this CodebooksDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief VIS version string (e.g., "3.8a") */
		std::string m_visVersion;

		/** @brief Collection of codebook DTOs contained in this version */
		std::vector<CodebookDto> m_items;
	};
}
