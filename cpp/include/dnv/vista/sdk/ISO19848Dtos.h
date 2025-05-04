/**
 * @file ISO19848Dtos.h
 * @brief Data Transfer Objects for ISO 19848 standard
 *
 * This file defines immutable data transfer objects used for serializing and deserializing
 * ISO 19848 data channel and format type information. These DTOs are used as an
 * intermediate representation when loading or saving JSON data.
 */

#pragma once

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Single Data Channel Type
	//-------------------------------------------------------------------------

	/**
	 * @brief Data Transfer Object (DTO) for a single data channel type name
	 *
	 * Represents a type name and its description as defined in ISO 19848.
	 * This is an immutable class, constructed once and not modifiable afterward.
	 */
	class DataChannelTypeNameDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		DataChannelTypeNameDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param type The type name
		 * @param description The description of the type
		 */
		DataChannelTypeNameDto( std::string type, std::string description );

		/** @brief Copy constructor */
		DataChannelTypeNameDto( const DataChannelTypeNameDto& ) = default;

		/** @brief Move constructor */
		DataChannelTypeNameDto( DataChannelTypeNameDto&& ) noexcept = default;

		/** @brief Destructor */
		~DataChannelTypeNameDto() = default;

		//-------------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the type name
		 * @return Type name
		 */
		const std::string& type() const;

		/**
		 * @brief Get the description
		 * @return Description of the type
		 */
		const std::string& description() const;

		//-------------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a DataChannelTypeNameDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<DataChannelTypeNameDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a DataChannelTypeNameDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized DataChannelTypeNameDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static DataChannelTypeNameDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this DataChannelTypeNameDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		nlohmann::json toJson() const;

		friend void from_json( const nlohmann::json& j, DataChannelTypeNameDto& dto );
		friend void to_json( nlohmann::json& j, const DataChannelTypeNameDto& dto );

	private:
		//-------------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		DataChannelTypeNameDto& operator=( const DataChannelTypeNameDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		DataChannelTypeNameDto& operator=( DataChannelTypeNameDto&& ) noexcept = delete;

		//-------------------------------------------------------------------------
		// Private Member Variables (Immutable)
		//-------------------------------------------------------------------------

		/** @brief Type name (JSON: "type") */
		std::string m_type;

		/** @brief Description of the type (JSON: "description") */
		std::string m_description;
	};

	//-------------------------------------------------------------------------
	// Collection of Data Channel Types
	//-------------------------------------------------------------------------

	/**
	 * @brief Data Transfer Object (DTO) for a collection of data channel type names
	 *
	 * Represents a collection of data channel type names and their descriptions.
	 * This is an immutable class, constructed once and not modifiable afterward.
	 */
	class DataChannelTypeNamesDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		DataChannelTypeNamesDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param values A collection of data channel type name values
		 */
		explicit DataChannelTypeNamesDto( std::vector<DataChannelTypeNameDto> values );

		/** @brief Copy constructor */
		DataChannelTypeNamesDto( const DataChannelTypeNamesDto& ) = default;

		/** @brief Move constructor */
		DataChannelTypeNamesDto( DataChannelTypeNamesDto&& ) noexcept = default;

		/** @brief Destructor */
		~DataChannelTypeNamesDto() = default;

		//-------------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the collection of data channel type names
		 * @return Collection of data channel type names
		 */
		const std::vector<DataChannelTypeNameDto>& values() const;

		//-------------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a DataChannelTypeNamesDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<DataChannelTypeNamesDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a DataChannelTypeNamesDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized DataChannelTypeNamesDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static DataChannelTypeNamesDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this DataChannelTypeNamesDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		nlohmann::json toJson() const;

		friend void from_json( const nlohmann::json& j, DataChannelTypeNamesDto& dto );
		friend void to_json( nlohmann::json& j, const DataChannelTypeNamesDto& dto );

	private:
		//-------------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		DataChannelTypeNamesDto& operator=( const DataChannelTypeNamesDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		DataChannelTypeNamesDto& operator=( DataChannelTypeNamesDto&& ) noexcept = delete;

		//-------------------------------------------------------------------------
		// Private Member Variables (Immutable)
		//-------------------------------------------------------------------------

		/** @brief Collection of data channel type name values (JSON: "values") */
		std::vector<DataChannelTypeNameDto> m_values;
	};

	//-------------------------------------------------------------------------
	// Single Format Data Type
	//-------------------------------------------------------------------------

	/**
	 * @brief Data Transfer Object (DTO) for a single format data type
	 *
	 * Represents a format data type and its description as defined in ISO 19848.
	 * This is an immutable class, constructed once and not modifiable afterward.
	 */
	class FormatDataTypeDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		FormatDataTypeDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param type The type name
		 * @param description The description of the type
		 */
		FormatDataTypeDto( std::string type, std::string description );

		/** @brief Copy constructor */
		FormatDataTypeDto( const FormatDataTypeDto& ) = default;

		/** @brief Move constructor */
		FormatDataTypeDto( FormatDataTypeDto&& ) noexcept = default;

		/** @brief Destructor */
		~FormatDataTypeDto() = default;

		//-------------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the type name
		 * @return Type name
		 */
		const std::string& type() const;

		/**
		 * @brief Get the description
		 * @return Description of the type
		 */
		const std::string& description() const;

		//-------------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a FormatDataTypeDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<FormatDataTypeDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a FormatDataTypeDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized FormatDataTypeDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static FormatDataTypeDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this FormatDataTypeDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		nlohmann::json toJson() const;

		friend void from_json( const nlohmann::json& j, FormatDataTypeDto& dto );
		friend void to_json( nlohmann::json& j, const FormatDataTypeDto& dto );

	private:
		//-------------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		FormatDataTypeDto& operator=( const FormatDataTypeDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		FormatDataTypeDto& operator=( FormatDataTypeDto&& ) noexcept = delete;

		//-------------------------------------------------------------------------
		// Private Member Variables (Immutable)
		//-------------------------------------------------------------------------

		/** @brief Type name (JSON: "type") */
		std::string m_type;

		/** @brief Description of the type (JSON: "description") */
		std::string m_description;
	};

	//-------------------------------------------------------------------------
	// Collection of Format Data Types
	//-------------------------------------------------------------------------

	/**
	 * @brief Data Transfer Object (DTO) for a collection of format data types
	 *
	 * Represents a collection of format data types and their descriptions.
	 * This is an immutable class, constructed once and not modifiable afterward.
	 */
	class FormatDataTypesDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		FormatDataTypesDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param values A collection of format data type values
		 */
		explicit FormatDataTypesDto( std::vector<FormatDataTypeDto> values );

		/** @brief Copy constructor */
		FormatDataTypesDto( const FormatDataTypesDto& ) = default;

		/** @brief Move constructor */
		FormatDataTypesDto( FormatDataTypesDto&& ) noexcept = default;

		/** @brief Destructor */
		~FormatDataTypesDto() = default;

		//-------------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the collection of format data types
		 * @return Collection of format data types
		 */
		const std::vector<FormatDataTypeDto>& values() const;

		//-------------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a FormatDataTypesDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<FormatDataTypesDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a FormatDataTypesDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized FormatDataTypesDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static FormatDataTypesDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this FormatDataTypesDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		nlohmann::json toJson() const;

		friend void from_json( const nlohmann::json& j, FormatDataTypesDto& dto );
		friend void to_json( nlohmann::json& j, const FormatDataTypesDto& dto );

	private:
		//-------------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		FormatDataTypesDto& operator=( const FormatDataTypesDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		FormatDataTypesDto& operator=( FormatDataTypesDto&& ) noexcept = delete;

		//-------------------------------------------------------------------------
		// Private Member Variables (Immutable)
		//-------------------------------------------------------------------------

		/** @brief Collection of format data type values (JSON: "values") */
		std::vector<FormatDataTypeDto> m_values;
	};
}
