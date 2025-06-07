#include "pch.h"

#include "dnv/vista/sdk/GmodTraversal.h"

#include "dnv/vista/sdk/Gmod.h"

namespace dnv::vista::sdk
{
	namespace GmodTraversal
	{
		namespace detail
		{
			Parents::Parents()
			{
				m_parents.reserve( 64 );
			}

			void Parents::push( const GmodNode* parent )
			{
				m_parents.push_back( parent );
				m_occurrences[parent->code()]++;
			}

			void Parents::pop()
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

			int Parents::occurrences( const GmodNode& node ) const
			{
				auto it = m_occurrences.find( node.code() );
				return ( it != m_occurrences.end() ) ? it->second : 0;
			}

			const GmodNode* Parents::lastOrDefault() const
			{
				return m_parents.empty() ? nullptr : m_parents.back();
			}

			const std::vector<const GmodNode*>& Parents::asList() const
			{
				return m_parents;
			}
		}

		bool traverse( const Gmod& gmodInstance, TraverseHandler handler, const TraversalOptions& options )
		{
			bool dummyState = false;
			TraverseHandlerWithState<bool> wrappedHandler =
				[handler]( [[maybe_unused]] bool& s, const std::vector<const GmodNode*>& parents_list, const GmodNode& nodeRef ) {
					return handler( parents_list, nodeRef );
				};
			return GmodTraversal::traverse<bool>( dummyState, gmodInstance.rootNode(), wrappedHandler, options );
		}

		bool traverse( const GmodNode& rootNode, TraverseHandler handler, const TraversalOptions& options )
		{
			bool dummyState = false;
			TraverseHandlerWithState<bool> wrappedHandler =
				[handler]( [[maybe_unused]] bool& s, const std::vector<const GmodNode*>& parents_list, const GmodNode& nodeRef ) {
					return handler( parents_list, nodeRef );
				};
			return GmodTraversal::traverse<bool>( dummyState, rootNode, wrappedHandler, options );
		}

		namespace
		{
			struct PathExistsContext
			{
				const GmodNode& toNode;
				std::vector<const GmodNode*> remainingParents_list;
				const std::vector<const GmodNode*>& fromPathList;

				PathExistsContext( const GmodNode& to, const std::vector<const GmodNode*>& fromPath )
					: toNode{ to },
					  fromPathList{ fromPath }
				{
				}

				PathExistsContext( const PathExistsContext& ) = delete;
				PathExistsContext( PathExistsContext&& ) = delete;
				PathExistsContext& operator=( const PathExistsContext& ) = delete;
				PathExistsContext& operator=( PathExistsContext&& ) = delete;
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

			TraverseHandlerWithState<PathExistsContext> handler =
				[]( PathExistsContext& ctx, const std::vector<const GmodNode*>& currentTraversalParents, const GmodNode& currentNode ) -> TraversalHandlerResult {
				if ( currentNode.code() != ctx.toNode.code() )
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

				if ( absolutePathToCurrentNodeParent.size() < ctx.fromPathList.size() )
				{
					return TraversalHandlerResult::Continue;
				}

				bool match = true;
				for ( size_t i = 0; i < ctx.fromPathList.size(); ++i )
				{
					if ( absolutePathToCurrentNodeParent[i]->code() != ctx.fromPathList[i]->code() )
					{
						match = false;
						break;
					}
				}

				if ( match )
				{
					ctx.remainingParents_list.clear();
					for ( size_t i = ctx.fromPathList.size(); i < absolutePathToCurrentNodeParent.size(); ++i )
					{
						ctx.remainingParents_list.push_back( absolutePathToCurrentNodeParent[i] );
					}

					return TraversalHandlerResult::Stop;
				}
				return TraversalHandlerResult::Continue;
			};

			bool traversalCompletedNaturally = GmodTraversal::traverse<PathExistsContext>(
				context,
				( lastAssetFunction ? *lastAssetFunction : gmodInstance.rootNode() ),
				handler );

			remainingParents = context.remainingParents_list;
			return !traversalCompletedNaturally;
		}
	}
}
