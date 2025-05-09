#include "pch.h"

#include "dnv/vista/sdk/Gmod.h"

#include "dnv/vista/sdk/GmodDto.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/GmodPath.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr const char* NODE_CATEGORY_PRODUCT = "PRODUCT";
		static constexpr const char* NODE_CATEGORY_ASSET = "ASSET";
		static constexpr const char* NODE_CATEGORY_ASSET_FUNCTION = "ASSET FUNCTION";

		static constexpr const char* NODE_TYPE_SELECTION = "SELECTION";
		static constexpr const char* NODE_TYPE_VALUE_TYPE = "TYPE";

		static constexpr const char* NODE_CATEGORY_VALUE_FUNCTION = "FUNCTION";

		/** List of node types classified as leaf nodes */
		static const std::unordered_set<std::string> s_leafTypesSet = { "ASSET FUNCTION LEAF", "PRODUCT FUNCTION LEAF" };

		/** List of node types classified as function nodes */
		static const std::unordered_set<std::string> s_potentialParentScopeTypes = { "SELECTION", "GROUP", "LEAF" };
	}

	namespace
	{
		struct PathExistsContext
		{
			const GmodNode& m_to;
			std::vector<const GmodNode*> m_fromPath;
			std::vector<const GmodNode*> m_remainingParents;

			PathExistsContext( const GmodNode& toNode, const std::vector<const GmodNode*>& fp )
				: m_to{ toNode },
				  m_fromPath{ fp }
			{
				SPDLOG_INFO( "PathExistsContext created: Target='{}', FromPath size={}", m_to.code(), m_fromPath.size() );
			}

			PathExistsContext( const PathExistsContext& ) = delete;
			PathExistsContext( PathExistsContext&& ) noexcept = delete;
			PathExistsContext& operator=( const PathExistsContext& ) = delete;
			PathExistsContext& operator=( PathExistsContext&& ) = delete;
		};

		Gmod::TraversalHandlerResult pathExistsHandler(
			PathExistsContext& context,
			const std::vector<const GmodNode*>& currentPathFromTraversalStart,
			const GmodNode& currentNode )
		{
			SPDLOG_DEBUG( "pathExistsHandler: CurrentNode='{}', TargetNode='{}', PathFromTraversalStart size={}",
				currentNode.code(), context.m_to.code(), currentPathFromTraversalStart.size() );

			if ( currentNode.code() != context.m_to.code() )
			{
				SPDLOG_DEBUG( "pathExistsHandler: CurrentNode '{}' is not the target node '{}'. Continuing traversal.", currentNode.code(), context.m_to.code() );
				return Gmod::TraversalHandlerResult::Continue;
			}

			SPDLOG_INFO( "pathExistsHandler: Target node '{}' reached. Reconstructing full path.", currentNode.code() );

			std::vector<const GmodNode*> fullPathToCurrentNode = currentPathFromTraversalStart;

			if ( fullPathToCurrentNode.empty() )
			{
				SPDLOG_INFO( "pathExistsHandler: Initial path from traversal start is empty." );
				if ( currentNode.isRoot() )
				{
					fullPathToCurrentNode.push_back( &currentNode );
					SPDLOG_INFO( "pathExistsHandler: Current node is GMOD root and initial path was empty. Full path set to just root: '{}'.", currentNode.code() );
				}
				else
				{
					SPDLOG_INFO( "pathExistsHandler: Initial path to non-root current node '{}' is empty. Full path remains empty if not root, or contains only root.", currentNode.code() );
					if ( !currentNode.isRoot() )
					{
					}
					else
					{
					}
				}
			}
			else if ( !fullPathToCurrentNode.front()->isRoot() )
			{
				SPDLOG_DEBUG( "pathExistsHandler: Path (head='{}', size={}) does not begin at GMOD root. Prepending path to root.",
					fullPathToCurrentNode.front()->code(), fullPathToCurrentNode.size() );

				std::vector<const GmodNode*> prependedPathSegment;
				const GmodNode* ascender = fullPathToCurrentNode.front();
				std::unordered_set<const GmodNode*> visitedInAscent;

				while ( ascender != nullptr && !ascender->isRoot() )
				{
					if ( !visitedInAscent.insert( ascender ).second )
					{
						SPDLOG_WARN( "pathExistsHandler: Cycle detected at node '{}' during upward path reconstruction to GMOD root. Aborting path validation.", ascender->code() );
						return Gmod::TraversalHandlerResult::Continue;
					}

					const auto& ascenderParents = ascender->parents();
					if ( ascenderParents.size() != 1 )
					{
						std::string errMsg = fmt::format( "Invalid state - node '{}' has {} parents (expected 1 for unambiguous path to GMOD root).", ascender->code(), ascenderParents.size() );
						SPDLOG_ERROR( "pathExistsHandler: {}", errMsg );
						throw std::runtime_error( errMsg );
					}

					const GmodNode* actualParent = ascenderParents.front();
					if ( !actualParent )
					{
						std::string errMsg = fmt::format( "Null parent found for node '{}' during upward path reconstruction to GMOD root.", ascender->code() );
						SPDLOG_ERROR( "pathExistsHandler: {}", errMsg );
						throw std::runtime_error( errMsg );
					}
					prependedPathSegment.insert( prependedPathSegment.begin(), actualParent );
					SPDLOG_INFO( "pathExistsHandler: Prepended '{}' to path segment during ascent.", actualParent->code() );
					ascender = actualParent;
				}

				if ( ascender == nullptr && ( prependedPathSegment.empty() || !prependedPathSegment.front()->isRoot() ) )
				{
					SPDLOG_WARN( "pathExistsHandler: Ascent to GMOD root failed to complete (ascender is null or prepended path does not start with root). Path may be detached from root." );
					return Gmod::TraversalHandlerResult::Continue;
				}

				fullPathToCurrentNode.insert( fullPathToCurrentNode.begin(), prependedPathSegment.begin(), prependedPathSegment.end() );
				if ( !fullPathToCurrentNode.empty() )
				{
					SPDLOG_INFO( "pathExistsHandler: Path reconstruction complete after prepending. Full path size: {}. Head: '{}'", fullPathToCurrentNode.size(), fullPathToCurrentNode.front()->code() );
				}
				else
				{
					SPDLOG_WARN( "pathExistsHandler: Path became empty after prepending attempt. This is unexpected." );
					return Gmod::TraversalHandlerResult::Continue;
				}
			}
			else
			{
				SPDLOG_INFO( "pathExistsHandler: Path (head='{}', size={}) already begins at GMOD root. No prepending needed.",
					fullPathToCurrentNode.front()->code(), fullPathToCurrentNode.size() );
			}

			SPDLOG_INFO( "pathExistsHandler: Validating reconstructed path (size {}) against FromPath (size {}).", fullPathToCurrentNode.size(), context.m_fromPath.size() );

			if ( fullPathToCurrentNode.size() < context.m_fromPath.size() )
			{
				SPDLOG_INFO( "pathExistsHandler: Reconstructed path is shorter than FromPath. No match." );
				return Gmod::TraversalHandlerResult::Continue;
			}

			bool match = true;
			for ( size_t i = 0; i < context.m_fromPath.size(); ++i )
			{
				const GmodNode* fromPathNode = context.m_fromPath[i];
				const GmodNode* actualPathNode = ( i < fullPathToCurrentNode.size() ) ? fullPathToCurrentNode[i] : nullptr;

				if ( !fromPathNode || !actualPathNode )
				{
					SPDLOG_WARN( "pathExistsHandler: Null node or out-of-bounds access during path comparison. FromPath[{}]={}, ActualPath[{}]={}. No match.",
						i, (void*)fromPathNode, i, (void*)actualPathNode );
					match = false;
					break;
				}
				SPDLOG_INFO( "pathExistsHandler: Comparing FromPath[{}] ('{}') with ActualPath[{}] ('{}')",
					i, fromPathNode->code(), i, actualPathNode->code() );

				if ( fromPathNode->code() != actualPathNode->code() )
				{
					SPDLOG_INFO( "pathExistsHandler: Path mismatch at index {}: FromPath='{}', ActualPath='{}'. No match.", i, fromPathNode->code(), actualPathNode->code() );
					match = false;
					break;
				}
			}

			if ( match )
			{
				SPDLOG_INFO( "pathExistsHandler: Path prefix match successful for target '{}'. Populating remaining parents.", currentNode.code() );
				context.m_remainingParents.clear();

				std::unordered_set<std::string> fromPathNodeCodes;
				for ( const auto* fpNode : context.m_fromPath )
				{
					if ( fpNode )
						fromPathNodeCodes.insert( fpNode->code() );
				}

				for ( const auto* pNode : fullPathToCurrentNode )
				{
					if ( pNode && fromPathNodeCodes.find( pNode->code() ) == fromPathNodeCodes.end() )
					{
						context.m_remainingParents.push_back( pNode );
						SPDLOG_INFO( "pathExistsHandler: Added '{}' to remaining parents.", pNode->code() );
					}
				}

				SPDLOG_INFO( "pathExistsHandler: Remaining parents populated ({} nodes). Stopping traversal.", context.m_remainingParents.size() );
				return Gmod::TraversalHandlerResult::Stop;
			}
			else
			{
				SPDLOG_INFO( "pathExistsHandler: Path prefix did not match. Continuing traversal." );
				return Gmod::TraversalHandlerResult::Continue;
			}
		}
	}

	//-------------------------------------------------------------------
	// Parents Implementation
	//-------------------------------------------------------------------

	void Gmod::Parents::push( const GmodNode* parent )
	{
		if ( !parent )
		{
			return;
		}

		m_nodes.push_back( parent );
		const std::string& code = parent->code();
		auto it = m_occurrences.find( code );
		if ( it != m_occurrences.end() )
		{
			it->second++;
			SPDLOG_TRACE( "Pushed parent: '{}', new occurrence count: {}", code, it->second );
		}
		else
		{
			m_occurrences.emplace( code, 1 );
			SPDLOG_TRACE( "Pushed parent: '{}', new occurrence count: 1", code );
		}
	}

	void Gmod::Parents::pop()
	{
		if ( m_nodes.empty() )
		{
			return;
		}
		const GmodNode* parent = m_nodes.back();
		const std::string& code = parent->code();
		auto it = m_occurrences.find( code );
		if ( it != m_occurrences.end() )
		{
			it->second--;
			SPDLOG_TRACE( "Popped parent: '{}', new occurrence count: {}", code, it->second );
			if ( it->second <= 0 )
			{
				m_occurrences.erase( it );
				SPDLOG_TRACE( "Removed '{}' from occurrence map.", code );
			}
		}
		else
		{
			SPDLOG_WARN( "Popped parent '{}' which was not found in occurrence map.", code );
		}
		m_nodes.pop_back();
	}

	int Gmod::Parents::occurrences( const GmodNode& node ) const
	{
		auto it = m_occurrences.find( node.code() );
		int count = ( it != m_occurrences.end() ) ? it->second : 0;
		SPDLOG_TRACE( "Occurrences check for node '{}': {}", node.code(), count );
		return count;
	}

	const GmodNode* Gmod::Parents::lastOrDefault() const
	{
		return m_nodes.empty() ? nullptr : m_nodes.back();
	}

	const std::vector<const GmodNode*>& Gmod::Parents::nodePointers() const
	{
		return m_nodes;
	}

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	Gmod::Gmod( VisVersion version, const GmodDto& dto )
		: m_visVersion{ version },
		  m_rootNode{}
	{
		SPDLOG_INFO( "Gmod constructor: Starting for VIS version {}", dto.visVersion() );

		std::vector<std::pair<std::string, GmodNode>> nodePairs;
		nodePairs.reserve( dto.items().size() );
		for ( const auto& nodeDto : dto.items() )
		{
			nodePairs.emplace_back( nodeDto.code(), GmodNode( version, nodeDto ) );
		}
		SPDLOG_INFO( "Gmod constructor: Created {} initial GmodNode objects for VIS version {}.", nodePairs.size(), VisVersionExtensions::toVersionString( version ) );

		size_t numItemsForChd = nodePairs.size();

		m_nodeMap = ChdDictionary<GmodNode>( std::move( nodePairs ) );
		SPDLOG_INFO( "Gmod constructor: ChdDictionary m_nodeMap constructed with an expected {} items.", numItemsForChd );

		if ( ( m_nodeMap.begin() == m_nodeMap.end() ) && !dto.items().empty() )
		{
			SPDLOG_ERROR( "Gmod constructor: m_nodeMap is empty after construction despite non-empty DTO items for VIS version {}. Aborting further GMOD initialization.", VisVersionExtensions::toVersionString( version ) );
			return;
		}

		SPDLOG_INFO( "Gmod constructor: Starting pointer fix-up for children and parents within m_nodeMap..." );
		for ( const auto& relation : dto.relations() )
		{
			if ( relation.size() >= 2 )
			{
				const std::string& parentCode = relation[0];
				const std::string& childCode = relation[1];

				try
				{
					GmodNode& parentNode = m_nodeMap[parentCode];
					try
					{
						GmodNode& childNode = m_nodeMap[childCode];

						parentNode.addChild( &childNode );
						childNode.addParent( &parentNode );
					}
					catch ( [[maybe_unused]] const std::out_of_range& oorChild )
					{
						SPDLOG_WARN( "Gmod constructor (linking): Child node '{}' (for parent '{}') not found in m_nodeMap. Relation skipped. Error: {}", childCode, parentCode, oorChild.what() );
					}
					catch ( [[maybe_unused]] const std::exception& exChildOrLink )
					{
						SPDLOG_ERROR( "Gmod constructor (linking): Exception while processing child '{}' or linking for parent '{}'. Error: {}", childCode, parentCode, exChildOrLink.what() );
					}
				}
				catch ( [[maybe_unused]] const std::out_of_range& oorParent )
				{
					SPDLOG_WARN( "Gmod constructor (linking): Parent node '{}' (for child '{}') not found in m_nodeMap. Relation skipped. Error: {}", parentCode, childCode, oorParent.what() );
				}
				catch ( [[maybe_unused]] const std::exception& exParent )
				{
					SPDLOG_ERROR( "Gmod constructor (linking): Exception while processing parent '{}' for relation with child '{}'. Error: {}", parentCode, childCode, exParent.what() );
				}
			}
			else
			{
				SPDLOG_WARN( "Gmod constructor (linking): Relation with insufficient size ({}) encountered. Skipping.", relation.size() );
			}
		}
		SPDLOG_INFO( "Gmod constructor: Pointer fix-up for children and parents complete." );

		SPDLOG_INFO( "Gmod constructor: Trimming nodes in m_nodeMap..." );
		if ( !m_nodeMap.isEmpty() )
		{
			std::vector<std::string> nodeCodesToTrim;
			nodeCodesToTrim.reserve( numItemsForChd );
			for ( Gmod::Iterator it = this->begin(); it != this->end(); ++it )
			{
				nodeCodesToTrim.push_back( it->code() );
			}

			for ( const std::string& code : nodeCodesToTrim )
			{
				try
				{
					GmodNode& nodeToTrim = m_nodeMap[code];
					nodeToTrim.trim();
				}
				catch ( [[maybe_unused]] const std::out_of_range& oor )
				{
					SPDLOG_ERROR( "Gmod constructor (trimming): Node '{}' found during iteration but not in m_nodeMap.at() during trim. Error: {}", code, oor.what() );
				}
				catch ( [[maybe_unused]] const std::exception& ex )
				{
					SPDLOG_ERROR( "Gmod constructor (trimming): Exception while trimming node '{}'. Error: {}", code, ex.what() );
				}
			}
		}
		SPDLOG_INFO( "Gmod constructor: Trimming nodes complete." );

		SPDLOG_INFO( "Gmod constructor: Initializing m_rootNode..." );
		try
		{
			m_rootNode = m_nodeMap["VE"];

			SPDLOG_INFO( "Gmod constructor: m_rootNode initialized from m_nodeMap, code: '{}'", m_rootNode.code() );
		}
		catch ( [[maybe_unused]] const std::out_of_range& oor )
		{
			SPDLOG_ERROR( "Gmod constructor: Root node 'VE' not found in m_nodeMap for VIS version {}. GMOD is likely invalid. Error: {}", VisVersionExtensions::toVersionString( version ), oor.what() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Gmod constructor: Exception while initializing m_rootNode from m_nodeMap for VIS version {}. Error: {}", VisVersionExtensions::toVersionString( version ), ex.what() );
		}

		SPDLOG_INFO( "Gmod constructor: Successfully created Gmod for VIS version {}. Node count in map: {}. Root node code: '{}'",
			VisVersionExtensions::toVersionString( version ), m_nodeMap.size(), m_rootNode.code().empty() ? "UNINITIALIZED OR MISSING" : m_rootNode.code() );
	}

	Gmod::Gmod( VisVersion version, const std::unordered_map<std::string, GmodNode>& nodeMap )
		: m_visVersion{ version },
		  m_rootNode{ [&nodeMap]() -> GmodNode {
			  auto it = nodeMap.find( "VE" );
			  if ( it != nodeMap.end() )
			  {
				  return GmodNode( it->second, true );
			  }
			  SPDLOG_WARN( "Root node 'VE' not found in provided nodeMap for Gmod constructor." );
			  return GmodNode();
		  }() },
		  m_nodeMap{ [&nodeMap]() {
			  std::vector<std::pair<std::string, GmodNode>> pairs;
			  pairs.reserve( nodeMap.size() );
			  for ( const auto& [code, node] : nodeMap )
			  {
				  pairs.emplace_back( code, GmodNode( node, true ) );
			  }
			  return ChdDictionary<GmodNode>( std::move( pairs ) );
		  }() }
	{
		SPDLOG_INFO( "Creating Gmod from existing map with {} nodes.", nodeMap.size() );

		if ( m_rootNode.code() != "VE" && nodeMap.count( "VE" ) )
		{
			SPDLOG_ERROR( "Failed to correctly initialize root node from provided map." );
		}
		if ( ( m_nodeMap.begin() == m_nodeMap.end() ) && !nodeMap.empty() )
		{
			SPDLOG_ERROR( "Failed to initialize node dictionary from provided map." );
		}
	}

	//-------------------------------------------------------------------
	// Basic Access Methods
	//-------------------------------------------------------------------

	const GmodNode& Gmod::operator[]( const std::string& key ) const
	{
		const GmodNode* nodePtr = nullptr;
		if ( tryGetNode( key, nodePtr ) && nodePtr != nullptr )
		{
			return *nodePtr;
		}

		throw std::out_of_range( "Node with key '" + key + "' not found in GMOD dictionary." );
	}

	VisVersion Gmod::visVersion() const
	{
		return m_visVersion;
	}

	const GmodNode& Gmod::rootNode() const
	{
		SPDLOG_INFO( "Getting root node: '{}'", m_rootNode.code() );
		return m_rootNode;
	}

	bool Gmod::tryGetNode( std::string_view code, const GmodNode*& outNodePtr ) const
	{
		outNodePtr = nullptr;
		try
		{
			if ( code.empty() )
			{
				SPDLOG_WARN( "TryGetNode: Attempted to look up empty node code" );
				return false;
			}

			if ( !m_nodeMap.tryGetValue( code, outNodePtr ) )
			{
				SPDLOG_WARN( "TryGetNode: Node '{}' not found in GMOD", code );
				return false;
			}

			if ( outNodePtr == nullptr )
			{
				SPDLOG_ERROR( "TryGetNode: m_nodeMap.tryGetValue succeeded but outNodePtr is null for code '{}'", code );
				return false;
			}

			SPDLOG_TRACE( "TryGetNode: Node '{}' found in GMOD", code );
			return true;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception in TryGetNode for '{}': {}", code, ex.what() );
			outNodePtr = nullptr;
			return false;
		}
	}

	bool Gmod::isEmpty() const
	{
		return ( m_nodeMap.begin() == m_nodeMap.end() );
	}

	bool Gmod::isPotentialParent( const std::string& type )
	{
		return s_potentialParentScopeTypes.find( type ) != s_potentialParentScopeTypes.end();
	}

	//-------------------------------------------------------------------
	// Path Parsing Methods
	//-------------------------------------------------------------------

	GmodPath Gmod::parsePath( std::string_view item ) const
	{
		return GmodPath::parse( item, m_visVersion );
	}

	bool Gmod::tryParsePath( std::string_view item, GmodPath& path ) const
	{
		SPDLOG_INFO( "TryParsePath: Attempting to parse path: {}", item );

		return GmodPath::tryParse( item, m_visVersion, path );
	}

	GmodPath Gmod::parseFromFullPath( std::string_view item ) const
	{
		return GmodPath::parseFullPath( item, m_visVersion );
	}

	bool Gmod::tryParseFromFullPath( std::string_view item, std::optional<GmodPath>& path ) const
	{
		GmodPath tempPath;

		if ( GmodPath::tryParseFullPath( item, m_visVersion, tempPath ) )
		{
			path = std::move( tempPath );
			return true;
		}

		return false;
	}

	//-------------------------------------------------------------------
	// Traversal Methods
	//-------------------------------------------------------------------

	bool Gmod::traverse( const TraverseHandler& handler, const TraversalOptions& options ) const
	{
		return traverse( rootNode(), handler, options );
	}

	bool Gmod::traverse( const GmodNode& rootNode, const TraverseHandler& handler, const TraversalOptions& options ) const
	{
		TraversalContext context( handler, options.maxTraversalOccurrence, options.maxNodes );
		SPDLOG_INFO( "Starting stateless traversal from node: {}", rootNode.code() );

		TraversalHandlerResult result = traverseNode( context, rootNode );
		SPDLOG_INFO( "Stateless traversal finished with result: {}", static_cast<int>( result ) );

		return result == TraversalHandlerResult::Continue;
	}

	Gmod::TraversalHandlerResult Gmod::traverseNode( TraversalContext& context, const GmodNode& node ) const
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
					SPDLOG_INFO( "Skipping subtree for node '{}': Occurrence limit ({}) met.", code, occurrences );
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
				SPDLOG_INFO( "Skipping occurrence check for node '{}' due to ProductSelectionAssignment.", code );
			}

			SPDLOG_INFO( "Visiting node: '{}' (Occurrence: {})", code, occurrences + 1 );

			context.parents.push( &node );

			TraversalHandlerResult result = context.handler( context.parents.nodePointers(), node );

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
			SPDLOG_ERROR( "Exception during stateless node traversal for node '{}': {}", node.code(), ex.what() );
			if ( !context.parents.nodePointers().empty() && context.parents.lastOrDefault() == &node )
			{
				context.parents.pop();
			}
			throw;
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "Unknown exception during stateless node traversal for node '{}'", node.code() );
			if ( !context.parents.nodePointers().empty() && context.parents.lastOrDefault() == &node )
			{
				context.parents.pop();
			}
			throw;
		}
	}

	bool Gmod::pathExistsBetween( const std::vector<const GmodNode*>& fromPath,
		const GmodNode& to,
		std::vector<const GmodNode*>& remainingParents ) const
	{
		const GmodNode* lastAssetFunction = nullptr;
		if ( !fromPath.empty() )
		{
			for ( auto it = fromPath.rbegin(); it != fromPath.rend(); ++it )
			{
				const GmodNode* n = *it;
				if ( n && Gmod::isAssetFunctionNode( n->metadata() ) )
				{
					const GmodNode* tempNodePtr = nullptr;
					if ( this->tryGetNode( n->code(), tempNodePtr ) && tempNodePtr == n )
					{
						lastAssetFunction = n;
						break;
					}
				}
			}
		}

		remainingParents.clear();

		PathExistsContext state( to, fromPath );
		const GmodNode* traversalStartNode = ( lastAssetFunction != nullptr ) ? lastAssetFunction : &this->rootNode();

		Gmod::TraversalOptions options;
		std::function<Gmod::TraversalHandlerResult( PathExistsContext&, const std::vector<const GmodNode*>&, const GmodNode& )> handler_func = &pathExistsHandler;

		bool reachedEnd = this->traverse(
			state,
			*traversalStartNode,
			handler_func,
			options );

		remainingParents = state.m_remainingParents;

		return !reachedEnd;
	}

	//-------------------------------------------------------------------
	// Iterator Methods
	//-------------------------------------------------------------------

	Gmod::Iterator::Iterator( ChdDictionary<GmodNode>::Iterator innerIt )
		: m_innerIt( std::move( innerIt ) ) {}

	Gmod::Iterator::reference Gmod::Iterator::operator*() const
	{
		return m_innerIt->second;
	}

	Gmod::Iterator::pointer Gmod::Iterator::operator->() const
	{
		return &m_innerIt->second;
	}

	Gmod::Iterator& Gmod::Iterator::operator++()
	{
		++m_innerIt;
		return *this;
	}

	Gmod::Iterator Gmod::Iterator::operator++( int )
	{
		Iterator temp = *this;
		++( *this );
		return temp;
	}

	bool Gmod::Iterator::operator==( const Iterator& other ) const
	{
		return m_innerIt == other.m_innerIt;
	}

	bool Gmod::Iterator::operator!=( const Iterator& other ) const
	{
		return !( *this == other );
	}

	Gmod::Iterator Gmod::begin() const
	{
		return Iterator( m_nodeMap.begin() );
	}

	Gmod::Iterator Gmod::end() const
	{
		return Iterator( m_nodeMap.end() );
	}

	Gmod::Iterator Gmod::begin()
	{
		return Iterator( m_nodeMap.begin() );
	}

	Gmod::Iterator Gmod::end()
	{
		return Iterator( m_nodeMap.end() );
	}

	//-------------------------------------------------------------------
	// Static Node Classification Methods
	//-------------------------------------------------------------------

	bool Gmod::isLeafNode( const std::string& fullType )
	{
		return s_leafTypesSet.find( fullType ) != s_leafTypesSet.end();
	}

	bool Gmod::isLeafNode( const GmodNodeMetadata& metadata )
	{
		return isLeafNode( metadata.fullType() );
	}

	bool Gmod::isFunctionNode( const std::string& category )
	{
		return category != NODE_CATEGORY_PRODUCT && category != NODE_CATEGORY_ASSET;
	}

	bool Gmod::isFunctionNode( const GmodNodeMetadata& metadata )
	{
		return isFunctionNode( metadata.category() );
	}

	bool Gmod::isProductSelection( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == NODE_CATEGORY_PRODUCT && metadata.type() == NODE_TYPE_SELECTION;
	}

	bool Gmod::isProductType( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == NODE_CATEGORY_PRODUCT && metadata.type() == NODE_TYPE_VALUE_TYPE;
	}

	bool Gmod::isAsset( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == NODE_CATEGORY_ASSET;
	}

	bool Gmod::isAssetFunctionNode( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == NODE_CATEGORY_ASSET_FUNCTION;
	}

	//-------------------------------------------------------------------
	// Static Relationship Classification Methods
	//-------------------------------------------------------------------

	bool Gmod::isProductTypeAssignment( const GmodNode* parent, const GmodNode* child )
	{
		if ( parent == nullptr || child == nullptr )
		{
			return false;
		}

		if ( parent->metadata().category().find( NODE_CATEGORY_VALUE_FUNCTION ) == std::string::npos )
		{
			return false;
		}

		return child->metadata().category() == NODE_CATEGORY_PRODUCT && child->metadata().type() == NODE_TYPE_VALUE_TYPE;
	}

	bool Gmod::isProductSelectionAssignment( const GmodNode* parent, const GmodNode* child )
	{
		if ( parent == nullptr || child == nullptr )
		{
			return false;
		}
		if ( parent->metadata().category().find( NODE_CATEGORY_VALUE_FUNCTION ) == std::string::npos )
		{
			return false;
		}
		return child->metadata().category().find( NODE_CATEGORY_PRODUCT ) != std::string::npos &&
			   child->metadata().type() == NODE_TYPE_SELECTION;
	}

	//-------------------------------------------------------------------
	// Gmod::TraversalContext Implementation
	//-------------------------------------------------------------------

	Gmod::TraversalContext::TraversalContext( const TraverseHandler& h, int maxOcc, size_t maxN )
		: handler( h ), maxTraversalOccurrence( maxOcc ), maxNodes( maxN )
	{
	}

	Gmod::TraversalOptions::TraversalOptions()
		: maxTraversalOccurrence( DEFAULT_MAX_TRAVERSAL_OCCURRENCE )
	{
	}

	Gmod::TraversalOptions::TraversalOptions( int maxTraversalOccurrence )
		: maxTraversalOccurrence( maxTraversalOccurrence )
	{
	}
}
