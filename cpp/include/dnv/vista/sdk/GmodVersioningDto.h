#pragma once

namespace dnv::vista::sdk
{
	/**
	 * @brief Data transfer object for GMOD node assignment changes
	 *
	 * Represents a change in assignment between VIS versions.
	 */
	class GmodVersioningAssignmentChangeDto final
	{
	public: // TODO
		/**
		 * @brief Default constructor
		 */
		GmodVersioningAssignmentChangeDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param oldAssignment Previous assignment value
		 * @param currentAssignment Current assignment value
		 */
		GmodVersioningAssignmentChangeDto( std::string oldAssignment, std::string currentAssignment );

		/**
		 * @brief Deserialize a GmodVersioningAssignmentChangeDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized GmodVersioningAssignmentChangeDto
		 */
		static GmodVersioningAssignmentChangeDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize a GmodVersioningAssignmentChangeDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, GmodVersioningAssignmentChangeDto& dto );

		/**
		 * @brief Serialize this GmodVersioningAssignmentChangeDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

		/** @brief Previous assignment value (JSON: "oldAssignment") */
		std::string oldAssignment;

		/** @brief Current assignment value (JSON: "currentAssignment") */
		std::string currentAssignment;
	};

	/**
	 * @brief Data transfer object for GMOD node conversion information
	 *
	 * Contains instructions for converting a node between VIS versions.
	 */
	class GmodNodeConversionDto final
	{
	public: // TODO
		/**
		 * @brief Default constructor
		 */
		GmodNodeConversionDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param operations Set of operations to apply
		 * @param source Source node code
		 * @param target Target node code
		 * @param oldAssignment Old assignment code
		 * @param newAssignment New assignment code
		 * @param deleteAssignment Whether to delete assignment
		 */
		GmodNodeConversionDto(
			std::unordered_set<std::string> operations,
			std::string source,
			std::string target,
			std::string oldAssignment,
			std::string newAssignment,
			bool deleteAssignment );

		/**
		 * @brief Deserialize a GmodNodeConversionDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized GmodNodeConversionDto
		 */
		static GmodNodeConversionDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize a GmodNodeConversionDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, GmodNodeConversionDto& dto );

		/**
		 * @brief Serialize this GmodNodeConversionDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

		/** @brief Set of operations to apply (JSON: "operations") */
		std::unordered_set<std::string> operations;

		/** @brief Source node code (JSON: "source") */
		std::string source;

		/** @brief Target node code (JSON: "target") */
		std::string target;

		/** @brief Old assignment code (JSON: "oldAssignment") */
		std::string oldAssignment;

		/** @brief New assignment code (JSON: "newAssignment") */
		std::string newAssignment;

		/** @brief Whether to delete assignment (JSON: "deleteAssignment") */
		bool deleteAssignment = false;
	};

	/**
	 * @brief Data transfer object for GMOD version conversion information
	 *
	 * Contains all node conversion information for a specific VIS version.
	 */
	class GmodVersioningDto final
	{
	public: // TODO
		/**
		 * @brief Default constructor
		 */
		GmodVersioningDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param visVersion VIS version identifier
		 * @param items Map of node codes to conversion information
		 */
		GmodVersioningDto( std::string visVersion, std::unordered_map<std::string, GmodNodeConversionDto> items );

		/**
		 * @brief Deserialize a GmodVersioningDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized GmodVersioningDto
		 */
		static GmodVersioningDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize a GmodVersioningDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, GmodVersioningDto& dto );

		/**
		 * @brief Serialize this GmodVersioningDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

		/** @brief VIS version identifier (JSON: "visRelease") */
		std::string visVersion;

		/** @brief Map of node codes to their conversion information (JSON: "items") */
		std::unordered_map<std::string, GmodNodeConversionDto> items;
	};
}
