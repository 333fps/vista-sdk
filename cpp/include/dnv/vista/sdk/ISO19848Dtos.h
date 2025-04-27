/**
 * @file ISO19848Dtos.h
 * @brief Data Transfer Objects for ISO 19848 standard
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
	 */
	class DataChannelTypeNameDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		DataChannelTypeNameDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param type The type name
		 * @param description The description of the type
		 */
		DataChannelTypeNameDto( std::string type, std::string description );

		/**
		 * @brief Copy constructor
		 */
		DataChannelTypeNameDto( const DataChannelTypeNameDto& ) = default;

		/**
		 * @brief Move constructor
		 */
		DataChannelTypeNameDto( DataChannelTypeNameDto&& ) noexcept = default;

		/**
		 * @brief Destructor
		 */
		~DataChannelTypeNameDto() = default;

		/**
		 * @brief Copy assignment operator
		 */
		DataChannelTypeNameDto& operator=( const DataChannelTypeNameDto& ) = default;

		/**
		 * @brief Move assignment operator
		 */
		DataChannelTypeNameDto& operator=( DataChannelTypeNameDto&& ) noexcept = default;

		//-------------------------------------------------------------------------
		// Accessor Methods
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
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @return Deserialized DTO
		 * @throws std::runtime_error If JSON format is invalid
		 */
		static DataChannelTypeNameDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, DataChannelTypeNameDto& dto );

		/**
		 * @brief Serialize to JSON
		 * @param allocator JSON allocator to use
		 * @return JSON value representation
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
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
	 */
	class DataChannelTypeNamesDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		DataChannelTypeNamesDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param values A collection of data channel type name values
		 */
		explicit DataChannelTypeNamesDto( std::vector<DataChannelTypeNameDto> values );

		/**
		 * @brief Copy constructor
		 */
		DataChannelTypeNamesDto( const DataChannelTypeNamesDto& ) = default;

		/**
		 * @brief Move constructor
		 */
		DataChannelTypeNamesDto( DataChannelTypeNamesDto&& ) noexcept = default;

		/**
		 * @brief Destructor
		 */
		~DataChannelTypeNamesDto() = default;

		/**
		 * @brief Copy assignment operator
		 */
		DataChannelTypeNamesDto& operator=( const DataChannelTypeNamesDto& ) = default;

		/**
		 * @brief Move assignment operator
		 */
		DataChannelTypeNamesDto& operator=( DataChannelTypeNamesDto&& ) noexcept = default;

		//-------------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the collection of data channel type names
		 * @return Collection of data channel type names
		 */
		const std::vector<DataChannelTypeNameDto>& values() const;

		//-------------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @return Deserialized DTO
		 * @throws std::runtime_error If JSON format is invalid
		 */
		static DataChannelTypeNamesDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, DataChannelTypeNamesDto& dto );

		/**
		 * @brief Serialize to JSON
		 * @param allocator JSON allocator to use
		 * @return JSON value representation
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
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
	 */
	class FormatDataTypeDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		FormatDataTypeDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param type The type name
		 * @param description The description of the type
		 */
		FormatDataTypeDto( std::string type, std::string description );

		/**
		 * @brief Copy constructor
		 */
		FormatDataTypeDto( const FormatDataTypeDto& ) = default;

		/**
		 * @brief Move constructor
		 */
		FormatDataTypeDto( FormatDataTypeDto&& ) noexcept = default;

		/**
		 * @brief Destructor
		 */
		~FormatDataTypeDto() = default;

		/**
		 * @brief Copy assignment operator
		 */
		FormatDataTypeDto& operator=( const FormatDataTypeDto& ) = default;

		/**
		 * @brief Move assignment operator
		 */
		FormatDataTypeDto& operator=( FormatDataTypeDto&& ) noexcept = default;

		//-------------------------------------------------------------------------
		// Accessor Methods
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
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @return Deserialized DTO
		 * @throws std::runtime_error If JSON format is invalid
		 */
		static FormatDataTypeDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, FormatDataTypeDto& dto );

		/**
		 * @brief Serialize to JSON
		 * @param allocator JSON allocator to use
		 * @return JSON value representation
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
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
	 */
	class FormatDataTypesDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		FormatDataTypesDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param values A collection of format data type values
		 */
		explicit FormatDataTypesDto( std::vector<FormatDataTypeDto> values );

		/**
		 * @brief Copy constructor
		 */
		FormatDataTypesDto( const FormatDataTypesDto& ) = default;

		/**
		 * @brief Move constructor
		 */
		FormatDataTypesDto( FormatDataTypesDto&& ) noexcept = default;

		/**
		 * @brief Destructor
		 */
		~FormatDataTypesDto() = default;

		/**
		 * @brief Copy assignment operator
		 */
		FormatDataTypesDto& operator=( const FormatDataTypesDto& ) = default;

		/**
		 * @brief Move assignment operator
		 */
		FormatDataTypesDto& operator=( FormatDataTypesDto&& ) noexcept = default;

		//-------------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the collection of format data types
		 * @return Collection of format data types
		 */
		const std::vector<FormatDataTypeDto>& values() const;

		//-------------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @return Deserialized DTO
		 * @throws std::runtime_error If JSON format is invalid
		 */
		static FormatDataTypesDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, FormatDataTypesDto& dto );

		/**
		 * @brief Serialize to JSON
		 * @param allocator JSON allocator to use
		 * @return JSON value representation
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		/** @brief Collection of format data type values (JSON: "values") */
		std::vector<FormatDataTypeDto> m_values;
	};
}
