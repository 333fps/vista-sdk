#pragma once

#include "Gmod.h"
#include "GmodNode.h"
#include "VisVersion.h"

namespace dnv::vista::sdk
{
	class Gmod;

	enum class TraversalHandlerResult
	{
		Stop,
		SkipSubtree,
		Continue,
	};

	struct TraversalOptions
	{
		static constexpr int DEFAULT_MAX_TRAVERSAL_OCCURRENCE = 1;
		int maxTraversalOccurrence = DEFAULT_MAX_TRAVERSAL_OCCURRENCE;
	};

	using TraverseHandler = std::function<TraversalHandlerResult( const std::vector<const GmodNode*>& parents, const GmodNode& node )>;

	template <typename TState>
	using TraverseHandlerWithState = std::function<TraversalHandlerResult( TState& state, const std::vector<const GmodNode*>& parents, const GmodNode& node )>;

	namespace GmodTraversal
	{
		namespace detail
		{
			class Parents
			{
			public:
				Parents();

				void push( const GmodNode* parent );
				void pop();
				int occurrences( const GmodNode& node ) const;
				const GmodNode* lastOrDefault() const;
				const std::vector<const GmodNode*>& asList() const;

			private:
				std::vector<const GmodNode*> m_parents;
				std::unordered_map<std::string, int> m_occurrences;
			};

			template <typename TState>
			struct TraversalContext
			{
				Parents& parents;
				TraverseHandlerWithState<TState> handler;
				TState& state;
				int maxTraversalOccurrence;

				TraversalContext( Parents& parents, TraverseHandlerWithState<TState> handler, TState& s, int maxOcc )
					: parents( parents ), handler( handler ), state( s ), maxTraversalOccurrence( maxOcc ) {}

				TraversalContext( const TraversalContext& ) = delete;
				TraversalContext( TraversalContext&& ) = delete;
				TraversalContext& operator=( const TraversalContext& ) = delete;
				TraversalContext& operator=( TraversalContext&& ) = delete;
			};

			template <typename TState>
			TraversalHandlerResult traverseNodeRecursive( TraversalContext<TState>& context, const GmodNode& node )
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
					int occ = context.parents.occurrences( node );

					if ( occ >= context.maxTraversalOccurrence )
					{
						SPDLOG_TRACE( "Node {} occurrence {} reached/exceeded max {}, skipping subtree", node.code(), occ, context.maxTraversalOccurrence );
						return TraversalHandlerResult::SkipSubtree;
					}
				}

				context.parents.push( &node );

				for ( const auto* child : node.children() )
				{
					if ( !child )
					{
						continue;
					}

					SPDLOG_TRACE( "Traversing child: {} of parent: {}", child->code(), node.code() );
					result = traverseNodeRecursive<TState>( context, *child );
					if ( result == TraversalHandlerResult::Stop )
					{
						context.parents.pop();
						return TraversalHandlerResult::Stop;
					}
				}

				context.parents.pop();
				return TraversalHandlerResult::Continue;
			}

		}

		template <typename TState>
		bool traverse( TState& state, const GmodNode& rootNode, TraverseHandlerWithState<TState> handler, const TraversalOptions& options = {} )
		{
			detail::Parents parentsStack;
			detail::TraversalContext<TState> context( parentsStack, handler, state, options.maxTraversalOccurrence );

			return detail::traverseNodeRecursive<TState>( context, rootNode ) == TraversalHandlerResult::Continue;
		}

		template <typename TState>
		bool traverse( const Gmod& gmodInstance, TState& state, TraverseHandlerWithState<TState> handler, const TraversalOptions& options = {} )
		{
			return GmodTraversal::traverse<TState>( state, gmodInstance.rootNode(), handler, options );
		}

		bool traverse( const Gmod& gmodInstance, TraverseHandler handler, const TraversalOptions& options = {} );
		bool traverse( const GmodNode& rootNode, TraverseHandler handler, const TraversalOptions& options = {} );

		bool pathExistsBetween(
			const Gmod& gmodInstance,
			const std::vector<const GmodNode*>& fromPath,
			const GmodNode& to,
			std::vector<const GmodNode*>& remainingParents );
	}
}
