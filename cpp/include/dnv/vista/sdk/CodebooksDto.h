/**
 * @file CodebooksDto.h
 * @brief Data transfer objects for ISO 19848 codebook serialization
 * @details Provides data transfer objects used for serializing and deserializing
 *          codebook information according to the ISO 19848 standard.
 *          These DTOs serve as an intermediate representation when loading or saving codebook data.
 * @see ISO 19848 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// Codebook Data Transfer Object
	//=====================================================================

	/**
	 * @brief Data transfer object for a single codebook
	 * @details Represents serialized information about a codebook from the ISO 19848 standard.
	 *          Each codebook contains a name identifier and a collection of values organized by groups.
	 * @todo Consider refactoring for stricter immutability (e.g., const members) if direct modification
	 *       by deserialization (beyond initial construction) is not desired.
	 */
	class CodebookDto final
	{
	public:
		//----------------------------------------------
		// Types and Aliases
		//----------------------------------------------

		/** @brief Type representing a collection of values within a group */
		using ValueGroup = std::vector<std::string>;

		/** @brief Type representing a mapping of group names to their values */
		using ValuesMap = std::unordered_map<std::string, ValueGroup>;

		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructor with parameters
		 * @param name The codebook name
		 * @param values The map of group names to values
		 */
		CodebookDto( std::string name, ValuesMap values );

		/** @brief Default constructor. */
		CodebookDto() = default;

		/** @brief Copy constructor */
		CodebookDto( const CodebookDto& ) = default;

		/** @brief Move constructor */
		CodebookDto( CodebookDto&& ) noexcept = default;

		/** @brief Destructor */
		~CodebookDto() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		CodebookDto& operator=( const CodebookDto& ) = delete;

		/** @brief Move assignment operator */
		CodebookDto& operator=( CodebookDto&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the name of this codebook
		 * @return The codebook name
		 */
		[[nodiscard]] const std::string& name() const;

		/**
		 * @brief Get the values map of this codebook
		 * @return The map of group names to their corresponding values
		 */
		[[nodiscard]] const ValuesMap& values() const;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a CodebookDto from a nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<CodebookDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a CodebookDto from a nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized CodebookDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static CodebookDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this CodebookDto to a nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		[[nodiscard]] nlohmann::json toJson() const;

	private:
		//-------------------------------------------------------------------
		// Private Serialization Methods
		//-------------------------------------------------------------------

		/*
		 * Friend declarations for nlohmann::json serialization/deserialization.
		 * These enable Argument-Dependent Lookup (ADL) so nlohmann::json can find
		 * these functions and allow them to access private members.
		 */
		friend void to_json( nlohmann::json& j, const CodebookDto& dto );
		friend void from_json( const nlohmann::json& j, CodebookDto& dto );

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief Name identifier of the codebook (e.g., "positions", "quantities") */
		std::string m_name;

		/** @brief Map of group names to their corresponding values */
		ValuesMap m_values;
	};

	//=====================================================================
	// Codebooks Data Transfer Object
	//=====================================================================

	/**
	 * @brief Data transfer object for a collection of codebooks
	 * @details Represents a complete set of codebooks for a specific VIS version,
	 *          used for serialization to and from JSON format.
	 * @todo Consider refactoring for stricter immutability (e.g., const members) if direct modification
	 *       by deserialization (beyond initial construction) is not desired.
	 */
	class CodebooksDto final
	{
	public:
		//----------------------------------------------
		// Types and Aliases
		//----------------------------------------------

		/** @brief Type representing a collection of codebook DTOs */
		using Items = std::vector<CodebookDto>;

		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructor with parameters
		 * @param visVersion The VIS version
		 * @param items The collection of codebook DTOs
		 */
		explicit CodebooksDto( std::string visVersion, Items items );

		/** @brief Default constructor. */
		CodebooksDto() = default;

		/** @brief Copy constructor */
		CodebooksDto( const CodebooksDto& ) = default;

		/** @brief Move constructor */
		CodebooksDto( CodebooksDto&& ) noexcept = default;

		/** @brief Destructor */
		~CodebooksDto() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		CodebooksDto& operator=( const CodebooksDto& ) = delete;

		/** @brief Move assignment operator */
		CodebooksDto& operator=( CodebooksDto&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the VIS version string
		 * @return The VIS version string
		 */
		[[nodiscard]] const std::string& visVersion() const;

		/**
		 * @brief Get the collection of codebooks
		 * @return The vector of codebook DTOs
		 */
		[[nodiscard]] const Items& items() const;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a CodebooksDto from a nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<CodebooksDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a CodebooksDto from a nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized CodebooksDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static CodebooksDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this CodebooksDto to a nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		[[nodiscard]] nlohmann::json toJson() const;

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief VIS version string (e.g., "3.8a") */
		std::string m_visVersion;

		/** @brief Collection of codebook DTOs contained in this version */
		Items m_items;
	};
}
