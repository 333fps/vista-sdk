#include "pch.h"

#include "dnv/vista/sdk/GmodTraversal.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	namespace
	{
		static constexpr std::string_view NODE_CATEGORY_ASSET_FUNCTION = "ASSET FUNCTION";
	}

	//=====================================================================
	// Traversal algorithms
	//=====================================================================

	namespace GmodTraversal
	{
		namespace detail
		{
			//----------------------------------------------
			// GmodTraversal::Parents class
			//----------------------------------------------

			Parents::Parents()
			{
				m_parents.reserve( 64 );
				m_occurrences.reserve( 4 );
			}

			//----------------------------
			// Stack operations
			//----------------------------

			void Parents::push( const GmodNode* parent )
			{
				m_parents.push_back( parent );

				std::string_view key = parent->code();
				if ( auto it = m_occurrences.find( key ); it != m_occurrences.end() )
				{
					++it->second;
				}
				else
				{
					m_occurrences[key] = 1;
				}
			}

			void Parents::pop()
			{
				if ( m_parents.empty() )
				{
					return;
				}

				const GmodNode* parent = m_parents.back();
				m_parents.pop_back();

				std::string_view key = parent->code();
				if ( auto it = m_occurrences.find( key ); it != m_occurrences.end() )
				{
					if ( it->second == 1 )
					{
						m_occurrences.erase( it );
					}
					else
					{
						--it->second;
					}
				}
			}

			size_t Parents::occurrences( const GmodNode& node ) const noexcept
			{
				std::string_view key = node.code();
				if ( auto it = m_occurrences.find( key ); it != m_occurrences.end() )
				{
					return it->second;
				}

				return 0;
			}

			const GmodNode* Parents::lastOrDefault() const noexcept
			{
				return m_parents.empty() ? nullptr : m_parents.back();
			}

			const std::vector<const GmodNode*>& Parents::asList() const noexcept
			{
				return m_parents;
			}

			//----------------------------------------------
			// Path analysis context
			//----------------------------------------------

			struct PathExistsContext
			{
				const GmodNode& to;
				std::vector<const GmodNode*> remainingParents;
				std::vector<const GmodNode*> fromPath;

				PathExistsContext( const GmodNode& toNode, const std::vector<const GmodNode*>& fromPathList )
					: to( toNode ), fromPath( fromPathList )
				{
					remainingParents.reserve( 32 );
				}

				PathExistsContext() = delete;
				PathExistsContext( const PathExistsContext& ) = delete;
				PathExistsContext( PathExistsContext&& ) noexcept = delete;
				~PathExistsContext() = default;
				PathExistsContext& operator=( const PathExistsContext& ) = delete;
				PathExistsContext& operator=( PathExistsContext&& ) noexcept = delete;
			};
		}

		//=====================================================================
		// Public API
		//=====================================================================

		bool traverse( const Gmod& gmodInstance, TraverseHandler handler, const TraversalOptions& options )
		{
			TraverseHandler capturedHandler = handler;
			TraverseHandlerWithState<TraverseHandler> wrapperHandler =
				[]( TraverseHandler& h, const std::vector<const GmodNode*>& parents, const GmodNode& node )
				-> TraversalHandlerResult { return h( parents, node ); };

			return traverse( gmodInstance, capturedHandler, wrapperHandler, options );
		}

		bool traverse( const GmodNode& rootNode, TraverseHandler handler, const TraversalOptions& options )
		{
			TraverseHandler capturedHandler = handler;
			TraverseHandlerWithState<TraverseHandler> wrapperHandler =
				[]( TraverseHandler& h, const std::vector<const GmodNode*>& parents, const GmodNode& node )
				-> TraversalHandlerResult { return h( parents, node ); };

			return traverse( capturedHandler, rootNode, wrapperHandler, options );
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

			detail::PathExistsContext context( to, fromPath );

			TraverseHandlerWithState<detail::PathExistsContext> handler =
				[]( detail::PathExistsContext& state, const std::vector<const GmodNode*>& parents, const GmodNode& node ) -> TraversalHandlerResult {
				if ( node.code() != state.to.code() )
					return TraversalHandlerResult::Continue;

				std::vector<const GmodNode*> actualParents;
				actualParents.reserve( parents.size() + 10 );

				if ( !parents.empty() && !parents[0]->isRoot() )
				{
					std::vector<const GmodNode*> pathToRoot;
					const GmodNode* current = parents[0];

					while ( current && !current->isRoot() )
					{
						if ( current->parents().size() != 1 )
						{
							throw std::runtime_error( "Invalid state - expected one parent" );
						}

						current = current->parents()[0];
						if ( current )
						{
							pathToRoot.push_back( current );
						}
					}

					std::reverse( pathToRoot.begin(), pathToRoot.end() );
					actualParents.insert( actualParents.end(), pathToRoot.begin(), pathToRoot.end() );
					actualParents.insert( actualParents.end(), parents.begin(), parents.end() );
				}
				else
				{
					actualParents = parents;
				}

				if ( actualParents.size() < state.fromPath.size() )
				{
					return TraversalHandlerResult::Continue;
				}

				bool match = true;
				for ( size_t i = 0; i < state.fromPath.size(); ++i )
				{
					if ( actualParents[i]->code() != state.fromPath[i]->code() )
					{
						match = false;
						break;
					}
				}

				if ( match )
				{
					state.remainingParents.clear();
					for ( const auto* p : actualParents )
					{
						bool found = false;
						for ( const auto* fp : state.fromPath )
						{
							if ( fp->code() == p->code() )
							{
								found = true;
								break;
							}
						}
						if ( !found )
						{
							state.remainingParents.push_back( p );
						}
					}
					return TraversalHandlerResult::Stop;
				}

				return TraversalHandlerResult::Continue;
			};

			bool reachedEnd = traverse(
				context,
				lastAssetFunction ? *lastAssetFunction : gmodInstance.rootNode(),
				handler );

			remainingParents = std::move( context.remainingParents );

			return !reachedEnd;
		}
	}
}
