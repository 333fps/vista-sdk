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
				Parents()
				{
					m_parents.reserve( 64 );
				}

				void push( const GmodNode* parent )
				{
					m_parents.push_back( parent );
					m_occurrences[parent->code()]++;
				}

				void pop()
				{
					if ( m_parents.empty() )
						return;

					const GmodNode* parent = m_parents.back();
					auto it = m_occurrences.find( parent->code() );
					if ( it != m_occurrences.end() )
					{
						it->second--;
						if ( it->second == 0 )
						{
							m_occurrences.erase( it );
						}
					}
					m_parents.pop_back();
				}

				int getOccurrences( const GmodNode& node ) const
				{
					auto it = m_occurrences.find( node.code() );
					return ( it != m_occurrences.end() ) ? it->second : 0;
				}

				const GmodNode* lastOrDefault() const
				{
					return m_parents.empty() ? nullptr : m_parents.back();
				}

				const std::vector<const GmodNode*>& asList() const
				{
					return m_parents;
				}
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
					int occ = context.parents_ref.getOccurrences( node );
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
		bool traverse( const Gmod& gmodInstance, TState& state, const GmodNode& rootNode, TraverseHandlerWithState<TState> handler, const TraversalOptions& options = {} )
		{
			detail::Parents parentsStack;
			detail::TraversalContext<TState> context( parentsStack, handler, state, options.maxTraversalOccurrence );
			return detail::TraverseNodeRecursive<TState>( context, rootNode ) == TraversalHandlerResult::Continue;
		}

		template <typename TState>
		bool traverse( const Gmod& gmodInstance, TState& state, TraverseHandlerWithState<TState> handler, const TraversalOptions& options = {} )
		{
			return GmodTraversal::traverse<TState>( gmodInstance, state, gmodInstance.rootNode(), handler, options );
		}

		bool traverse( const Gmod& gmodInstance, TraverseHandler handler, const TraversalOptions& options = {} );
		bool traverse( const Gmod& gmodInstance, const GmodNode& rootNode, TraverseHandler handler, const TraversalOptions& options = {} );

		bool pathExistsBetween(
			const Gmod& gmodInstance,
			const std::vector<const GmodNode*>& fromPath,
			const GmodNode& to,
			std::vector<const GmodNode*>& remainingParents );
	}
}
