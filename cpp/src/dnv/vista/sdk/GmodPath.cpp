#include "pch.h"

#include "dnv/vista/sdk/GmodPath.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/Locations.h"

namespace dnv::vista::sdk
{
	namespace internal
	{
		//--------------------------------------------------------------------------
		// LocationSetsVisitor
		//--------------------------------------------------------------------------

		struct LocationSetsVisitor
		{
			int currentParentStart;

			LocationSetsVisitor() : currentParentStart( -1 ) {}

			const GmodNode* getNodeFromPath( int index,
				const std::vector<GmodNode*>& pathParents,
				const GmodNode& pathTargetNode ) const
			{
				if ( index < 0 )
				{
					return nullptr;
				}

				if ( static_cast<size_t>( index ) < pathParents.size() )
				{
					return pathParents[static_cast<size_t>( index )];
				}

				if ( static_cast<size_t>( index ) == pathParents.size() )
				{
					return &pathTargetNode;
				}

				return nullptr;
			}

			std::optional<std::tuple<int, int, std::optional<Location>>> visit(
				const GmodNode& node,
				int i,
				const std::vector<GmodNode*>& pathParents,
				const GmodNode& pathTargetNode )
			{
				bool isGmodPotentialParent = Gmod::isPotentialParent( node.metadata().type() );
				bool isCurrentNodeTheTargetPathNode = ( static_cast<size_t>( i ) == pathParents.size() );

				if ( currentParentStart == -1 )
				{
					if ( isGmodPotentialParent )
					{
						currentParentStart = i;
					}

					if ( node.isIndividualizable( isCurrentNodeTheTargetPathNode ) )
					{
						return std::make_tuple( i, i, node.location() );
					}
				}
				else
				{
					if ( isGmodPotentialParent || isCurrentNodeTheTargetPathNode )
					{
						std::optional<std::tuple<int, int, std::optional<Location>>> nodes;

						if ( currentParentStart + 1 == i )
						{
							if ( node.isIndividualizable( isCurrentNodeTheTargetPathNode ) )
								nodes = std::make_tuple( i, i, node.location() );
						}
						else
						{
							int skippedOne = -1;
							bool hasComposition = false;
							for ( int j = currentParentStart + 1; j <= i; ++j )
							{
								const GmodNode* setnode = getNodeFromPath( j, pathParents, pathTargetNode );
								if ( !setnode )
								{
									SPDLOG_WARN( "LocationSetsVisitor: getNodeFromPath returned null for index {}.", j );

									continue;
								}
								const GmodNode& setNode = *setnode;
								bool isSetNodeTheTargetPathNode = ( static_cast<size_t>( j ) == pathParents.size() );

								if ( !setNode.isIndividualizable( isSetNodeTheTargetPathNode, true ) )
								{
									if ( nodes.has_value() )
									{
										skippedOne = j;
									}

									continue;
								}

								if ( nodes.has_value() &&
									 std::get<2>( nodes.value() ).has_value() &&
									 setNode.location().has_value() &&
									 std::get<2>( nodes.value() ).value() != setNode.location().value() )
								{
									SPDLOG_ERROR( "LocationSetsVisitor: Different locations in the same nodeset for node codes {} and {}.",
										std::get<2>( nodes.value() ).has_value() ? getNodeFromPath( std::get<0>( nodes.value() ), pathParents, pathTargetNode )->code() : "N/A",
										setNode.code() );
									throw std::runtime_error( "Mapping error: different locations in the same nodeset." );
								}

								if ( skippedOne != -1 )
								{
									SPDLOG_ERROR( "LocationSetsVisitor: Can't skip in the middle of individualizable set at index {}.", skippedOne );
									throw std::runtime_error( "Can't skip in the middle of individualizable set." );
								}

								if ( setNode.isFunctionComposition() )
								{
									hasComposition = true;
								}

								std::optional<Location> currentSetLocation;
								if ( nodes.has_value() && std::get<2>( nodes.value() ).has_value() )
								{
									currentSetLocation = std::get<2>( nodes.value() );
								}
								else
								{
									currentSetLocation = setNode.location();
								}

								int startIdx = nodes.has_value() ? std::get<0>( nodes.value() ) : j;
								int endIdx = j;
								nodes = std::make_tuple( startIdx, endIdx, currentSetLocation );
							}

							if ( nodes.has_value() &&
								 std::get<0>( nodes.value() ) == std::get<1>( nodes.value() ) &&
								 hasComposition )
							{
								nodes.reset();
							}
						}

						currentParentStart = i;
						if ( nodes.has_value() )
						{
							bool hasLeafNode = false;
							for ( int j = std::get<0>( nodes.value() ); j <= std::get<1>( nodes.value() ); ++j )
							{
								const GmodNode* setnode = getNodeFromPath( j, pathParents, pathTargetNode );
								if ( !setnode )
								{
									continue;
								}

								const GmodNode& setNode = *setnode;
								bool isSetNodeTheTargetPathNode = ( static_cast<size_t>( j ) == pathParents.size() );

								if ( Gmod::isLeafNode( setNode.metadata() ) || isSetNodeTheTargetPathNode )
								{
									hasLeafNode = true;
									break;
								}
							}
							if ( hasLeafNode )
							{
								return nodes;
							}
						}
					}

					if ( isCurrentNodeTheTargetPathNode && node.isIndividualizable( isCurrentNodeTheTargetPathNode ) )
						return std::make_tuple( i, i, node.location() );
				}
				return std::nullopt;
			}
		};

		//--------------------------------------------------------------------------
		// Parsing
		//--------------------------------------------------------------------------

		struct PathNode
		{
			std::string code;
			std::optional<Location> location;
		};

		struct ParseContext
		{
			std::deque<PathNode> partsQueue;
			PathNode toFind;
			std::optional<std::unordered_map<std::string, Location>> nodeLocations;
			std::optional<GmodPath> resultingPath;
			const Gmod& gmod;
			std::vector<GmodNode*> ownedNodesForCurrentPath;

			ParseContext( std::deque<PathNode> initialParts, const Gmod& g, PathNode firstToFind )
				: partsQueue( std::move( initialParts ) ), toFind( std::move( firstToFind ) ), gmod( g )
			{
			}

			ParseContext( const ParseContext& ) = delete;
			ParseContext( ParseContext&& ) noexcept = delete;
			ParseContext& operator=( const ParseContext& ) = delete;
			ParseContext& operator=( ParseContext&& ) noexcept = delete;
		};

		TraversalHandlerResult parseInternalTraversalHandler(
			ParseContext& context,
			const std::vector<const GmodNode*>& traversedParents,
			const GmodNode& currentNode )
		{
			SPDLOG_TRACE( "parseInternalTraversalHandler: currentNode='{}', toFind='{}'", currentNode.code().data(), context.toFind.code );

			bool foundCurrentToFind = ( currentNode.code() == context.toFind.code );

			if ( !foundCurrentToFind && Gmod::isLeafNode( currentNode.metadata() ) )
			{
				SPDLOG_TRACE( "parseInternalTraversalHandler: Leaf node '{}' does not match toFind '{}'. Skipping subtree.", currentNode.code().data(), context.toFind.code );
				return TraversalHandlerResult::SkipSubtree;
			}

			if ( !foundCurrentToFind )
			{
				SPDLOG_TRACE( "parseInternalTraversalHandler: Node '{}' does not match toFind '{}'. Continuing search.", currentNode.code().data(), context.toFind.code );
				return TraversalHandlerResult::Continue;
			}

			SPDLOG_DEBUG( "parseInternalTraversalHandler: Found node for toFind='{}'", context.toFind.code );

			if ( context.toFind.location.has_value() )
			{
				if ( !context.nodeLocations.has_value() )
				{
					context.nodeLocations.emplace();
				}
				context.nodeLocations->emplace( std::string( context.toFind.code ), context.toFind.location.value() );
				SPDLOG_TRACE( "parseInternalTraversalHandler: Stored location for '{}'", context.toFind.code );
			}

			if ( !context.partsQueue.empty() )
			{
				context.toFind = context.partsQueue.front();
				context.partsQueue.pop_front();
				SPDLOG_TRACE( "parseInternalTraversalHandler: Advanced toFind to '{}'. Parts remaining: {}", context.toFind.code, context.partsQueue.size() );
				return TraversalHandlerResult::Continue;
			}

			SPDLOG_DEBUG( "parseInternalTraversalHandler: All parts found. Constructing path." );

			std::vector<GmodNode*> finalPathParents;
			finalPathParents.reserve( traversedParents.size() + context.gmod.rootNode().parents().size() + 5 );

			for ( const GmodNode* traversedParent : traversedParents )
			{
				if ( !traversedParent )
					continue;
				GmodNode* nodeToAdd = const_cast<GmodNode*>( traversedParent );
				if ( context.nodeLocations.has_value() )
				{
					auto it = context.nodeLocations->find( std::string( traversedParent->code() ) );
					if ( it != context.nodeLocations->end() )
					{
						GmodNode* newOwnedNode = new GmodNode( traversedParent->tryWithLocation( std::make_optional( it->second ) ) );
						context.ownedNodesForCurrentPath.push_back( newOwnedNode );
						nodeToAdd = newOwnedNode;
						SPDLOG_TRACE( "parseInternalTraversalHandler: Applied stored location to parent '{}', new ptr {}", traversedParent->code().data(), fmt::ptr( newOwnedNode ) );
					}
				}
				finalPathParents.push_back( nodeToAdd );
			}

			GmodNode* finalEndNode;
			if ( context.toFind.location.has_value() )
			{
				GmodNode* newOwnedNode = new GmodNode( currentNode.tryWithLocation( context.toFind.location ) );
				context.ownedNodesForCurrentPath.push_back( newOwnedNode );
				finalEndNode = newOwnedNode;
				SPDLOG_TRACE( "parseInternalTraversalHandler: Applied final location to end_node '{}', new ptr {}", currentNode.code().data(), fmt::ptr( newOwnedNode ) );
			}
			else
			{
				finalEndNode = const_cast<GmodNode*>( &currentNode );
			}

			const GmodNode* currentAncestor = ( !finalPathParents.empty() && !finalPathParents[0]->parents().empty() && finalPathParents[0]->parents().size() == 1 )
												  ? finalPathParents[0]->parents()[0]
												  : ( !finalEndNode->parents().empty() && finalEndNode->parents().size() == 1 ? finalEndNode->parents()[0] : nullptr );

			if ( !currentAncestor || currentAncestor->parents().size() > 1 )
			{
				SPDLOG_WARN( "parseInternalTraversalHandler: Path does not have a clear single-parent lineage to root. Stopping." );

				return TraversalHandlerResult::Stop;
			}

			if ( currentAncestor && currentAncestor->parents().size() > 1 && currentAncestor != &context.gmod.rootNode() )
			{
				SPDLOG_WARN( "parseInternalTraversalHandler: Path does not extend cleanly to a single-parent root before GMOD root. Stopping." );

				return TraversalHandlerResult::Stop;
			}

			std::vector<GmodNode*> prependedNodes;
			while ( currentAncestor && currentAncestor->parents().size() == 1 )
			{
				GmodNode* nodeToPrepend = const_cast<GmodNode*>( currentAncestor );
				if ( context.nodeLocations.has_value() )
				{
					auto it = context.nodeLocations->find( std::string( currentAncestor->code() ) );
					if ( it != context.nodeLocations->end() )
					{
						GmodNode* newOwnedNode = new GmodNode( currentAncestor->tryWithLocation( std::make_optional( it->second ) ) );
						context.ownedNodesForCurrentPath.push_back( newOwnedNode );
						nodeToPrepend = newOwnedNode;
						SPDLOG_TRACE( "parseInternalTraversalHandler: Applied stored location to prepended ancestor '{}'", currentAncestor->code().data() );
					}
				}
				prependedNodes.push_back( nodeToPrepend );
				if ( currentAncestor->parents().empty() )
				{
					currentAncestor = nullptr;
				}
				else
				{
					currentAncestor = currentAncestor->parents()[0];
				}

				if ( currentAncestor && currentAncestor->parents().size() > 1 && currentAncestor != &context.gmod.rootNode() )
				{
					SPDLOG_WARN( "parseInternalTraversalHandler: Encountered multi-parent ancestor {} before GMOD root during prepend. Stopping.", currentAncestor->code().data() );

					return TraversalHandlerResult::Stop;
				}
			}

			std::reverse( prependedNodes.begin(), prependedNodes.end() );
			finalPathParents.insert( finalPathParents.begin(), prependedNodes.begin(), prependedNodes.end() );

			if ( finalPathParents.empty() || finalPathParents.front() != &context.gmod.rootNode() )
			{
				finalPathParents.insert( finalPathParents.begin(), const_cast<GmodNode*>( &context.gmod.rootNode() ) );
				SPDLOG_TRACE( "parseInternalTraversalHandler: Prepended GMOD root node." );
			}

			internal::LocationSetsVisitor locationSetsVisitor;
			for ( size_t i = 0; i < finalPathParents.size() + 1; ++i )
			{
				GmodNode* nodeInPath = ( i < finalPathParents.size() ) ? finalPathParents[i] : finalEndNode;
				if ( !nodeInPath )
					continue;

				std::optional<std::tuple<int, int, std::optional<Location>>> setDetails =
					locationSetsVisitor.visit( *nodeInPath, static_cast<int>( i ), finalPathParents, *finalEndNode );

				if ( setDetails.has_value() )
				{
					const auto& setTuple = setDetails.value();
					int setStartIdx = std::get<0>( setTuple );
					int setEndIdx = std::get<1>( setTuple );
					const std::optional<Location>& setCommonLocation = std::get<2>( setTuple );

					if ( setStartIdx == setEndIdx )
						continue;

					if ( setCommonLocation.has_value() )
					{
						SPDLOG_TRACE( "parseInternalTraversalHandler: Applying common location from set ({}-{}) to nodes.", setStartIdx, setEndIdx );
						for ( int k = setStartIdx; k <= setEndIdx; ++k )
						{
							GmodNode** nodesToUpdateInPath;
							if ( static_cast<size_t>( k ) < finalPathParents.size() )
							{
								nodesToUpdateInPath = &finalPathParents[static_cast<size_t>( k )];
							}
							else
							{
								nodesToUpdateInPath = &finalEndNode;
							}
							GmodNode* currentNodeInSet = *nodesToUpdateInPath;

							bool needsNewNode = true;
							if ( currentNodeInSet->location().has_value() && currentNodeInSet->location().value() == setCommonLocation.value() )
							{
								needsNewNode = false;
							}
							else if ( !currentNodeInSet->location().has_value() && !setCommonLocation.has_value() )
							{
								needsNewNode = false;
							}

							if ( needsNewNode )
							{
								GmodNode* newNodeWithSetLocation = new GmodNode( currentNodeInSet->tryWithLocation( setCommonLocation ) );
								context.ownedNodesForCurrentPath.push_back( newNodeWithSetLocation );
								*nodesToUpdateInPath = newNodeWithSetLocation;
								SPDLOG_TRACE( "parseInternalTraversalHandler: Node '{}' in set updated with common location. New ptr {}", currentNodeInSet->code().data(), fmt::ptr( newNodeWithSetLocation ) );
							}
						}
					}
				}
				else
				{
					if ( nodeInPath->location().has_value() )
					{
						SPDLOG_ERROR( "parseInternalTraversalHandler: Node '{}' has a location but was not processed by set logic. Path invalid.", nodeInPath->code().data() );
						return TraversalHandlerResult::Stop;
					}
				}
			}

			GmodPath pathObject;
			pathObject.m_gmod = &context.gmod;
			pathObject.m_parents = finalPathParents;
			pathObject.m_node = finalEndNode;
			pathObject.m_visVersion = finalEndNode->visVersion();
			pathObject.m_ownedNodes = context.ownedNodesForCurrentPath;
			context.ownedNodesForCurrentPath.clear();

			context.resultingPath.emplace( std::move( pathObject ) );
			SPDLOG_INFO( "parseInternalTraversalHandler: Successfully constructed GmodPath." );
			return TraversalHandlerResult::Stop;
		}
	}

