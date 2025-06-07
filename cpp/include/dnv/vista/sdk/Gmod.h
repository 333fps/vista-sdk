/**
 * @file Gmod.h
 * @brief Generic Product Model (GMOD) interface for the DNV Vessel Information Structure (VIS).
 * @details This file defines the Gmod class, a core component of the VISTA SDK that represents
 *          the hierarchical structure of vessel components and systems according to the
 *          DNV Vessel Information Structure standard. It provides functionalities for
 *          accessing, navigating, and interpreting GMOD data.
 * @see ISO 19848 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

#include "ChdDictionary.h"
#include "GmodNode.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	enum class VisVersion;
	class GmodDto;
	class GmodPath;

	//=====================================================================
	// Gmod class
	//=====================================================================

	/**
	 * @brief Represents the Generic Product Model (GMOD) for a specific VIS version.
	 * @details The Gmod class provides an interface to the hierarchical structure of
	 *          vessel components and systems. It allows for node lookup, path parsing,
	 *          and iteration over its constituent nodes. The GMOD is typically initialized
	 *          from a GmodDto object or a pre-populated map of nodes.
	 *          Instances of this class are non-copyable and non-movable.
	 */
	class Gmod final
	{
	public:
		class Enumerator;

	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Constructs a Gmod instance from a GmodDto.
		 * @param version The VIS version this GMOD corresponds to.
		 * @param dto The data transfer object containing the GMOD structure and nodes.
		 * @throws std::invalid_argument If the provided DTO is invalid or essential data is missing.
		 * @throws std::out_of_range If essential nodes (like 'VE') are not found during construction.
		 * @throws Other exceptions based on underlying operations (e.g., memory allocation).
		 */
		Gmod( VisVersion version, const GmodDto& dto );

		/**
		 * @brief Constructs a Gmod instance from an initial map of nodes.
		 * @details This constructor is typically used for testing or specialized GMOD setup.
		 *          The GMOD copies these nodes. It's the caller's responsibility to ensure
		 *          the provided nodes form a consistent, linkable structure if relations
		 *          are implied or need to be established post-construction by other means.
		 *          This constructor does not perform relation linking or trimming by default.
		 * @param version The VIS version this GMOD corresponds to.
		 * @param initialNodeMap An unordered map where keys are node codes and values are GmodNode objects.
		 * @throws std::out_of_range If the root node 'VE' is not found in `initialNodeMap`.
		 */
		Gmod( VisVersion version, const std::unordered_map<std::string, GmodNode>& initialNodeMap );

		/** @brief Default constructor. */
		Gmod() = delete;

		/** @brief Copy constructor */
		Gmod( const Gmod& ) = default;

		/** @brief Move constructor */
		Gmod( Gmod&& ) noexcept = default;

		/** @brief Destructor */
		~Gmod() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		Gmod& operator=( const Gmod& ) = default;

		/** @brief Move assignment operator */
		Gmod& operator=( Gmod&& ) noexcept = default;

		//----------------------------------------------
		// Lookup operators
		//----------------------------------------------

		/**
		 * @brief Accesses a GmodNode by its code.
		 * @param key The code of the GmodNode to access.
		 * @return A const reference to the GmodNode.
		 * @throws std::out_of_range If no node with the specified key exists.
		 */
		const GmodNode& operator[]( std::string_view key ) const;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the VIS version of this GMOD instance.
		 * @return The VisVersion enum value.
		 */
		[[nodiscard]] VisVersion visVersion() const;

		/**
		 * @brief Gets the root node of the GMOD hierarchy.
		 * @details The root node (typically with code "VE") serves as the entry point
		 *          for traversing the GMOD structure.
		 * @return A const reference to the root GmodNode.
		 * @throws std::runtime_error If the GMOD is not properly initialized or has no root node.
		 */
		[[nodiscard]] const GmodNode& rootNode() const;

		//----------------------------------------------
		// Node query methods
		//----------------------------------------------

		/**
		 * @brief Tries to get a GmodNode by its code.
		 * @param code The code of the GmodNode to find (as a string_view).
		 * @param[out] node Output parameter; set to a pointer to the GmodNode if found, otherwise nullptr.
		 * @return True if the node was found, false otherwise.
		 */
		bool tryGetNode( std::string_view code, const GmodNode*& node ) const;

		//----------------------------------------------
		// Path parsing & navigation
		//----------------------------------------------

		/**
		 * @brief Parses a GMOD path string relative to the GMOD's root node.
		 * @param item The GMOD path string (e.g., "SYS[Propulsion System]/E[Main Engine]").
		 * @return A GmodPath object representing the parsed path.
		 * @throws std::invalid_argument If the path string is invalid or cannot be parsed.
		 */
		GmodPath parsePath( std::string_view item ) const;

		/**
		 * @brief Tries to parse a GMOD path string relative to the GMOD's root node.
		 * @param item The GMOD path string.
		 * @param[out] path Output parameter; set to the parsed GmodPath if successful.
		 * @return True if parsing was successful, false otherwise.
		 */
		bool tryParsePath( std::string_view item, std::optional<GmodPath>& path ) const;

		/**
		 * @brief Parses a full GMOD path string, which must start from the root "VE".
		 * @param item The full GMOD path string (e.g., "VE/SYS[Propulsion System]/E[Main Engine]").
		 * @return A GmodPath object representing the parsed path.
		 * @throws std::invalid_argument If the path string is invalid, not a full path, or cannot be parsed.
		 */
		GmodPath parseFromFullPath( const std::string& item ) const;

		/**
		 * @brief Tries to parse a full GMOD path string.
		 * @param item The full GMOD path string.
		 * @param[out] path Output parameter; set to the parsed GmodPath if successful.
		 * @return True if parsing was successful, false otherwise.
		 */
		bool tryParseFromFullPath( const std::string& item, std::optional<GmodPath>& path ) const;

		//----------------------------------------------
		// Static utility methods
		//----------------------------------------------

		/**
		 * @brief Checks if a GMOD node type string indicates a potential parent node.
		 * @param type The node type string (e.g., "SYS", "E", "VE").
		 * @return True if the type suggests it can be a parent, false otherwise.
		 */
		static bool isPotentialParent( const std::string& type );

		/**
		 * @brief Checks if a GMOD node, based on its metadata, is a leaf node.
		 * @param metadata The metadata of the GmodNode.
		 * @return True if the node is considered a leaf node, false otherwise.
		 */
		static bool isLeafNode( const GmodNodeMetadata& metadata );

		/**
		 * @brief Checks if a GMOD node, based on its metadata, is a function node.
		 * @param metadata The metadata of the GmodNode.
		 * @return True if the node is a function node, false otherwise.
		 */
		static bool isFunctionNode( const GmodNodeMetadata& metadata );

		/**
		 * @brief Checks if a GMOD node, based on its metadata, represents a product selection.
		 * @param metadata The metadata of the GmodNode.
		 * @return True if the node is a product selection, false otherwise.
		 */
		static bool isProductSelection( const GmodNodeMetadata& metadata );

		/**
		 * @brief Checks if a GMOD node, based on its metadata, represents a product type.
		 * @param metadata The metadata of the GmodNode.
		 * @return True if the node is a product type, false otherwise.
		 */
		static bool isProductType( const GmodNodeMetadata& metadata );

		/**
		 * @brief Checks if a GMOD node, based on its metadata, represents an asset.
		 * @param metadata The metadata of the GmodNode.
		 * @return True if the node is an asset, false otherwise.
		 */
		static bool isAsset( const GmodNodeMetadata& metadata );

		/**
		 * @brief Checks if a GMOD node, based on its metadata, is an asset function node.
		 * @param metadata The metadata of the GmodNode.
		 * @return True if the node is an asset function node, false otherwise.
		 */
		static bool isAssetFunctionNode( const GmodNodeMetadata& metadata );

		/**
		 * @brief Checks if the relationship between a parent and child node constitutes a product type assignment.
		 * @param parent Pointer to the parent GmodNode. Can be nullptr.
		 * @param child Pointer to the child GmodNode. Can be nullptr.
		 * @return True if it's a product type assignment, false otherwise or if inputs are invalid.
		 */
		static bool isProductTypeAssignment( const GmodNode* parent, const GmodNode* child );

		/**
		 * @brief Checks if the relationship between a parent and child node constitutes a product selection assignment.
		 * @param parent Pointer to the parent GmodNode. Can be nullptr.
		 * @param child Pointer to the child GmodNode. Can be nullptr.
		 * @return True if it's a product selection assignment, false otherwise or if inputs are invalid.
		 */
		static bool isProductSelectionAssignment( const GmodNode* parent, const GmodNode* child );

		//----------------------------------------------
		// Enumeration
		//----------------------------------------------

		/**
		 * @brief Gets an enumerator for iterating over all nodes within this Gmod instance.
		 * @return An Enumerator object.
		 */
		Enumerator enumerator() const;

		//----------------------------------------------
		// Gmod::Enumerator class
		//----------------------------------------------

		/**
		 * @brief An enumerator for iterating over all nodes within a Gmod instance.
		 * @details Provides a way to access each GmodNode in the GMOD's internal collection.
		 *          The order of iteration depends on the underlying ChdDictionary.
		 */
		class Enumerator final
		{
			friend class Gmod;

			//-----------------------------
			// Construction / destruction
			//-----------------------------

		private:
			/**
			 * @brief Private constructor, typically called by Gmod::enumerator().
			 * @param map Pointer to the ChdDictionary of GmodNodes to iterate over.
			 */
			Enumerator( const ChdDictionary<GmodNode>* map ) noexcept;

		public:
			/** @brief Default constructor. */
			Enumerator() = delete;

			/** @brief Copy constructor */
			Enumerator( const Enumerator& ) = delete;

			/** @brief Move constructor */
			Enumerator( Enumerator&& ) noexcept = delete;

			//-----------------------------
			// Assignment operators
			//-----------------------------

			/** @brief Copy assignment operator */
			Enumerator& operator=( const Enumerator& ) = delete;

			/** @brief Move assignment operator */
			Enumerator& operator=( Enumerator&& ) noexcept = delete;

			//-----------------------------
			// Iteration interface
			//-----------------------------

			/**
			 * @brief Gets the current GmodNode in the iteration.
			 * @return A const reference to the current GmodNode.
			 * @throws std::runtime_error If called when the enumerator is in an invalid state
			 *                            (e.g., before the first moveNext() or after iteration has ended).
			 */
			[[nodiscard]] const GmodNode& current() const;

			/**
			 * @brief Advances the enumerator to the next GmodNode in the collection.
			 * @return True if the enumerator was successfully advanced to the next node;
			 *         false if the end of the collection has been passed.
			 */
			bool next() noexcept;

			/**
			 * @brief Resets the enumerator to its initial state, positioned before the first node.
			 * @details After calling reset, next() must be called to access the first node.
			 */
			void reset() noexcept;

			//-----------------------------
			// Private member variables
			//-----------------------------

			const ChdDictionary<GmodNode>* m_sourceMapPtr;
			ChdDictionary<GmodNode>::Iterator m_currentMapIterator;
			bool m_isInitialState;
		};

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The VIS version this GMOD instance conforms to. */
		VisVersion m_visVersion{};

		/**
		 * @brief Pointer to the root node ("VE") of the GMOD hierarchy.
		 * @details This pointer refers to a GmodNode instance managed within m_nodeMap.
		 *          It is initialized during Gmod construction.
		 */
		GmodNode* m_rootNode;

		/**
		 * @brief The primary storage for GMOD nodes.
		 * @details This dictionary maps node codes (strings) to GmodNode objects.
		 *          It owns the GmodNode instances.
		 */
		ChdDictionary<GmodNode> m_nodeMap;
	};
}
