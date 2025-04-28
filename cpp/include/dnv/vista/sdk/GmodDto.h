/**
 * @file GmodDto.h
 * @brief Data transfer objects for ISO 19848 Generic Product Model (GMOD) serialization
 * @details Provides data transfer objects used for serializing and deserializing
 *          the Generic Product Model (GMOD) according to the ISO 19848 standard.
 * @see ISO 19848:2018 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// GMOD Node Data Transfer Object
	//-------------------------------------------------------------------

	/**
	 * @brief Data transfer object for a GMOD (Generic Product Model) node
	 * @details Represents a node in the Generic Product Model as defined by ISO 19848.
	 *          Contains all metadata associated with a node including its category, type, code, name,
	 *          and optional attributes.
	 */
	class GmodNodeDto final
	{
	public:
		//-------------------------------------------------------------------
		// Types and Aliases
		//-------------------------------------------------------------------

		/** @brief Shorthand for the normal assignment names map type */
		using NormalAssignmentNamesMap = std::unordered_map<std::string, std::string>;

		//-------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		GmodNodeDto() = delete;

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

		/** @brief Copy constructor */
		GmodNodeDto( const GmodNodeDto& ) = default;

		/** @brief Move constructor */
		GmodNodeDto( GmodNodeDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodNodeDto() = default;

		//-------------------------------------------------------------------
		// Public Interface - Accessor Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Get the category classification
		 * @return The node's category
		 */
		const std::string& category() const;

		/**
		 * @brief Get the type classification
		 * @return The node's type
		 */
		const std::string& type() const;

		/**
		 * @brief Get the unique code identifier
		 * @return The node's unique code
		 */
		const std::string& code() const;

		/**
		 * @brief Get the human-readable name
		 * @return The node's name
		 */
		const std::string& name() const;

		/**
		 * @brief Get the optional common name/alias
		 * @return The common name if available, empty optional otherwise
		 */
		const std::optional<std::string>& commonName() const;

		/**
		 * @brief Get the optional detailed definition
		 * @return The detailed definition if available, empty optional otherwise
		 */
		const std::optional<std::string>& definition() const;

		/**
		 * @brief Get the optional common definition
		 * @return The common definition if available, empty optional otherwise
		 */
		const std::optional<std::string>& commonDefinition() const;

		/**
		 * @brief Get the optional installation flag
		 * @return The installation flag if available, empty optional otherwise
		 */
		const std::optional<bool>& installSubstructure() const;

		/**
		 * @brief Get the optional assignment name mapping
		 * @return The assignment name mapping if available, empty optional otherwise
		 */
		const std::optional<NormalAssignmentNamesMap>& normalAssignmentNames() const;

		//-------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a GmodNodeDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<GmodNodeDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a GmodNodeDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized GmodNodeDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		static GmodNodeDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this GmodNodeDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		GmodNodeDto& operator=( const GmodNodeDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		GmodNodeDto& operator=( GmodNodeDto&& ) noexcept = delete;

		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief Category classification of the node (e.g., "PRODUCT", "ASSET") */
		const std::string m_category;

		/** @brief Type classification within the category (e.g., "SELECTION", "TYPE") */
		const std::string m_type;

		/** @brief Unique code identifier for the node */
		const std::string m_code;

		/** @brief Human-readable name of the node */
		const std::string m_name;

		/** @brief Optional common name or alias */
		const std::optional<std::string> m_commonName;

		/** @brief Optional detailed definition */
		const std::optional<std::string> m_definition;

		/** @brief Optional common definition */
		const std::optional<std::string> m_commonDefinition;

		/** @brief Optional installation flag */
		const std::optional<bool> m_installSubstructure;

		/** @brief Optional mapping of normal assignment names */
		const std::optional<NormalAssignmentNamesMap> m_normalAssignmentNames;
	};

	//-------------------------------------------------------------------
	// GMOD Data Transfer Object
	//-------------------------------------------------------------------

	/**
	 * @brief Data transfer object for a complete GMOD (Generic Product Model)
	 * @details Represents the entire Generic Product Model for a specific VIS version,
	 *          containing all nodes and their relationships as defined in ISO 19848.
	 */
	class GmodDto final
	{
	public:
		//-------------------------------------------------------------------
		// Types and Aliases
		//-------------------------------------------------------------------

		/** @brief Type representing a relation between nodes */
		using Relation = std::vector<std::string>;

		/** @brief Type representing a collection of relations */
		using Relations = std::vector<Relation>;

		/** @brief Type representing a collection of GMOD nodes */
		using Items = std::vector<GmodNodeDto>;

		//-------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------

		/** @brief Default constructor - deleted for immutability */
		GmodDto() = delete;

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

		/** @brief Copy constructor */
		GmodDto( const GmodDto& ) = default;

		/** @brief Move constructor */
		GmodDto( GmodDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodDto() = default;

		//-------------------------------------------------------------------
		// Public Interface - Accessor Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Get the VIS version string
		 * @return The VIS version
		 */
		const std::string& visVersion() const;

		/**
		 * @brief Get the collection of GMOD node DTOs
		 * @return The vector of node DTOs
		 */
		const Items& items() const;

		/**
		 * @brief Get the collection of relationships between nodes
		 * @return The vector of relation arrays
		 */
		const Relations& relations() const;

		//-------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a GmodDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return Optional containing the deserialized object if successful, empty optional otherwise
		 */
		static std::optional<GmodDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a GmodDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized GmodDto
		 * @throws std::invalid_argument If required fields are missing or invalid
		 */
		static GmodDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this GmodDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------
		// Assignment Operators - deleted for immutability
		//-------------------------------------------------------------------

		/** @brief Copy assignment operator - deleted for immutability */
		GmodDto& operator=( const GmodDto& ) = delete;

		/** @brief Move assignment operator - deleted for immutability */
		GmodDto& operator=( GmodDto&& ) noexcept = delete;

		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief VIS version string (e.g., "3.8a") */
		const std::string m_visVersion;

		/** @brief Collection of GMOD node DTOs */
		const Items m_items;

		/** @brief Collection of relationships between nodes */
		const Relations m_relations;
	};
}
