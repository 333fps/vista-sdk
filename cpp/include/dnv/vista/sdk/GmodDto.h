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
		// Construction / Destruction
		//-------------------------------------------------------------------

		/** @brief Default constructor */
		GmodNodeDto() = default;

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
			std::optional<std::unordered_map<std::string, std::string>> normalAssignmentNames = std::nullopt );

		/** @brief Copy constructor */
		GmodNodeDto( const GmodNodeDto& ) = default;

		/** @brief Move constructor */
		GmodNodeDto( GmodNodeDto&& ) noexcept = default;

		/** @brief Copy assignment operator */
		GmodNodeDto& operator=( const GmodNodeDto& ) = default;

		/** @brief Move assignment operator */
		GmodNodeDto& operator=( GmodNodeDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodNodeDto() = default;

		//-------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------

		/** @brief Get the category classification */
		const std::string& category() const;

		/** @brief Get the type classification */
		const std::string& type() const;

		/** @brief Get the unique code identifier */
		const std::string& code() const;

		/** @brief Get the human-readable name */
		const std::string& name() const;

		/** @brief Get the optional common name/alias */
		const std::optional<std::string>& commonName() const;

		/** @brief Get the optional detailed definition */
		const std::optional<std::string>& definition() const;

		/** @brief Get the optional common definition */
		const std::optional<std::string>& commonDefinition() const;

		/** @brief Get the optional installation flag */
		const std::optional<bool>& installSubstructure() const;

		/** @brief Get the optional assignment name mapping */
		const std::optional<std::unordered_map<std::string, std::string>>& normalAssignmentNames() const;

		//-------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Deserialize a GmodNodeDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized GmodNodeDto
		 */
		static GmodNodeDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize a GmodNodeDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, GmodNodeDto& dto );

		/**
		 * @brief Serialize this GmodNodeDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

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
		std::optional<std::unordered_map<std::string, std::string>> m_normalAssignmentNames;
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
		// Construction / Destruction
		//-------------------------------------------------------------------

		/** @brief Default constructor */
		GmodDto() = default;

		/**
		 * @brief Constructor with parameters
		 * @param visVersion The VIS version string
		 * @param items Collection of GMOD node DTOs
		 * @param relations Collection of relationships between nodes
		 */
		GmodDto(
			std::string visVersion,
			std::vector<GmodNodeDto> items,
			std::vector<std::vector<std::string>> relations );

		/** @brief Copy constructor */
		GmodDto( const GmodDto& ) = default;

		/** @brief Move constructor */
		GmodDto( GmodDto&& ) noexcept = default;

		/** @brief Copy assignment operator */
		GmodDto& operator=( const GmodDto& ) = default;

		/** @brief Move assignment operator */
		GmodDto& operator=( GmodDto&& ) noexcept = default;

		/** @brief Destructor */
		~GmodDto() = default;

		//-------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------

		/** @brief Get the VIS version string */
		const std::string& visVersion() const;

		/** @brief Get the collection of GMOD node DTOs */
		const std::vector<GmodNodeDto>& items() const;

		/** @brief Get the collection of relationships between nodes */
		const std::vector<std::vector<std::string>>& relations() const;

		//-------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Deserialize a GmodDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @return The deserialized GmodDto
		 */
		static GmodDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize a GmodDto from a RapidJSON object
		 * @param json The RapidJSON object to deserialize
		 * @param dto Output parameter to receive the deserialized object
		 * @return True if deserialization was successful, false otherwise
		 */
		static bool tryFromJson( const rapidjson::Value& json, GmodDto& dto );

		/**
		 * @brief Serialize this GmodDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief VIS version string (e.g., "3.8a") */
		std::string m_visVersion;

		/** @brief Collection of GMOD node DTOs */
		std::vector<GmodNodeDto> m_items;

		/** @brief Collection of relationships between nodes */
		std::vector<std::vector<std::string>> m_relations;
	};
}