	GmodPath::GmodPath( const Gmod& gmod, GmodNode* node, std::vector<GmodNode*> parents )
		: m_gmod( &gmod ),
		  m_node( node ),
		  m_parents( std::move( parents ) )
	{
		SPDLOG_TRACE( "GmodPath constructor called with node: {} and {} parents.",
			( m_node ? std::string( m_node->code() ) : "null" ), m_parents.size() );

		if ( !m_node )
		{
			SPDLOG_ERROR( "GmodPath constructor: node cannot be null." );
			throw std::invalid_argument( "GmodPath constructor: node cannot be null." );
		}

		m_visVersion = m_node->visVersion();
		SPDLOG_DEBUG( "GmodPath constructor: VisVersion set to {}.", static_cast<int>( m_visVersion ) );

		if ( m_parents.empty() )
		{
			if ( m_node != &m_gmod->rootNode() )
			{
				std::string nodeCode = std::string( m_node->code() );
				std::string rootCode = std::string( m_gmod->rootNode().code() );
				SPDLOG_ERROR( "Invalid GMOD path - no parents, and node '{}' is not the GMOD root '{}'.", nodeCode, rootCode );
				throw std::invalid_argument( fmt::format( "Invalid GMOD path - no parents, and node '{}' is not the GMOD root '{}'.", nodeCode, rootCode ) );
			}
		}
		else
		{
			if ( m_parents.front() == nullptr )
			{
				SPDLOG_ERROR( "Invalid GMOD path: first parent is null." );
				throw std::invalid_argument( "Invalid GMOD path: first parent is null." );
			}

			if ( m_parents.front() != &m_gmod->rootNode() )
			{
				std::string firstParentCode = std::string( m_parents.front()->code() );
				std::string rootCode = std::string( m_gmod->rootNode().code() );
				SPDLOG_ERROR( "Invalid GMOD path - first parent '{}' should be GMOD root '{}'.", firstParentCode, rootCode );
				throw std::invalid_argument( fmt::format( "Invalid GMOD path - first parent '{}' should be GMOD root '{}'.", firstParentCode, rootCode ) );
			}

			for ( size_t i = 0; i < m_parents.size(); ++i )
			{
				GmodNode* currentParentNode = m_parents[i];
				GmodNode* childNodeToCheck = ( i + 1 < m_parents.size() ) ? m_parents[i + 1] : m_node;

				if ( !currentParentNode )
				{
					SPDLOG_ERROR( "Invalid GMOD path: null parent encountered in parents list at index {}.", i );
					throw std::invalid_argument( fmt::format( "Invalid GMOD path: null parent encountered in parents list at index {}.", i ) );
				}
				if ( !childNodeToCheck )
				{
					SPDLOG_ERROR( "Invalid GMOD path: child node (or final node) is null for parent at index {}.", i );
					throw std::invalid_argument( fmt::format( "Invalid GMOD path: child node (or final node) is null for parent at index {}.", i ) );
				}

				if ( !currentParentNode->isChild( *childNodeToCheck ) )
				{
					std::string childCode = std::string( childNodeToCheck->code() );
					std::string parentCode = std::string( currentParentNode->code() );
					SPDLOG_ERROR( "Invalid GMOD path - node '{}' not child of '{}'.", childCode, parentCode );
					throw std::invalid_argument( fmt::format( "Invalid GMOD path - node '{}' not child of '{}'.", childCode, parentCode ) );
				}
			}
		}
		SPDLOG_DEBUG( "GmodPath structural validation passed for node: {}.", m_node->code() );

		try
		{
			internal::LocationSetsVisitor visitor;
			for ( size_t i = 0; i < m_parents.size() + 1; ++i )
			{
				GmodNode* nodeToVisit = ( i < m_parents.size() ) ? m_parents[i] : m_node;
				if ( !nodeToVisit )
				{
					SPDLOG_ERROR( "GmodPath constructor: Null node encountered at index {} during LocationSetsVisitor phase.", i );
					throw std::runtime_error( fmt::format( "Null node encountered at index {} during LocationSetsVisitor phase.", i ) );
				}
				visitor.visit( *nodeToVisit, static_cast<int>( i ), m_parents, *m_node );
			}
			SPDLOG_DEBUG( "GmodPath LocationSetsVisitor validation passed for node: {}.", m_node->code() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "GmodPath construction for node '{}' failed during LocationSetsVisitor validation: {}", m_node->code(), ex.what() );
			throw;
		}

		SPDLOG_DEBUG( "GmodPath constructed successfully for node: {}.", m_node->code() );
	}

	GmodPath::GmodPath()
		: m_visVersion( VisVersion::Unknown ),
		  m_gmod( nullptr ),
		  m_node( nullptr )
	{
		SPDLOG_TRACE( "GmodPath default constructor called." );
	}

	GmodPath::GmodPath( const GmodPath& other )
		: m_visVersion( other.m_visVersion ),
		  m_gmod( other.m_gmod ),
		  m_node( other.m_node ),
		  m_parents( other.m_parents )
	{
		SPDLOG_TRACE( "GmodPath copy constructor called for path ending with node: {}.", ( m_node ? std::string( m_node->code() ) : "null" ) );
		m_ownedNodes.reserve( other.m_ownedNodes.size() );
		for ( GmodNode* ownedNodeToCopy : other.m_ownedNodes )
		{
			if ( ownedNodeToCopy )
			{
				m_ownedNodes.push_back( new GmodNode( *ownedNodeToCopy ) );
			}
			else
			{
				m_ownedNodes.push_back( nullptr );
			}
		}
		SPDLOG_DEBUG( "GmodPath copy constructor: {} owned nodes copied.", m_ownedNodes.size() );
	}

	GmodPath::GmodPath( GmodPath&& other ) noexcept
		: m_visVersion( other.m_visVersion ),
		  m_gmod( other.m_gmod ),
		  m_node( other.m_node ),
		  m_parents( std::move( other.m_parents ) ),
		  m_ownedNodes( std::move( other.m_ownedNodes ) )
	{
		SPDLOG_TRACE( "GmodPath move constructor called for path ending with node: {}.", ( m_node ? std::string( m_node->code() ) : "null" ) );
		other.m_node = nullptr;
		other.m_gmod = nullptr;
		other.m_visVersion = VisVersion::Unknown;
		SPDLOG_DEBUG( "GmodPath move constructor: Source path members reset." );
	}

	GmodPath& GmodPath::operator=( const GmodPath& other )
	{
		SPDLOG_TRACE( "GmodPath copy assignment operator called." );
		if ( this == &other )
		{
			SPDLOG_DEBUG( "GmodPath copy assignment: Self-assignment detected, no action taken." );
			return *this;
		}

		for ( GmodNode* ownedNode : m_ownedNodes )
		{
			delete ownedNode;
		}
		m_ownedNodes.clear();

		m_visVersion = other.m_visVersion;
		m_gmod = other.m_gmod;
		m_parents = other.m_parents;
		m_node = other.m_node;

		m_ownedNodes.reserve( other.m_ownedNodes.size() );
		for ( GmodNode* ownedNodeToCopy : other.m_ownedNodes )
		{
			if ( ownedNodeToCopy )
			{
				m_ownedNodes.push_back( new GmodNode( *ownedNodeToCopy ) );
			}
			else
			{
				m_ownedNodes.push_back( nullptr );
			}
		}
		SPDLOG_DEBUG( "GmodPath copy assignment: Assigned from path ending with node '{}', {} owned nodes copied.",
			( m_node ? std::string( m_node->code() ) : "null" ), m_ownedNodes.size() );
		return *this;
	}

	GmodPath& GmodPath::operator=( GmodPath&& other ) noexcept
	{
		SPDLOG_TRACE( "GmodPath move assignment operator called." );
		if ( this == &other )
		{
			SPDLOG_DEBUG( "GmodPath move assignment: Self-assignment detected, no action taken." );
			return *this;
		}

		for ( GmodNode* ownedNode : m_ownedNodes )
		{
			delete ownedNode;
		}
		m_ownedNodes.clear();

		m_visVersion = other.m_visVersion;
		m_gmod = other.m_gmod;
		m_parents = std::move( other.m_parents );
		m_node = other.m_node;
		m_ownedNodes = std::move( other.m_ownedNodes );

		other.m_node = nullptr;
		other.m_gmod = nullptr;
		other.m_visVersion = VisVersion::Unknown;
		SPDLOG_DEBUG( "GmodPath move assignment: Assigned from path ending with node '{}', source path members reset.",
			( m_node ? std::string( m_node->code() ) : "null" ) );

		return *this;
	}

	GmodPath::~GmodPath()
	{
		SPDLOG_TRACE( "GmodPath destructor: Deleting {} owned nodes.", m_ownedNodes.size() );
		for ( GmodNode* ownedNode : m_ownedNodes )
		{
			delete ownedNode;
		}
		m_ownedNodes.clear();
	}

