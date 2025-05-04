/**
 * @file GmodDto.h
 * @brief Data transfer objects for ISO 19848 Generic Product Model (GMOD) serialization
 * @details Provides data transfer objects used for serializing and deserializing
 *          the Generic Product Model (GMOD) according to the ISO 19848 standard.
 *          These DTOs serve as an intermediate representation when loading or saving GMOD data.
 * @see ISO 19848 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// GMOD Node Data Transfer Object
	//=====================================================================

	/**
	 * @brief Data transfer object for a GMOD (Generic Product Model) node
	 * @details Represents a node in the Generic Product Model as defined by ISO 19848.
	 *          Contains all metadata associated with a node including its category, type, code, name,
	 *          and optional attributes.
	 * @todo Consider refactoring for stricter immutability (e.g., const members) if direct modification
	 *       by deserialization (beyond initial construction) is not desired.
	 */
	class GmodNodeDto final
	{
	public:
		//----------------------------------------------
		// Types and Aliases
		//----------------------------------------------

		/** @brief Shorthand for the normal assignment names map type */
		using NormalAssignmentNamesMap = std::unordered_map<std::string, std::string>;

		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructor with parameters
		 * @param category The category classification
		 * @param type The type classification
		 * @param code The unique code identifier
		 * @param name The human-readable name
		 * @param commonName Optional common name/alias
		 * @param definition Optional detailed definition
		 * @param commonDefinition Optional common definition
		 * @param installSubstructure Optional installation flag
		 * @param normalAssignmentNames Optional assignment name mapping
		 */
		GmodNodeDto(
			std::string category,
			std::string type,
			std::string code,
			std::string name,
			std::optional<std::string> commonName = std::nullopt,
			std::optional<std::string> definition = std::nullopt,
			std::optional<std::string> commonDefinition = std::nullopt,
			std::optional<bool> installSubstructure = std::nullopt,
			std::optional<NormalAssignmentNamesMap> normalAssignmentNames = std::nullopt );

		/** @brief Default constructor. */
		GmodNodeDto() = default;

		/** @brief Copy constructor */
		GmodNodeDto( const GmodNodeDto& ) = default;

		/** @brief Move constructor */
		GmodNodeDto( GmodNodeDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodNodeDto() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		GmodNodeDto& operator=( const GmodNodeDto& ) = delete;

		/** @brief Move assignment operator */
		GmodNodeDto& operator=( GmodNodeDto&& ) noexcept = default;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the category classification
		 * @return The node's category
		 */
		[[nodiscard]] const std::string& category() const;

		/**
		 * @brief Get the type classification
		 * @return The node's type
		 */
		[[nodiscard]] const std::string& type() const;

		/**
		 * @brief Get the unique code identifier
		 * @return The node's unique code
		 */
		[[nodiscard]] const std::string& code() const;

		/**
		 * @brief Get the human-readable name
		 * @return The node's name
		 */
		[[nodiscard]] const std::string& name() const;

		/**
		 * @brief Get the optional common name/alias
		 * @return The common name if available, empty optional otherwise
		 */
		[[nodiscard]] const std::optional<std::string>& commonName() const;

		/**
		 * @brief Get the optional detailed definition
		 * @return The detailed definition if available, empty optional otherwise
		 */
		[[nodiscard]] const std::optional<std::string>& definition() const;

		/**
		 * @brief Get the optional common definition
		 * @return The common definition if available, empty optional otherwise
		 */
		[[nodiscard]] const std::optional<std::string>& commonDefinition() const;

		/**
		 * @brief Get the optional installation flag
		 * @return The installation flag if available, empty optional otherwise
		 */
		[[nodiscard]] const std::optional<bool>& installSubstructure() const;

		/**
		 * @brief Get the optional assignment name mapping
		 * @return The assignment name mapping if available, empty optional otherwise
		 */
		[[nodiscard]] const std::optional<NormalAssignmentNamesMap>& normalAssignmentNames() const;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a GmodNodeDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<GmodNodeDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a GmodNodeDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized GmodNodeDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static GmodNodeDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this GmodNodeDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
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
		friend void to_json( nlohmann::json& j, const GmodNodeDto& dto );
		friend void from_json( const nlohmann::json& j, GmodNodeDto& dto );

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief Category classification of the node (e.g., "PRODUCT", "ASSET") */
		std::string m_category;

		/** @brief Type classification within the category (e.g., "SELECTION", "TYPE") */
		std::string m_type;

		/** @brief Unique code identifier for the node */
		std::string m_code;

		/** @brief Human-readable name of the node */
		std::string m_name;

		/** @brief Optional common name or alias */
		std::optional<std::string> m_commonName;

		/** @brief Optional detailed definition */
		std::optional<std::string> m_definition;

		/** @brief Optional common definition */
		std::optional<std::string> m_commonDefinition;

		/** @brief Optional installation flag */
		std::optional<bool> m_installSubstructure;

		/** @brief Optional mapping of normal assignment names */
		std::optional<NormalAssignmentNamesMap> m_normalAssignmentNames;
	};

	//=====================================================================
	// GMOD Data Transfer Object
	//=====================================================================

	/**
	 * @brief Data transfer object for a complete GMOD (Generic Product Model)
	 * @details Represents the entire Generic Product Model for a specific VIS version,
	 *          containing all nodes and their relationships as defined in ISO 19848.
	 * @todo Consider refactoring for stricter immutability (e.g., const members) if direct modification
	 *       by deserialization (beyond initial construction) is not desired.
	 */
	class GmodDto final
	{
	public:
		//----------------------------------------------
		// Types and Aliases
		//----------------------------------------------

		/** @brief Type representing a relation between nodes (typically a pair of node codes) */
		using Relation = std::vector<std::string>;

		/** @brief Type representing a collection of relations */
		using Relations = std::vector<Relation>;

		/** @brief Type representing a collection of GMOD nodes */
		using Items = std::vector<GmodNodeDto>;

		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructor with parameters
		 * @param visVersion The VIS version string
		 * @param items Collection of GMOD node DTOs
		 * @param relations Collection of relationships between nodes
		 */
		GmodDto(
			std::string visVersion,
			Items items,
			Relations relations );

		/** @brief Default constructor. */
		GmodDto() = default;

		/** @brief Copy constructor */
		GmodDto( const GmodDto& ) = default;

		/** @brief Move constructor */
		GmodDto( GmodDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodDto() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		GmodDto& operator=( const GmodDto& ) = delete;

		/** @brief Move assignment operator */
		GmodDto& operator=( GmodDto&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the VIS version string
		 * @return The VIS version
		 */
		[[nodiscard]] const std::string& visVersion() const;

		/**
		 * @brief Get the collection of GMOD node DTOs
		 * @return The vector of node DTOs
		 */
		[[nodiscard]] const Items& items() const;

		/**
		 * @brief Get the collection of relationships between nodes
		 * @return The vector of relation arrays
		 */
		[[nodiscard]] const Relations& relations() const;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a GmodDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<GmodDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a GmodDto from an nlohmann::json object
		 * @param json The nlohmann::json object to deserialize
		 * @return The deserialized GmodDto
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static GmodDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this GmodDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		[[nodiscard]] nlohmann::json toJson() const;

	private:
		//-------------------------------------------------------------------
		// Private Serialization Methods
		//-------------------------------------------------------------------

		/*
		 * Friend declarations for nlohmann::json serialization/deserialization.
		 * These enable Argument-Dependent Lookup (ADL) so nlohmann::json can find
		 * these functions and allow them to access private members.
		 */
		friend void to_json( nlohmann::json& j, const GmodDto& dto );
		friend void from_json( const nlohmann::json& j, GmodDto& dto );

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief VIS version string (e.g., "3.8a") */
		std::string m_visVersion;

		/** @brief Collection of GMOD node DTOs */
		Items m_items;

		/** @brief Collection of relationships between nodes */
		Relations m_relations;
	};
}
