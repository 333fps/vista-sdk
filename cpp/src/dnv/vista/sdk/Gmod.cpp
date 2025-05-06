#include "pch.h"

#include "dnv/vista/sdk/Gmod.h"

#include "dnv/vista/sdk/GmodDto.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/GmodPath.h"

namespace dnv::vista::sdk
{
	const std::unordered_set<std::string> Gmod::s_leafTypesSet = { "ASSET FUNCTION LEAF", "PRODUCT FUNCTION LEAF" };
	const std::unordered_set<std::string> Gmod::s_potentialParentScopeTypes = { "SELECTION", "GROUP", "LEAF" };
}

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Parents Implementation
	//-------------------------------------------------------------------

	void Parents::push( const GmodNode* parent )
	{
		if ( !parent )
			return;
		m_nodes.push_back( std::move( parent ) );
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

	void Parents::pop()
	{
		if ( m_nodes.empty() )
			return;
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

	int Parents::occurrences( const GmodNode& node ) const
	{
		auto it = m_occurrences.find( node.code() );
		int count = ( it != m_occurrences.end() ) ? it->second : 0;
		SPDLOG_TRACE( "Occurrences check for node '{}': {}", node.code(), count );
		return count;
	}

	const GmodNode* Parents::lastOrDefault() const
	{
		return m_nodes.empty() ? nullptr : m_nodes.back();
	}

	const std::vector<const GmodNode*>& Parents::nodePointers() const
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
		std::unordered_map<std::string, GmodNode> tempNodeMap;
		tempNodeMap.reserve( dto.items().size() );

		for ( const auto& nodeDto : dto.items() )
		{
			tempNodeMap.emplace( std::piecewise_construct,
				std::forward_as_tuple( nodeDto.code() ),
				std::forward_as_tuple( version, nodeDto ) );
		}

		for ( const auto& relation : dto.relations() )
		{
			if ( relation.size() >= 2 )
			{
				const std::string& parentCode = relation[0];
				const std::string& childCode = relation[1];

				auto parentIt = tempNodeMap.find( parentCode );
				auto childIt = tempNodeMap.find( childCode );

				if ( parentIt != tempNodeMap.end() && childIt != tempNodeMap.end() )
				{
					parentIt->second.addChild( &childIt->second );
					childIt->second.addParent( &parentIt->second );
				}
				else
				{
					if ( parentIt == tempNodeMap.end() )
						SPDLOG_WARN( "Relation skipped: Parent node '{}' not found.", parentCode );
					if ( childIt == tempNodeMap.end() )
						SPDLOG_WARN( "Relation skipped: Child node '{}' not found.", childCode );
				}
			}
		}

		for ( auto& [code, node] : tempNodeMap )
		{
			node.trim();
		}

		auto rootIt = tempNodeMap.find( "VE" );
		if ( rootIt != tempNodeMap.end() )
		{
			m_rootNode = GmodNode( rootIt->second, true );
		}
		else
		{
			SPDLOG_WARN( "Root node 'VE' not found in GMOD data." );
		}

		std::vector<std::pair<std::string, GmodNode>> nodePairs;
		nodePairs.reserve( tempNodeMap.size() );
		for ( auto& [code, node] : tempNodeMap )
		{
			nodePairs.emplace_back( code, std::move( node ) );
		}

		m_nodeMap = ChdDictionary<GmodNode>( std::move( nodePairs ) );

		[[maybe_unused]] size_t dictionarySize = 0;
		if ( m_nodeMap.begin() != m_nodeMap.end() )
		{
			dictionarySize = dto.items().size();
		}

		SPDLOG_INFO( "Creating Gmod with {} items - dictionary has approx {} entries", dto.items().size(), dictionarySize );

		if ( ( m_nodeMap.begin() == m_nodeMap.end() ) && !dto.items().empty() )
		{
			SPDLOG_ERROR( "Failed to initialize node dictionary in constructor!" );
		}
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

	Gmod::Gmod( Gmod&& other ) noexcept
		: m_visVersion( other.m_visVersion ), m_rootNode( std::move( other.m_rootNode ) ), m_nodeMap( std::move( other.m_nodeMap ) )
	{
		other.m_visVersion = VisVersion::Unknown;
	}

	Gmod::~Gmod()
	{
	}

	Gmod& Gmod::operator=( Gmod&& other ) noexcept
	{
		SPDLOG_DEBUG( "Gmod move assignment." );
		if ( this != &other )
		{
			m_visVersion = other.m_visVersion;
			m_rootNode = std::move( other.m_rootNode );
			m_nodeMap = std::move( other.m_nodeMap );
		}
		return *this;
	}

	//-------------------------------------------------------------------
	// Basic Access Methods
	//-------------------------------------------------------------------

	const GmodNode& Gmod::operator[]( const std::string& key ) const
	{
		static GmodNode nullNode;
		const GmodNode* nodePtr = nullptr;
		if ( tryGetNode( key, nodePtr ) && nodePtr != nullptr )
		{
			return *nodePtr;
		}
		SPDLOG_WARN( "Node with key '{}' not found in GMOD dictionary via operator[]", key );

		return nullNode;
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

	GmodPath Gmod::parsePath( const std::string& item ) const
	{
		return GmodPath::parse( item, m_visVersion );
	}

	bool Gmod::tryParsePath( const std::string& item, GmodPath& path ) const
	{
		SPDLOG_INFO( "TryParsePath: Attempting to parse path: {}", item );
		return GmodPath::tryParse( item, m_visVersion, path );
	}

	GmodPath Gmod::parseFromFullPath( const std::string& item ) const
	{
		return GmodPath::parseFullPath( item, m_visVersion );
	}

	bool Gmod::tryParseFromFullPath( const std::string& item, std::optional<GmodPath>& path ) const
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
		TraversalHandlerResult result = traverseNode( context, rootNode );

		SPDLOG_INFO( "Traversal completed with result: {}", static_cast<int>( result ) );
		return result != TraversalHandlerResult::Stop;
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
			SPDLOG_TRACE( "Skipping node '{}' and its subtree due to InstallSubstructure=false", node.code() );
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
					SPDLOG_DEBUG( "Skipping subtree for node '{}': Occurrence limit ({}) met.", code, occurrences );
					return TraversalHandlerResult::SkipSubtree;
				}
				else if ( occurrences > context.maxTraversalOccurrence )
				{
					SPDLOG_ERROR( "Traversal stopped: Occurrence limit ({}) exceeded for node '{}' ({} occurrences). Potential cycle or logic error.",
						context.maxTraversalOccurrence, code, occurrences );
					return TraversalHandlerResult::Stop;
				}
			}
			else
			{
				SPDLOG_TRACE( "Skipping occurrence check for node '{}' due to ProductSelectionAssignment.", code );
			}

			SPDLOG_TRACE( "Visiting node: '{}' (Occurrence: {})", code, occurrences + 1 );

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
			context.parents.pop();
			return TraversalHandlerResult::Stop;
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "Unknown exception during stateless node traversal for node '{}'", node.code() );
			context.parents.pop();
			return TraversalHandlerResult::Stop;
		}
	}

	bool Gmod::pathExistsBetween( const std::vector<const GmodNode*>& parentsPrefix,
		const GmodNode& target, std::vector<const GmodNode*>& remainingParents ) const
	{
		remainingParents.clear();
		SPDLOG_DEBUG( "Checking path existence to target node '{}' starting from provided prefix.", target.code() );

		const GmodNode* targetNodePtr = nullptr;
		if ( !tryGetNode( target.code(), targetNodePtr ) || targetNodePtr == nullptr )
		{
			SPDLOG_WARN( "pathExistsBetween: Target node '{}' not found in current Gmod.", target.code() );
			return false;
		}

		std::vector<const GmodNode*> fullPathToRootPtrs;
		const GmodNode* currentNodePtr = targetNodePtr;
		bool foundRoot = false;
		std::unordered_set<const GmodNode*> visited;

		try
		{
			while ( currentNodePtr != nullptr )
			{
				if ( !visited.insert( currentNodePtr ).second )
				{
					SPDLOG_ERROR( "pathExistsBetween: Cycle detected while walking up from '{}'. Node '{}' encountered again.", target.code(), currentNodePtr->code() );
					return false;
				}

				fullPathToRootPtrs.insert( fullPathToRootPtrs.begin(), currentNodePtr );

				if ( currentNodePtr->isRoot() )
				{
					foundRoot = true;
					break;
				}

				const auto& currentParents = currentNodePtr->parents();
				if ( currentParents.empty() )
				{
					SPDLOG_WARN( "pathExistsBetween: Node '{}' has no parents before reaching root.", currentNodePtr->code() );
					return false;
				}
				if ( currentParents.size() > 1 )
				{
					SPDLOG_WARN( "pathExistsBetween: Node '{}' has multiple parents ({}). Cannot determine unique path to root.",
						currentNodePtr->code(), currentParents.size() );
					return false;
				}

				currentNodePtr = currentParents.front();
			}
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "pathExistsBetween: Exception while walking up parent chain from '{}': {}", target.code(), ex.what() );

			return false;
		}

		if ( !foundRoot )
		{
			SPDLOG_ERROR( "pathExistsBetween: Failed to find root node 'VE' when walking up from '{}'.", target.code() );

			return false;
		}

		if ( parentsPrefix.empty() )
		{
			SPDLOG_DEBUG( "pathExistsBetween: parentsPrefix is empty. Path considered valid." );
			if ( fullPathToRootPtrs.size() > 1 )
			{
				remainingParents.assign( fullPathToRootPtrs.begin(), fullPathToRootPtrs.end() - 1 );
			}

			return true;
		}
		else
		{
			if ( fullPathToRootPtrs.size() < parentsPrefix.size() )
			{
				SPDLOG_DEBUG( "pathExistsBetween: Path to root ({}) is shorter than parentsPrefix ({}). Prefix mismatch.",
					fullPathToRootPtrs.size(), parentsPrefix.size() );
				return false;
			}

			for ( size_t i = 0; i < parentsPrefix.size(); ++i )
			{
				if ( !fullPathToRootPtrs[i] || !parentsPrefix[i] )
				{
					SPDLOG_DEBUG( "pathExistsBetween: Null pointer encountered during prefix comparison at index {}.", i );
					return false;
				}

				if ( parentsPrefix[i]->code() != fullPathToRootPtrs[i]->code() )
				{
					SPDLOG_DEBUG( "pathExistsBetween: Prefix mismatch at index {}. Expected '{}', Found '{}'.",
						i, parentsPrefix[i]->code(), fullPathToRootPtrs[i]->code() );
					return false;
				}
			}
			SPDLOG_DEBUG( "pathExistsBetween: parentsPrefix matches the path to root." );
		}

		size_t startIndex = parentsPrefix.size();

		if ( startIndex < fullPathToRootPtrs.size() - 1 )
		{
			remainingParents.reserve( fullPathToRootPtrs.size() - startIndex - 1 );

			for ( size_t i = startIndex; i < fullPathToRootPtrs.size() - 1; ++i )
			{
				if ( fullPathToRootPtrs[i] )
				{
					remainingParents.emplace_back( fullPathToRootPtrs[i] );
				}
				else
				{
					SPDLOG_ERROR( "pathExistsBetween: Encountered null pointer in constructed path to root at index {} while building remaining parents. Aborting.", i );
					remainingParents.clear();
					return false;
				}
			}
		}

		SPDLOG_INFO( "pathExistsBetween: Path found from prefix to '{}'. Remaining intermediate parents count: {}", target.code(), remainingParents.size() );
		return true;
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
		return category != "PRODUCT" && category != "ASSET";
	}

	bool Gmod::isFunctionNode( const GmodNodeMetadata& metadata )
	{
		return isFunctionNode( metadata.category() );
	}

	bool Gmod::isProductSelection( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == "PRODUCT" && metadata.type() == "SELECTION";
	}

	bool Gmod::isProductType( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == "PRODUCT" && metadata.type() == "TYPE";
	}

	bool Gmod::isAsset( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == "ASSET";
	}

	bool Gmod::isAssetFunctionNode( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == "ASSET FUNCTION";
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
		if ( parent->metadata().category() == "FUNCTION" )
		{
			return false;
		}

		return child->metadata().category() == "PRODUCT" && child->metadata().type() == "TYPE";
	}

	bool Gmod::isProductSelectionAssignment( const GmodNode* parent, const GmodNode* child )
	{
		if ( parent == nullptr || child == nullptr )
		{
			return false;
		}
		if ( parent->metadata().category() == "FUNCTION" )
		{
			return false;
		}

		return child->metadata().category() != "PRODUCT" || child->metadata().type() != "SELECTION";
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