	bool GmodPath::operator==( const GmodPath& other ) const noexcept
	{
		SPDLOG_TRACE( "GmodPath::operator== called." );
		if ( this == &other )
		{
			SPDLOG_DEBUG( "GmodPath::operator==: Same instance, returning true." );

			return true;
		}

		if ( m_parents.size() != other.m_parents.size() )
		{
			SPDLOG_DEBUG( "GmodPath::operator==: Different parent counts ({} vs {}), returning false.",
				m_parents.size(), other.m_parents.size() );

			return false;
		}

		if ( m_visVersion != other.m_visVersion )
		{
			SPDLOG_DEBUG( "GmodPath::operator==: Different VisVersions ({} vs {}), returning false.",
				static_cast<int>( m_visVersion ), static_cast<int>( other.m_visVersion ) );

			return false;
		}

		/* Compare parents */
		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			{ /* Null pointers */
				if ( ( !m_parents[i] && other.m_parents[i] ) || ( m_parents[i] && !other.m_parents[i] ) )
				{
					SPDLOG_DEBUG( "GmodPath::operator==: One parent node is null at index {}, returning false.", i );

					return false;
				}

				if ( !m_parents[i] && !other.m_parents[i] )
				{
					continue;
				}
			}

			{ /* Compare codes */
				if ( m_parents[i]->code() != other.m_parents[i]->code() )
				{
					SPDLOG_DEBUG( "GmodPath::operator==: Different parent codes at index {} ({} vs {}), returning false.",
						i, std::string( m_parents[i]->code() ), std::string( other.m_parents[i]->code() ) );

					return false;
				}
			}

			{ /* Compare locations */
				bool thisHasLocation = m_parents[i]->location().has_value();
				bool otherHasLocation = other.m_parents[i]->location().has_value();

				if ( thisHasLocation != otherHasLocation )
				{
					SPDLOG_DEBUG( "GmodPath::operator==: Location presence mismatch for parent at index {} ({} vs {}), returning false.",
						i, thisHasLocation, otherHasLocation );

					return false;
				}

				if ( thisHasLocation && otherHasLocation &&
					 m_parents[i]->location().value() != other.m_parents[i]->location().value() )
				{
					SPDLOG_DEBUG( "GmodPath::operator==: Different parent locations at index {} ({} vs {}), returning false.",
						i, m_parents[i]->location().value().toString(), other.m_parents[i]->location().value().toString() );

					return false;
				}
			}
		}

		{ /* Null target nodes */
			if ( ( !m_node && other.m_node ) || ( m_node && !other.m_node ) )
			{
				SPDLOG_DEBUG( "GmodPath::operator==: One target node is null while the other is not, returning false." );

				return false;
			}

			if ( !m_node && !other.m_node )
			{
				SPDLOG_DEBUG( "GmodPath::operator==: Both target nodes are null, paths are considered equal." );

				return true;
			}
		}

		{ /* Target node code */

			if ( m_node->code() != other.m_node->code() )
			{
				SPDLOG_DEBUG( "GmodPath::operator==: Different target node codes ({} vs {}), returning false.",
					std::string( m_node->code() ), std::string( other.m_node->code() ) );

				return false;
			}
		}

		{ /* Target node location */

			bool thisHasLocation = m_node->location().has_value();
			bool otherHasLocation = other.m_node->location().has_value();

			if ( thisHasLocation != otherHasLocation )
			{
				SPDLOG_DEBUG( "GmodPath::operator==: Location presence mismatch for target node ({} vs {}), returning false.",
					thisHasLocation, otherHasLocation );

				return false;
			}

			if ( thisHasLocation && otherHasLocation &&
				 m_node->location().value() != other.m_node->location().value() )
			{
				SPDLOG_DEBUG( "GmodPath::operator==: Different target node locations ({} vs {}), returning false.",
					m_node->location().value().toString(), other.m_node->location().value().toString() );

				return false;
			}
		}

		SPDLOG_DEBUG( "GmodPath::operator==: Paths are equal by value comparison, returning true." );

