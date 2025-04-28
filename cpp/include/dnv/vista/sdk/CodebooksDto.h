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
		// Types and Aliases
		//-------------------------------------------------------------------

		/** @brief Type representing a collection of values within a group */
		using ValueGroup = std::vector<std::string>;

		/** @brief Type representing a mapping of group names to their values */
		using ValuesMap = std::unordered_map<std::string, ValueGroup>;

		//-------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		CodebookDto() = delete;

		/**
		 * @brief Constructor with parameters
		 * @param name The codebook name
		 * @param values The map of group names to values
		 */
		CodebookDto( std::string name, ValuesMap values );

		/** @brief Copy constructor */
		CodebookDto( const CodebookDto& ) = default;

		/** @brief Move constructor */
		CodebookDto( CodebookDto&& ) noexcept = default;

		/** @brief Destructor */
		~CodebookDto() = default;

		//-------------------------------------------------------------------
		// Public Interface - Accessor Methods
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
		const ValuesMap& values() const;

		//-------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a CodebookDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<CodebookDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a CodebookDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized CodebookDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		static CodebookDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this CodebookDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		CodebookDto& operator=( const CodebookDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		CodebookDto& operator=( CodebookDto&& ) noexcept = delete;

		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief Name identifier of the codebook (e.g., "positions", "quantities") */
		const std::string m_name;

		/** @brief Map of group names to their corresponding values */
		const ValuesMap m_values;
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
		// Types and Aliases
		//-------------------------------------------------------------------

		/** @brief Type representing a collection of codebook DTOs */
		using Items = std::vector<CodebookDto>;

		//-------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		CodebooksDto() = delete;

		/**
		 * @brief Constructor with parameters
		 * @param visVersion The VIS version
		 * @param items The collection of codebook DTOs
		 */
		CodebooksDto( std::string visVersion, Items items );

		/** @brief Copy constructor */
		CodebooksDto( const CodebooksDto& ) = default;

		/** @brief Move constructor */
		CodebooksDto( CodebooksDto&& ) noexcept = default;

		/** @brief Destructor */
		~CodebooksDto() = default;

		//-------------------------------------------------------------------
		// Public Interface - Accessor Methods
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
		const Items& items() const;

		//-------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a CodebooksDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<CodebooksDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a CodebooksDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized CodebooksDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		static CodebooksDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this CodebooksDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		CodebooksDto& operator=( const CodebooksDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		CodebooksDto& operator=( CodebooksDto&& ) noexcept = delete;

		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief VIS version string (e.g., "3.8a") */
		const std::string m_visVersion;

		/** @brief Collection of codebook DTOs contained in this version */
		const Items m_items;
	};
}
