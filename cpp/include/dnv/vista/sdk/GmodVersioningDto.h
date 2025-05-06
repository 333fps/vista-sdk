/**
 * @file GmodVersioningDto.h
 * @brief Data transfer objects for GMOD version conversion information
 * @details Provides data transfer objects used for serializing and deserializing
 *          rules and changes related to converting GMOD (Generic Product Model)
 *          data between different VIS (Vessel Information Structure) versions.
 *          These DTOs serve as an intermediate representation when loading or
 *          saving GMOD versioning data, typically from JSON formatted files.
 *          The versioning information itself is guided by the principles of
 *          managing GMOD data as outlined in ISO 19848.
 * @see ISO 19848 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// GMOD Versioning Data Transfer Objects
	//=====================================================================

	/**
	 * @brief Data transfer object for GMOD node assignment changes during versioning.
	 * @details Represents the change in an assignment's value for a GMOD node when
	 *          converting between VIS versions. It specifies the old and new
	 *          values of a particular assignment.
	 * @todo Consider refactoring for stricter immutability (e.g., const members) if direct modification
	 *       by deserialization (beyond initial construction) is not desired.
	 */
	class GmodVersioningAssignmentChangeDto final
	{
	public:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructor with parameters
		 * @param oldAssignment The previous assignment value
		 * @param currentAssignment The current assignment value
		 */
		explicit GmodVersioningAssignmentChangeDto( std::string oldAssignment, std::string currentAssignment );

		/** @brief Default constructor. */
		GmodVersioningAssignmentChangeDto() = default;

		/** @brief Copy constructor */
		GmodVersioningAssignmentChangeDto( const GmodVersioningAssignmentChangeDto& ) = default;

		/** @brief Move constructor */
		GmodVersioningAssignmentChangeDto( GmodVersioningAssignmentChangeDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodVersioningAssignmentChangeDto() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		GmodVersioningAssignmentChangeDto& operator=( const GmodVersioningAssignmentChangeDto& ) = delete;

		/** @brief Move assignment operator */
		GmodVersioningAssignmentChangeDto& operator=( GmodVersioningAssignmentChangeDto&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the old assignment value.
		 * @return The previous assignment value.
		 */
		[[nodiscard]] const std::string& oldAssignment() const;

		/**
		 * @brief Get the current assignment value.
		 * @return The current (new) assignment value.
		 */
		[[nodiscard]] const std::string& currentAssignment() const;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a GmodVersioningAssignmentChangeDto from an nlohmann::json object.
		 * @param json The nlohmann::json object to deserialize.
		 * @return Optional containing the deserialized object if successful, empty optional otherwise.
		 */
		static std::optional<GmodVersioningAssignmentChangeDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a GmodVersioningAssignmentChangeDto from an nlohmann::json object.
		 * @param json The nlohmann::json object to deserialize.
		 * @return The deserialized GmodVersioningAssignmentChangeDto.
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors).
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur.
		 */
		static GmodVersioningAssignmentChangeDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this GmodVersioningAssignmentChangeDto to an nlohmann::json object.
		 * @return The serialized nlohmann::json object.
		 */
		[[nodiscard]] nlohmann::json toJson() const;

	private:
		//----------------------------------------------
		// Private Serialization Methods
		//----------------------------------------------

		/*
		 * Friend declarations for nlohmann::json serialization/deserialization.
		 * These enable Argument-Dependent Lookup (ADL) so nlohmann::json can find
		 * these functions and allow them to access private members.
		 */
		friend void from_json( const nlohmann::json& j, GmodVersioningAssignmentChangeDto& dto );
		friend void to_json( nlohmann::json& j, const GmodVersioningAssignmentChangeDto& dto );

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief Previous assignment value (JSON: "oldAssignment") */
		std::string m_oldAssignment;

		/** @brief Current assignment value (JSON: "currentAssignment") */
		std::string m_currentAssignment;
	};

	//=====================================================================
	// GMOD Node Conversion Transfer Object
	//=====================================================================

	/**
	 * @brief Data transfer object for GMOD node conversion rules.
	 * @details Represents the set of rules for converting a specific GMOD node
	 *          from one VIS version to another. This includes operations to apply,
	 *          source and target node codes, and details about assignment changes.
	 * @todo Consider refactoring for stricter immutability (e.g., const members) if direct modification
	 *       by deserialization (beyond initial construction) is not desired.
	 */
	class GmodNodeConversionDto final
	{
	public:
		//----------------------------------------------
		// Types and Aliases
		//----------------------------------------------

		/** @brief Type representing a set of operation names */
		using OperationSet = std::unordered_set<std::string>;

		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructor with parameters.
		 * @param operations The set of operations to apply during conversion.
		 * @param source The source GMOD node code (from the old VIS version).
		 * @param target The target GMOD node code (in the new VIS version).
		 * @param oldAssignment The old assignment code being converted.
		 * @param newAssignment The new assignment code after conversion.
		 * @param deleteAssignment Flag indicating if the assignment should be deleted.
		 */
		explicit GmodNodeConversionDto(
			OperationSet operations,
			std::string source,
			std::string target,
			std::string oldAssignment,
			std::string newAssignment,
			bool deleteAssignment );

		/** @brief Default constructor. */
		GmodNodeConversionDto() = default;

		/** @brief Copy constructor */
		GmodNodeConversionDto( const GmodNodeConversionDto& ) = default;

		/** @brief Move constructor */
		GmodNodeConversionDto( GmodNodeConversionDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodNodeConversionDto() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		GmodNodeConversionDto& operator=( const GmodNodeConversionDto& ) = delete;

		/** @brief Move assignment operator */
		GmodNodeConversionDto& operator=( GmodNodeConversionDto&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the set of operations to apply.
		 * @return The set of operation names.
		 */
		[[nodiscard]] const OperationSet& operations() const;

		/**
		 * @brief Get the source GMOD node code.
		 * @return The source node code.
		 */
		[[nodiscard]] const std::string& source() const;

		/**
		 * @brief Get the target GMOD node code.
		 * @return The target node code.
		 */
		[[nodiscard]] const std::string& target() const;

		/**
		 * @brief Get the old assignment code.
		 * @return The old assignment code.
		 */
		[[nodiscard]] const std::string& oldAssignment() const;

		/**
		 * @brief Get the new assignment code.
		 * @return The new assignment code.
		 */
		[[nodiscard]] const std::string& newAssignment() const;

		/**
		 * @brief Check if the assignment should be deleted.
		 * @return True if the assignment should be deleted, false otherwise.
		 */
		[[nodiscard]] bool deleteAssignment() const;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a GmodNodeConversionDto from an nlohmann::json object.
		 * @param json The nlohmann::json object to deserialize.
		 * @return Optional containing the deserialized object if successful, empty optional otherwise.
		 */
		static std::optional<GmodNodeConversionDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a GmodNodeConversionDto from an nlohmann::json object.
		 * @param json The nlohmann::json object to deserialize.
		 * @return The deserialized GmodNodeConversionDto.
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors).
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur.
		 */
		static GmodNodeConversionDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this GmodNodeConversionDto to an nlohmann::json object.
		 * @return The serialized nlohmann::json object.
		 */
		[[nodiscard]] nlohmann::json toJson() const;

	private:
		//----------------------------------------------
		// Private Serialization Methods
		//----------------------------------------------

		/*
		 * Friend declarations for nlohmann::json serialization/deserialization.
		 * These enable Argument-Dependent Lookup (ADL) so nlohmann::json can find
		 * these functions and allow them to access private members.
		 */
		friend void from_json( const nlohmann::json& j, GmodNodeConversionDto& dto );
		friend void to_json( nlohmann::json& j, const GmodNodeConversionDto& dto );

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

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

	//=====================================================================
	// GMOD Versioning Data Transfer Object
	//=====================================================================

	/**
	 * @brief Data transfer object for a complete set of GMOD versioning rules.
	 * @details Represents all GMOD node conversion rules for a specific target VIS version.
	 *          It contains a map of GMOD node codes (typically from the source version)
	 *          to their respective conversion details.
	 * @todo Consider refactoring for stricter immutability (e.g., const members) if direct modification
	 *       by deserialization (beyond initial construction) is not desired.
	 */
	class GmodVersioningDto final
	{
	public:
		//----------------------------------------------
		// Types and Aliases
		//----------------------------------------------

		/** @brief Type representing a mapping of node codes to conversion information */
		using ItemsMap = std::unordered_map<std::string, GmodNodeConversionDto>;

		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructor with parameters.
		 * @param visVersion The target VIS version string for these conversion rules.
		 * @param items A map of GMOD node codes to their conversion DTOs.
		 */
		explicit GmodVersioningDto( std::string visVersion, ItemsMap items );

		/** @brief Default constructor. */
		GmodVersioningDto() = default;

		/** @brief Copy constructor */
		GmodVersioningDto( const GmodVersioningDto& ) = default;

		/** @brief Move constructor */
		GmodVersioningDto( GmodVersioningDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodVersioningDto() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		GmodVersioningDto& operator=( const GmodVersioningDto& ) = delete;

		/** @brief Move assignment operator */
		GmodVersioningDto& operator=( GmodVersioningDto&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the target VIS version string.
		 * @return The VIS version string.
		 */
		[[nodiscard]] const std::string& visVersion() const;

		/**
		 * @brief Get the map of GMOD node codes to their conversion rules.
		 * @return The map of node codes to their conversion DTOs.
		 */
		[[nodiscard]] const ItemsMap& items() const;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a GmodVersioningDto from an nlohmann::json object.
		 * @param json The nlohmann::json object to deserialize.
		 * @return Optional containing the deserialized object if successful, empty optional otherwise.
		 */
		static std::optional<GmodVersioningDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a GmodVersioningDto from an nlohmann::json object.
		 * @param json The nlohmann::json object to deserialize.
		 * @return The deserialized GmodVersioningDto.
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors).
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur.
		 */
		static GmodVersioningDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this GmodVersioningDto to an nlohmann::json object.
		 * @return The serialized nlohmann::json object.
		 */
		[[nodiscard]] nlohmann::json toJson() const;

	private:
		//----------------------------------------------
		// Private Serialization Methods
		//----------------------------------------------

		/*
		 * Friend declarations for nlohmann::json serialization/deserialization.
		 * These enable Argument-Dependent Lookup (ADL) so nlohmann::json can find
		 * these functions and allow them to access private members.
		 */
		friend void from_json( const nlohmann::json& j, GmodVersioningDto& dto );
		friend void to_json( nlohmann::json& j, const GmodVersioningDto& dto );

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief Target VIS version string (e.g., "3.8a") (JSON: "visRelease"). */
		std::string m_visVersion;

		/** @brief Map of GMOD node codes to their conversion rules (JSON: "items"). */
		ItemsMap m_items;
	};
}
