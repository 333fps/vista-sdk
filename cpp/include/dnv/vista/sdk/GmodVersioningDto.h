/**
 * @file GmodVersioningDto.h
 * @brief Data transfer objects for GMOD version conversion
 * @details Provides classes for serializing and deserializing Generic Product Model (GMOD)
 *          version conversion information according to the ISO 19848 standard.
 */

#pragma once

namespace dnv::vista::sdk
{
	/**
	 * @brief Data transfer object for GMOD node assignment changes
	 * @details Represents a change in assignment between VIS versions.
	 */
	class GmodVersioningAssignmentChangeDto final
	{
	public:
		//-------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------

		/** @brief Default constructor */
		GmodVersioningAssignmentChangeDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param oldAssignment Previous assignment value
		 * @param currentAssignment Current assignment value
		 */
		GmodVersioningAssignmentChangeDto( std::string oldAssignment, std::string currentAssignment );

		/** @brief Copy constructor */
		GmodVersioningAssignmentChangeDto( const GmodVersioningAssignmentChangeDto& ) = default;

		/** @brief Move constructor */
		GmodVersioningAssignmentChangeDto( GmodVersioningAssignmentChangeDto&& ) noexcept = default;

		/** @brief Copy assignment operator */
		GmodVersioningAssignmentChangeDto& operator=( const GmodVersioningAssignmentChangeDto& ) = default;

		/** @brief Move assignment operator */
		GmodVersioningAssignmentChangeDto& operator=( GmodVersioningAssignmentChangeDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodVersioningAssignmentChangeDto() = default;

		//-------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------

		/** @brief Get the previous assignment value */
		const std::string& oldAssignment() const;

		/** @brief Get the current assignment value */
		const std::string& currentAssignment() const;

		//-------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------

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

	private:
		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief Previous assignment value (JSON: "oldAssignment") */
		std::string m_oldAssignment;

		/** @brief Current assignment value (JSON: "currentAssignment") */
		std::string m_currentAssignment;
	};

	/**
	 * @brief Data transfer object for GMOD node conversion information
	 * @details Contains instructions for converting a node between VIS versions.
	 */
	class GmodNodeConversionDto final
	{
	public:
		//-------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------

		/** @brief Default constructor */
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

		/** @brief Copy constructor */
		GmodNodeConversionDto( const GmodNodeConversionDto& ) = default;

		/** @brief Move constructor */
		GmodNodeConversionDto( GmodNodeConversionDto&& ) noexcept = default;

		/** @brief Copy assignment operator */
		GmodNodeConversionDto& operator=( const GmodNodeConversionDto& ) = default;

		/** @brief Move assignment operator */
		GmodNodeConversionDto& operator=( GmodNodeConversionDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodNodeConversionDto() = default;

		//-------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------

		/** @brief Get the set of operations to apply */
		const std::unordered_set<std::string>& operations() const;

		/** @brief Get the source node code */
		const std::string& source() const;

		/** @brief Get the target node code */
		const std::string& target() const;

		/** @brief Get the old assignment code */
		const std::string& oldAssignment() const;

		/** @brief Get the new assignment code */
		const std::string& newAssignment() const;

		/** @brief Get whether to delete assignment */
		bool deleteAssignment() const;

		//-------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------

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

	private:
		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief Set of operations to apply (JSON: "operations") */
		std::unordered_set<std::string> m_operations;

		/** @brief Source node code (JSON: "source") */
		std::string m_source;

		/** @brief Target node code (JSON: "target") */
		std::string m_target;

		/** @brief Old assignment code (JSON: "oldAssignment") */
		std::string m_oldAssignment;

		/** @brief New assignment code (JSON: "newAssignment") */
		std::string m_newAssignment;

		/** @brief Whether to delete assignment (JSON: "deleteAssignment") */
		bool m_deleteAssignment;
	};

	/**
	 * @brief Data transfer object for GMOD version conversion information
	 * @details Contains all node conversion information for a specific VIS version.
	 */
	class GmodVersioningDto final
	{
	public:
		//-------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------

		/** @brief Default constructor */
		GmodVersioningDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param visVersion VIS version identifier
		 * @param items Map of node codes to conversion information
		 */
		GmodVersioningDto( std::string visVersion, std::unordered_map<std::string, GmodNodeConversionDto> items );

		/** @brief Copy constructor */
		GmodVersioningDto( const GmodVersioningDto& ) = default;

		/** @brief Move constructor */
		GmodVersioningDto( GmodVersioningDto&& ) noexcept = default;

		/** @brief Copy assignment operator */
		GmodVersioningDto& operator=( const GmodVersioningDto& ) = default;

		/** @brief Move assignment operator */
		GmodVersioningDto& operator=( GmodVersioningDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodVersioningDto() = default;

		//-------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------

		/** @brief Get the VIS version identifier */
		const std::string& visVersion() const;

		/** @brief Get the map of node codes to conversion information */
		const std::unordered_map<std::string, GmodNodeConversionDto>& items() const;

		//-------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------

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

	private:
		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief VIS version identifier (JSON: "visRelease") */
		std::string m_visVersion;

		/** @brief Map of node codes to their conversion information (JSON: "items") */
		std::unordered_map<std::string, GmodNodeConversionDto> m_items;
	};
}
