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
	public: // TODO
		//-------------------------------------------------------------------------
		// Constructors
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

		//-------------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @return Deserialized DTO
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

		//-------------------------------------------------------------------------
		// Data Members
		//-------------------------------------------------------------------------

		/**
		 * @brief Type name (JSON: "type")
		 */
		std::string type;

		/**
		 * @brief Description of the type (JSON: "description")
		 */
		std::string description;
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
	public: // TODO
		//-------------------------------------------------------------------------
		// Constructors
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

		//-------------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @return Deserialized DTO
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

		//-------------------------------------------------------------------------
		// Data Members
		//-------------------------------------------------------------------------

		/**
		 * @brief Collection of data channel type name values (JSON: "values")
		 */
		std::vector<DataChannelTypeNameDto> values;
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
	public: // TODO
		//-------------------------------------------------------------------------
		// Constructors
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

		//-------------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @return Deserialized DTO
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

		//-------------------------------------------------------------------------
		// Data Members
		//-------------------------------------------------------------------------

		/**
		 * @brief Type name (JSON: "type")
		 */
		std::string type;

		/**
		 * @brief Description of the type (JSON: "description")
		 */
		std::string description;
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
	public: // TODO
		//-------------------------------------------------------------------------
		// Constructors
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

		//-------------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize from JSON
		 * @param json JSON value to deserialize from
		 * @return Deserialized DTO
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

		//-------------------------------------------------------------------------
		// Data Members
		//-------------------------------------------------------------------------

		/**
		 * @brief Collection of format data type values (JSON: "values")
		 */
		std::vector<FormatDataTypeDto> values;
	};
}
