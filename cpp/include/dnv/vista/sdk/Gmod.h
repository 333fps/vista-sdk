/**
 * @file Gmod.h
 * @brief Generic Product Model (GMOD) interface for the DNV Vessel Information Structure (VIS)
 *
 * This file defines the Gmod class, a core component of the VISTA SDK that represents
 * the hierarchical structure of vessel components and systems according to the
 * DNV Vessel Information Structure standard.
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
	// Gmod Class
	//=====================================================================

	/**
	 * @brief Generic Product Model (GMOD) for vessel information systems
	 *
	 * Represents the hierarchical structure of vessel components and systems
	 * according to the DNV Vessel Information Structure (VIS) standard. The GMOD
	 * provides functionality for traversing the hierarchy, locating nodes, and
	 * working with vessel component paths.
	 *
	 * The GMOD is constructed from a GmodDto object. During construction, it builds
	 * an internal graph of GmodNode objects. A critical part of this process involves
	 * resolving and linking parent-child relationships using raw pointers. This includes
	 * an internal pointer fix-up mechanism to ensure pointer validity after nodes
	 * are organized into their final storage.
	 *
	 * The class is non-copyable but movable.
	 */
	class Gmod final
	{
	public:
		//----------------------------------------------
		// Public Type Definitions
		//----------------------------------------------

		/**
		 * @brief Result values returned by traversal handler functions
		 */
		enum class TraversalHandlerResult
		{
			Stop = 0,
			SkipSubtree,
			Continue
		};

		/**
		 * @brief Options to configure GMOD traversal behavior
		 */
		struct TraversalOptions final
		{
			/** @brief Default maximum number of times a node can be visited */
			static constexpr int DEFAULT_MAX_TRAVERSAL_OCCURRENCE = 1;

			/** @brief Maximum number of times a node can occur in a traversal path */
			int maxTraversalOccurrence = DEFAULT_MAX_TRAVERSAL_OCCURRENCE;

			/** @brief Maximum total nodes to visit during traversal */
			size_t maxNodes = 100000;

			/** @brief Default constructor */
			TraversalOptions();

			/**
			 * @brief Constructor with custom occurrence limit
			 * @param maxTraversalOccurrenceValue Maximum times a node can be visited
			 */
			explicit TraversalOptions( int maxTraversalOccurrenceValue );
		};

		/**
		 * @brief Handler function type for traversal without state
		 * @param parents Vector of pointers to parent nodes in the current path
		 * @param node The current node being visited
		 * @return Result indicating how to continue traversal
		 */
		using TraverseHandler = std::function<TraversalHandlerResult( const std::vector<const GmodNode*>& parents, const GmodNode& node )>;

		/**
		 * @brief Iterator for traversing nodes in the GMOD
		 */
		class Iterator final
		{
		public:
			/** @brief STL iterator traits */
			using iterator_category = std::forward_iterator_tag;
			using value_type = GmodNode;
			using difference_type = std::ptrdiff_t;
			using pointer = const GmodNode*;
			using reference = const GmodNode&;

			/** @brief Default constructor */
			Iterator() = default;

			/**
			 * @brief Constructor from inner iterator
			 * @param innerIt CHD dictionary iterator
			 */
			Iterator( ChdDictionary<GmodNode>::Iterator innerIt );

			/** @brief Dereference operator */
			reference operator*() const;

			/** @brief Arrow operator */
			pointer operator->() const;

			/** @brief Pre-increment operator */
			Iterator& operator++();

			/** @brief Post-increment operator */
			Iterator operator++( int );

			/** @brief Equality operator */
			bool operator==( const Iterator& other ) const;

			/** @brief Inequality operator */
			bool operator!=( const Iterator& other ) const;

		private:
			/** @brief Inner CHD dictionary iterator */
			ChdDictionary<GmodNode>::Iterator m_innerIt;
		};

	private:
		//=====================================================================
		// Parents Class
		//=====================================================================

		/**
		 * @brief Helper class to manage parent nodes during traversal
		 *
		 * Tracks the parent nodes in traversal and manages occurrence counting
		 * to prevent infinite recursion in cyclic structures.
		 */
		class Parents final
		{
		public:
			//----------------------------------------------
			// Public Methods
			//----------------------------------------------

			/**
			 * @brief Add a parent node to the chain
			 * @param parent Node to add
			 */
			void push( const GmodNode* parent );

			/**
			 * @brief Remove the last parent node
			 */
			void pop();

			/**
			 * @brief Count occurrences of a node in the parent chain
			 * @param node Node to count
			 * @return Number of times the node appears in the chain
			 */
			int occurrences( const GmodNode& node ) const;

			/**
			 * @brief Get the last node in the chain, if any
			 * @return Pointer to last node, or nullptr if chain is empty
			 */
			const GmodNode* lastOrDefault() const;

			/**
			 * @brief Direct access to the parent node pointers
			 * @return Constant reference to the vector of parent node pointers
			 */
			const std::vector<const GmodNode*>& nodePointers() const;

		private:
			//----------------------------------------------
			// Private Members
			//----------------------------------------------

			/** @brief Parent node pointers in traversal order. */
			std::vector<const GmodNode*> m_nodes;

			/** @brief Node occurrence counter. */
			std::unordered_map<std::string, int> m_occurrences;
		};

	public:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/** @brief Default constructor is deleted - Gmod requires initialization data */
		Gmod() = delete;

		/**
		 * @brief Construct from DTO and VIS version
		 * @param version The Vessel Information Structure version
		 * @param dto Data transfer object containing GMOD data
		 */
		Gmod( VisVersion version, const GmodDto& dto );

		/**
		 * @brief Construct from node map and VIS version
		 * @param version The Vessel Information Structure version
		 * @param nodeMap Map of node codes to GmodNode objects
		 */
		Gmod( VisVersion version, const std::unordered_map<std::string, GmodNode>& nodeMap );

		/** @brief Copy constructor */
		Gmod( const Gmod& ) = delete;

		/** @brief Move constructor */
		Gmod( Gmod&& ) noexcept = default;

		/** @brief Destructor */
		~Gmod() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		Gmod& operator=( const Gmod& ) = delete;

		/** @brief Move assignment operator */
		Gmod& operator=( Gmod&& ) noexcept = default;

		//----------------------------------------------
		// Basic Access Methods
		//----------------------------------------------

		/**
		 * @brief Access node by code using the subscript operator.
		 * @param key Node code to look up.
		 * @return Constant reference to the node if found.
		 * @throws std::out_of_range if the key is not found in the GMOD.
		 */
		[[nodiscard]] const GmodNode& operator[]( const std::string& key ) const;

		/**
		 * @brief Get the VIS version used by this GMOD
		 * @return The VIS version
		 */
		[[nodiscard]] VisVersion visVersion() const;

		/**
		 * @brief Get the root node of the GMOD hierarchy.
		 *
		 * The root node is typically the node with code "VE".
		 * This method returns a reference to the internally stored root node,
		 * which has undergone the pointer fix-up process during construction.
		 *
		 * @return Constant reference to the root node.
		 */
		[[nodiscard]] const GmodNode& rootNode() const;

		/**
		 * @brief Tries to get a pointer to the node associated with the specified code.
		 * @param code The code of the node to find.
		 * @param[out] outNodePtr Reference to a pointer that will be set to the address of the found node.
		 * @return true if the node was found, false otherwise.
		 */
		bool tryGetNode( std::string_view code, const GmodNode*& outNodePtr ) const;

		/**
		 * @brief Check if the GMOD node dictionary is empty
		 * @return true if the node map is empty, false otherwise
		 */
		[[nodiscard]] bool isEmpty() const;

		/**
		 * @brief Checks if the given type is a potential parent type.
		 * @param type The type to check.
		 * @return True if the type is a potential parent, false otherwise.
		 */
		static bool isPotentialParent( const std::string& type );

		//----------------------------------------------
		// Path Parsing Methods
		//----------------------------------------------

		/**
		 * @brief Parse a path string into a GmodPath
		 * @param item Path string to parse (e.g. "VE/D101/C201")
		 * @return Parsed GmodPath
		 * @throws std::invalid_argument if parsing fails
		 */
		[[nodiscard]] GmodPath parsePath( std::string_view item ) const;

		/**
		 * @brief Try to parse a path string into a GmodPath
		 * @param item Path string to parse
		 * @param[out] path The parsed path, if successful
		 * @return true if parsing succeeded, false otherwise
		 */
		[[nodiscard]] bool tryParsePath( std::string_view item, GmodPath& path ) const;

		/**
		 * @brief Parse a full path string
		 * @param item Full path string to parse
		 * @return Parsed GmodPath
		 * @throws std::invalid_argument if parsing fails
		 */
		[[nodiscard]] GmodPath parseFromFullPath( std::string_view item ) const;

		/**
		 * @brief Try to parse a full path string
		 * @param item Full path string to parse
		 * @param[out] path The parsed path, if successful
		 * @return true if parsing succeeded, false otherwise
		 */
		[[nodiscard]] bool tryParseFromFullPath( std::string_view item, std::optional<GmodPath>& path ) const;

		//----------------------------------------------
		// Traversal Methods
		//----------------------------------------------

		/**
		 * @brief Traverse the GMOD hierarchy from root node
		 * @param handler Function called for each node visited. Receives parent pointers and current node.
		 * @param options Traversal configuration options
		 * @return true if traversal completed without being stopped by the handler
		 */
		bool traverse( const TraverseHandler& handler, const TraversalOptions& options = {} ) const;

		/**
		 * @brief Traverse the GMOD hierarchy from a specific starting node
		 * @param rootNodeValue Node to start traversal from
		 * @param handler Function called for each node visited. Receives parent pointers and current node.
		 * @param options Traversal configuration options
		 * @return true if traversal completed without being stopped by the handler
		 */
		bool traverse( const GmodNode& rootNodeValue, const TraverseHandler& handler, const TraversalOptions& options = {} ) const;

		//----------------------------------------------
		// Template Traversal Methods (Stateful)
		//----------------------------------------------

		/**
		 * @brief Stateful traversal of GMOD from root node
		 * @tparam TState Type of state object to maintain during traversal
		 * @param state State object passed to handler for context preservation
		 * @param handler Function called for each node visited. Receives state, parent pointers, and current node.
		 * @param options Traversal configuration options
		 * @return true if traversal completed (handler did not return Stop for the overall traversal), false otherwise.
		 */
		template <typename TState>
		bool traverse(
			TState& state,
			const std::function<TraversalHandlerResult( TState& state, const std::vector<const GmodNode*>& parents, const GmodNode& node )>& handler,
			const TraversalOptions& options = {} ) const;

		/**
		 * @brief Stateful traversal of GMOD from specific starting node
		 * @tparam TState Type of state object to maintain during traversal
		 * @param state State object passed to handler for context preservation
		 * @param rootNodeValue Node to start traversal from
		 * @param handler Function called for each node visited. Receives state, parent pointers, and current node.
		 * @param options Traversal configuration options
		 * @return true if traversal completed (handler did not return Stop for the overall traversal), false otherwise.
		 */
		template <typename TState>
		bool traverse(
			TState& state,
			const GmodNode& rootNodeValue,
			const std::function<TraversalHandlerResult( TState& state, const std::vector<const GmodNode*>& parents, const GmodNode& node )>& handler,
			const TraversalOptions& options = {} ) const;

		/**
		 * @brief Check if a path exists between a sequence of parent nodes and a target node.
		 *
		 * This method determines if there's a valid path from the root node ('VE')
		 * to the target `to` node, such that the beginning of this path matches the
		 * provided `fromPath`. The specific path from the `to` node back to the root is
		 * constructed by iteratively ascending via the first parent listed for each node
		 * (effectively using `node.parents().front()` at each step).
		 * If this constructed path starts with the `fromPath` prefix, then `remainingParents`
		 * is populated with the nodes in the path *after* the `fromPath` prefix,
		 * up to and including the `to` node itself.
		 *
		 * @param fromPath A vector of constant GmodNode pointers representing the required prefix path
		 *                 that must match the beginning of the path to the `to` node.
		 * @param to The target GmodNode to which a path is sought.
		 * @param[out] remainingParents If a path with the matching prefix is found, this vector is
		 *                            populated with the GmodNode pointers that constitute the
		 *                            remainder of the path from the end of `fromPath` to `to` (inclusive).
		 *                            The vector is cleared if no such path is found or if an error occurs.
		 * @return true if a path exists with the matching `fromPath` prefix and `remainingParents` is populated,
		 *         false otherwise (e.g., prefix mismatch, `to` node not found in a path from root,
		 *         cycle detected, or structural issue like a node in the upward path having no parents).
		 */
		bool pathExistsBetween(
			const std::vector<const GmodNode*>& fromPath, const GmodNode& to,
			std::vector<const GmodNode*>& remainingParents ) const;

		//----------------------------------------------
		// Iterator Methods
		//----------------------------------------------

		/** @brief Get an iterator to the beginning of the GMOD nodes.
		 *  Allows range-based for loops over the GmodNode values.
		 */
		[[nodiscard]] Iterator begin() const;

		/** @brief Get an iterator to the end of the GMOD nodes.
		 *  Used in conjunction with begin() for iteration.
		 */
		[[nodiscard]] Iterator end() const;

		[[nodiscard]] Iterator begin();
		[[nodiscard]] Iterator end();

		//----------------------------------------------
		// Static Node Classification Methods
		//----------------------------------------------

		/**
		 * @brief Check if a node is a leaf node by type
		 * @param fullType The node type string to check
		 * @return true if it's a leaf node
		 */
		static bool isLeafNode( const std::string& fullType );

		/**
		 * @brief Check if a node is a leaf node by metadata
		 * @param metadata Node metadata to check
		 * @return true if it's a leaf node
		 */
		static bool isLeafNode( const GmodNodeMetadata& metadata );

		/**
		 * @brief Check if a node is a function node by category
		 * @param category Node category to check
		 * @return true if it's a function node
		 */
		static bool isFunctionNode( const std::string& category );

		/**
		 * @brief Check if a node is a function node by metadata
		 * @param metadata Node metadata to check
		 * @return true if it's a function node
		 */
		static bool isFunctionNode( const GmodNodeMetadata& metadata );

		/**
		 * @brief Check if a node is a product selection by metadata
		 * @param metadata Node metadata to check
		 * @return true if it's a product selection
		 */
		static bool isProductSelection( const GmodNodeMetadata& metadata );

		/**
		 * @brief Check if a node is a product type by metadata
		 * @param metadata Node metadata to check
		 * @return true if it's a product type
		 */
		static bool isProductType( const GmodNodeMetadata& metadata );

		/**
		 * @brief Check if a node is an asset by metadata
		 * @param metadata Node metadata to check
		 * @return true if it's an asset
		 */
		static bool isAsset( const GmodNodeMetadata& metadata );

		/**
		 * @brief Check if a node is an asset function node by metadata
		 * @param metadata Node metadata to check
		 * @return true if it's an asset function node
		 */
		static bool isAssetFunctionNode( const GmodNodeMetadata& metadata );

		//----------------------------------------------
		// Static Relationship Classification Methods
		//----------------------------------------------

		/**
		 * @brief Check if there's a product type assignment relationship
		 * @param parent Parent node to check
		 * @param child Child node to check
		 * @return true if there's a product type assignment
		 */
		static bool isProductTypeAssignment( const GmodNode* parent, const GmodNode* child );

		/**
		 * @brief Check if there's a product selection assignment relationship
		 * @param parent Parent node to check
		 * @param child Child node to check
		 * @return true if there's a product selection assignment
		 */
		static bool isProductSelectionAssignment( const GmodNode* parent, const GmodNode* child );

	private:
		//----------------------------------------------
		// Private Traversal Context Definitions
		//----------------------------------------------

		/**
		 * @brief Context data for traversal operations
		 */
		struct TraversalContext final
		{
			Parents parents;
			const TraverseHandler& handler;
			int maxTraversalOccurrence;
			size_t nodesVisited = 0;
			size_t maxNodes;

			/**
			 * @brief Construct a traversal context
			 * @param h Handler function
			 * @param maxOcc Maximum occurrences
			 * @param maxN Maximum nodes to visit
			 */
			TraversalContext( const TraverseHandler& h, int maxOcc, size_t maxN );

			/** @brief Assignment operator deleted*/
			TraversalContext& operator=( const TraversalContext& ) = delete;
		};

		/**
		 * @brief Context data for stateful traversal operations
		 */
		template <typename TState>
		struct StatefulTraversalContext
		{
			Parents parents;
			TState& state;
			const std::function<TraversalHandlerResult( TState&, const std::vector<const GmodNode*>&, const GmodNode& )>& handler;
			int maxTraversalOccurrence;
			size_t nodesVisited = 0;
			size_t maxNodes;

			/**
			 * @brief Construct a stateful traversal context
			 * @param s User-defined state object
			 * @param h Handler function
			 * @param maxOcc Maximum occurrences
			 * @param maxN Maximum nodes to visit
			 */
			StatefulTraversalContext(
				TState& s,
				const std::function<TraversalHandlerResult( TState&, const std::vector<const GmodNode*>&, const GmodNode& )>& h,
				int maxOcc,
				size_t maxN )
				: state( s ), handler( h ), maxTraversalOccurrence( maxOcc ), maxNodes( maxN ) {}

			/** @brief Assignment operator deleted */
			StatefulTraversalContext& operator=( const StatefulTraversalContext& ) = delete;
		};

		//----------------------------------------------
		// Private Traversal Methods
		//----------------------------------------------

		/**
		 * @brief Internal recursive traversal method
		 * @param context Traversal context
		 * @param node Current node to process
		 * @return Traversal result
		 */
		TraversalHandlerResult traverseNode( TraversalContext& context, const GmodNode& node ) const;

		/**
		 * @brief Internal recursive stateful traversal method
		 * @param context Stateful traversal context
		 * @param node Current node to process
		 * @return Traversal result
		 */
		template <typename TState>
		TraversalHandlerResult traverseNode( StatefulTraversalContext<TState>& context, const GmodNode& node ) const;

		//----------------------------------------------
		// Private Members
		//----------------------------------------------

		/** @brief VIS version for this GMOD */
		VisVersion m_visVersion;

		/** @brief Root node of the hierarchy */
		GmodNode m_rootNode;

		/** @brief Optimized dictionary for node lookups */
		ChdDictionary<GmodNode> m_nodeMap;
	};

	//=====================================================================
	// Template Implementations
	//=====================================================================

	/**
	 * @brief Implementation of stateful traversal from root node
	 */
	template <typename TState>
	bool Gmod::traverse(
		TState& state,
		const std::function<Gmod::TraversalHandlerResult( TState&, const std::vector<const GmodNode*>&, const GmodNode& )>& handler,
		const Gmod::TraversalOptions& options ) const
	{
		return traverse( state, rootNode(), handler, options );
	}

	/**
	 * @brief Implementation of stateful traversal from specific node
	 */
	template <typename TState>
	bool Gmod::traverse(
		TState& state,
		const GmodNode& rootNodeValue,
		const std::function<Gmod::TraversalHandlerResult( TState&, const std::vector<const GmodNode*>&, const GmodNode& )>& handler,
		const Gmod::TraversalOptions& options ) const
	{
		StatefulTraversalContext<TState> context(
			state, handler, options.maxTraversalOccurrence, options.maxNodes );

		SPDLOG_DEBUG( "Starting stateful traversal from node: {}", rootNodeValue.code() );
		TraversalHandlerResult result = traverseNode( context, rootNodeValue );
		SPDLOG_DEBUG( "Stateful traversal finished with result: {}", static_cast<int>( result ) );

		return result == TraversalHandlerResult::Continue;
	}

	/**
	 * @brief Implementation of stateful recursive traversal
	 */
	template <typename TState>
	Gmod::TraversalHandlerResult Gmod::traverseNode( StatefulTraversalContext<TState>& context, const GmodNode& node ) const
	{
		if ( context.nodesVisited >= context.maxNodes )
		{
			SPDLOG_WARN( "Traversal stopped: Maximum node visit limit ({}) reached.", context.maxNodes );
			return TraversalHandlerResult::Stop;
		}
		context.nodesVisited++;

		if ( node.metadata().installSubstructure().has_value() && !node.metadata().installSubstructure().value() )
		{
			SPDLOG_INFO( "Skipping node '{}' and its subtree due to InstallSubstructure=false", node.code() );
			return TraversalHandlerResult::Continue;
		}

		try
		{
			[[maybe_unused]] const std::string& code = node.code();
			const int occurrences = context.parents.occurrences( node );

			bool skipOccurrenceCheck = isProductSelectionAssignment( context.parents.lastOrDefault(), &node );

			if ( !skipOccurrenceCheck )
			{
				if ( occurrences == context.maxTraversalOccurrence )
				{
					SPDLOG_DEBUG( "Skipping subtree for node '{}': Occurrence limit ({}) met.", code, occurrences );
					return TraversalHandlerResult::SkipSubtree;
				}
				else if ( occurrences > context.maxTraversalOccurrence )
				{
					std::string errMsg = fmt::format( "Traversal stopped: Occurrence limit ({}) exceeded for node '{}' ({} occurrences). Potential cycle or logic error.",
						context.maxTraversalOccurrence, code, occurrences );
					SPDLOG_ERROR( errMsg );
					throw std::runtime_error( errMsg );
				}
			}
			else
			{
				SPDLOG_DEBUG( "Skipping occurrence check for node '{}' due to ProductSelectionAssignment.", code );
			}

			SPDLOG_DEBUG( "Visiting node: '{}' (Occurrence: {})", code, occurrences + 1 );

			context.parents.push( &node );

			TraversalHandlerResult result = context.handler( context.state, context.parents.nodePointers(), node );

			if ( result == TraversalHandlerResult::Continue )
			{
				for ( const GmodNode* childPtr : node.children() )
				{
					if ( childPtr != nullptr )
					{
						result = traverseNode( context, *childPtr );
						if ( result == TraversalHandlerResult::Stop )
						{
							break;
						}
					}
					else
					{
						SPDLOG_WARN( "Null child pointer encountered for parent '{}' during traversal.", node.code() );
					}
				}
			}

			context.parents.pop();

			if ( result == TraversalHandlerResult::SkipSubtree )
			{
				return TraversalHandlerResult::Continue;
			}

			return result;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during stateful node traversal for node '{}': {}", node.code(), ex.what() );
			if ( !context.parents.nodePointers().empty() && context.parents.lastOrDefault() == &node )
			{
				context.parents.pop();
			}
			throw;
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "Unknown exception during stateful node traversal for node '{}'", node.code() );
			if ( !context.parents.nodePointers().empty() && context.parents.lastOrDefault() == &node )
			{
				context.parents.pop();
			}
			throw;
		}
	}
}
