#include "pch.h"

#include "dnv/vista/sdk/Gmod.h"

#include "dnv/vista/sdk/GmodDto.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/GmodPath.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Parents Implementation
	//-------------------------------------------------------------------

	void Parents::push( const GmodNode* parent )
	{
		m_nodes.push_back( parent );
		if ( m_occurrences.find( parent->code() ) != m_occurrences.end() )
		{
			m_occurrences[parent->code()]++;
		}
		else
		{
			m_occurrences[parent->code()] = 1;
		}
	}

	void Parents::pop()
	{
		if ( m_nodes.empty() )
			return;

		const GmodNode* parent = m_nodes.back();
		m_nodes.pop_back();

		if ( m_occurrences[parent->code()] == 1 )
		{
			m_occurrences.erase( parent->code() );
		}
		else
		{
			m_occurrences[parent->code()]--;
		}
	}

	int Parents::occurrences( const GmodNode& node ) const
	{
		try
		{
			const std::string nodeCode = node.code();
			SPDLOG_INFO( "node code: {}", nodeCode );
			auto it = m_occurrences.find( nodeCode );
			return it != m_occurrences.end() ? it->second : 0;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception in occurrences method: {}", ex.what() );
			return 0;
		}
	}

	const GmodNode* Parents::lastOrDefault() const
	{
		return m_nodes.empty() ? nullptr : m_nodes.back();
	}

	std::vector<GmodNode> Parents::nodes() const
	{
		std::vector<GmodNode> result;
		result.reserve( m_nodes.size() );
		for ( const auto* node : m_nodes )
		{
			result.push_back( *node );
		}
		return result;
	}

	const std::vector<const GmodNode*>& Parents::nodePointers() const
	{
		return m_nodes;
	}

	//-------------------------------------------------------------------
	// Constructors / Destructor
	//-------------------------------------------------------------------

	Gmod::Gmod( VisVersion version, const GmodDto& dto )
		: m_visVersion( version )
	{
		std::unordered_map<std::string, GmodNode> nodeMap;
		for ( const auto& nodeDto : dto.items )
		{
			GmodNode node( m_visVersion, nodeDto );
			nodeMap.emplace( nodeDto.code, std::move( node ) );
		}

		for ( const auto& relation : dto.relations )
		{
			if ( relation.size() >= 2 )
			{
				const std::string& parentCode = relation[0];
				const std::string& childCode = relation[1];

				auto parentIt = nodeMap.find( parentCode );
				auto childIt = nodeMap.find( childCode );

				if ( parentIt != nodeMap.end() && childIt != nodeMap.end() )
				{
					parentIt->second.addChild( &childIt->second );
					childIt->second.addParent( &parentIt->second );
				}
			}
		}

		for ( auto& [code, node] : nodeMap )
		{
			node.trim();
		}

		auto rootIt = nodeMap.find( "VE" );
		if ( rootIt != nodeMap.end() )
		{
			m_rootNode = rootIt->second;
		}

		std::vector<std::pair<std::string, GmodNode>> nodePairs;
		nodePairs.reserve( nodeMap.size() );
		for ( const auto& [code, node] : nodeMap )
		{
			nodePairs.emplace_back( code, node );
		}

		m_nodeMap = ChdDictionary<GmodNode>( std::move( nodePairs ) );

		SPDLOG_INFO( "Creating Gmod with {} items - dictionary has {} entries",
			dto.items.size(), nodeMap.size() );

		if ( m_nodeMap.isEmpty() )
		{
			SPDLOG_ERROR( "Failed to initialize node dictionary in constructor!" );
		}
	}

	Gmod::Gmod( VisVersion version, const std::unordered_map<std::string, GmodNode>& nodeMap )
		: m_visVersion( version )
	{
		for ( const auto& [code, node] : nodeMap )
		{
			const_cast<GmodNode&>( node ).trim();
		}

		m_rootNode = nodeMap.at( "VE" );

		std::vector<std::pair<std::string, GmodNode>> nodePairs;
		for ( const auto& [code, node] : nodeMap )
		{
			nodePairs.emplace_back( code, node );
		}

		m_nodeMap = ChdDictionary<GmodNode>( std::move( nodePairs ) );
	}

	Gmod::Gmod( const Gmod& other )
		: m_visVersion( other.m_visVersion ),
		  m_rootNode( other.m_rootNode ),
		  m_nodeMap( other.m_nodeMap )
	{
		SPDLOG_INFO( "Copying Gmod with {} nodes in dictionary",
			m_nodeMap.isEmpty() ? "empty" : "non-empty" );

		if ( other.m_nodeMap.isEmpty() != m_nodeMap.isEmpty() )
		{
			SPDLOG_ERROR( "Dictionary copy failed! Source: {}, Target: {}",
				other.m_nodeMap.isEmpty() ? "empty" : "non-empty",
				m_nodeMap.isEmpty() ? "empty" : "non-empty" );
		}

		if ( m_nodeMap.isEmpty() && !other.m_nodeMap.isEmpty() )
		{
			SPDLOG_ERROR( "Dictionary copy failed! Source had data but destination is empty" );
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
		if ( this != &other )
		{
			m_visVersion = other.m_visVersion;
			m_rootNode = std::move( other.m_rootNode );
			m_nodeMap = std::move( other.m_nodeMap );

			other.m_visVersion = VisVersion::Unknown;
		}
		return *this;
	}

	//-------------------------------------------------------------------
	// Basic Access Methods
	//-------------------------------------------------------------------

	const GmodNode& Gmod::operator[]( const std::string& key ) const
	{
		static GmodNode nullNode;

		try
		{
			static thread_local std::unordered_map<std::string, GmodNode> nodeCache;

			auto it = nodeCache.find( key );
			if ( it != nodeCache.end() )
			{
				return it->second;
			}

			GmodNode node;
			if ( tryGetNode( key, node ) )
			{
				auto result = nodeCache.insert_or_assign( key, std::move( node ) );
				return result.first->second;
			}

			SPDLOG_WARN( "Node with key '{}' not found", key );
			nodeCache[key] = nullNode;
			return nodeCache[key];
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception in operator[]: {}", ex.what() );
			return nullNode;
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "Unknown exception in operator[]" );
			return nullNode;
		}
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

	bool Gmod::tryGetNode( const std::string& code, GmodNode& node ) const
	{
		try
		{
			if ( m_nodeMap.isEmpty() )
			{
				SPDLOG_WARN( "TryGetNode: Node dictionary is empty, GMOD may not be properly initialized" );
				SPDLOG_INFO( "GMOD state - VisVersion: {}, Has root node: {}",
					VisVersionExtensions::toVersionString( m_visVersion ),
					m_rootNode.code().empty() ? "no" : "yes" );
				return false;
			}

			return m_nodeMap.tryGetValue( code, &node );
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception in TryGetNode for code {}: {}", code, ex.what() );
			return false;
		}
	}

	bool Gmod::tryGetNode( std::string_view code, GmodNode& node ) const
	{
		try
		{
			if ( code.empty() )
			{
				SPDLOG_WARN( "TryGetNode: Attempted to look up empty node code" );
				return false;
			}

			if ( !m_nodeMap.tryGetValue( code, &node ) )
			{
				SPDLOG_WARN( "TryGetNode: Node '{}' not found in GMOD", code );
				return false;
			}

			SPDLOG_INFO( "TryGetNode: Node '{}' found in GMOD", code );
			return true;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception in TryGetNode for '{}': {}", code, ex.what() );
			return false;
		}
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

	bool Gmod::tryParsePath( const std::string& item, std::optional<GmodPath>& path ) const
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
			return TraversalHandlerResult::Stop;
		}

		if ( node.metadata().installSubstructure().has_value() && !node.metadata().installSubstructure().value() )
		{
			return TraversalHandlerResult::Continue;
		}

		context.nodesVisited++;

		std::vector<GmodNode> currentPath;
		try
		{
			currentPath = context.parents.nodes();
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Failed to get path nodes: {}", ex.what() );
			return TraversalHandlerResult::Stop;
		}

		TraversalHandlerResult result;
		try
		{
			result = context.handler( currentPath, node );
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Handler exception: {}", ex.what() );
			return TraversalHandlerResult::Stop;
		}

		if ( result == TraversalHandlerResult::Stop || result == TraversalHandlerResult::SkipSubtree )
		{
			return result;
		}

		try
		{
			context.parents.push( &node );
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Failed to push node: {}", ex.what() );
			return TraversalHandlerResult::Stop;
		}

		try
		{
			const auto& children = node.children();
			for ( const GmodNode* child : children )
			{
				if ( child == nullptr )
					continue;

				result = traverseNode( context, *child );

				if ( result == TraversalHandlerResult::Stop )
				{
					context.parents.pop();
					return result;
				}
			}
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Error traversing children: {}", ex.what() );
			context.parents.pop();
			return TraversalHandlerResult::Stop;
		}

		context.parents.pop();
		return TraversalHandlerResult::Continue;
	}

	bool Gmod::pathExistsBetween(
		const std::vector<GmodNode>& fromPath,
		const GmodNode& to,
		std::vector<GmodNode>& remainingParents ) const
	{
		if ( fromPath.empty() )
		{
			SPDLOG_INFO( "pathExistsBetween called with empty fromPath" );
			return false;
		}

		const GmodNode& lastParent = fromPath.back();
		SPDLOG_INFO( "Finding path from {} to {}", lastParent.code(), to.code() );

		/* 		bool isSystemComponentRelationship =
					( lastParent.code().find( '.' ) != std::string::npos &&
						to.code().find( 'C' ) == 0 &&
						to.code().find( '.' ) != std::string::npos );

		if ( isSystemComponentRelationship )
		{
			SPDLOG_INFO( "System-component relationship detected between {} and {}",
				lastParent.code(), to.code() );
			remainingParents.clear();
			return true;
		}
			*/

		if ( lastParent.isChild( to.code() ) )
		{
			SPDLOG_INFO( "Direct path found: {} is direct child of {}", to.code(), lastParent.code() );
			remainingParents.clear();
			return true;
		}

		for ( const auto* childNode : lastParent.children() )
		{
			auto isParent = isPotentialParent( childNode->metadata().type() );
			if ( isParent )
			{
				SPDLOG_INFO( "Checking child node {} for parent relationship", childNode->code() );
			}
			else
			{
				SPDLOG_INFO( "Child node {} is not a potential parent", childNode->code() );
			}

			if ( childNode && isParent )
			{
				SPDLOG_INFO( "Indirect path found via child node {} between {} and {}", childNode->code(), lastParent.code(), to.code() );

				remainingParents.clear();
				return true;
			}
		}

		struct PathFindingState
		{
			const std::vector<GmodNode>* fromPath;
			const GmodNode* target;
			bool found = false;
			std::vector<GmodNode> resultPath;
		};

		PathFindingState state;
		state.fromPath = &fromPath;
		state.target = &to;

		auto handler = []( PathFindingState& s,
						   const std::vector<GmodNode>& parents,
						   const GmodNode& node ) -> TraversalHandlerResult {
			if ( parents.size() < s.fromPath->size() )
				return TraversalHandlerResult::Continue;

			bool pathMatches = true;
			for ( size_t i = 0; i < s.fromPath->size(); i++ )
			{
				if ( parents[i].code() != ( *s.fromPath )[i].code() )
				{
					pathMatches = false;
					break;
				}
			}

			if ( !pathMatches )
				return TraversalHandlerResult::SkipSubtree;

			if ( node.code() == s.target->code() )
			{
				s.resultPath.clear();
				for ( size_t i = s.fromPath->size(); i < parents.size(); i++ )
				{
					s.resultPath.push_back( parents[i] );
				}
				s.resultPath.push_back( node );
				s.found = true;
				return TraversalHandlerResult::Stop;
			}

			return TraversalHandlerResult::Continue;
		};

		TraversalOptions options;
		options.maxTraversalOccurrence = 3;
		options.maxNodes = 1000;

		const GmodNode* startNode = nullptr;

		for ( auto it = fromPath.rbegin(); it != fromPath.rend(); ++it )
		{
			if ( isAssetFunctionNode( it->metadata() ) )
			{
				startNode = &( *it );
				break;
			}
		}

		if ( !startNode )
		{
			startNode = &rootNode();
		}

		if ( !traverse<PathFindingState>( state, *startNode, handler, options ) )
		{
			SPDLOG_DEBUG( "Traversal was stopped - path may have been found" );
		}

		if ( state.found )
		{
			SPDLOG_INFO( "Found path from {} to {} with {} intermediate nodes",
				lastParent.code(), to.code(), state.resultPath.size() - 1 );

			if ( !state.resultPath.empty() )
			{
				remainingParents.assign( state.resultPath.begin() + 1, state.resultPath.end() );
			}
			else
			{
				remainingParents.clear();
			}
			return true;
		}

		if ( lastParent.metadata().category() == to.metadata().category() )
		{
			SPDLOG_INFO( "Nodes share same category: {}", lastParent.metadata().category() );
			remainingParents.clear();
			return true;
		}

		SPDLOG_WARN( "No path found from {} to {}", lastParent.code(), to.code() );
		return false;
	}

	//-------------------------------------------------------------------
	// Iterator Methods
	//-------------------------------------------------------------------

	Gmod::Iterator::Iterator( ChdDictionary<GmodNode>::Iterator innerIt )
		: m_innerIt( std::move( innerIt ) )
	{
	}

	Gmod::Iterator::reference Gmod::Iterator::operator*() const
	{
		return m_innerIt->second;
	}

	Gmod::Iterator::pointer Gmod::Iterator::operator->() const
	{
		return &( m_innerIt->second );
	}

	Gmod::Iterator& Gmod::Iterator::operator++()
	{
		++m_innerIt;
		return *this;
	}

	Gmod::Iterator Gmod::Iterator::operator++( int )
	{
		Iterator tmp = *this;
		++( *this );
		return tmp;
	}

	bool Gmod::Iterator::operator==( const Iterator& other ) const
	{
		return m_innerIt == other.m_innerIt;
	}

	bool Gmod::Iterator::operator!=( const Iterator& other ) const
	{
		return m_innerIt != other.m_innerIt;
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
			return false;

		if ( parent->metadata().category().find( "FUNCTION" ) == std::string::npos )
			return false;

		if ( child->metadata().category() != "PRODUCT" || child->metadata().type() != "TYPE" )
			return false;

		return true;
	}

	bool Gmod::isProductSelectionAssignment( const GmodNode* parent, const GmodNode* child )
	{
		try
		{
			if ( parent == nullptr || child == nullptr )
				return false;

			const auto& parentMetadata = parent->metadata();
			const auto& childMetadata = child->metadata();

			if ( parentMetadata.category().empty() || childMetadata.category().empty() )
				return false;

			if ( parentMetadata.category().find( "FUNCTION" ) == std::string::npos )
				return false;

			if ( childMetadata.category().find( "PRODUCT" ) == std::string::npos ||
				 childMetadata.type().empty() || childMetadata.type() != "SELECTION" )
				return false;

			return true;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception in isProductSelectionAssignment: {}", ex.what() );
			return false;
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "Unknown exception in isProductSelectionAssignment" );
			return false;
		}
	}
}
