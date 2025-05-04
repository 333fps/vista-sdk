/**
 * @file GmodVersioningDto.h
 * @brief Data transfer objects for GMOD version conversion
 *
 * This file defines the data transfer objects used for serializing and deserializing
 * GMOD version conversion information according to the ISO 19848 standard. These DTOs
 * are used as an intermediate representation when loading or saving version conversion
 * data to JSON.
 */

#pragma once

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// GMOD Versioning Data Transfer Objects
	//-------------------------------------------------------------------

	/**
	 * @brief Data transfer object for GMOD node assignment changes
	 *
	 * Represents serialized information about assignment changes between VIS versions.
	 * Each assignment change contains the previous and current assignment values.
	 */
	class GmodVersioningAssignmentChangeDto final
	{
	public:
		//-------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		GmodVersioningAssignmentChangeDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param oldAssignment The previous assignment value
		 * @param currentAssignment The current assignment value
		 */
		GmodVersioningAssignmentChangeDto( std::string oldAssignment, std::string currentAssignment );

		/** @brief Copy constructor */
		GmodVersioningAssignmentChangeDto( const GmodVersioningAssignmentChangeDto& ) = default;

		/** @brief Move constructor */
		GmodVersioningAssignmentChangeDto( GmodVersioningAssignmentChangeDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodVersioningAssignmentChangeDto() = default;

		//-------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------

		/**
		 * @brief Get the old assignment value
		 * @return The previous assignment value
		 */
		const std::string& oldAssignment() const;

		/**
		 * @brief Get the current assignment value
		 * @return The current assignment value
		 */
		const std::string& currentAssignment() const;

		//-------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a GmodVersioningAssignmentChangeDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<GmodVersioningAssignmentChangeDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a GmodVersioningAssignmentChangeDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized GmodVersioningAssignmentChangeDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static GmodVersioningAssignmentChangeDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this GmodVersioningAssignmentChangeDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		nlohmann::json toJson() const;

		friend void from_json( const nlohmann::json& j, GmodVersioningAssignmentChangeDto& dto );
		friend void to_json( nlohmann::json& j, const GmodVersioningAssignmentChangeDto& dto );

	private:
		//-------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		GmodVersioningAssignmentChangeDto& operator=( const GmodVersioningAssignmentChangeDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		GmodVersioningAssignmentChangeDto& operator=( GmodVersioningAssignmentChangeDto&& ) noexcept = delete;

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
	 *
	 * Represents serialized information about how to convert a GMOD node between VIS versions.
	 * Contains operations to apply, source and target codes, and assignment change information.
	 */
	class GmodNodeConversionDto final
	{
	public:
		//-------------------------------------------------------------------
		// Types and Aliases
		//-------------------------------------------------------------------

		/** @brief Type representing a set of operation names */
		using OperationSet = std::unordered_set<std::string>;

		//-------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		GmodNodeConversionDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param operations The set of operations to apply
		 * @param source The source node code
		 * @param target The target node code
		 * @param oldAssignment The old assignment code
		 * @param newAssignment The new assignment code
		 * @param deleteAssignment Whether the assignment should be deleted
		 */
		GmodNodeConversionDto(
			OperationSet operations,
			std::string source,
			std::string target,
			std::string oldAssignment,
			std::string newAssignment,
			bool deleteAssignment );

		/** @brief Copy constructor */
		GmodNodeConversionDto( const GmodNodeConversionDto& ) = default;

		/** @brief Move constructor */
		GmodNodeConversionDto( GmodNodeConversionDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodNodeConversionDto() = default;

		//-------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------

		/**
		 * @brief Get the set of operations to apply
		 * @return The set of operation names
		 */
		const OperationSet& operations() const;

		/**
		 * @brief Get the source node code
		 * @return The source node code
		 */
		const std::string& source() const;

		/**
		 * @brief Get the target node code
		 * @return The target node code
		 */
		const std::string& target() const;

		/**
		 * @brief Get the old assignment code
		 * @return The old assignment code
		 */
		const std::string& oldAssignment() const;

		/**
		 * @brief Get the new assignment code
		 * @return The new assignment code
		 */
		const std::string& newAssignment() const;

		/**
		 * @brief Check if the assignment should be deleted
		 * @return True if the assignment should be deleted, false otherwise
		 */
		bool deleteAssignment() const;

		//-------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a GmodNodeConversionDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<GmodNodeConversionDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a GmodNodeConversionDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized GmodNodeConversionDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static GmodNodeConversionDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this GmodNodeConversionDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		nlohmann::json toJson() const;

		friend void from_json( const nlohmann::json& j, GmodNodeConversionDto& dto );
		friend void to_json( nlohmann::json& j, const GmodNodeConversionDto& dto );

	private:
		//-------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		GmodNodeConversionDto& operator=( const GmodNodeConversionDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		GmodNodeConversionDto& operator=( GmodNodeConversionDto&& ) noexcept = delete;

		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief Set of operations to apply (JSON: "operations") */
		OperationSet m_operations;

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
	 * @brief Data transfer object for a collection of GMOD version conversions
	 *
	 * Represents a complete set of node conversion information for a specific VIS version,
	 * used for serialization to and from JSON format.
	 */
	class GmodVersioningDto final
	{
	public:
		//-------------------------------------------------------------------
		// Types and Aliases
		//-------------------------------------------------------------------

		/** @brief Type representing a mapping of node codes to conversion information */
		using ItemsMap = std::unordered_map<std::string, GmodNodeConversionDto>;

		//-------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		GmodVersioningDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param visVersion The VIS version
		 * @param items The map of node codes to conversion information
		 */
		GmodVersioningDto( std::string visVersion, ItemsMap items );

		/** @brief Copy constructor */
		GmodVersioningDto( const GmodVersioningDto& ) = default;

		/** @brief Move constructor */
		GmodVersioningDto( GmodVersioningDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodVersioningDto() = default;

		//-------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------

		/**
		 * @brief Get the VIS version string
		 * @return The VIS version string
		 */
		const std::string& visVersion() const;

		/**
		 * @brief Get the map of node codes to conversion information
		 * @return The map of node codes to their conversion DTOs
		 */
		const ItemsMap& items() const;

		//-------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a GmodVersioningDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<GmodVersioningDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a GmodVersioningDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized GmodVersioningDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static GmodVersioningDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this GmodVersioningDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		nlohmann::json toJson() const;

		friend void from_json( const nlohmann::json& j, GmodVersioningDto& dto );
		friend void to_json( nlohmann::json& j, const GmodVersioningDto& dto );

	private:
		//-------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		GmodVersioningDto& operator=( const GmodVersioningDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		GmodVersioningDto& operator=( GmodVersioningDto&& ) noexcept = delete;

		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief VIS version string (e.g., "3.8a") */
		std::string m_visVersion;

		/** @brief Map of node codes to their conversion information */
		ItemsMap m_items;
	};
}
