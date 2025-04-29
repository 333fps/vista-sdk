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
		DataChannelTypeNameDto() = delete;

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
		// Public Interface - Accessor Methods
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
		 * @brief Try to deserialize a DataChannelTypeNameDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<DataChannelTypeNameDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a DataChannelTypeNameDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized DataChannelTypeNameDto
		 * @throws std::invalid_argument If JSON format is invalid
		 */
		static DataChannelTypeNameDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this DataChannelTypeNameDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

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
		const std::string m_type;

		/** @brief Description of the type (JSON: "description") */
		const std::string m_description;
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
		DataChannelTypeNamesDto() = delete;

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
		// Public Interface - Accessor Methods
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
		 * @brief Try to deserialize a DataChannelTypeNamesDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<DataChannelTypeNamesDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a DataChannelTypeNamesDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized DataChannelTypeNamesDto
		 * @throws std::invalid_argument If JSON format is invalid
		 */
		static DataChannelTypeNamesDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this DataChannelTypeNamesDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

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
		const std::vector<DataChannelTypeNameDto> m_values;
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
		FormatDataTypeDto() = delete;

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
		// Public Interface - Accessor Methods
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
		 * @brief Try to deserialize a FormatDataTypeDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<FormatDataTypeDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a FormatDataTypeDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized FormatDataTypeDto
		 * @throws std::invalid_argument If JSON format is invalid
		 */
		static FormatDataTypeDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this FormatDataTypeDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

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
		const std::string m_type;

		/** @brief Description of the type (JSON: "description") */
		const std::string m_description;
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
		FormatDataTypesDto() = delete;

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
		// Public Interface - Accessor Methods
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
		 * @brief Try to deserialize a FormatDataTypesDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<FormatDataTypesDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a FormatDataTypesDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized FormatDataTypesDto
		 * @throws std::invalid_argument If JSON format is invalid
		 */
		static FormatDataTypesDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this FormatDataTypesDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

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
		const std::vector<FormatDataTypeDto> m_values;
	};
}
