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
				std::vector<const GmodNode*> m_parents;
				std::unordered_map<std::string, int> m_occurrences;

			public:
				Parents();

				void push( const GmodNode* parent );
				void pop();
				int occurrences( const GmodNode& node ) const;
				const GmodNode* lastOrDefault() const;
				const std::vector<const GmodNode*>& asList() const;
			};

			template <typename TState>
			struct TraversalContext
			{
				Parents& parents_ref;
				TraverseHandlerWithState<TState> handler_func;
				TState& state_ref;
				int maxTraversalOccurrence_val;

				TraversalContext( Parents& parents, TraverseHandlerWithState<TState> handler, TState& s, int maxOcc )
					: parents_ref( parents ), handler_func( handler ), state_ref( s ), maxTraversalOccurrence_val( maxOcc ) {}

				TraversalContext( const TraversalContext& ) = delete;
				TraversalContext( TraversalContext&& ) = delete;
				TraversalContext& operator=( const TraversalContext& ) = delete;
				TraversalContext& operator=( TraversalContext&& ) = delete;
			};

			template <typename TState>
			TraversalHandlerResult TraverseNodeRecursive( TraversalContext<TState>& context, const GmodNode& node )
			{
				TraversalHandlerResult result = context.handler_func( context.state_ref, context.parents_ref.asList(), node );

				if ( result == TraversalHandlerResult::Stop || result == TraversalHandlerResult::SkipSubtree )
				{
					return result;
				}
				if ( node.metadata().installSubstructure().has_value() && !node.metadata().installSubstructure().value() )
				{
					return TraversalHandlerResult::Continue;
				}

				bool skipOccurrenceCheck = Gmod::isProductSelectionAssignment( context.parents_ref.lastOrDefault(), &node );
				if ( !skipOccurrenceCheck )
				{
					int occ = context.parents_ref.occurrences( node );
					if ( occ >= context.maxTraversalOccurrence_val )
					{
						return TraversalHandlerResult::SkipSubtree;
					}
				}

				context.parents_ref.push( &node );

				for ( const auto* child : node.children() )
				{
					if ( !child )
						continue;

					TraversalHandlerResult child_result = TraverseNodeRecursive<TState>( context, *child );

					if ( child_result == TraversalHandlerResult::Stop )
					{
						context.parents_ref.pop();
						return TraversalHandlerResult::Stop;
					}
				}

				context.parents_ref.pop();
				return TraversalHandlerResult::Continue;
			}
		}

		template <typename TState>
		bool traverse( TState& state, const GmodNode& rootNode, TraverseHandlerWithState<TState> handler, const TraversalOptions& options = {} )
		{
			detail::Parents parentsStack;
			detail::TraversalContext<TState> context( parentsStack, handler, state, options.maxTraversalOccurrence );
			return detail::TraverseNodeRecursive<TState>( context, rootNode ) == TraversalHandlerResult::Continue;
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
