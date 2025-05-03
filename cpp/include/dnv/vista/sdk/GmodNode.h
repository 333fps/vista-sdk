/**
 * @file GmodNode.h
 * @brief Generic Model (GMOD) node and metadata classes
 *
 * This file defines the GmodNode and GmodNodeMetadata classes which represent
 * the fundamental building blocks of the Generic Product Model as defined in
 * ISO 19848. These classes provide the node structure, relationships, and
 * type classification used throughout the VISTA SDK.
 *
 * @see ISO 19848 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

#include "GmodDto.h"
#include "Locations.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Forward declarations
	//-------------------------------------------------------------------

	enum class VisVersion;
	class ParsingErrors;
	class Gmod;

	/**
	 * @brief Metadata for a GMOD (Generic Product Model) node
	 *
	 * Contains descriptive information about a node including its category, type,
	 * name, and other optional properties as defined in ISO 19848.
	 */
	class GmodNodeMetadata final
	{
	public:
		//-------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------

		/**
		 * @brief Construct a new GmodNodeMetadata object
		 *
		 * @param category The category classification of the node (e.g., "PRODUCT", "ASSET")
		 * @param type The type classification within the category (e.g., "SELECTION", "TYPE")
		 * @param name The human-readable name of the node
		 * @param commonName Optional common name or alias for the node
		 * @param definition Optional detailed definition of the node
		 * @param commonDefinition Optional common definition for the node
		 * @param installSubstructure Optional flag indicating if substructure should be installed
		 * @param normalAssignmentNames Optional mapping of normal assignment names
		 */
		GmodNodeMetadata(
			const std::string& category,
			const std::string& type,
			const std::string& name,
			const std::optional<std::string>& commonName = std::nullopt,
			const std::optional<std::string>& definition = std::nullopt,
			const std::optional<std::string>& commonDefinition = std::nullopt,
			const std::optional<bool>& installSubstructure = std::nullopt,
			const std::unordered_map<std::string, std::string>& normalAssignmentNames = {} );

		//-------------------------------------------------------------------
		// Basic Property Accessors
		//-------------------------------------------------------------------

		/**
		 * @brief Get the category of the node
		 * @return Reference to the category string
		 */
		const std::string& category() const;

		/**
		 * @brief Get the type of the node
		 * @return Reference to the type string
		 */
		const std::string& type() const;

		/**
		 * @brief Get the full type string (category + type)
		 * @return Reference to the full type string
		 */
		const std::string& fullType() const;

		/**
		 * @brief Get the name of the node
		 * @return Reference to the name string
		 */
		const std::string& name() const;

		/**
		 * @brief Get the common name of the node
		 * @return Optional reference to the common name
		 */
		const std::optional<std::string>& commonName() const;

		/**
		 * @brief Get the definition of the node
		 * @return Optional reference to the definition
		 */
		const std::optional<std::string>& definition() const;

		/**
		 * @brief Get the common definition of the node
		 * @return Optional reference to the common definition
		 */
		const std::optional<std::string>& commonDefinition() const;

		/**
		 * @brief Get the install substructure flag
		 * @return Optional reference to the install substructure flag
		 */
		const std::optional<bool>& installSubstructure() const;

		/**
		 * @brief Get the normal assignment names mapping
		 * @return Reference to the map of normal assignment names
		 */
		const std::unordered_map<std::string, std::string>& normalAssignmentNames() const;

		//-------------------------------------------------------------------
		// Operators
		//-------------------------------------------------------------------

		/**
		 * @brief Equality comparison operator
		 * @param other The metadata to compare with
		 * @return True if all properties are equal
		 */
		bool operator==( const GmodNodeMetadata& other ) const;

		/**
		 * @brief Inequality comparison operator
		 * @param other The metadata to compare with
		 * @return True if any property differs
		 */
		bool operator!=( const GmodNodeMetadata& other ) const;

	private:
		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief Category classification */
		std::string m_category;

		/** @brief Type classification */
		std::string m_type;

		/** @brief Human-readable name */
		std::string m_name;

		/** @brief Optional common name/alias */
		std::optional<std::string> m_commonName;

		/** @brief Optional detailed definition */
		std::optional<std::string> m_definition;

		/** @brief Optional common definition */
		std::optional<std::string> m_commonDefinition;

		/** @brief Optional installation flag */
		std::optional<bool> m_installSubstructure;

		/** @brief Assignment name mapping */
		std::unordered_map<std::string, std::string> m_normalAssignmentNames;

		/** @brief Combined category and type */
		std::string m_fullType;
	};

	/**
	 * @brief A node in the Generic Product Model (GMOD)
	 *
	 * Represents a single node in the hierarchical structure of the Generic Product Model
	 * as defined by ISO 19848. Contains metadata, relationships to parent/child
	 * nodes, and optional location information.
	 */
	class GmodNode final
	{
	public:
		//-------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------

		GmodNode();

		GmodNode( const GmodNode& other, bool b );

		/**
		 * @brief Construct from DTO
		 *
		 * @param version The VIS version
		 * @param dto The data transfer object containing node information
		 */
		GmodNode( VisVersion version, const GmodNodeDto& dto );

		/**
		 * @brief Copy constructor (deleted).
		 *
		 * Copying GmodNodes is disallowed because it would lead to shallow copies
		 * of the internal parent/child pointer vectors, causing ownership ambiguity
		 * and potential dangling pointers or double-free issues. Nodes are managed
		 * centrally, typically within a Gmod instance.
		 */
		GmodNode( const GmodNode& ) = delete;

		/**
		 * @brief Copy assignment operator (deleted).
		 *
		 * Copy-assigning GmodNodes is disallowed for the same reasons as copy construction.
		 */
		GmodNode& operator=( const GmodNode& ) = delete;

		/**
		 * @brief Move constructor
		 */
		GmodNode( GmodNode&& ) noexcept;

		/**
		 * @brief Move assignment operator
		 */
		GmodNode& operator=( GmodNode&& ) noexcept = default;

		/**
		 * @brief Destructor
		 */
		~GmodNode() = default;

		//-------------------------------------------------------------------
		// Basic Property Accessors
		//-------------------------------------------------------------------

		/**
		 * @brief Get the unique code identifier
		 * @return Reference to the code string
		 */
		const std::string& code() const;

		/**
		 * @brief Get the location information
		 * @return Optional reference to the location
		 */
		const std::optional<Location>& location() const;

		/**
		 * @brief Get the VIS version
		 * @return The VIS version enum value
		 */
		VisVersion visVersion() const;

		/**
		 * @brief Get the node metadata
		 * @return Reference to the metadata
		 */
		const GmodNodeMetadata& metadata() const;

		/**
		 * @brief Calculate a hash code for this GmodNode.
		 * @return A size_t hash code value.
		 */
		[[nodiscard]] size_t hashCode() const noexcept;

		//-------------------------------------------------------------------
		// Relationship Accessors
		//-------------------------------------------------------------------

		/**
		 * @brief Get the child nodes
		 * @return Reference to the vector of child node pointers
		 */
		const std::vector<GmodNode*>& children() const;

		/**
		 * @brief Get the parent nodes
		 * @return Reference to the vector of parent node pointers
		 */
		const std::vector<GmodNode*>& parents() const;

		/**
		 * @brief Get the product type node associated with this node
		 * @return Pointer to the product type node, or nullptr if not applicable
		 */
		const GmodNode* productType() const;

		/**
		 * @brief Get the product selection node associated with this node
		 * @return Pointer to the product selection node, or nullptr if not applicable
		 */
		const GmodNode* productSelection() const;

		//-------------------------------------------------------------------
		// Node Location Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Create a copy of this node without location information
		 * @return A new GmodNode with location removed
		 */
		GmodNode withoutLocation() const;

		/**
		 * @brief Create a copy of this node with the specified location
		 *
		 * @param location The location string to add
		 * @return A new GmodNode with the specified location
		 * @throws std::invalid_argument If the location string is invalid
		 */
		GmodNode withLocation( const std::string& location ) const;

		/**
		 * @brief Create a copy of this node with the specified location object
		 *
		 * @param location The location object to add
		 * @return A new GmodNode with the specified location
		 */
		GmodNode withLocation( const Location& location ) const;

		/**
		 * @brief Try to create a copy of this node with the specified location
		 *
		 * @param locationStr The location string to add
		 * @return A new GmodNode with the location if valid, otherwise a copy of this node
		 */
		GmodNode tryWithLocation( const std::string& locationStr ) const;

		/**
		 * @brief Try to create a copy of this node with the specified location, capturing errors
		 *
		 * @param locationStr The location string to add
		 * @param errors Object to receive parsing errors
		 * @return A new GmodNode with the location if valid, otherwise a copy of this node
		 */
		GmodNode tryWithLocation( const std::string& locationStr, ParsingErrors& errors ) const;

		/**
		 * @brief Try to create a copy of this node with the optional location
		 *
		 * @param location The optional location to add
		 * @return A new GmodNode with the location if provided, otherwise a copy of this node
		 */
		GmodNode tryWithLocation( const std::optional<Location>& location ) const;

		//-------------------------------------------------------------------
		// Node Type Checking Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Check if the node is individualizable
		 *
		 * A node is individualizable if it can be assigned a unique instance identifier.
		 *
		 * @param isTargetNode Flag indicating if this is a target node
		 * @param isInSet Flag indicating if this node is in a set
		 * @return True if the node is individualizable
		 */
		bool isIndividualizable( bool isTargetNode = false, bool isInSet = false ) const;

		/**
		 * @brief Check if the node is a function composition
		 * @return True if the node is an asset or product function composition
		 */
		bool isFunctionComposition() const;

		/**
		 * @brief Check if the node is mappable
		 * @return True if the node can be mapped to other nodes
		 */
		bool isMappable() const;

		/**
		 * @brief Check if the node is a product selection
		 * @return True if the node is a product selection
		 */
		bool isProductSelection() const;

		/**
		 * @brief Check if the node is a product type
		 * @return True if the node is a product type
		 */
		bool isProductType() const;

		/**
		 * @brief Check if the node is an asset
		 * @return True if the node is an asset
		 */
		bool isAsset() const;

		/**
		 * @brief Check if the node is a leaf node
		 * @return True if the node is a leaf node in the hierarchy
		 */
		bool isLeafNode() const;

		/**
		 * @brief Check if the node is a function node
		 * @return True if the node is a function node
		 */
		bool isFunctionNode() const;

		/**
		 * @brief Check if the node is an asset function node
		 * @return True if the node is an asset function node
		 */
		bool isAssetFunctionNode() const;

		/**
		 * @brief Check if the node is the root node
		 * @return True if the node is the root of the hierarchy (code is "VE")
		 */
		bool isRoot() const;

		//-------------------------------------------------------------------
		// Node Relationship Query Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Check if this node has a specific child node
		 *
		 * @param node The potential child node
		 * @return True if the node is a child of this node
		 */
		bool isChild( const GmodNode& node ) const;

		/**
		 * @brief Check if this node has a child with a specific code
		 *
		 * @param code The code to check for
		 * @return True if a child with the specified code exists
		 */
		bool isChild( const std::string& code ) const;

		//-------------------------------------------------------------------
		// Operators
		//-------------------------------------------------------------------

		/**
		 * @brief Equality comparison method (explicit)
		 *
		 * Nodes are considered equal if they have the same code and location
		 *
		 * @param other The node to compare with
		 * @return True if nodes are equal
		 */
		[[nodiscard]] bool equals( const GmodNode& other ) const;

		/**
		 * @brief Equality comparison operator
		 *
		 * Nodes are considered equal if they have the same code and location
		 *
		 * @param other The node to compare with
		 * @return True if nodes are equal
		 */
		bool operator==( const GmodNode& other ) const;

		/**
		 * @brief Inequality comparison operator
		 *
		 * @param other The node to compare with
		 * @return True if nodes are not equal
		 */
		bool operator!=( const GmodNode& other ) const;

		//-------------------------------------------------------------------
		// Utility Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Convert the node to a string representation
		 * @return String representation in the format "code[-location]"
		 */
		std::string toString() const;

		/**
		 * @brief Write the node's string representation to a string stream
		 * @param builder The string stream to write to
		 */
		void toString( std::stringstream& builder ) const;

		//-------------------------------------------------------------------
		// Relationship Management Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Add a child node
		 * @param child Pointer to the child node to add
		 */
		void addChild( GmodNode* child );

		/**
		 * @brief Add a parent node
		 * @param parent Pointer to the parent node to add
		 */
		void addParent( GmodNode* parent );

		/**
		 * @brief Trim excess capacity and build child code index
		 *
		 * Optimizes memory usage and builds an index of child codes for fast lookups.
		 */
		void trim();

		//-------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------

		/** @brief Unique identifier code */
		std::string m_code;

		/** @brief Optional location information */
		std::optional<Location> m_location;

		/** @brief VIS version of this node */
		VisVersion m_visVersion;

		/** @brief Node metadata */
		GmodNodeMetadata m_metadata;

		/** @brief Child nodes */
		std::vector<GmodNode*> m_children;

		/** @brief Parent nodes */
		std::vector<GmodNode*> m_parents;

		/** @brief Set of child codes for fast lookups */
		std::unordered_set<std::string> m_childrenSet;
	};
}
