/**
 * @file CodebooksDto.h
 * @brief Data transfer objects for ISO 19848 codebook serialization
 * @details Provides data transfer objects used for serializing and deserializing
 *          codebook information according to the ISO 19848 standard.
 *          These DTOs serve as an intermediate representation when loading or saving codebook data.
 * @see ISO 19848 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

#include <simdjson.h>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <optional>

namespace dnv::vista::sdk
{
	//=====================================================================
	// CodebookDto class
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
		// Types and aliases
		//----------------------------------------------

		/** @brief Type representing a collection of values within a group */
		using ValueGroup = std::vector<std::string>;

		/** @brief Type representing a mapping of group names to their values */
		using ValuesMap = std::unordered_map<std::string, ValueGroup>;

		//----------------------------------------------
		// Construction / destruction
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
		// Assignment operators
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
		[[nodiscard]] std::string_view name() const noexcept;

		/**
		 * @brief Get the values map of this codebook
		 * @return The map of group names to their corresponding values
		 */
		[[nodiscard]] const ValuesMap& values() const noexcept;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a CodebookDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<CodebookDto> tryFromJson( simdjson::dom::element element ) noexcept;

		/**
		 * @brief Try to deserialize a CodebookDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<CodebookDto> tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept;

		/**
		 * @brief Deserialize a CodebookDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return The deserialized CodebookDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		[[nodiscard]] static CodebookDto fromJson( simdjson::dom::element element );

		/**
		 * @brief Deserialize a CodebookDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return The deserialized CodebookDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		[[nodiscard]] static CodebookDto fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser );

		/**
		 * @brief Serialize this CodebookDto to a JSON string
		 * @return The serialized JSON string
		 */
		[[nodiscard]] std::string toJsonString() const;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief Name identifier of the codebook (e.g., "positions", "quantities") */
		std::string m_name;

		/** @brief Map of group names to their corresponding values */
		ValuesMap m_values;
	};

	//=====================================================================
	// CodebooksDto class
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
		// Types and aliases
		//----------------------------------------------

		/** @brief Type representing a collection of codebook DTOs */
		using Items = std::vector<CodebookDto>;

		//----------------------------------------------
		// Construction / destruction
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
		// Assignment operators
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
		[[nodiscard]] const std::string& visVersion() const noexcept;

		/**
		 * @brief Get the collection of codebooks
		 * @return The vector of codebook DTOs
		 */
		[[nodiscard]] const Items& items() const noexcept;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a CodebooksDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<CodebooksDto> tryFromJson( simdjson::dom::element element ) noexcept;

		/**
		 * @brief Try to deserialize a CodebooksDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<CodebooksDto> tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept;

		/**
		 * @brief Deserialize a CodebooksDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return The deserialized CodebooksDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		[[nodiscard]] static CodebooksDto fromJson( simdjson::dom::element element );

		/**
		 * @brief Deserialize a CodebooksDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return The deserialized CodebooksDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		[[nodiscard]] static CodebooksDto fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser );

		/**
		 * @brief Serialize this CodebooksDto to a JSON string
		 * @return The serialized JSON string
		 */
		[[nodiscard]] std::string toJsonString() const;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief VIS version string (e.g., "3.8a") */
		std::string m_visVersion;

		/** @brief Collection of codebook DTOs contained in this version */
		Items m_items;
	};
}
