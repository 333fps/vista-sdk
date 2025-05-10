#include "pch.h"
#include "dnv/vista/sdk/GmodTraversal.h"
#include "dnv/vista/sdk/Gmod.h"

namespace dnv::vista::sdk
{
	namespace
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

		struct PathExistsContext
		{
			const GmodNode& to_node_ref;
			std::vector<const GmodNode*> remainingParents_list;
			const std::vector<const GmodNode*>& fromPath_list_ref;

			PathExistsContext( const GmodNode& to, const std::vector<const GmodNode*>& fromPath )
				: to_node_ref( to ), fromPath_list_ref( fromPath ) {}
		};
	}

	namespace GmodTraversal
	{
		bool traverse( const Gmod& gmodInstance, TraverseHandler handler, const TraversalOptions& options )
		{
			bool dummyState = false;
			TraverseHandlerWithState<bool> wrappedHandler =
				[handler]( [[maybe_unused]] bool& s, const std::vector<const GmodNode*>& parents_list, const GmodNode& node_ref ) {
					return handler( parents_list, node_ref );
				};
			return GmodTraversal::traverse<bool>( gmodInstance, dummyState, gmodInstance.rootNode(), wrappedHandler, options );
		}

		bool traverse( const Gmod& gmodInstance, const GmodNode& rootNode, TraverseHandler handler, const TraversalOptions& options )
		{
			bool dummyState = false;
			TraverseHandlerWithState<bool> wrappedHandler =
				[handler]( [[maybe_unused]] bool& s, const std::vector<const GmodNode*>& parents_list, const GmodNode& node_ref ) {
					return handler( parents_list, node_ref );
				};
			return GmodTraversal::traverse<bool>( gmodInstance, dummyState, rootNode, wrappedHandler, options );
		}

		namespace
		{
			struct PathExistsContext
			{
				const GmodNode& to_node_ref;
				std::vector<const GmodNode*> remainingParents_list;
				const std::vector<const GmodNode*>& fromPath_list_ref;

				PathExistsContext( const GmodNode& to, const std::vector<const GmodNode*>& fromPath )
					: to_node_ref( to ), fromPath_list_ref( fromPath ) {}
			};
		}

		bool pathExistsBetween(
			const Gmod& gmodInstance,
			const std::vector<const GmodNode*>& fromPath,
			const GmodNode& to,
			std::vector<const GmodNode*>& remainingParents )
		{
			remainingParents.clear();
			const GmodNode* lastAssetFunction = nullptr;

			for ( auto it = fromPath.rbegin(); it != fromPath.rend(); ++it )
			{
				if ( ( *it ) && Gmod::isAssetFunctionNode( ( *it )->metadata() ) )
				{
					lastAssetFunction = *it;
					break;
				}
			}

			PathExistsContext context( to, fromPath );

			TraverseHandlerWithState<PathExistsContext> handler_func =
				[]( PathExistsContext& ctx, const std::vector<const GmodNode*>& currentTraversalParents, const GmodNode& currentNode ) -> TraversalHandlerResult {
				if ( currentNode.code() != ctx.to_node_ref.code() )
				{
					return TraversalHandlerResult::Continue;
				}

				std::vector<const GmodNode*> absolutePathToCurrentNodeParent = currentTraversalParents;
				if ( !absolutePathToCurrentNodeParent.empty() && !absolutePathToCurrentNodeParent[0]->isRoot() )
				{
					std::vector<const GmodNode*> prefixPath;
					const GmodNode* head = absolutePathToCurrentNodeParent[0];
					while ( head && !head->isRoot() )
					{
						if ( head->parents().empty() )
							break;
						if ( head->parents().size() != 1 )
							throw std::runtime_error( "Invalid state - expected one parent during path reconstruction for PathExistsBetween" );
						head = head->parents()[0];
						if ( head )
							prefixPath.insert( prefixPath.begin(), head );
						else
							break;
					}
					absolutePathToCurrentNodeParent.insert( absolutePathToCurrentNodeParent.begin(), prefixPath.begin(), prefixPath.end() );
				}

				std::vector<const GmodNode*> pathForValidation = absolutePathToCurrentNodeParent;
				pathForValidation.push_back( &currentNode );

				if ( pathForValidation.size() < ctx.fromPath_list_ref.size() )
				{
					return TraversalHandlerResult::Continue;
				}

				bool match = true;
				for ( size_t i = 0; i < ctx.fromPath_list_ref.size(); ++i )
				{
					if ( pathForValidation[i]->code() != ctx.fromPath_list_ref[i]->code() )
					{
						match = false;
						break;
					}
				}

				if ( match )
				{
					for ( const auto* p_eval : pathForValidation )
					{
						bool foundInFromPath = false;
						for ( const auto* p_from : ctx.fromPath_list_ref )
						{
							if ( p_eval->code() == p_from->code() )
							{
								foundInFromPath = true;
								break;
							}
						}
						if ( !foundInFromPath )
						{
							ctx.remainingParents_list.push_back( p_eval );
						}
					}
					return TraversalHandlerResult::Stop;
				}
				return TraversalHandlerResult::Continue;
			};

			bool traversalCompletedNaturally = GmodTraversal::traverse<PathExistsContext>(
				gmodInstance,
				context,
				( lastAssetFunction ? *lastAssetFunction : gmodInstance.rootNode() ),
				handler_func );

			remainingParents = context.remainingParents_list;
			return !traversalCompletedNaturally;
		}
	}
}
