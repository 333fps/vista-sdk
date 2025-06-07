#include "pch.h"

#include "dnv/vista/sdk/GmodTraversal.h"

#include "dnv/vista/sdk/Gmod.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr std::string_view NODE_CATEGORY_ASSET_FUNCTION = "ASSET FUNCTION";
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

	namespace GmodTraversal
	{
		namespace detail
		{
			Parents::Parents()
			{
				m_parents.reserve( 64 );
				m_occurrences.reserve( 32 );
			}

			void Parents::push( const GmodNode* parent )
			{
				m_parents.push_back( parent );
				std::string key = parent->code();
				auto [it, inserted] = m_occurrences.try_emplace( key, 1 );

				if ( !inserted )
				{
					it->second++;
				}
			}

			void Parents::pop()
			{
				if ( m_parents.empty() ) [[unlikely]]
				{
					return;
				}

				const GmodNode* parent = m_parents.back();
				m_parents.pop_back();

				std::string key = parent->code();
				if ( auto it = m_occurrences.find( key ); it != m_occurrences.end() )
				{
					if ( --it->second == 0 )
					{
						m_occurrences.erase( it );
					}
				}
			}

			int Parents::occurrences( const GmodNode& node ) const
			{
				std::string key = node.code();
				if ( auto it = m_occurrences.find( key ); it != m_occurrences.end() )
				{
					return it->second;
				}

				return 0;
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
				if ( *it && ( *it )->metadata().category() == NODE_CATEGORY_ASSET_FUNCTION )
				{
					lastAssetFunction = *it;
					break;
				}
			}

			PathExistsContext context( to, fromPath );

			TraverseHandlerWithState<PathExistsContext> handler =
				[]( PathExistsContext& ctx, const std::vector<const GmodNode*>& currentTraversalParents, const GmodNode& currentNode ) -> TraversalHandlerResult {
				if ( currentNode.code() != ctx.toNode.code() ) [[likely]]
					return TraversalHandlerResult::Continue;

				const std::vector<const GmodNode*>* parents = &currentTraversalParents;
				std::vector<const GmodNode*> actualParents;

				if ( !parents->empty() && !( *parents )[0]->isRoot() )
				{
					actualParents = *parents;
					parents = &actualParents;

					const GmodNode* head = ( *parents )[0];
					while ( head && !head->isRoot() )
					{
						if ( head->parents().empty() )
						{
							break;
						}
						if ( head->parents().size() != 1 ) [[unlikely]]
						{
							throw std::runtime_error( "Invalid state - expected one parent during path reconstruction for PathExistsBetween" );
						}
						head = head->parents()[0];
						if ( head )
						{
							actualParents.insert( actualParents.begin(), head );
						}
					}
				}

				if ( parents->size() < ctx.fromPathList.size() ) [[likely]]
					return TraversalHandlerResult::Continue;

				for ( size_t i = 0; i < ctx.fromPathList.size(); ++i )
				{
					if ( ( *parents )[i]->code() != ctx.fromPathList[i]->code() ) [[likely]]
					{
						return TraversalHandlerResult::Continue;
					}
				}

				ctx.remainingParents_list.clear();
				const size_t remainingSize = parents->size() - ctx.fromPathList.size();
				ctx.remainingParents_list.reserve( remainingSize );

				for ( size_t i = ctx.fromPathList.size(); i < parents->size(); ++i )
				{
					ctx.remainingParents_list.push_back( ( *parents )[i] );
				}

				return TraversalHandlerResult::Stop;
			};

			bool traversalCompletedNaturally = GmodTraversal::traverse<PathExistsContext>(
				context,
				( lastAssetFunction ? *lastAssetFunction : gmodInstance.rootNode() ),
				handler );

			remainingParents = std::move( context.remainingParents_list );

			return !traversalCompletedNaturally;
		}
	}
}
