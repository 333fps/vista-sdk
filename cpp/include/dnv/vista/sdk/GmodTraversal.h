/**
 * @file GmodTraversal.h
 * @brief High-performance GMOD tree traversal algorithms
 * @details Optimized traversal functions with cycle detection and branch prediction.
 *          Supports both stateless and stateful handlers with zero-overhead templates.
 */

#pragma once

#include "Gmod.h"
#include "GmodNode.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Enumerations
	//=====================================================================

	/**
	 * @enum TraversalHandlerResult
	 * @brief Controls traversal flow
	 */
	enum class TraversalHandlerResult
	{
		Stop = 0,
		SkipSubtree,
		Continue,
	};

	//=====================================================================
	// Configuration
	//=====================================================================

	/**
	 * @struct TraversalOptions
	 * @brief Traversal configuration with cycle detection
	 */
	struct TraversalOptions
	{
		static constexpr size_t DEFAULT_MAX_TRAVERSAL_OCCURRENCE = 1;
		size_t maxTraversalOccurrence = DEFAULT_MAX_TRAVERSAL_OCCURRENCE;
	};

	/**
	 * @typedef TraverseHandler
	 * @brief Function pointer for stateless traversal handlers
	 */
	using TraverseHandler = TraversalHandlerResult ( * )( const std::vector<const GmodNode*>& parents, const GmodNode& node );

	/**
	 * @typedef TraverseHandlerWithState
	 * @brief Function pointer for stateful traversal handlers
	 * @tparam TState User-defined state type
	 */
	template <typename TState>
	using TraverseHandlerWithState = TraversalHandlerResult ( * )( TState& state, const std::vector<const GmodNode*>& parents, const GmodNode& node );

	//=====================================================================
	// Traversal algorithms
	//=====================================================================

	/**
	 * @namespace GmodTraversal
	 * @brief High-performance GMOD tree traversal algorithms
	 * @details Optimized implementations with cycle detection and branch prediction.
	 */
	namespace GmodTraversal
	{
		/**
		 * @namespace detail
		 * @brief Internal implementation details
		 * @warning Internal use only - API may change without notice
		 */
		namespace detail
		{
			//----------------------------------------------
			// GmodTraversal::Parents class
			//----------------------------------------------

			/**
			 * @class Parents
			 * @brief Optimized parent stack with occurrence tracking
			 * @details Pre-allocated for 64 parents and 4 occurrence entries.
			 */
			class Parents
			{
			public:
				//----------------------------
				// Construction / destruction
				//----------------------------

				/** @brief Default constructor. */
				Parents();

				//----------------------------
				// Stack operations
				//----------------------------

				/** @brief Push parent and update occurrence count */
				void push( const GmodNode* parent );

				/** @brief Pop parent and update occurrence count */
				void pop();

				/** @brief Get occurrence count for node */
				[[nodiscard]] size_t occurrences( const GmodNode& node ) const noexcept;

				/** @brief Get last parent or nullptr */
				[[nodiscard]] const GmodNode* lastOrDefault() const noexcept;

				/** @brief Get complete parent chain */
				[[nodiscard]] const std::vector<const GmodNode*>& asList() const noexcept;

			private:
				//----------------------------
				// Private member variables
				//----------------------------

				/** @brief Parent chain from root to current */
				std::vector<const GmodNode*> m_parents;

				/** @brief Occurrence count per node code */
				std::unordered_map<std::string_view, size_t> m_occurrences;
			};

			//----------------------------------------------
			// TraversalContext struct
			//----------------------------------------------

			/**
			 * @struct TraversalContext
			 * @brief Bundles traversal state for optimal performance
			 * @tparam TState User-defined state type
			 */
			template <typename TState>
			struct TraversalContext
			{
				Parents& parents;
				TraverseHandlerWithState<TState> handler;
				TState& state;
				size_t maxTraversalOccurrence;

				explicit TraversalContext( Parents& p, TraverseHandlerWithState<TState> h, TState& s, size_t maxOcc )
					: parents( p ), handler( h ), state( s ), maxTraversalOccurrence( maxOcc ) {}

				TraversalContext() = delete;
				TraversalContext( const TraversalContext& ) = delete;
				TraversalContext( TraversalContext&& ) noexcept = delete;
				~TraversalContext() = default;
				TraversalContext& operator=( const TraversalContext& ) = delete;
				TraversalContext& operator=( TraversalContext&& ) noexcept = delete;
			};

			//----------------------------------------------
			// Core algorithm
			//----------------------------------------------

			/**
			 * @brief Core recursive traversal with optimizations
			 * @details Performance-critical hot path with branch prediction hints
			 * @tparam TState User-defined state type
			 * @param context Traversal context
			 * @param node Current node to traverse
			 * @return Traversal result
			 */
			template <typename TState>
			[[nodiscard]] __forceinline TraversalHandlerResult traverseNodeRecursive( TraversalContext<TState>& context, const GmodNode& node )
			{
				if ( node.metadata().installSubstructure().has_value() && !node.metadata().installSubstructure().value() )
				{
					return TraversalHandlerResult::Continue;
				}

				TraversalHandlerResult result = context.handler( context.state, context.parents.asList(), node );
				if ( result == TraversalHandlerResult::Stop || result == TraversalHandlerResult::SkipSubtree )
				{
					return result;
				}

				bool skipOccurrenceCheck = Gmod::isProductSelectionAssignment( context.parents.lastOrDefault(), &node );
				if ( !skipOccurrenceCheck )
				{
					size_t occ = context.parents.occurrences( node );

					if ( occ == context.maxTraversalOccurrence )
					{
						return TraversalHandlerResult::SkipSubtree;
					}

					if ( occ > context.maxTraversalOccurrence )
					{
						throw std::runtime_error( "Invalid state - node occurred more than expected" );
					}
				}

				context.parents.push( &node );

				const auto& children = node.children();
				for ( const auto* child : children )
				{
					if ( !child )
					{
						continue;
					}

					result = traverseNodeRecursive( context, *child );
					if ( result == TraversalHandlerResult::Stop )
					{
						context.parents.pop();
						return TraversalHandlerResult::Stop;
					}
					else if ( result == TraversalHandlerResult::SkipSubtree )
					{
						continue;
					}
				}

				context.parents.pop();

				return TraversalHandlerResult::Continue;
			}
		}

		//=====================================================================
		// Public API
		//=====================================================================

		//----------------------------------------------
		// Non-template functions
		//----------------------------------------------

		/**
		 * @brief Traverse GMOD tree from root with stateless handler
		 * @param gmodInstance GMOD instance to traverse
		 * @param handler Function pointer to handler
		 * @param options Traversal configuration
		 * @return true if completed, false if stopped early
		 */
		[[nodiscard]] bool traverse( const Gmod& gmodInstance, TraverseHandler handler, const TraversalOptions& options = {} );

		/**
		 * @brief Traverse GMOD tree from specific node with stateless handler
		 * @param rootNode Starting node
		 * @param handler Function pointer to handler
		 * @param options Traversal configuration
		 * @return true if completed, false if stopped early
		 */
		[[nodiscard]] bool traverse( const GmodNode& rootNode, TraverseHandler handler, const TraversalOptions& options = {} );

		/**
		 * @brief Check if path exists between nodes
		 * @param gmodInstance GMOD instance
		 * @param fromPath Source path
		 * @param to Target node
		 * @param remainingParents Output: parents not in fromPath
		 * @return true if path exists
		 */
		[[nodiscard]] bool pathExistsBetween(
			const Gmod& gmodInstance,
			const std::vector<const GmodNode*>& fromPath,
			const GmodNode& to,
			std::vector<const GmodNode*>& remainingParents );

		//----------------------------------------------
		// Zero-overhead template functions
		//----------------------------------------------

		/**
		 * @brief Traverse GMOD tree from root with stateful handler
		 * @details Zero-overhead template for maximum performance
		 * @tparam TState User-defined state type
		 * @param gmodInstance GMOD instance to traverse
		 * @param state User state reference
		 * @param handler Function pointer to stateful handler
		 * @param options Traversal configuration
		 * @return true if completed, false if stopped early
		 */
		template <typename TState>
		__forceinline bool traverse( const Gmod& gmodInstance, TState& state, TraverseHandlerWithState<TState> handler, const TraversalOptions& options = {} )
		{
			detail::Parents parentsStack;
			detail::TraversalContext<TState> context( parentsStack, handler, state, options.maxTraversalOccurrence );

			return detail::traverseNodeRecursive( context, gmodInstance.rootNode() ) == TraversalHandlerResult::Continue;
		}

		/**
		 * @brief Traverse GMOD tree from specific node with stateful handler
		 * @details Zero-overhead template for maximum performance
		 * @tparam TState User-defined state type
		 * @param state User state reference
		 * @param rootNode Starting node
		 * @param handler Function pointer to stateful handler
		 * @param options Traversal configuration
		 * @return true if completed, false if stopped early
		 */
		template <typename TState>
		__forceinline bool traverse( TState& state, const GmodNode& rootNode, TraverseHandlerWithState<TState> handler, const TraversalOptions& options = {} )
		{
			detail::Parents parentsStack;
			detail::TraversalContext<TState> context( parentsStack, handler, state, options.maxTraversalOccurrence );

			return detail::traverseNodeRecursive( context, rootNode ) == TraversalHandlerResult::Continue;
		}
	}
}