		return true;
	}

	bool GmodPath::operator!=( const GmodPath& other ) const noexcept
	{
		SPDLOG_TRACE( "GmodPath::operator!= called." );
		bool areEqual = ( *this == other );
		SPDLOG_DEBUG( "GmodPath::operator!=: Result of equality check was {}. Returning {}.", areEqual, !areEqual );
		return !areEqual;
	}

	GmodNode* GmodPath::operator[]( size_t index ) const
	{
		SPDLOG_TRACE( "GmodPath::operator[] (const) called with index: {}.", index );
		if ( index >= ( m_parents.size() + 1 ) )
		{
			SPDLOG_ERROR( "GmodPath::operator[] (const): Index {} out of bounds. Path length is {}.", index, m_parents.size() + 1 );
			throw std::out_of_range( fmt::format( "Index {} out of range for GmodPath indexer. Path length is {}.", index, m_parents.size() + 1 ) );
		}

		if ( index < m_parents.size() )
		{
			GmodNode* parentNode = m_parents[index];
			SPDLOG_DEBUG( "GmodPath::operator[] (const): Index {} corresponds to parent node: {} (code: {}).",
				index, fmt::ptr( parentNode ), ( parentNode ? std::string( parentNode->code() ) : "null" ) );
			return parentNode;
		}
		else
		{
			SPDLOG_DEBUG( "GmodPath::operator[] (const): Index {} corresponds to final node m_node: {} (code: {}).",
				index, fmt::ptr( m_node ), ( m_node ? std::string( m_node->code() ) : "null" ) );
			return m_node;
		}
	}

	GmodNode*& GmodPath::operator[]( size_t index )
	{
		SPDLOG_TRACE( "GmodPath::operator[] (non-const) called with index: {}.", index );
		if ( index >= ( m_parents.size() + 1 ) )
		{
			SPDLOG_ERROR( "GmodPath::operator[] (non-const): Index {} out of bounds. Path length is {}.", index, m_parents.size() + 1 );
			throw std::out_of_range( fmt::format( "Index {} out of range for GmodPath indexer. Path length is {}.", index, m_parents.size() + 1 ) );
		}

		if ( index < m_parents.size() )
		{
			SPDLOG_DEBUG( "GmodPath::operator[] (non-const): Index {} corresponds to parent node reference: {}.",
				index, fmt::ptr( m_parents[index] ) );
			return m_parents[index];
		}
		else
		{
			SPDLOG_DEBUG( "GmodPath::operator[] (non-const): Index {} corresponds to final node m_node reference: {}.",
				index, fmt::ptr( m_node ) );
			return m_node;
		}
	}

	GmodPath GmodPath::build()
	{
		SPDLOG_TRACE( "GmodPath::build() called for path with m_node: {}.", ( m_node ? m_node->code().data() : "null" ) );

		return *this;
	}

	bool GmodPath::isValid( const std::vector<GmodNode*>& parents, const GmodNode& node )
	{
		SPDLOG_TRACE( "GmodPath::isValid(parentsCount={}, nodeCode={}) called", parents.size(), node.code().data() );

		if ( parents.empty() )
		{
			SPDLOG_DEBUG( "GmodPath::isValid: parents is empty, returning false." );
			return false;
		}

		GmodNode* firstParent = parents[0];
		if ( firstParent == nullptr )
		{
			SPDLOG_DEBUG( "GmodPath::isValid: First parent in parents is null, returning false." );
			return false;
		}
		if ( !firstParent->isRoot() )
		{
			SPDLOG_DEBUG( "GmodPath::isValid: First parent (code: {}) is not root, returning false.", firstParent->code().data() );
			return false;
		}

		for ( size_t i = 0; i < parents.size(); ++i )
		{
			GmodNode* currentParentNode = parents[i];
			if ( currentParentNode == nullptr )
			{
				SPDLOG_WARN( "GmodPath::isValid: Null parent encountered in parents at index {}.", i );
				return false;
			}

			const GmodNode* childNodeToCheck;
			if ( i + 1 < parents.size() )
			{
				childNodeToCheck = parents[i + 1];
				if ( childNodeToCheck == nullptr )
				{
					SPDLOG_WARN( "GmodPath::isValid: Null child (next parent) encountered in parents at index {}.", i + 1 );
					return false;
				}
			}
			else
			{
				childNodeToCheck = &node;
			}

			if ( !currentParentNode->isChild( *childNodeToCheck ) )
			{
				SPDLOG_DEBUG( "GmodPath::isValid: Node '{}' is not a child of '{}' (parent at index {}). Returning false.",
					childNodeToCheck->code().data(), currentParentNode->code().data(), i );
				return false;
			}
		}

		SPDLOG_DEBUG( "GmodPath::isValid: Path segment is valid. Returning true." );
		return true;
	}

	bool GmodPath::isValid( const std::vector<GmodNode*>& parents, const GmodNode& node, size_t& missingLinkAt )
	{
		SPDLOG_TRACE( "GmodPath::isValid(parentsCount={}, nodeCode={}, out missingLinkAt) called", parents.size(), node.code().data() );

		missingLinkAt = std::numeric_limits<size_t>::max();

		if ( parents.empty() )
		{
			SPDLOG_WARN( "GmodPath::isValid OL2: parents is empty, returning false." );
			return false;
		}

		GmodNode* firstParent = parents[0];
		if ( firstParent == nullptr )
		{
			SPDLOG_WARN( "GmodPath::isValid OL2: First parent in parents is null, returning false." );
			missingLinkAt = 0;
			return false;
		}

		if ( !firstParent->isRoot() )
		{
			SPDLOG_WARN( "GmodPath::isValid OL2: First parent (code: {}) is not root, returning false.", firstParent->code().data() );

			missingLinkAt = std::numeric_limits<size_t>::max();
			return false;
		}

		for ( size_t i = 0; i < parents.size(); ++i )
		{
			GmodNode* currentParentNode = parents[i];
			if ( currentParentNode == nullptr )
			{
				SPDLOG_WARN( "GmodPath::isValid OL2: Null parent encountered in parents at index {}.", i );
				missingLinkAt = i;
				return false;
			}

			const GmodNode* childNodeToCheck;
			if ( i + 1 < parents.size() )
			{
				childNodeToCheck = parents[i + 1];
				if ( childNodeToCheck == nullptr )
				{
					SPDLOG_WARN( "GmodPath::isValid OL2: Null child (next parent) encountered in parents at index {}.", i + 1 );
					missingLinkAt = i;
					return false;
				}
			}
			else
			{
				childNodeToCheck = &node;
			}

			if ( !currentParentNode->isChild( *childNodeToCheck ) )
			{
				SPDLOG_WARN( "GmodPath::isValid OL2: Node '{}' is not a child of '{}' (parent at index {}). Returning false.",
					childNodeToCheck->code().data(), currentParentNode->code().data(), i );
				missingLinkAt = i;
				return false;
			}
		}

		SPDLOG_DEBUG( "GmodPath::isValid OL2: Path segment is valid. Returning true." );
		return true;
	}

	bool GmodPath::isMappable()
	{
		SPDLOG_TRACE( "GmodPath::isMappable() called." );
		if ( !m_node )
		{
			SPDLOG_WARN( "GmodPath::isMappable(): m_node is null, returning false." );

			return false;
		}

		bool result = m_node->isMappable();
		SPDLOG_DEBUG( "GmodPath::isMappable(): m_node ('{}') isMappable() returned {}.", m_node->code().data(), result );

		return result;
	}

	bool GmodPath::isIndividualizable()
	{
		SPDLOG_TRACE( "GmodPath::isIndividualizable() called." );

		if ( !m_node )
		{
			SPDLOG_WARN( "GmodPath::isIndividualizable(): m_node is null. Path is not considered individualizable." );
			return false;
		}

		internal::LocationSetsVisitor visitor;
		size_t currentPathLength = length();
		SPDLOG_DEBUG( "GmodPath::isIndividualizable(): Path length is {}.", currentPathLength );

		for ( size_t i = 0; i < currentPathLength; ++i )
		{
			GmodNode* currentNodeInPath = ( *this )[i];
			if ( !currentNodeInPath )
			{
				SPDLOG_WARN( "GmodPath::isIndividualizable(): Node at index {} is null, skipping.", i );
				continue;
			}
			SPDLOG_TRACE( "GmodPath::isIndividualizable(): Visiting node at index {}: {} (code: {}).",
				i, fmt::ptr( currentNodeInPath ), currentNodeInPath->code().data() );

			std::optional<std::tuple<int, int, std::optional<Location>>> set =
				visitor.visit( *currentNodeInPath, static_cast<int>( i ), m_parents, *m_node );

			if ( set.has_value() )
			{
				SPDLOG_DEBUG( "GmodPath::isIndividualizable(): Found an individualizable set at index {}. Returning true.", i );
				return true;
			}
			SPDLOG_TRACE( "GmodPath::isIndividualizable(): No individualizable set found for node at index {}.", i );
		}

		SPDLOG_DEBUG( "GmodPath::isIndividualizable(): No individualizable sets found in the path. Returning false." );
		return false;
	}

	VisVersion GmodPath::visVersion() const noexcept
	{
		SPDLOG_TRACE( "GmodPath::visVersion() called, returning m_visVersion: {}.", static_cast<int>( m_visVersion ) );
		return m_visVersion;
	}

	size_t GmodPath::hashCode() const noexcept
	{
		SPDLOG_TRACE( "GmodPath::hashCode() called." );
		size_t seed = 0;

		std::hash<GmodNode*> nodeHasher;

		for ( const GmodNode* parentNode : m_parents )
		{
			size_t parentHash = parentNode ? nodeHasher( const_cast<GmodNode*>( parentNode ) ) : 0;
			seed ^= parentHash + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
		}

		size_t nodeHash = m_node ? nodeHasher( m_node ) : 0;
		seed ^= nodeHash + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );

		SPDLOG_DEBUG( "GmodPath::hashCode() calculated hash: {}.", seed );
		return seed;
	}

	const Gmod* GmodPath::gmod() const noexcept
	{
		return m_gmod;
	}

	GmodNode* GmodPath::node() const noexcept
	{
		return m_node;
	}

	const std::vector<GmodNode*>& GmodPath::parents() const noexcept
	{
		return m_parents;
	}

	size_t GmodPath::length() const noexcept
	{
		SPDLOG_TRACE( "GmodPath::length() called." );
		size_t len = m_parents.size() + 1;
		SPDLOG_DEBUG( "GmodPath::length() returning {}.", len );
		return len;
	}

	GmodNode* GmodPath::rootNode() const noexcept
	{
		SPDLOG_TRACE( "GmodPath::rootNode() called." );
		if ( !m_parents.empty() )
		{
			GmodNode* rootNode = m_parents.front();
			SPDLOG_DEBUG( "GmodPath::rootNode() returning from m_parents.front(): {} (code: {}).",
				fmt::ptr( rootNode ), ( rootNode ? std::string( rootNode->code() ) : "null" ) );
			return rootNode;
		}
		SPDLOG_DEBUG( "GmodPath::rootNode() m_parents is empty, returning m_node: {} (code: {}).",
			fmt::ptr( m_node ), ( m_node ? std::string( m_node->code() ) : "null" ) );
		return m_node;
	}

	GmodNode* GmodPath::parentNode() const noexcept
	{
		SPDLOG_TRACE( "GmodPath::parentNode() called." );
		if ( !m_parents.empty() )
		{
			GmodNode* parentNode = m_parents.back();
			SPDLOG_DEBUG( "GmodPath::parentNode() returning from m_parents.back(): {} (code: {}).",
				fmt::ptr( parentNode ), ( parentNode ? std::string( parentNode->code() ) : "null" ) );
			return parentNode;
		}
		SPDLOG_DEBUG( "GmodPath::parentNode() m_parents is empty, returning nullptr." );
		return nullptr;
	}

	std::vector<GmodIndividualizableSet> GmodPath::individualizableNodes() const
	{
		SPDLOG_TRACE( "GmodPath::individualizableNodes() called." );
		std::vector<GmodIndividualizableSet> result;
		internal::LocationSetsVisitor visitor;

		size_t currentPathLength = length();
		SPDLOG_DEBUG( "GmodPath::individualizableNodes() path length: {}.", currentPathLength );

		if ( !m_node )
		{
			SPDLOG_ERROR( "GmodPath::individualizableNodes() m_node is null. Cannot proceed." );
			throw std::runtime_error( "GmodPath::individualizableNodes() m_node is null." );
		}

		for ( size_t i = 0; i < currentPathLength; ++i )
		{
			GmodNode* currentNodeInPath = ( *this )[i];
			if ( !currentNodeInPath )
			{
				SPDLOG_WARN( "GmodPath::individualizableNodes() found null node at index {} via operator[] while iterating path. Skipping.", i );
				continue;
			}
			SPDLOG_TRACE( "GmodPath::individualizableNodes() visiting node at index {}: {} (code: {}).",
				i, fmt::ptr( currentNodeInPath ), currentNodeInPath->code() );

			auto set = visitor.visit( *currentNodeInPath, static_cast<int>( i ), m_parents, *m_node );

			if ( !set.has_value() )
			{
				SPDLOG_TRACE( "GmodPath::individualizableNodes() visitor.visit returned no set for index {}.", i );
				continue;
			}

			const auto& setTuple = set.value();
			int startIdx = std::get<0>( setTuple );
			int endIdx = std::get<1>( setTuple );
			SPDLOG_DEBUG( "GmodPath::individualizableNodes() visitor.visit returned set for index {}: start={}, end={}.", i, startIdx, endIdx );

			std::vector<int> indicesForSet;
			if ( startIdx == endIdx )
			{
				indicesForSet.push_back( startIdx );
			}
			else
			{
				indicesForSet.reserve( static_cast<size_t>( endIdx - startIdx + 1 ) );
				for ( int j = startIdx; j <= endIdx; ++j )
				{
					indicesForSet.push_back( j );
				}
			}

			try
			{
				SPDLOG_DEBUG( "GmodPath::individualizableNodes() creating GmodIndividualizableSet with {} indices (start: {}, end: {}).",
					indicesForSet.size(), startIdx, endIdx );

				result.emplace_back( indicesForSet, *this );
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "GmodPath::individualizableNodes() failed to create GmodIndividualizableSet for indices starting at {}: {}.", startIdx, ex.what() );
				throw;
			}
		}
		SPDLOG_INFO( "GmodPath::individualizableNodes() found {} sets.", result.size() );

		return result;
	}

	std::optional<std::string> GmodPath::normalAssignmentName( size_t nodeDepth ) const
	{
		SPDLOG_TRACE( "GmodPath::normalAssignmentName called with nodeDepth: {}.", nodeDepth );
		if ( nodeDepth >= length() )
		{
			SPDLOG_WARN( "GmodPath::normalAssignmentName: nodeDepth {} is out of bounds for path length {}.", nodeDepth, length() );

			return std::nullopt;
		}

		GmodNode* targetNode = ( *this )[nodeDepth];
		if ( !targetNode )
		{
			SPDLOG_WARN( "GmodPath::normalAssignmentName: Node at depth {} is null.", nodeDepth );

			return std::nullopt;
		}

		SPDLOG_DEBUG( "GmodPath::normalAssignmentName: Node at depth {} is {} (code: {}).",
			nodeDepth, fmt::ptr( targetNode ), targetNode->code() );

		if ( targetNode->isMappable() )
		{
			const auto& meta = targetNode->metadata();
			const std::unordered_map<std::string, std::string>& assignments = meta.normalAssignmentNames();

			if ( !assignments.empty() )
			{
				const std::string& chosenName = assignments.begin()->second;
				SPDLOG_DEBUG( "GmodPath::normalAssignmentName: Node is mappable, returning a normal assignment name from map: '{}'.", chosenName );

				return chosenName;
			}
			else
			{
				SPDLOG_DEBUG( "GmodPath::normalAssignmentName: Node is mappable, but normal assignment name is not set." );

				return std::nullopt;
			}
		}
		else
		{
			SPDLOG_DEBUG( "GmodPath::normalAssignmentName: Node at depth {} is not mappable.", nodeDepth );

			return std::nullopt;
		}
	}

	std::vector<std::pair<size_t, std::string>> GmodPath::commonNames() const
	{
		SPDLOG_TRACE( "GmodPath::commonNames() called." );
		std::vector<std::pair<size_t, std::string>> result;

		size_t currentPathLength = length();
		if ( currentPathLength == 0 )
		{
			SPDLOG_DEBUG( "GmodPath::commonNames(): Path length is 0, returning empty list." );
			return result;
		}

		for ( size_t depth = 0; depth < currentPathLength; ++depth )
		{
			GmodNode* nodeInPath = ( *this )[depth];
			if ( !nodeInPath )
			{
				SPDLOG_WARN( "GmodPath::commonNames(): Node at depth {} is null, skipping.", depth );

				continue;
			}
			GmodNode& nodeInPathRef = *nodeInPath;
			const auto& nodeInPathMetadata = nodeInPathRef.metadata();

			SPDLOG_TRACE( "GmodPath::commonNames(): Processing node at depth {}: {} (code: {}).",
				depth, fmt::ptr( &nodeInPathRef ), nodeInPathRef.code() );

			bool isTargetNode = ( depth == ( m_parents.size() ) );

			if ( !( nodeInPathRef.isLeafNode() || isTargetNode ) || !nodeInPathRef.isFunctionNode() )
			{
				SPDLOG_TRACE( "GmodPath::commonNames(): Node at depth {} (code: {}) did not meet criteria (isLeaf: {}, isTarget: {}, isFunction: {}). Skipping.",
					depth, nodeInPathRef.code(), nodeInPathRef.isLeafNode(), isTargetNode, nodeInPathRef.isFunctionNode() );

				continue;
			}
			SPDLOG_DEBUG( "GmodPath::commonNames(): Node at depth {} (code: {}) met criteria.", depth, nodeInPathRef.code() );

			std::string nameForYield;
			std::optional<std::string> metadataCommonName = nodeInPathMetadata.commonName();

			if ( metadataCommonName.has_value() )
			{
				nameForYield = metadataCommonName.value();
			}
			else
			{
				nameForYield = nodeInPathMetadata.name();
			}
			SPDLOG_TRACE( "GmodPath::commonNames(): Initial name for node at depth {}: '{}'.", depth, nameForYield );

			const std::unordered_map<std::string, std::string>& assignments = nodeInPathMetadata.normalAssignmentNames();

			SPDLOG_TRACE( "GmodPath::commonNames(): Node at depth {} has {} normal assignment names in its map.", depth, assignments.size() );

			if ( m_node )
			{
				std::string keyFromPathNodeCode = std::string( m_node->code() );
				auto nodePathIterator = assignments.find( keyFromPathNodeCode );
				if ( nodePathIterator != assignments.end() )
				{
					nameForYield = nodePathIterator->second;
					SPDLOG_TRACE( "GmodPath::commonNames(): Name overridden by GmodPath.Node.Code key '{}': '{}'.", keyFromPathNodeCode, nameForYield );
				}
			}

			for ( int i = static_cast<int>( m_parents.size() ) - 1; i >= static_cast<int>( depth ); --i )
			{
				if ( static_cast<size_t>( i ) < m_parents.size() )
				{
					GmodNode* parentForKey = m_parents[static_cast<size_t>( i )];
					if ( parentForKey )
					{
						std::string keyFromPathParentCode = std::string( parentForKey->code() );
						auto parentPathIterator = assignments.find( keyFromPathParentCode );
						if ( parentPathIterator != assignments.end() )
						{
							nameForYield = parentPathIterator->second;
							SPDLOG_TRACE( "GmodPath::commonNames(): Name overridden by GmodPath.Parent[{}].Code key '{}': '{}'.", i, keyFromPathParentCode, nameForYield );
						}
					}
				}
			}

			SPDLOG_DEBUG( "GmodPath::commonNames(): Adding common name for depth {}: '{}'.", depth, nameForYield );

			result.emplace_back( depth, nameForYield );
		}

		SPDLOG_INFO( "GmodPath::commonNames() found {} common names.", result.size() );

		return result;
	}

	std::string GmodPath::toString() const
	{
		SPDLOG_TRACE( "GmodPath::toString() const called." );

		std::stringstream builder;
		toString( builder, '/' );

		std::string result = builder.str();
		SPDLOG_DEBUG( "GmodPath::toString() const generated: '{}'", result );

		return result;
	}

	void GmodPath::toString( std::stringstream& builder, char separator ) const
	{
		SPDLOG_TRACE( "GmodPath::toString(std::stringstream& builder, char separator='{}') called. Initial stream state good: {}", separator, builder.good() );

		bool needsSeparator = ( builder.tellp() > 0 );

		for ( const GmodNode* parentNode : m_parents )
		{
			if ( !parentNode )
			{
				SPDLOG_WARN( "GmodPath::toString(builder): Encountered null parent node, skipping." );
				continue;
			}

			if ( m_gmod && parentNode == &m_gmod->rootNode() )
			{
				SPDLOG_TRACE( "GmodPath::toString(builder): Parent node '{}' is the GMOD root, skipping.", parentNode->code() );
				continue;
			}

			if ( !Gmod::isLeafNode( parentNode->metadata() ) )
			{
				SPDLOG_TRACE( "GmodPath::toString(builder): Parent node '{}' is not a leaf node, skipping.", parentNode->code() );
				continue;
			}

			if ( needsSeparator )
			{
				builder.put( separator );
			}

			parentNode->toString( builder );
			SPDLOG_TRACE( "GmodPath::toString(builder): Appended leaf parent '{}'.", parentNode->code() );

			needsSeparator = true;
		}

		if ( m_node )
		{
			if ( needsSeparator )
			{
				builder.put( separator );
			}
			m_node->toString( builder );
			SPDLOG_TRACE( "GmodPath::toString(builder): Appended final node '{}'.", m_node->code() );
		}
		else
		{
			SPDLOG_WARN( "GmodPath::toString(builder): m_node is null. Path string might be incomplete." );
		}
		SPDLOG_DEBUG( "GmodPath::toString(builder, separator) finished." );
	}

	std::string GmodPath::toStringDump() const
	{
		SPDLOG_TRACE( "GmodPath::toStringDump() const called." );

		std::stringstream builder;
		toStringDump( builder );

		std::string result = builder.str();

		SPDLOG_DEBUG( "GmodPath::toStringDump() const generated: '{}'", result );

		return result;
	}

	void GmodPath::toStringDump( std::stringstream& builder ) const
	{
		SPDLOG_TRACE( "GmodPath::toStringDump(builder) called. Initial stream pos: {}", builder.tellp() );

		Enumerator iter = fullPath();
		bool firstItemPrinted = false;

		while ( iter.next() )
		{
			size_t currentDepth = iter.m_currentIndex;
			GmodNode* pathNode = iter.current();

			if ( !pathNode )
			{
				SPDLOG_WARN( "GmodPath::toStringDump(builder): Enumerator returned null node at depth {}, skipping.", currentDepth );
				continue;
			}

			SPDLOG_TRACE( "GmodPath::toStringDump(builder): Processing node '{}' at depth {}.", pathNode->code().data(), currentDepth );

			if ( currentDepth == 0 )
			{
				SPDLOG_TRACE( "GmodPath::toStringDump(builder): Skipping root node (depth 0) '{}'.", pathNode->code().data() );
				continue;
			}

			if ( firstItemPrinted )
			{
				builder << " | ";
				SPDLOG_TRACE( "GmodPath::toStringDump(builder): Appended separator ' | ' before node '{}'.", pathNode->code().data() );
			}

			builder << pathNode->code().data();
			SPDLOG_TRACE( "GmodPath::toStringDump(builder): Appended node code '{}'.", pathNode->code().data() );

			const auto& metadata = pathNode->metadata();
			const std::string& nodeName = metadata.name();
			if ( !nodeName.empty() )
			{
				builder << "/N:" << nodeName;
				SPDLOG_TRACE( "GmodPath::toStringDump(builder): Appended Name '/N:{}'.", nodeName );
			}

			const std::optional<std::string>& commonNameRef = metadata.commonName();
			if ( commonNameRef.has_value() && !commonNameRef.value().empty() )
			{
				builder << "/CN:" << commonNameRef.value();
				SPDLOG_TRACE( "GmodPath::toStringDump(builder): Appended CommonName '/CN:{}'.", commonNameRef.value() );
			}

			std::optional<std::string> normalAssignment = normalAssignmentName( currentDepth );
			if ( normalAssignment.has_value() && !normalAssignment.value().empty() )
			{
				builder << "/NAN:" << normalAssignment.value();
				SPDLOG_TRACE( "GmodPath::toStringDump(builder): Appended NormalAssignmentName '/NAN:{}'.", normalAssignment.value() );
			}

			firstItemPrinted = true;
		}

		SPDLOG_DEBUG( "GmodPath::toStringDump(builder) finished. Final stream content length (approx): {}", builder.tellp() );
	}

	std::string GmodPath::toFullPathString() const
	{
		SPDLOG_TRACE( "GmodPath::toFullPathString() const called." );

		std::stringstream builder;
		toFullPathString( builder );

		std::string result = builder.str();
		SPDLOG_DEBUG( "GmodPath::toFullPathString() const generated: '{}'", result );

		return result;
	}

	void GmodPath::toFullPathString( std::stringstream& builder ) const
	{
		SPDLOG_TRACE( "GmodPath::toFullPathString(std::stringstream& builder) called. Initial stream pos: {}", builder.tellp() );

		size_t totalLength = length();
		if ( totalLength == 0 && m_node == nullptr )
		{
			SPDLOG_DEBUG( "GmodPath::toFullPathString(builder): Path is effectively empty, doing nothing." );
			return;
		}

		for ( const auto& pair : fullPath() )
		{
			size_t currentDepth = pair.first;
			const GmodNode* pathNode = pair.second;

			if ( !pathNode )
			{
				SPDLOG_WARN( "GmodPath::toFullPathString(builder): Encountered null node at depth {}.", currentDepth );
				continue;
			}

			pathNode->toString( builder );
			SPDLOG_TRACE( "GmodPath::toFullPathString(builder): Appended node '{}' (depth {}).", pathNode->code(), currentDepth );

			if ( currentDepth < totalLength - 1 )
			{
				builder.put( '/' );
				SPDLOG_TRACE( "GmodPath::toFullPathString(builder): Appended separator after depth {}.", currentDepth );
			}
		}
		SPDLOG_DEBUG( "GmodPath::toFullPathString(builder) finished. Final stream pos: {}", builder.str() );
	}

	GmodPath GmodPath::withoutLocations() const
	{
		SPDLOG_TRACE( "GmodPath::withoutLocations() called for path ending with node: {}.", ( m_node ? m_node->code().data() : "null" ) );

		GmodPath newPath;

		if ( !m_gmod )
		{
			SPDLOG_WARN( "GmodPath::withoutLocations(): Original path's Gmod is null. Returning an empty path." );
			return newPath;
		}
		newPath.m_gmod = this->m_gmod;
		newPath.m_visVersion = this->m_visVersion;

		newPath.m_parents.reserve( this->m_parents.size() );

		for ( GmodNode* originalParentNode : this->m_parents )
		{
			if ( !originalParentNode )
			{
				newPath.m_parents.push_back( nullptr );
				SPDLOG_TRACE( "GmodPath::withoutLocations(): Original parent is null, adding null to new path's parents." );
				continue;
			}

			GmodNode parentNodeWithoutLocation = originalParentNode->withoutLocation();
			GmodNode* newHeapParentNode = new GmodNode( parentNodeWithoutLocation );

			newPath.m_parents.push_back( newHeapParentNode );
			newPath.m_ownedNodes.push_back( newHeapParentNode );
			SPDLOG_TRACE( "GmodPath::withoutLocations(): Created new parent node {} (was {}) without location.",
				newHeapParentNode->code().data(), originalParentNode->code().data() );
		}

		if ( this->m_node )
		{
			GmodNode finalNodeWithoutLocation = this->m_node->withoutLocation();
			GmodNode* newHeapFinalNode = new GmodNode( finalNodeWithoutLocation );

			newPath.m_node = newHeapFinalNode;
			newPath.m_ownedNodes.push_back( newHeapFinalNode );
			SPDLOG_TRACE( "GmodPath::withoutLocations(): Created new final node {} (was {}) without location.",
				newHeapFinalNode->code().data(), this->m_node->code().data() );

			newPath.m_visVersion = newHeapFinalNode->visVersion();
		}
		else
		{
			newPath.m_node = nullptr;
			SPDLOG_WARN( "GmodPath::withoutLocations(): Original path's final node is null." );
			if ( newPath.m_parents.empty() || !newPath.m_parents.front() )
			{
				newPath.m_visVersion = VisVersion::Unknown;
			}
			else
			{
				if ( newPath.m_parents.front() )
				{
					newPath.m_visVersion = newPath.m_parents.front()->visVersion();
				}
			}
		}

		SPDLOG_DEBUG( "GmodPath::withoutLocations() created new path. Final node: {}. Parents count: {}. Owned nodes count: {}.",
			( newPath.m_node ? newPath.m_node->code().data() : "null" ), newPath.m_parents.size(), newPath.m_ownedNodes.size() );

		return newPath;
	}

	GmodPath GmodPath::parse( std::string_view pathString, VisVersion visVersion )
	{
		SPDLOG_TRACE( "GmodPath::parse(pathString='{}', visVersion={}) called.", pathString, static_cast<int>( visVersion ) );
		std::optional<GmodPath> outPath;
		if ( tryParse( pathString, visVersion, outPath ) )
		{
			if ( outPath.has_value() )
			{
				SPDLOG_DEBUG( "GmodPath::parse: Successfully parsed pathString '{}' for VisVersion {}.", pathString, static_cast<int>( visVersion ) );
				return std::move( outPath.value() );
			}
			else
			{
				SPDLOG_ERROR( "GmodPath::parse: tryParse succeeded for '{}' but returned no path.", pathString );
				throw std::runtime_error( fmt::format( "GmodPath::parse: tryParse succeeded for '{}' but returned no path.", pathString ) );
			}
		}
		else
		{
			SPDLOG_ERROR( "GmodPath::parse: Could not parse pathString '{}' for VisVersion {}.", pathString, static_cast<int>( visVersion ) );
			throw std::invalid_argument( fmt::format( "Could not parse GmodPath from string: '{}' for VisVersion {}", pathString, static_cast<int>( visVersion ) ) );
		}
	}

	bool GmodPath::tryParse( std::string_view pathString, VisVersion visVersion, std::optional<GmodPath>& outPath )
	{
		SPDLOG_TRACE( "GmodPath::tryParse(pathString='{}', visVersion={}, outPath) called.", pathString, static_cast<int>( visVersion ) );
		outPath.reset();

		try
		{
			VIS& vis = VIS::instance();
			const Gmod& gmod = vis.gmod( visVersion );
			const Locations& locations = vis.locations( visVersion );

			bool result = tryParse( pathString, gmod, locations, outPath );

			if ( result )
			{
				SPDLOG_DEBUG( "GmodPath::tryParse: Successfully parsed pathString '{}' for VisVersion {}. Path created: {}",
					pathString, static_cast<int>( visVersion ), outPath.has_value() );
			}
			else
			{
				SPDLOG_WARN( "GmodPath::tryParse: Failed to parse pathString '{}' for VisVersion {}.", pathString, static_cast<int>( visVersion ) );
			}
			return result;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "GmodPath::tryParse: Exception during parsing or Gmod/Locations retrieval for pathString '{}', VisVersion {}: {}",
				pathString, static_cast<int>( visVersion ), ex.what() );
			return false;
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "GmodPath::tryParse: Unknown exception during parsing or Gmod/Locations retrieval for pathString '{}', VisVersion {}.",
				pathString, static_cast<int>( visVersion ) );
			return false;
		}
	}

	GmodPath GmodPath::parse( std::string_view pathString, const Gmod& gmod, const Locations& locations )
	{
		SPDLOG_TRACE( "GmodPath::parse(pathString='{}', gmod={}, location={}) called.",
			pathString, fmt::ptr( &gmod ), fmt::ptr( &locations ) );

		std::unique_ptr<GmodParsePathResult> result = parseInternal( pathString, gmod, locations );

		if ( !result )
		{
			SPDLOG_DEBUG( "GmodPath::parse: parseInternal returned nullptr for pathString '{}'.", pathString );
			throw std::runtime_error( fmt::format( "GmodPath::parse: parseInternal returned nullptr for pathString '{}'.", std::string( pathString ) ) );
		}

		if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() ) )
		{
			SPDLOG_DEBUG( "GmodPath::parse: Successfully parsed pathString '{}'.", pathString );
			return std::move( okResult->m_path );
		}
		else if ( auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( result.get() ) )
		{
			SPDLOG_ERROR( "GmodPath::parse: Error parsing pathString '{}': {}", pathString, errResult->error );
			throw std::invalid_argument( errResult->error );
		}
		else
		{
			SPDLOG_DEBUG( "GmodPath::parse: Unexpected result type from parseInternal for pathString '{}'.", pathString );
			throw std::runtime_error( fmt::format( "GmodPath::parse: Unexpected result type from parseInternal for pathString '{}'.", std::string( pathString ) ) );
		}
	}

	bool GmodPath::tryParse( std::string_view pathString, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath )
	{
		SPDLOG_TRACE( "GmodPath::tryParse(pathString='{}', gmod={}, locations={}, outPath) called.",
			pathString, fmt::ptr( &gmod ), fmt::ptr( &locations ) );
		outPath.reset();

		try
		{
			std::unique_ptr<GmodParsePathResult> result = parseInternal( pathString, gmod, locations );

			if ( !result )
			{
				SPDLOG_ERROR( "GmodPath::tryParse: parseInternal returned nullptr for pathString '{}'.", pathString );
				return false;
			}

			if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() ) )
			{
				SPDLOG_DEBUG( "GmodPath::tryParse: Successfully parsed pathString '{}'.", pathString );
				outPath.emplace( std::move( okResult->m_path ) );
				return true;
			}
			else if ( [[maybe_unused]] auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( result.get() ) )
			{
				SPDLOG_WARN( "GmodPath::tryParse: Error parsing pathString '{}': {}", pathString, errResult->error );
				return false;
			}
			else
			{
				SPDLOG_ERROR( "GmodPath::tryParse: Unexpected result type from parseInternal for pathString '{}'.", pathString );
				return false;
			}
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "GmodPath::tryParse: Exception during parsing for pathString '{}': {}", pathString, ex.what() );
			return false;
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "GmodPath::tryParse: Unknown exception during parsing for pathString '{}'.", pathString );
			return false;
		}
	}

	GmodPath GmodPath::parseFullPath( std::string_view pathString, VisVersion visVersion )
	{
		SPDLOG_TRACE( "GmodPath::parseFullPath(pathString='{}', visVersion={}) called.", pathString, static_cast<int>( visVersion ) );
		VIS& vis = VIS::instance();
		const Gmod& gmod = vis.gmod( visVersion );
		const Locations& locations = vis.locations( visVersion );
		std::unique_ptr<GmodParsePathResult> result = parseFullPathInternal( pathString, gmod, locations );

		if ( !result )
		{
			SPDLOG_DEBUG( "GmodPath::parseFullPath: parseFullPathInternal returned nullptr for pathString '{}'.", pathString );
			throw std::runtime_error( fmt::format( "GmodPath::parseFullPath: parseFullPathInternal returned nullptr for pathString '{}'.", std::string( pathString ) ) );
		}

		if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() ) )
		{
			SPDLOG_DEBUG( "GmodPath::parseFullPath: Successfully parsed pathString '{}'.", pathString );
			return std::move( okResult->m_path );
		}
		else if ( [[maybe_unused]] auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( result.get() ) )
		{
			SPDLOG_ERROR( "GmodPath::parseFullPath: Error parsing pathString '{}': {}", pathString, errResult->error );
			throw std::invalid_argument( errResult->error );
		}
		else
		{
			SPDLOG_DEBUG( "GmodPath::parseFullPath: Unexpected result type from parseFullPathInternal for pathString '{}'.", pathString );
			throw std::runtime_error( fmt::format( "GmodPath::parseFullPath: Unexpected result type from parseFullPathInternal for pathString '{}'.", std::string( pathString ) ) );
		}
	}

	bool GmodPath::tryParseFullPath( std::string_view pathString, VisVersion visVersion, std::optional<GmodPath>& outPath )
	{
		SPDLOG_TRACE( "GmodPath::tryParseFullPath(pathString='{}', visVersion={}, outPath) called.",
			pathString, static_cast<int>( visVersion ) );
		outPath.reset();

		try
		{
			VIS& vis = VIS::instance();
			const Gmod& gmod = vis.gmod( visVersion );
			const Locations& locations = vis.locations( visVersion );

			std::unique_ptr<GmodParsePathResult> result = parseFullPathInternal( pathString, gmod, locations );

			if ( !result )
			{
				SPDLOG_ERROR( "GmodPath::tryParseFullPath: parseFullPathInternal returned nullptr for pathString '{}'.", pathString );
				return false;
			}

			if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() ) )
			{
				SPDLOG_DEBUG( "GmodPath::tryParseFullPath: Successfully parsed pathString '{}'.", pathString );
				outPath.emplace( std::move( okResult->m_path ) );
				return true;
			}
			else if ( [[maybe_unused]] auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( result.get() ) )
			{
				SPDLOG_WARN( "GmodPath::tryParseFullPath: Error parsing pathString '{}': {}", pathString, errResult->error );
				return false;
			}
			else
			{
				SPDLOG_ERROR( "GmodPath::tryParseFullPath: Unexpected result type from parseFullPathInternal for pathString '{}'.", pathString );
				return false;
			}
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "GmodPath::tryParseFullPath: Exception during parsing or Gmod/Locations retrieval for pathString '{}', VisVersion {}: {}",
				pathString, static_cast<int>( visVersion ), ex.what() );
			return false;
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "GmodPath::tryParseFullPath: Unknown exception during parsing or Gmod/Locations retrieval for pathString '{}', VisVersion {}.",
				pathString, static_cast<int>( visVersion ) );
			return false;
		}
	}

	bool GmodPath::tryParseFullPath( std::string_view pathString, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath )
	{
		SPDLOG_TRACE( "GmodPath::tryParseFullPath(pathString='{}', gmod={}, locations={}, outPath) called.",
			pathString, fmt::ptr( &gmod ), fmt::ptr( &locations ) );
		outPath.reset();

		try
		{
			if ( gmod.visVersion() != locations.visVersion() )
			{
				SPDLOG_ERROR( "GmodPath::tryParseFullPath: GMOD VisVersion {} does not match Locations VisVersion {}.",
					static_cast<int>( gmod.visVersion() ), static_cast<int>( locations.visVersion() ) );

				return false;
			}

			std::unique_ptr<GmodParsePathResult> result = parseFullPathInternal( pathString, gmod, locations );

			if ( !result )
			{
				SPDLOG_ERROR( "GmodPath::tryParseFullPath: parseFullPathInternal returned nullptr for pathString '{}'.", pathString );
				return false;
			}

			if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() ) )
			{
				SPDLOG_DEBUG( "GmodPath::tryParseFullPath: Successfully parsed pathString '{}'.", pathString );
				outPath.emplace( std::move( okResult->m_path ) );
				return true;
			}
			else if ( [[maybe_unused]] auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( result.get() ) )
			{
				SPDLOG_WARN( "GmodPath::tryParseFullPath: Error parsing pathString '{}': {}", pathString, errResult->error );
				return false;
			}
			else
			{
				SPDLOG_ERROR( "GmodPath::tryParseFullPath: Unexpected result type from parseFullPathInternal for pathString '{}'.", pathString );
				return false;
			}
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "GmodPath::tryParseFullPath: Exception during parsing for pathString '{}': {}", pathString, ex.what() );
			return false;
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "GmodPath::tryParseFullPath: Unknown exception during parsing for pathString '{}'.", pathString );
			return false;
		}
	}

	GmodPath::Enumerator GmodPath::fullPath() const
	{
		return Enumerator( this, 0 );
	}

	GmodPath::Enumerator GmodPath::fullPathFrom( size_t fromDepth ) const
	{
		return Enumerator( this, fromDepth );
	}

	std::unique_ptr<GmodParsePathResult> GmodPath::parseInternal( std::string_view item, const Gmod& gmod, const Locations& locations )
	{
		SPDLOG_TRACE( "GmodPath::parseInternal called with item: '{}', GMOD: {}, Locations: {}", item.data(), fmt::ptr( &gmod ), fmt::ptr( &locations ) );

		if ( gmod.visVersion() != locations.visVersion() )
		{
			SPDLOG_ERROR( "GmodPath::parseInternal: GMOD VisVersion {} does not match Locations VisVersion {}.",
				static_cast<int>( gmod.visVersion() ), static_cast<int>( locations.visVersion() ) );
			return std::make_unique<GmodParsePathResult::Err>( "Got different VIS versions for Gmod and Locations arguments" );
		}

		if ( item.empty() )
		{
			SPDLOG_WARN( "GmodPath::parseInternal: Item string is empty." );
			return std::make_unique<GmodParsePathResult::Err>( "Item is empty" );
		}

		size_t start = item.find_first_not_of( " \t\n\r\f\v" );
		if ( start == std::string_view::npos )
		{
			SPDLOG_WARN( "GmodPath::parseInternal: Item string is all whitespace." );
			return std::make_unique<GmodParsePathResult::Err>( "Item is empty" );
		}
		item = item.substr( start );
		size_t end = item.find_last_not_of( " \t\n\r\f\v" );
		item = item.substr( 0, end + 1 );

		if ( !item.empty() && item.front() == '/' )
		{
			item.remove_prefix( 1 );
		}
		if ( item.empty() )
		{
			SPDLOG_WARN( "GmodPath::parseInternal: Item string is empty after trim/remove_prefix." );
			return std::make_unique<GmodParsePathResult::Err>( "Item is empty" );
		}

		std::deque<internal::PathNode> partsQueue;
		std::string_view currentSegment = item;
		while ( !currentSegment.empty() )
		{
			size_t slashPosition = currentSegment.find( '/' );
			std::string_view part = currentSegment.substr( 0, slashPosition );

			internal::PathNode currentPathNode;
			size_t dashPosition = part.find( '-' );
			const GmodNode* tempNodeCheck = nullptr;

			if ( dashPosition != std::string_view::npos )
			{
				std::string_view codePart = part.substr( 0, dashPosition );
				std::string_view locationPart = part.substr( dashPosition + 1 );
				currentPathNode.code = std::string( codePart );

				if ( !gmod.tryGetNode( codePart, tempNodeCheck ) )
				{
					SPDLOG_ERROR( "GmodPath::parseInternal: Failed to get GmodNode for code part '{}' from segment '{}'", codePart.data(), part.data() );
					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Failed to get GmodNode for {}", std::string( part ) ) );
				}
				Location parsedLocation;
				if ( !locations.tryParse( locationPart, parsedLocation ) )
				{
					SPDLOG_ERROR( "GmodPath::parseInternal: Failed to parse location part '{}' from segment '{}'", locationPart.data(), part.data() );
					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Failed to parse location {}", std::string( locationPart ) ) );
				}
				currentPathNode.location = parsedLocation;
			}
			else
			{
				currentPathNode.code = std::string( part );
				if ( !gmod.tryGetNode( part, tempNodeCheck ) )
				{
					SPDLOG_ERROR( "GmodPath::parseInternal: Failed to get GmodNode for segment '{}'", part.data() );
					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Failed to get GmodNode for {}", std::string( part ) ) );
				}
			}
			partsQueue.push_back( currentPathNode );

			if ( slashPosition == std::string_view::npos )
			{
				break;
			}
			currentSegment = currentSegment.substr( slashPosition + 1 );
		}

		if ( partsQueue.empty() )
		{
			SPDLOG_ERROR( "GmodPath::parseInternal: Failed to find any parts in item string '{}'.", item.data() );
			return std::make_unique<GmodParsePathResult::Err>( "Failed find any parts" );
		}
		for ( const auto& parsedNode : partsQueue )
		{
			if ( parsedNode.code.empty() )
			{
				SPDLOG_ERROR( "GmodPath::parseInternal: Found part with empty code." );
				return std::make_unique<GmodParsePathResult::Err>( "Found part with empty code" );
			}
		}

		internal::PathNode firstToFind = partsQueue.front();
		partsQueue.pop_front();

		const GmodNode* baseNode = nullptr;
		if ( !gmod.tryGetNode( firstToFind.code, baseNode ) || !baseNode )
		{
			SPDLOG_ERROR( "GmodPath::parseInternal: Failed to find base node for code '{}'.", firstToFind.code );
			return std::make_unique<GmodParsePathResult::Err>( "Failed to find base node" );
		}

		internal::ParseContext context( std::move( partsQueue ), gmod, firstToFind );

		std::function<TraversalHandlerResult( internal::ParseContext&, const std::vector<const GmodNode*>&, const GmodNode& )> handler =
			internal::parseInternalTraversalHandler;
		GmodTraversal::traverse( context, *baseNode, handler );

		if ( context.resultingPath.has_value() )
		{
			SPDLOG_INFO( "GmodPath::parseInternal: Path parsing successful for item '{}'.", item.data() );
			return std::make_unique<GmodParsePathResult::Ok>( std::move( context.resultingPath.value() ) );
		}
		else
		{
			SPDLOG_ERROR( "GmodPath::parseInternal: Failed to find path after traversal for item '{}'.", item.data() );

			for ( GmodNode* ownedNode : context.ownedNodesForCurrentPath )
			{
				delete ownedNode;
			}
			return std::make_unique<GmodParsePathResult::Err>( "Failed to find path after traversal" );
		}
	}

	std::unique_ptr<GmodParsePathResult> GmodPath::parseFullPathInternal( std::string_view item, const Gmod& gmod, const Locations& locations )
	{
		SPDLOG_TRACE( "GmodPath::parseFullPathInternal called with item: '{}'", item );
		assert( gmod.visVersion() == locations.visVersion() );

		bool isEmptyOrWhitespace = item.empty();
		if ( !item.empty() )
		{
			bool allWhitespace = true;
			for ( char whiteSpace : item )
			{
				if ( !std::isspace( static_cast<unsigned char>( whiteSpace ) ) )
				{
					allWhitespace = false;

					break;
				}
			}
			if ( allWhitespace )
			{
				isEmptyOrWhitespace = true;
			}
		}

		if ( isEmptyOrWhitespace )
		{
			SPDLOG_WARN( "GmodPath::parseFullPathInternal: Item string is empty or all whitespace." );

			return std::make_unique<GmodParsePathResult::Err>( std::string( "Item is empty" ) );
		}

		std::string_view rootNodeCode = gmod.rootNode().code();
		if ( item.length() < rootNodeCode.length() || item.substr( 0, rootNodeCode.length() ) != rootNodeCode )
		{
			SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Path must start with root node code '{}'. Path was: '{}'", rootNodeCode, item );

			return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Path must start with {}", std::string( rootNodeCode ) ) );
		}

		std::vector<GmodNode> parsedNodes;
		if ( item.length() > 0 )
		{
			parsedNodes.reserve( item.length() / 3 );
		}

		std::string_view currentProcessing = item;
		size_t searchOffset = 0;
		while ( searchOffset < currentProcessing.length() )
		{
			size_t nextSlashPos = currentProcessing.find( '/', searchOffset );
			std::string_view nodeCode;

			if ( nextSlashPos == std::string_view::npos )
			{
				nodeCode = currentProcessing.substr( searchOffset );
				searchOffset = currentProcessing.length();
			}
			else
			{
				nodeCode = currentProcessing.substr( searchOffset, nextSlashPos - searchOffset );
				searchOffset = nextSlashPos + 1;
			}

			if ( nodeCode.empty() && searchOffset >= currentProcessing.length() && currentProcessing.back() == '/' )
			{
				/*
				 * TODO: Use the same logic as in GmodPath::parseInternal
				 */
			}

			size_t dashIndex = nodeCode.find( '-' );
			const GmodNode* tempNode = nullptr;

			if ( dashIndex == std::string_view::npos )
			{
				if ( !gmod.tryGetNode( nodeCode, tempNode ) || !tempNode )
				{
					SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Failed to get GmodNode for '{}'", nodeCode );

					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Failed to get GmodNode for {}", std::string( nodeCode ) ) );
				}

				parsedNodes.push_back( *tempNode );
			}
			else
			{
				std::string_view codepart = nodeCode.substr( 0, dashIndex );
				if ( !gmod.tryGetNode( codepart, tempNode ) || !tempNode )
				{
					SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Failed to get GmodNode for code part '{}'", codepart );
					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Failed to get GmodNode for {}", std::string( codepart ) ) );
				}
				std::string_view location = nodeCode.substr( dashIndex + 1 );
				Location parsedLocation;
				if ( !locations.tryParse( location, parsedLocation ) )
				{
					SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Failed to parse location '{}'", location );
					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Failed to parse location - {}", std::string( location ) ) );
				}

				parsedNodes.push_back( tempNode->tryWithLocation( std::make_optional( parsedLocation ) ) );
			}
		}

		if ( parsedNodes.empty() )
		{
			SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Failed to find any nodes in path string '{}'.", item );
			return std::make_unique<GmodParsePathResult::Err>( std::string( "Failed to find any nodes" ) );
		}

		GmodNode tempEndNode = parsedNodes.back();
		parsedNodes.pop_back();

		std::vector<GmodNode*> tempParentsForValidation;
		tempParentsForValidation.reserve( parsedNodes.size() );
		for ( GmodNode& pv : parsedNodes )
		{
			tempParentsForValidation.push_back( &pv );
		}

		size_t missingLinkAt;
		if ( !GmodPath::isValid( tempParentsForValidation, tempEndNode, missingLinkAt ) )
		{
			SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Sequence of nodes is invalid. Path: '{}'", item );
			return std::make_unique<GmodParsePathResult::Err>( std::string( "Sequence of nodes are invalid" ) );
		}

		internal::LocationSetsVisitor locationVisitor;
		std::optional<int> previousNonNullLocationIdx;

		std::array<std::pair<int, int>, 16> setsArray;
		int setCounter = 0;

		std::vector<GmodNode*> tempParentsForVisit;
		tempParentsForVisit.reserve( parsedNodes.size() );
		for ( GmodNode& node : parsedNodes )
		{
			tempParentsForVisit.push_back( &node );
		}

		for ( int i = 0; i < static_cast<int>( parsedNodes.size() ) + 1; ++i )
		{
			GmodNode& nodeForVisit = ( i < static_cast<int>( parsedNodes.size() ) ) ? parsedNodes[static_cast<size_t>( i )] : tempEndNode;

			std::optional<std::tuple<int, int, std::optional<Location>>> setDetails =
				locationVisitor.visit( nodeForVisit, i, tempParentsForVisit, tempEndNode );

			if ( !setDetails.has_value() )
			{
				if ( !previousNonNullLocationIdx.has_value() && nodeForVisit.location().has_value() )
				{
					previousNonNullLocationIdx = i;
				}
				continue;
			}
			int setStartIdx = std::get<0>( setDetails.value() );
			int setEndIdx = std::get<1>( setDetails.value() );
			std::optional<Location> setCommonLocation = std::get<2>( setDetails.value() );

			if ( previousNonNullLocationIdx.has_value() )
			{
				for ( int j = previousNonNullLocationIdx.value(); j < setStartIdx; ++j )
				{
					GmodNode& previousNode = ( j >= 0 && static_cast<size_t>( j ) < parsedNodes.size() )
												 ? parsedNodes[static_cast<size_t>( j )]
												 : tempEndNode;
					if ( previousNode.location().has_value() )
					{
						SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Expected nodes outside set to be without individualization. Found {} with location.", previousNode.code().data() );
						return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Expected all nodes in the set to be without individualization. Found {}", std::string( previousNode.code() ) ) );
					}
				}
			}
			previousNonNullLocationIdx.reset();

			if ( setCounter < 16 )
			{
				setsArray[static_cast<size_t>( setCounter++ )] = { setStartIdx, setEndIdx };
			}
			else
			{
				SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Exceeded fixed limit for setsArray. Max 16 sets supported. Path: '{}'", item );
				throw std::out_of_range( fmt::format( "Exceeded maximum of 16 location sets supported while parsing path: '{}'", std::string( item ) ) );
			}

			if ( setStartIdx == setEndIdx )
				continue;

			for ( int j = setStartIdx; j <= setEndIdx; ++j )
			{
				if ( j < static_cast<int>( parsedNodes.size() ) )
				{
					if ( j >= 0 )
					{
						parsedNodes[static_cast<size_t>( j )] = parsedNodes[static_cast<size_t>( j )].tryWithLocation( setCommonLocation );
					}
					else
					{
						SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Negative index j ({}) encountered for parsedNodes access.", j );
					}
				}
				else
				{
					tempEndNode = tempEndNode.tryWithLocation( setCommonLocation );
				}
			}
		}

		std::pair<int, int> currentSet = { -1, -1 };
		int currentSetIndex = 0;
		for ( int i = 0; i < static_cast<int>( parsedNodes.size() ) + 1; ++i )
		{
			while ( currentSetIndex < setCounter && currentSet.second < i )
			{
				if ( static_cast<size_t>( currentSetIndex ) < setsArray.size() )
				{
					currentSet = setsArray[static_cast<size_t>( currentSetIndex++ )];
				}
				else
				{
					SPDLOG_ERROR( "GmodPath::parseFullPathInternal: currentSetIndex ({}) is out of bounds for setsArray (size {}). This indicates a critical logic error. Path: '{}'", currentSetIndex, setsArray.size(), item );
					throw std::logic_error( fmt::format( "Internal logic error: currentSetIndex ({}) is out of bounds for setsArray (size {}). Path: '{}'", currentSetIndex, setsArray.size(), std::string( item ) ) );
				}
			}

			bool insideSet = ( currentSet.first != -1 && i >= currentSet.first && i <= currentSet.second );

			GmodNode& nodeCheck = ( static_cast<size_t>( i ) < parsedNodes.size() )
									  ? parsedNodes[static_cast<size_t>( i )]
									  : tempEndNode;

			const GmodNode* expectedLocationNode = nullptr;
			if ( currentSet.second != -1 )
			{
				if ( currentSet.second >= 0 && currentSet.second < static_cast<int>( parsedNodes.size() ) )
				{
					expectedLocationNode = &parsedNodes[static_cast<size_t>( currentSet.second )];
				}
				else
				{
					expectedLocationNode = &tempEndNode;
				}
			}

			if ( insideSet )
			{
				if ( !expectedLocationNode || nodeCheck.location() != expectedLocationNode->location() )
				{
					[[maybe_unused]] auto formatLocation = []( const std::optional<Location>& location ) -> std::string {
						if ( location.has_value() )
						{
							return location.value().toString();
						}
						return "nullopt";
					};

					SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Expected nodes in set to be individualized the same. Found {} with location {}, expected {}.",
						nodeCheck.code().data(),
						formatLocation( nodeCheck.location() ),
						formatLocation( expectedLocationNode ? expectedLocationNode->location() : std::optional<Location>() ) );

					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Expected all nodes in the set to be individualized the same. Found {} with location", std::string( nodeCheck.code() ) ) );
				}
			}
			else
			{
				if ( nodeCheck.location().has_value() )
				{
					SPDLOG_ERROR( "GmodPath::parseFullPathInternal: Expected nodes outside set to be without individualization. Found {} with location.", nodeCheck.code() );
					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Expected all nodes outside set to be without individualization. Found {}", std::string( nodeCheck.code() ) ) );
				}
			}
		}

		auto finalPath = std::make_unique<GmodPath>();
		finalPath->m_gmod = &gmod;
		finalPath->m_visVersion = gmod.visVersion();

		finalPath->m_parents.reserve( parsedNodes.size() );
		for ( const GmodNode& parsedNode : parsedNodes )
		{
			GmodNode* heapParent = new GmodNode( parsedNode );
			finalPath->m_parents.push_back( heapParent );
			finalPath->m_ownedNodes.push_back( heapParent );
		}

		GmodNode* heapEndNode = new GmodNode( tempEndNode );
		finalPath->m_node = heapEndNode;
		finalPath->m_ownedNodes.push_back( heapEndNode );

		if ( finalPath->m_node )
		{
			finalPath->m_visVersion = finalPath->m_node->visVersion();
		}

		SPDLOG_DEBUG( "GmodPath::parseFullPathInternal: Successfully parsed full path '{}'.", item );
		return std::make_unique<GmodParsePathResult::Ok>( std::move( *finalPath ) );
	}

	GmodPath::Enumerator::Enumerator( const GmodPath* pathInst, size_t startIndex )
		: m_pathInstance( pathInst )
	{
		SPDLOG_TRACE( "GmodPath::Enumerator constructor for path: {}, startIndex: {}", fmt::ptr( pathInst ), startIndex );
		if ( !m_pathInstance )
		{
			SPDLOG_WARN( "GmodPath::Enumerator constructor: m_pathInstance is null. Initializing m_currentIndex to 0 (as an end-like state for an empty range)." );
			m_currentIndex = 0;
			return;
		}

		m_currentIndex = startIndex;
		SPDLOG_DEBUG( "GmodPath::Enumerator constructed. m_pathInstance: {}, m_currentIndex: {}", fmt::ptr( m_pathInstance ), m_currentIndex );
	}

	bool GmodPath::Enumerator::operator!=( const GmodPath::Enumerator& other ) const
	{
		return m_pathInstance != other.m_pathInstance || m_currentIndex != other.m_currentIndex;
	}

	bool GmodPath::Enumerator::operator==( const GmodPath::Enumerator& other ) const
	{
		return m_pathInstance == other.m_pathInstance && m_currentIndex == other.m_currentIndex;
	}

	GmodPath::Enumerator::value_type GmodPath::Enumerator::operator*() const
	{
		SPDLOG_TRACE( "GmodPath::Enumerator::operator*() called. Current index: {}", m_currentIndex );
		if ( !m_pathInstance )
		{
			SPDLOG_ERROR( "GmodPath::Enumerator::operator*(): m_pathInstance is null." );
			throw std::logic_error( "Dereferencing an enumerator with a null path instance." );
		}
		if ( m_currentIndex >= m_pathInstance->length() )
		{
			SPDLOG_ERROR( "GmodPath::Enumerator::operator*(): Index {} is out of bounds (or at end) for path length {}.", m_currentIndex, m_pathInstance->length() );
			throw std::out_of_range( "Dereferencing GmodPath::Enumerator out of bounds or at end" );
		}
		GmodNode* node = ( *m_pathInstance )[m_currentIndex];
		SPDLOG_DEBUG( "GmodPath::Enumerator::operator*(): Returning pair {{ {}, {} (code: {}) }}",
			m_currentIndex, fmt::ptr( node ), ( node ? std::string( node->code() ) : "null" ) );
		return { m_currentIndex, node };
	}

	GmodPath::Enumerator& GmodPath::Enumerator::operator++()
	{
		SPDLOG_TRACE( "GmodPath::Enumerator::operator++() called. Current index before increment: {}", m_currentIndex );
		if ( m_pathInstance && m_currentIndex < m_pathInstance->length() )
		{
			m_currentIndex++;
			SPDLOG_DEBUG( "GmodPath::Enumerator::operator++(): Incremented index to {}.", m_currentIndex );
		}
		else if ( m_pathInstance )
		{
			SPDLOG_TRACE( "GmodPath::Enumerator::operator++(): At or past end (index {}, length {}). No change beyond length.", m_currentIndex, m_pathInstance->length() );
		}
		else
		{
			SPDLOG_WARN( "GmodPath::Enumerator::operator++(): m_pathInstance is null. No change." );
		}

		return *this;
	}

	GmodPath::Enumerator::Enumerator( const Enumerator& other )
		: m_pathInstance( other.m_pathInstance ), m_currentIndex( other.m_currentIndex )
	{
		SPDLOG_TRACE( "GmodPath::Enumerator copy constructor. Copied m_pathInstance: {}, m_currentIndex: {}", fmt::ptr( m_pathInstance ), m_currentIndex );
	}

	GmodPath::Enumerator& GmodPath::Enumerator::begin()
	{
		return *this;
	}

	GmodPath::Enumerator GmodPath::Enumerator::end() const
	{
		if ( m_pathInstance )
		{
			return Enumerator( m_pathInstance, m_pathInstance->length() );
		}
		return Enumerator( nullptr, 0 );
	}

	void GmodPath::Enumerator::reset()
	{
		SPDLOG_TRACE( "GmodPath::Enumerator::reset()" );
		m_currentIndex = static_cast<size_t>( -1 );
	}

	bool GmodPath::Enumerator::next()
	{
		SPDLOG_TRACE( "GmodPath::Enumerator::next() called. Current index: {}", m_currentIndex );
		if ( !m_pathInstance )
		{
			SPDLOG_WARN( "GmodPath::Enumerator::next(): m_pathInstance is null." );
			return false;
		}

		if ( m_currentIndex == std::numeric_limits<size_t>::max() )
		{
			if ( m_pathInstance->length() == 0 )
			{
				SPDLOG_TRACE( "GmodPath::Enumerator::next(): Path is empty." );
				return false;
			}
			m_currentIndex = 0;
		}
		else if ( m_currentIndex + 1 >= m_pathInstance->length() )
		{
			SPDLOG_TRACE( "GmodPath::Enumerator::next(): End of path reached or exceeded. Next index would be {}, path length {}.", m_currentIndex + 1, m_pathInstance->length() );
			return false;
		}
		else
		{
			m_currentIndex++;
		}
		SPDLOG_DEBUG( "GmodPath::Enumerator::next(): Advanced to index {}.", m_currentIndex );
		return true;
	}

	GmodNode* GmodPath::Enumerator::current() const
	{
		SPDLOG_TRACE( "GmodPath::Enumerator::current() called. Current index: {}", m_currentIndex );
		if ( !m_pathInstance )
		{
			SPDLOG_WARN( "GmodPath::Enumerator::current(): m_pathInstance is null." );
			return nullptr;
		}
		if ( m_currentIndex == static_cast<size_t>( -1 ) || m_currentIndex >= m_pathInstance->length() )
		{
			SPDLOG_WARN( "GmodPath::Enumerator::current(): Index {} is out of bounds or invalid for path length {}.", m_currentIndex, m_pathInstance->length() );

			return nullptr;
		}
		GmodNode* node = ( *m_pathInstance )[m_currentIndex];
		SPDLOG_DEBUG( "GmodPath::Enumerator::current(): Returning node at index {}: {} (code: {})",
			m_currentIndex, fmt::ptr( node ), ( node ? std::string( node->code() ) : "null" ) );
		return node;
	}

	GmodParsePathResult::Ok::Ok( GmodPath path )
		: GmodParsePathResult(), m_path( std::move( path ) )
	{
	}

	GmodParsePathResult::Err::Err( std::string errorString )
		: GmodParsePathResult(), error( std::move( errorString ) )
	{
	}

	GmodIndividualizableSet::GmodIndividualizableSet( const std::vector<int>& nodeIndices, const GmodPath& sourcePath )
		: m_nodeIndices( nodeIndices ), m_path( sourcePath )
	{
		SPDLOG_TRACE( "GmodIndividualizableSet constructor: Initializing with {} indices.", nodeIndices.size() );

		if ( m_nodeIndices.empty() )
		{
			SPDLOG_ERROR( "GmodIndividualizableSet constructor: nodeIndices cannot be empty." );
			throw std::invalid_argument( "GmodIndividualizableSet cant be empty" );
		}

		for ( int nodeIdx : m_nodeIndices )
		{
			if ( static_cast<size_t>( nodeIdx ) >= m_path.length() || nodeIdx < 0 )
			{
				SPDLOG_ERROR( "GmodIndividualizableSet constructor: Node index {} is out of bounds for path length {}.", nodeIdx, m_path.length() );
				throw std::out_of_range( fmt::format( "Node index {} is out of bounds.", nodeIdx ) );
			}
			GmodNode* currentNode = m_path[static_cast<size_t>( nodeIdx )];
			if ( !currentNode )
			{
				SPDLOG_ERROR( "GmodIndividualizableSet constructor: Node at index {} is null in the source path.", nodeIdx );
				throw std::runtime_error( fmt::format( "Node at index {} is null in source path.", nodeIdx ) );
			}

			bool isTargetNode = ( static_cast<size_t>( nodeIdx ) == m_path.length() - 1 );
			bool isInSet = ( m_nodeIndices.size() > 1 );
			if ( !currentNode->isIndividualizable( isTargetNode, isInSet ) )
			{
				SPDLOG_ERROR( "GmodIndividualizableSet constructor: Node '{}' (at index {}) is not individualizable in the given context.", currentNode->code().data(), nodeIdx );
				throw std::invalid_argument( "GmodIndividualizableSet nodes must be individualizable" );
			}
		}

		if ( m_nodeIndices.size() > 1 )
		{
			GmodNode* firstNode = m_path[static_cast<size_t>( m_nodeIndices[0] )];
			std::optional<Location> expectedLocation = firstNode->location();

			for ( size_t k = 1; k < m_nodeIndices.size(); ++k )
			{
				int currentIdx = m_nodeIndices[k];
				GmodNode* currentNode = m_path[static_cast<size_t>( currentIdx )];
				if ( currentNode->location() != expectedLocation )
				{
					SPDLOG_ERROR( "GmodIndividualizableSet constructor: Nodes have different locations. Node '{}' (at index {}) has location while first node in set had different or no location.",
						currentNode->code().data(), currentIdx );
					throw std::invalid_argument( "GmodIndividualizableSet nodes have different locations" );
				}
			}
		}

		bool foundPartOfShortPath = false;
		for ( int nodeIdx : m_nodeIndices )
		{
			GmodNode* currentNode = m_path[static_cast<size_t>( nodeIdx )];
			if ( currentNode == m_path.node() || currentNode->isLeafNode() )
			{
				foundPartOfShortPath = true;
				break;
			}
		}

		if ( !foundPartOfShortPath )
		{
			SPDLOG_ERROR( "GmodIndividualizableSet constructor: No nodes in the set are part of the short path (final node or leaf node)." );
			throw std::invalid_argument( "GmodIndividualizableSet has no nodes that are part of short path" );
		}
		SPDLOG_TRACE( "GmodIndividualizableSet constructor: Successfully initialized and validated." );
	}

	std::vector<GmodNode*> GmodIndividualizableSet::nodes() const
	{
		SPDLOG_TRACE( "GmodIndividualizableSet::nodes() called." );
		std::vector<GmodNode*> resultNodes;
		if ( m_nodeIndices.empty() )
		{
			SPDLOG_DEBUG( "GmodIndividualizableSet::nodes(): m_nodeIndices is empty, returning empty vector." );
			return resultNodes;
		}

		resultNodes.reserve( m_nodeIndices.size() );
		for ( int nodeIdx : m_nodeIndices )
		{
			if ( nodeIdx < 0 )
			{
				SPDLOG_WARN( "GmodIndividualizableSet::nodes(): Negative node index {} encountered, skipping.", nodeIdx );
				continue;
			}

			size_t currentIdx = static_cast<size_t>( nodeIdx );
			if ( currentIdx >= m_path.length() )
			{
				SPDLOG_ERROR( "GmodIndividualizableSet::nodes(): Node index {} is out of bounds for path length {}.", currentIdx, m_path.length() );
				throw std::out_of_range( fmt::format( "Node index {} is out of bounds for GmodPath (length {}) in GmodIndividualizableSet.", currentIdx, m_path.length() ) );
			}

			GmodNode* node = m_path[currentIdx];
			if ( !node )
			{
				SPDLOG_ERROR( "GmodIndividualizableSet::nodes(): Node at index {} (from nodeIdx {}) is null in m_path. Invalid state.", currentIdx, nodeIdx );
				throw std::runtime_error( fmt::format( "Node at index {} (from nodeIdx {}) is null in GmodPath for GmodIndividualizableSet.", currentIdx, nodeIdx ) );
			}
			resultNodes.push_back( node );

			SPDLOG_TRACE( "GmodIndividualizableSet::nodes(): Added node at index {} (ptr: {}) to result.", currentIdx, fmt::ptr( node ) );
		}
		SPDLOG_DEBUG( "GmodIndividualizableSet::nodes() returning vector with {} nodes.", resultNodes.size() );

		return resultNodes;
	}

	const std::vector<int>& GmodIndividualizableSet::nodeIndices() const noexcept
	{
		SPDLOG_TRACE( "GmodIndividualizableSet::nodeIndices() const noexcept. Returning m_nodeIndices (size: {}).", m_nodeIndices.size() );

		return m_nodeIndices;
	}

	void GmodIndividualizableSet::setLocation( const std::optional<Location>& location )
	{
		SPDLOG_TRACE( "GmodIndividualizableSet::setLocation called for {} nodes.", m_nodeIndices.size() );
		if ( m_nodeIndices.empty() )
		{
			SPDLOG_WARN( "GmodIndividualizableSet::setLocation: Node indices empty, cannot set location." );

			return;
		}

		for ( int nodeIdx : m_nodeIndices )
		{
			if ( nodeIdx < 0 || static_cast<size_t>( nodeIdx ) >= m_path.length() )
			{
				SPDLOG_ERROR( "GmodIndividualizableSet::setLocation: Node index {} is out of bounds for path length {}. Skipping this index.", nodeIdx, m_path.length() );

				continue;
			}

			GmodNode*& nodeInPath = m_path[static_cast<size_t>( nodeIdx )];
			GmodNode* oldNode = nodeInPath;

			if ( !oldNode )
			{
				SPDLOG_ERROR( "GmodIndividualizableSet::setLocation: Node at index {} in m_path is null. Cannot modify.", nodeIdx );

				continue;
			}

			SPDLOG_TRACE( "GmodIndividualizableSet::setLocation: Modifying node at index {} (original ptr: {}, code: {}).", nodeIdx, fmt::ptr( oldNode ), oldNode->code().data() );

			GmodNode newNode = location.has_value()
								   ? oldNode->tryWithLocation( location )
								   : oldNode->withoutLocation();

			GmodNode* newHeapNode = new GmodNode( newNode );

			nodeInPath = newHeapNode;
			SPDLOG_TRACE( "GmodIndividualizableSet::setLocation: Path at index {} now points to new node {} (code: {}).", nodeIdx, fmt::ptr( newHeapNode ), newHeapNode->code().data() );

			bool foundAndRemoved = false;
			auto& ownedNodesList = m_path.m_ownedNodes;

			for ( auto it = ownedNodesList.begin(); it != ownedNodesList.end(); ++it )
			{
				if ( *it == oldNode )
				{
					SPDLOG_TRACE( "GmodIndividualizableSet::setLocation: Removing old node {} from m_path's owned list and deleting.", fmt::ptr( oldNode ) );
					delete oldNode;
					ownedNodesList.erase( it );
					foundAndRemoved = true;
					break;
				}
			}
			if ( !foundAndRemoved )
			{
				SPDLOG_WARN( "GmodIndividualizableSet::setLocation: Old node {} was not found in m_path.m_ownedNodes. Potential issue if it was supposed to be owned.", fmt::ptr( oldNode ) );
			}
			ownedNodesList.push_back( newHeapNode );
			SPDLOG_TRACE( "GmodIndividualizableSet::setLocation: Added new node {} to m_path's owned list.", fmt::ptr( newHeapNode ) );
		}
	}

	std::optional<Location> GmodIndividualizableSet::location() const
	{
		SPDLOG_TRACE( "GmodIndividualizableSet::location() const called." );
		if ( m_nodeIndices.empty() )
		{
			SPDLOG_WARN( "GmodIndividualizableSet::location(): Node indices empty." );
			return std::nullopt;
		}

		int firstNodeIdx = m_nodeIndices[0];
		if ( firstNodeIdx < 0 || static_cast<size_t>( firstNodeIdx ) >= m_path.length() )
		{
			SPDLOG_ERROR( "GmodIndividualizableSet::location(): First node index {} is out of bounds for path length {}.", firstNodeIdx, m_path.length() );
			return std::nullopt;
		}

		const GmodNode* firstNode = m_path[static_cast<size_t>( firstNodeIdx )];
		if ( !firstNode )
		{
			SPDLOG_ERROR( "GmodIndividualizableSet::location(): First node (index {}) in m_path is null.", firstNodeIdx );
			return std::nullopt;
		}

		std::optional<Location> loc = firstNode->location();
		SPDLOG_DEBUG( "GmodIndividualizableSet::location(): Location of first node (index {}, code {}): {}.",
			firstNodeIdx, firstNode->code().data(), ( loc.has_value() ? "present" : "not present" ) );
		return loc;
	}

	std::string GmodIndividualizableSet::toString() const
	{
		SPDLOG_TRACE( "GmodIndividualizableSet::toString() const called." );
		std::stringstream ss;
		bool firstNodeAppended = false;

		for ( size_t j = 0; j < m_nodeIndices.size(); ++j )
		{
			int nodeIdx = m_nodeIndices[j];
			if ( nodeIdx < 0 || static_cast<size_t>( nodeIdx ) >= m_path.length() )
			{
				SPDLOG_ERROR( "GmodIndividualizableSet::toString(): Node index {} is out of bounds for path length {}. Skipping.", nodeIdx, m_path.length() );
				continue;
			}

			GmodNode* currentNode = m_path[static_cast<size_t>( nodeIdx )];
			if ( !currentNode )
			{
				SPDLOG_ERROR( "GmodIndividualizableSet::toString(): Node at index {} in m_path is null. Skipping.", nodeIdx );
				continue;
			}

			if ( currentNode->isLeafNode() || j == m_nodeIndices.size() - 1 )
			{
				if ( firstNodeAppended )
				{
					ss << '/';
				}
				currentNode->toString( ss );
				firstNodeAppended = true;
				SPDLOG_TRACE( "GmodIndividualizableSet::toString(): Appended node '{}' (index {}).", currentNode->code().data(), nodeIdx );
			}
		}
		std::string result = ss.str();
		SPDLOG_DEBUG( "GmodIndividualizableSet::toString() result: '{}'", result );
		return result;
	}
}
