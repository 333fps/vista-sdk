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
		// Construction / destruction
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
		// Assignment operators
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
		 * @brief Try to deserialize a GmodVersioningAssignmentChangeDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<GmodVersioningAssignmentChangeDto> tryFromJson( simdjson::dom::element element ) noexcept;

		/**
		 * @brief Try to deserialize a GmodVersioningAssignmentChangeDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<GmodVersioningAssignmentChangeDto> tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept;

		/**
		 * @brief Deserialize a GmodVersioningAssignmentChangeDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return The deserialized GmodVersioningAssignmentChangeDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 */
		[[nodiscard]] static GmodVersioningAssignmentChangeDto fromJson( simdjson::dom::element element );

		/**
		 * @brief Deserialize a GmodVersioningAssignmentChangeDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return The deserialized GmodVersioningAssignmentChangeDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 */
		[[nodiscard]] static GmodVersioningAssignmentChangeDto fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser );

		/**
		 * @brief Serialize this GmodVersioningAssignmentChangeDto to a JSON string
		 * @return The serialized JSON string
		 */
		[[nodiscard]] std::string toJsonString() const;

	private:
		//----------------------------------------------
		// Private member variables
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
		// Types and aliases
		//----------------------------------------------

		/** @brief Type representing a set of operation names */
		using OperationSet = std::unordered_set<std::string>;

		//----------------------------------------------
		// Construction / destruction
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
		// Assignment operators
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
		 * @brief Try to deserialize a GmodNodeConversionDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<GmodNodeConversionDto> tryFromJson( simdjson::dom::element element ) noexcept;

		/**
		 * @brief Try to deserialize a GmodNodeConversionDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<GmodNodeConversionDto> tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept;

		/**
		 * @brief Deserialize a GmodNodeConversionDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return The deserialized GmodNodeConversionDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 */
		[[nodiscard]] static GmodNodeConversionDto fromJson( simdjson::dom::element element );

		/**
		 * @brief Deserialize a GmodNodeConversionDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return The deserialized GmodNodeConversionDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 */
		[[nodiscard]] static GmodNodeConversionDto fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser );

		/**
		 * @brief Serialize this GmodNodeConversionDto to a JSON string
		 * @return The serialized JSON string
		 */
		[[nodiscard]] std::string toJsonString() const;

	private:
		//----------------------------------------------
		// Private member variables
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
		// Types and aliases
		//----------------------------------------------

		/** @brief Type representing a mapping of node codes to conversion information */
		using ItemsMap = std::unordered_map<std::string, GmodNodeConversionDto>;

		//----------------------------------------------
		// Construction / destruction
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
		// Assignment operators
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
		 * @brief Try to deserialize a GmodVersioningDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<GmodVersioningDto> tryFromJson( simdjson::dom::element element ) noexcept;

		/**
		 * @brief Try to deserialize a GmodVersioningDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		[[nodiscard]] static std::optional<GmodVersioningDto> tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept;

		/**
		 * @brief Deserialize a GmodVersioningDto from a simdjson element
		 * @param element The simdjson element to deserialize
		 * @return The deserialized GmodVersioningDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 */
		[[nodiscard]] static GmodVersioningDto fromJson( simdjson::dom::element element );

		/**
		 * @brief Deserialize a GmodVersioningDto from a JSON string
		 * @param jsonString The JSON string to parse
		 * @param parser Pre-allocated parser for reuse
		 * @return The deserialized GmodVersioningDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 */
		[[nodiscard]] static GmodVersioningDto fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser );

		/**
		 * @brief Serialize this GmodVersioningDto to a JSON string
		 * @return The serialized JSON string
		 */
		[[nodiscard]] std::string toJsonString() const;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief Target VIS version string (e.g., "3.8a") (JSON: "visRelease"). */
		std::string m_visVersion;

		/** @brief Map of GMOD node codes to their conversion rules (JSON: "items"). */
		ItemsMap m_items;
	};
}
