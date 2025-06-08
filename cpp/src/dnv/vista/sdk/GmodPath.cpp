/**
 * @file GmodPath.cpp
 * @brief Implementation of GmodPath and related classes for representing paths in the Generic Product Model (GMOD).
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodPath.h"

#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/GmodTraversal.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/VIS.h"

namespace dnv::vista::sdk
{
	namespace internal
	{
		//--------------------------------------------------------------------------
		// LocationSetsVisitor
		//--------------------------------------------------------------------------

		struct LocationSetsVisitor
		{
			size_t currentParentStart;

			LocationSetsVisitor() : currentParentStart( std::numeric_limits<size_t>().max() ) {}

			std::optional<std::tuple<size_t, size_t, std::optional<Location>>> visit(
				const GmodNode& node,
				size_t i,
				const std::vector<GmodNode*>& pathParents,
				const GmodNode& pathTargetNode )
			{
				bool isParent = Gmod::isPotentialParent( node.metadata().type() );
				bool isTargetNode = ( static_cast<size_t>( i ) == pathParents.size() );

				if ( currentParentStart == std::numeric_limits<size_t>().max() )
				{
					if ( isParent )
						currentParentStart = i;
					if ( node.isIndividualizable( isTargetNode ) )
						return std::make_tuple( i, i, node.location() );
				}
				else
				{
					if ( isParent || isTargetNode )
					{
						std::optional<std::tuple<size_t, size_t, std::optional<Location>>> nodes = std::nullopt;

						if ( currentParentStart + 1 == i )
						{
							if ( node.isIndividualizable( isTargetNode ) )
								nodes = std::make_tuple( i, i, node.location() );
						}
						else
						{
							size_t skippedOne = std::numeric_limits<size_t>().max();
							bool hasComposition = false;

							for ( size_t j = currentParentStart + 1; j <= i; ++j )
							{
								const GmodNode* setNode = ( j < pathParents.size() ) ? pathParents[j] : &pathTargetNode;

								if ( !setNode->isIndividualizable( j == pathParents.size(), true ) )
								{
									if ( nodes.has_value() )
										skippedOne = j;
									continue;
								}

								if ( nodes.has_value() &&
									 std::get<2>( nodes.value() ).has_value() &&
									 setNode->location().has_value() &&
									 std::get<2>( nodes.value() ) != setNode->location() )
								{
									throw std::runtime_error( "Mapping error: different locations in the same nodeset" );
								}

								if ( skippedOne != std::numeric_limits<size_t>().max() )
									throw std::runtime_error( "Can't skip in the middle of individualizable set" );

								if ( setNode->isFunctionComposition() )
									hasComposition = true;

								auto location = nodes.has_value() && std::get<2>( nodes.value() ).has_value() ? std::get<2>( nodes.value() ) : setNode->location();
								size_t start = nodes.has_value() ? std::get<0>( nodes.value() ) : j;
								size_t end = j;
								nodes = std::make_tuple( start, end, location );
							}

							if ( nodes.has_value() && std::get<0>( nodes.value() ) == std::get<1>( nodes.value() ) && hasComposition )
								nodes = std::nullopt;
						}

						currentParentStart = i;
						if ( nodes.has_value() )
						{
							bool hasLeafNode = false;
							size_t startIdx = std::get<0>( nodes.value() );
							size_t endIdx = std::get<1>( nodes.value() );

							for ( size_t j = startIdx; j <= endIdx; ++j )
							{
								const GmodNode* setNode = ( j < pathParents.size() ) ? pathParents[j] : &pathTargetNode;
								if ( setNode->isLeafNode() || j == pathParents.size() )
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

					if ( isTargetNode && node.isIndividualizable( isTargetNode ) )
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
			bool foundCurrentToFind = ( currentNode.code() == context.toFind.code );

			if ( !foundCurrentToFind && Gmod::isLeafNode( currentNode.metadata() ) )
			{
				return TraversalHandlerResult::SkipSubtree;
			}

			if ( !foundCurrentToFind )
			{
				return TraversalHandlerResult::Continue;
			}

			if ( context.toFind.location.has_value() )
			{
				if ( !context.nodeLocations.has_value() )
				{
					context.nodeLocations.emplace();
				}
				context.nodeLocations->emplace( std::string( context.toFind.code ), context.toFind.location.value() );
			}

			if ( !context.partsQueue.empty() )
			{
				context.toFind = context.partsQueue.front();
				context.partsQueue.pop_front();

				return TraversalHandlerResult::Continue;
			}

			std::vector<GmodNode*> pathParents;
			pathParents.reserve( traversedParents.size() + 1 );

			for ( const GmodNode* parent : traversedParents )
			{
				if ( !parent )
					continue;

				if ( context.nodeLocations.has_value() )
				{
					auto it = context.nodeLocations->find( std::string( parent->code() ) );
					if ( it != context.nodeLocations->end() )
					{
						GmodNode* newNode = new GmodNode( parent->withLocation( it->second ) );
						context.ownedNodesForCurrentPath.push_back( newNode );
						pathParents.push_back( newNode );

						continue;
					}
				}
				pathParents.push_back( const_cast<GmodNode*>( parent ) );
			}

			GmodNode* endNode;
			if ( context.toFind.location.has_value() )
			{
				GmodNode* newNode = new GmodNode( currentNode.withLocation( context.toFind.location.value() ) );
				context.ownedNodesForCurrentPath.push_back( newNode );
				endNode = newNode;
			}
			else
			{
				endNode = const_cast<GmodNode*>( &currentNode );
			}

			GmodNode* startNode = nullptr;
			if ( !pathParents.empty() && !pathParents[0]->parents().empty() && pathParents[0]->parents().size() == 1 )
			{
				startNode = const_cast<GmodNode*>( pathParents[0]->parents()[0] );
			}
			else if ( !endNode->parents().empty() && endNode->parents().size() == 1 )
			{
				startNode = const_cast<GmodNode*>( endNode->parents()[0] );
			}

			while ( startNode && startNode->parents().size() == 1 )
			{
				pathParents.insert( pathParents.begin(), startNode );
				startNode = const_cast<GmodNode*>( startNode->parents()[0] );
			}

			if ( pathParents.empty() || pathParents[0] != &context.gmod.rootNode() )
			{
				pathParents.insert( pathParents.begin(), const_cast<GmodNode*>( &context.gmod.rootNode() ) );
			}

			internal::LocationSetsVisitor locationSetsVisitor;
			for ( size_t i = 0; i < pathParents.size() + 1; ++i )
			{
				GmodNode* nodeInPath = ( i < pathParents.size() ) ? pathParents[i] : endNode;
				if ( !nodeInPath )
					continue;

				std::optional<std::tuple<size_t, size_t, std::optional<Location>>> setDetails =
					locationSetsVisitor.visit( *nodeInPath, i, pathParents, *endNode );

				if ( setDetails.has_value() )
				{
					const auto& setTuple = setDetails.value();
					size_t setStartIdx = std::get<0>( setTuple );
					size_t setEndIdx = std::get<1>( setTuple );
					const std::optional<Location>& setCommonLocation = std::get<2>( setTuple );

					if ( setStartIdx == setEndIdx )
						continue;

					if ( setCommonLocation.has_value() )
					{
						for ( size_t k = setStartIdx; k <= setEndIdx; ++k )
						{
							GmodNode** nodesToUpdateInPath;
							if ( k < pathParents.size() )
							{
								nodesToUpdateInPath = &pathParents[k];
							}
							else
							{
								nodesToUpdateInPath = &endNode;
							}
							GmodNode* currentNodeInSet = *nodesToUpdateInPath;

							bool needsNewNode = true;
							if ( currentNodeInSet->location().has_value() && setCommonLocation.has_value() &&
								 currentNodeInSet->location().value() == setCommonLocation.value() )
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
			pathObject.m_parents = pathParents;
			pathObject.m_node = endNode;
			pathObject.m_visVersion = endNode->visVersion();
			pathObject.m_ownedNodes = std::move( context.ownedNodesForCurrentPath );
			context.ownedNodesForCurrentPath.clear();

			context.resultingPath.emplace( std::move( pathObject ) );
			return TraversalHandlerResult::Stop;
		}
	}

	//=====================================================================
	// GmodPath class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	GmodPath::GmodPath( const Gmod& gmod, GmodNode* node, std::vector<GmodNode*> parents )
		: m_gmod{ &gmod },
		  m_node{ node },
		  m_parents{ std::move( parents ) }
	{
		if ( !m_node )
		{
			throw std::invalid_argument( "GmodPath constructor: node cannot be null." );
		}

		m_visVersion = m_node->visVersion();

		if ( m_parents.empty() )
		{
			if ( m_node != &m_gmod->rootNode() )
			{
				std::string nodeCode = std::string( m_node->code() );
				std::string rootCode = std::string( m_gmod->rootNode().code() );
				throw std::invalid_argument( fmt::format( "Invalid GMOD path - no parents, and node '{}' is not the GMOD root '{}'.", nodeCode, rootCode ) );
			}
		}
		else
		{
			if ( m_parents.front() == nullptr )
			{
				throw std::invalid_argument( "Invalid GMOD path: first parent is null." );
			}

			if ( m_parents.front() != &m_gmod->rootNode() )
			{
				std::string firstParentCode = std::string( m_parents.front()->code() );
				std::string rootCode = std::string( m_gmod->rootNode().code() );
				throw std::invalid_argument( fmt::format( "Invalid GMOD path - first parent '{}' should be GMOD root '{}'.", firstParentCode, rootCode ) );
			}

			for ( size_t i = 0; i < m_parents.size(); ++i )
			{
				GmodNode* currentParentNode = m_parents[i];
				GmodNode* childNodeToCheck = ( i + 1 < m_parents.size() ) ? m_parents[i + 1] : m_node;

				if ( !currentParentNode )
				{
					throw std::invalid_argument( fmt::format( "Invalid GMOD path: null parent encountered in parents list at index {}.", i ) );
				}
				if ( !childNodeToCheck )
				{
					throw std::invalid_argument( fmt::format( "Invalid GMOD path: child node (or final node) is null for parent at index {}.", i ) );
				}

				if ( !currentParentNode->isChild( *childNodeToCheck ) )
				{
					std::string childCode = std::string( childNodeToCheck->code() );
					std::string parentCode = std::string( currentParentNode->code() );
					throw std::invalid_argument( fmt::format( "Invalid GMOD path - node '{}' not child of '{}'.", childCode, parentCode ) );
				}
			}
		}

		try
		{
			internal::LocationSetsVisitor visitor;
			for ( size_t i = 0; i < m_parents.size() + 1; ++i )
			{
				GmodNode* nodeToVisit = ( i < m_parents.size() ) ? m_parents[i] : m_node;
				if ( !nodeToVisit )
				{
					throw std::runtime_error( fmt::format( "Null node encountered at index {} during LocationSetsVisitor phase.", i ) );
				}
				visitor.visit( *nodeToVisit, i, m_parents, *m_node );
			}
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			if ( m_node )
			{
				SPDLOG_ERROR( "GmodPath construction for node '{}' failed during LocationSetsVisitor validation: {}", m_node->code(), ex.what() );
			}

			throw;
		}
	}

	GmodPath::GmodPath()
		: m_visVersion{ VisVersion::Unknown },
		  m_gmod{ nullptr },
		  m_node{ nullptr }
	{
	}

	GmodPath::GmodPath( const GmodPath& other )
		: m_visVersion{ other.m_visVersion },
		  m_gmod{ other.m_gmod },
		  m_node{ nullptr },
		  m_parents{}
	{
		m_ownedNodes.reserve( other.m_ownedNodes.size() );
		std::unordered_map<const GmodNode*, GmodNode*> nodeMapping;

		for ( GmodNode* ownedNodeToCopy : other.m_ownedNodes )
		{
			if ( ownedNodeToCopy )
			{
				GmodNode* newCopy = new GmodNode( *ownedNodeToCopy );
				m_ownedNodes.push_back( newCopy );
				nodeMapping[ownedNodeToCopy] = newCopy;
			}
			else
			{
				m_ownedNodes.push_back( nullptr );
			}
		}

		m_parents.reserve( other.m_parents.size() );
		for ( GmodNode* originalParent : other.m_parents )
		{
			auto it = nodeMapping.find( originalParent );
			if ( it != nodeMapping.end() )
			{
				m_parents.push_back( it->second );
			}
			else
			{
				m_parents.push_back( originalParent );
			}
		}

		auto nodeIt = nodeMapping.find( other.m_node );
		if ( nodeIt != nodeMapping.end() )
		{
			m_node = nodeIt->second;
		}
		else
		{
			m_node = other.m_node;
		}
	}

	GmodPath::GmodPath( GmodPath&& other ) noexcept
		: m_visVersion{ other.m_visVersion },
		  m_gmod{ other.m_gmod },
		  m_node{ other.m_node },
		  m_parents{ std::move( other.m_parents ) },
		  m_ownedNodes{ std::move( other.m_ownedNodes ) }
	{
		other.m_node = nullptr;
		other.m_gmod = nullptr;
		other.m_visVersion = VisVersion::Unknown;
	}

	GmodPath::~GmodPath()
	{
		for ( GmodNode* ownedNode : m_ownedNodes )
		{
			delete ownedNode;
		}
		m_ownedNodes.clear();
	}

	//----------------------------------------------
	// Assignment operators
	//----------------------------------------------

	GmodPath& GmodPath::operator=( const GmodPath& other )
	{
		if ( this == &other )
		{
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

		return *this;
	}

	GmodPath& GmodPath::operator=( GmodPath&& other ) noexcept
	{
		if ( this == &other )
		{
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

		return *this;
	}

	//----------------------------------------------
	// Equality operators
	//----------------------------------------------

	bool GmodPath::operator==( const GmodPath& other ) const noexcept
	{
		if ( this == &other )
		{
			return true;
		}

		if ( m_parents.size() != other.m_parents.size() )
		{
			return false;
		}

		if ( m_visVersion != other.m_visVersion )
		{
			return false;
		}

		/* Compare parents */
		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			{ /* Null pointers */
				if ( ( !m_parents[i] && other.m_parents[i] ) || ( m_parents[i] && !other.m_parents[i] ) )
				{
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
					return false;
				}
			}

			{ /* Compare locations */
				bool thisHasLocation = m_parents[i]->location().has_value();
				bool otherHasLocation = other.m_parents[i]->location().has_value();

				if ( thisHasLocation != otherHasLocation )
				{
					return false;
				}

				if ( thisHasLocation && otherHasLocation &&
					 m_parents[i]->location().value() != other.m_parents[i]->location().value() )
				{
					return false;
				}
			}
		}

		{ /* Null target nodes */
			if ( ( !m_node && other.m_node ) || ( m_node && !other.m_node ) )
			{
				return false;
			}

			if ( !m_node && !other.m_node )
			{
				return true;
			}
		}

		{ /* Target node code */

			if ( m_node->code() != other.m_node->code() )
			{
				return false;
			}
		}

		{ /* Target node location */

			bool thisHasLocation = m_node->location().has_value();
			bool otherHasLocation = other.m_node->location().has_value();

			if ( thisHasLocation != otherHasLocation )
			{
				return false;
			}

			if ( thisHasLocation && otherHasLocation &&
				 m_node->location().value() != other.m_node->location().value() )
			{
				return false;
			}
		}

		return true;
	}

	bool GmodPath::operator!=( const GmodPath& other ) const noexcept
	{
		return !( *this == other );
	}

	//----------------------------------------------
	// Lookup operators
	//----------------------------------------------

	GmodNode* GmodPath::operator[]( size_t index ) const
	{
		if ( index >= ( m_parents.size() + 1 ) )
		{
			throw std::out_of_range( fmt::format( "Index {} out of range for GmodPath indexer. Path length is {}.", index, m_parents.size() + 1 ) );
		}

		if ( index < m_parents.size() )
		{
			GmodNode* parentNode = m_parents[index];
			return parentNode;
		}
		else
		{
			return m_node;
		}
	}

	GmodNode*& GmodPath::operator[]( size_t index )
	{
		if ( index >= ( m_parents.size() + 1 ) )
		{
			throw std::out_of_range( fmt::format( "Index {} out of range for GmodPath indexer. Path length is {}.", index, m_parents.size() + 1 ) );
		}

		if ( index < m_parents.size() )
		{
			return m_parents[index];
		}
		else
		{
			return m_node;
		}
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	VisVersion GmodPath::visVersion() const noexcept
	{
		return m_visVersion;
	}

	size_t GmodPath::hashCode() const noexcept
	{
		size_t hashCode = 0;

		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			if ( m_parents[i] )
			{
				size_t nodeHash = m_parents[i]->hashCode();
				hashCode ^= nodeHash + 0x9e3779b9 + ( hashCode << 6 ) + ( hashCode >> 2 );
			}
		}

		if ( m_node )
		{
			size_t nodeHash = m_node->hashCode();
			hashCode ^= nodeHash + 0x9e3779b9 + ( hashCode << 6 ) + ( hashCode >> 2 );
		}

		return hashCode;
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
		return m_parents.size() + 1;
	}

	std::vector<GmodIndividualizableSet> GmodPath::individualizableSets() const
	{
		std::vector<GmodIndividualizableSet> result;
		internal::LocationSetsVisitor visitor;

		for ( size_t i = 0; i < length(); ++i )
		{
			GmodNode* node = ( *this )[i];
			if ( !node )
				continue;

			auto set = visitor.visit( *node, i, m_parents, *m_node );
			if ( !set.has_value() )
				continue;

			const auto& [startIdx, endIdx, location] = set.value();
			int start = static_cast<int>( startIdx );
			int end = static_cast<int>( endIdx );

			if ( start == end )
			{
				std::vector<int> indices = { start };
				result.emplace_back( std::move( indices ), *this );
				continue;
			}

			std::vector<int> nodes;
			nodes.reserve( static_cast<size_t>( end - start + 1 ) );
			for ( int j = start; j <= end; ++j )
			{
				nodes.push_back( j );
			}

			result.emplace_back( std::move( nodes ), *this );
		}

		return result;
	}

	std::optional<std::string> GmodPath::normalAssignmentName( size_t nodeDepth ) const
	{
		if ( nodeDepth >= length() )
		{
			return std::nullopt;
		}

		GmodNode* nodeInPath = ( *this )[nodeDepth];
		if ( !nodeInPath )
		{
			return std::nullopt;
		}

		const auto& normalAssignmentNames = nodeInPath->metadata().normalAssignmentNames();
		if ( normalAssignmentNames.empty() )
		{
			return std::nullopt;
		}

		if ( length() == 0 )
		{
			return std::nullopt;
		}

		for ( size_t i = length() - 1;; --i )
		{
			GmodNode* child = ( *this )[i];
			if ( child )
			{
				auto it = normalAssignmentNames.find( child->code().data() );
				if ( it != normalAssignmentNames.end() )
				{
					return it->second;
				}
			}

			if ( i == 0 )
			{
				break;
			}
		}

		return std::nullopt;
	}

	std::vector<std::pair<size_t, std::string>> GmodPath::commonNames() const
	{
		std::vector<std::pair<size_t, std::string>> result;

		auto enumerator = this->enumerator();
		while ( enumerator.next() )
		{
			const auto& [depth, node] = enumerator.current();

			if ( !node )
			{
				continue;
			}

			bool isTarget = ( depth == m_parents.size() );

			if ( !( node->isLeafNode() || isTarget ) || !node->isFunctionNode() )
			{
				continue;
			}

			std::string name;
			const auto& commonName = node->metadata().commonName();
			if ( commonName.has_value() && !commonName->empty() )
			{
				name = *commonName;
			}
			else
			{
				name = node->metadata().name();
			}

			const auto& normalAssignmentNames = node->metadata().normalAssignmentNames();

			if ( !normalAssignmentNames.empty() )
			{
				if ( m_node )
				{
					auto nodeCodeIt = normalAssignmentNames.find( std::string( m_node->code() ) );
					if ( nodeCodeIt != normalAssignmentNames.end() )
					{
						name = nodeCodeIt->second;
					}
				}

				if ( !m_parents.empty() && depth < m_parents.size() )
				{
					for ( size_t i = m_parents.size() - 1; i >= depth; --i )
					{
						if ( m_parents[i] )
						{
							auto parentCodeIt = normalAssignmentNames.find( std::string( m_parents[i]->code() ) );
							if ( parentCodeIt != normalAssignmentNames.end() )
							{
								name = parentCodeIt->second;
								break;
							}
						}

						if ( i == depth )
						{
							break;
						}
					}
				}
			}

			result.emplace_back( depth, name );
		}

		return result;
	}

	//----------------------------------------------
	// Utility methods
	//----------------------------------------------

	bool GmodPath::isValid( const std::vector<GmodNode*>& parents, const GmodNode& node )
	{
		int missingLinkAt;
		return isValid( parents, node, missingLinkAt );
	}

	bool GmodPath::isValid( const std::vector<GmodNode*>& parents, const GmodNode& node, int& missingLinkAt )
	{
		missingLinkAt = -1;

		if ( parents.empty() )
			return false;

		if ( !parents.empty() && !parents[0]->isRoot() )
			return false;

		for ( size_t i = 0; i < parents.size(); ++i )
		{
			const GmodNode* parent = parents[i];
			const GmodNode* child = ( i + 1 < parents.size() ) ? parents[i + 1] : &node;

			if ( !parent->isChild( *child ) )
			{
				missingLinkAt = static_cast<int>( i );
				return false;
			}
		}

		return true;
	}

	bool GmodPath::isMappable() const
	{
		if ( !m_node )
		{
			return false;
		}

		return m_node->isMappable();
	}

	std::string GmodPath::toString() const
	{
		std::stringstream builder;
		toString( builder, '/' );

		std::string result = builder.str();

		return result;
	}

	std::string GmodPath::toStringDump() const
	{
		std::stringstream builder;
		toStringDump( builder );

		return builder.str();
	}

	std::string GmodPath::toFullPathString() const
	{
		std::stringstream builder;
		toFullPathString( builder );

		return builder.str();
	}

	GmodPath GmodPath::withoutLocations() const
	{
		if ( !m_gmod || !m_node )
		{
			return GmodPath{};
		}

		std::vector<GmodNode*> newParents;
		newParents.reserve( m_parents.size() );

		for ( GmodNode* parent : m_parents )
		{
			if ( parent )
			{
				const GmodNode* baseNode = nullptr;
				if ( m_gmod->tryGetNode( parent->code(), baseNode ) && baseNode )
				{
					newParents.push_back( const_cast<GmodNode*>( baseNode ) );
				}
			}
		}

		const GmodNode* baseFinalNode = nullptr;
		if ( m_gmod->tryGetNode( m_node->code(), baseFinalNode ) && baseFinalNode )
		{
			return GmodPath( *m_gmod, const_cast<GmodNode*>( baseFinalNode ), std::move( newParents ) );
		}

		return GmodPath{};
	}

	void GmodPath::toString( std::stringstream& builder, char separator ) const
	{
		for ( const auto& parent : m_parents )
		{
			if ( !Gmod::isLeafNode( parent->metadata() ) )
			{
				continue;
			}

			parent->toString( builder );
			builder << separator;
		}

		m_node->toString( builder );
	}

	void GmodPath::toStringDump( std::stringstream& builder ) const
	{
		auto enumerator = this->enumerator();
		bool first = true;

		while ( enumerator.next() )
		{
			const auto& [depth, pathNode] = enumerator.current();

			if ( depth == 0 )
			{
				continue;
			}

			if ( !first )
			{
				builder << " | ";
			}

			builder << pathNode->code().data();

			const auto& name = pathNode->metadata().name();
			if ( !name.empty() )
			{
				builder << "/N:" << name;
			}

			const auto& commonName = pathNode->metadata().commonName();
			if ( commonName.has_value() && !commonName->empty() )
			{
				builder << "/CN:" << *commonName;
			}

			auto normalAssignment = normalAssignmentName( depth );
			if ( normalAssignment && !normalAssignment->empty() )
			{
				builder << "/NAN:" << *normalAssignment;
			}

			first = false;
		}
	}

	void GmodPath::toFullPathString( std::stringstream& builder ) const
	{
		auto enumerator = this->enumerator();

		while ( enumerator.next() )
		{
			const auto& [depth, pathNode] = enumerator.current();

			if ( !pathNode )
			{
				continue;
			}

			pathNode->toString( builder );

			if ( depth != ( length() - 1 ) )
			{
				builder << '/';
			}
		}
	}

	//----------------------------------------------
	// Public static parsing methods
	//----------------------------------------------

	GmodPath GmodPath::parse( std::string_view item, VisVersion visVersion )
	{
		std::optional<GmodPath> path;
		if ( !tryParse( item, visVersion, path ) )
		{
			throw std::invalid_argument( "Couldnt parse path" );
		}

		return path.value();
	}

	GmodPath GmodPath::parse( std::string_view item, const Gmod& gmod, const Locations& locations )
	{
		std::unique_ptr<GmodParsePathResult> result = parseInternal( item, gmod, locations );

		if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() ) )
		{
			return std::move( okResult->path );
		}
		else if ( auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( result.get() ) )
		{
			throw std::invalid_argument( errResult->error );
		}
		else
		{
			throw std::runtime_error( "Unexpected result" );
		}
	}

	GmodPath GmodPath::parseFullPath( std::string_view item, VisVersion visVersion )
	{
		VIS& vis = VIS::instance();
		const Gmod& gmod = vis.gmod( visVersion );
		const Locations& locations = vis.locations( visVersion );
		std::unique_ptr<GmodParsePathResult> result = parseFullPathInternal( item, gmod, locations );

		if ( !result )
		{
			throw std::runtime_error( fmt::format( "GmodPath::parseFullPath: parseFullPathInternal returned nullptr for item '{}'.", std::string( item ) ) );
		}

		if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() ) )
		{
			return std::move( okResult->path );
		}
		else if ( [[maybe_unused]] auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( result.get() ) )
		{
			throw std::invalid_argument( errResult->error );
		}
		else
		{
			throw std::runtime_error( fmt::format( "GmodPath::parseFullPath: Unexpected result type from parseFullPathInternal for item '{}'.", std::string( item ) ) );
		}
	}

	bool GmodPath::tryParse( std::string_view item, VisVersion visVersion, std::optional<GmodPath>& outPath )
	{
		outPath.reset();

		VIS& vis = VIS::instance();
		const Gmod& gmod = vis.gmod( visVersion );
		const Locations& locations = vis.locations( visVersion );

		return tryParse( item, gmod, locations, outPath );
	}

	bool GmodPath::tryParse( std::string_view item, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath )
	{
		std::unique_ptr<GmodParsePathResult> result = parseInternal( item, gmod, locations );
		outPath.reset();

		if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() ) )
		{
			outPath.emplace( std::move( okResult->path ) );
			return true;
		}

		return false;
	}

	bool GmodPath::tryParseFullPath( std::string_view item, VisVersion visVersion, std::optional<GmodPath>& outPath )
	{
		outPath.reset();

		VIS& vis = VIS::instance();
		const Gmod& gmod = vis.gmod( visVersion );
		const Locations& locations = vis.locations( visVersion );

		return tryParseFullPath( item, gmod, locations, outPath );
	}

	bool GmodPath::tryParseFullPath( std::string_view item, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath )
	{
		std::unique_ptr<GmodParsePathResult> result = parseFullPathInternal( item, gmod, locations );

		if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() ) )
		{
			outPath.emplace( std::move( okResult->path ) );
			return true;
		}

		outPath.reset();
		return false;
	}

	//----------------------------
	// Enumeration
	//----------------------------

	GmodPath::Enumerator GmodPath::enumerator( size_t fromDepth ) const
	{
		return Enumerator( this, fromDepth );
	}

	//----------------------------------------------
	// Private static parsing methods
	//----------------------------------------------

	std::unique_ptr<GmodParsePathResult> GmodPath::parseInternal( std::string_view item, const Gmod& gmod, const Locations& locations )
	{
		if ( gmod.visVersion() != locations.visVersion() )
			throw std::invalid_argument( "Got different VIS versions for Gmod and Locations arguments" );

		if ( item.empty() )
			return std::make_unique<GmodParsePathResult::Err>( "Item is empty" );

		size_t start = item.find_first_not_of( " \t\n\r\f\v" );
		if ( start == std::string_view::npos )
			return std::make_unique<GmodParsePathResult::Err>( "Item is empty" );

		item = item.substr( start );
		size_t end = item.find_last_not_of( " \t\n\r\f\v" );
		item = item.substr( 0, end + 1 );

		if ( !item.empty() && item.front() == '/' )
			item.remove_prefix( 1 );

		if ( item.empty() )
			return std::make_unique<GmodParsePathResult::Err>( "Item is empty" );

		std::deque<internal::PathNode> parts;
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
					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Failed to get GmodNode for {}", std::string( part ) ) );

				Location parsedLocation;
				if ( !locations.tryParse( locationPart, parsedLocation ) )
					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Failed to parse location {}", std::string( locationPart ) ) );

				currentPathNode.location = parsedLocation;
			}
			else
			{
				currentPathNode.code = std::string( part );
				if ( !gmod.tryGetNode( part, tempNodeCheck ) )
					return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Failed to get GmodNode for {}", std::string( part ) ) );
			}
			parts.push_back( currentPathNode );

			if ( slashPosition == std::string_view::npos )
				break;
			currentSegment = currentSegment.substr( slashPosition + 1 );
		}

		if ( parts.empty() )
			return std::make_unique<GmodParsePathResult::Err>( "Failed find any parts" );

		for ( const auto& parsedNode : parts )
		{
			if ( parsedNode.code.empty() )
				return std::make_unique<GmodParsePathResult::Err>( "Found part with empty code" );
		}

		internal::PathNode toFind = parts.front();
		parts.pop_front();

		const GmodNode* baseNode = nullptr;
		if ( !gmod.tryGetNode( toFind.code, baseNode ) || !baseNode )
			return std::make_unique<GmodParsePathResult::Err>( "Failed to get GmodNode for " + toFind.code );

		internal::ParseContext context( std::move( parts ), gmod, toFind );

		TraverseHandlerWithState<internal::ParseContext> handler =
			[]( internal::ParseContext& state, const std::vector<const GmodNode*>& parents, const GmodNode& node ) -> TraversalHandlerResult {
			return internal::parseInternalTraversalHandler( state, parents, node );
		};

		GmodTraversal::traverse( context, *baseNode, handler );

		if ( context.resultingPath.has_value() )
		{
			const auto& resultPath = context.resultingPath.value();

			if ( resultPath.m_parents.empty() )
			{
				if ( resultPath.m_node != &gmod.rootNode() )
				{
					return std::make_unique<GmodParsePathResult::Err>( "Single node path must be the root node" );
				}
			}
			else
			{
				if ( resultPath.m_parents.empty() || resultPath.m_parents[0] != &gmod.rootNode() )
				{
					return std::make_unique<GmodParsePathResult::Err>( "Path must start from root node" );
				}
			}

			int missingLinkAt;
			if ( !GmodPath::isValid( resultPath.m_parents, *resultPath.m_node, missingLinkAt ) )
			{
				return std::make_unique<GmodParsePathResult::Err>( "Invalid path structure - missing parent-child relationship" );
			}

			return std::make_unique<GmodParsePathResult::Ok>( std::move( context.resultingPath.value() ) );
		}
		else
		{
			for ( GmodNode* ownedNode : context.ownedNodesForCurrentPath )
				delete ownedNode;

			return std::make_unique<GmodParsePathResult::Err>( "Failed to find path after traversal for item '" + std::string( item ) + "'." );
		}
	}

	std::unique_ptr<GmodParsePathResult> GmodPath::parseFullPathInternal( std::string_view item, const Gmod& gmod, const Locations& locations )
	{
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
			return std::make_unique<GmodParsePathResult::Err>( std::string( "GmodPath::parseFullPathInternal: Item string is empty or all whitespace." ) );
		}

		std::string_view rootNodeCode = gmod.rootNode().code();
		if ( item.length() < rootNodeCode.length() || item.substr( 0, rootNodeCode.length() ) != rootNodeCode )
		{
			return std::make_unique<GmodParsePathResult::Err>(
				"GmodPath::parseFullPathInternal: Path must start with root node code '" +
				std::string( rootNodeCode ) + "'. Path was: '" + std::string( item ) + "'" );
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

			if ( nodeCode.empty() )
			{
				return std::make_unique<GmodParsePathResult::Err>(
					fmt::format( "GmodPath::parseFullPathInternal: Path string '{}' contains an empty segment.", item ) );
			}

			size_t dashIndex = nodeCode.find( '-' );
			const GmodNode* tempNode = nullptr;

			if ( dashIndex == std::string_view::npos )
			{
				if ( !gmod.tryGetNode( nodeCode, tempNode ) || !tempNode )
				{
					return std::make_unique<GmodParsePathResult::Err>(
						fmt::format( "Failed to get GmodNode for {}", std::string( nodeCode ) ) );
				}

				parsedNodes.push_back( *tempNode );
			}
			else
			{
				std::string_view codepart = nodeCode.substr( 0, dashIndex );
				if ( !gmod.tryGetNode( codepart, tempNode ) || !tempNode )
				{
					return std::make_unique<GmodParsePathResult::Err>(
						fmt::format( "Failed to get GmodNode for {}", std::string( codepart ) ) );
				}
				std::string_view location = nodeCode.substr( dashIndex + 1 );
				Location parsedLocation;
				if ( !locations.tryParse( location, parsedLocation ) )
				{
					return std::make_unique<GmodParsePathResult::Err>(
						fmt::format( "Failed to parse location - {}", std::string( location ) ) );
				}

				parsedNodes.push_back( tempNode->tryWithLocation( std::make_optional( parsedLocation ) ) );
			}
		}

		if ( parsedNodes.empty() )
		{
			return std::make_unique<GmodParsePathResult::Err>(
				"GmodPath::parseFullPathInternal: Failed to find any nodes in path string '" + std::string( item ) + "'." );
		}

		GmodNode tempEndNode = parsedNodes.back();
		parsedNodes.pop_back();

		std::vector<GmodNode*> tempParentsForValidation;
		tempParentsForValidation.reserve( parsedNodes.size() );
		for ( GmodNode& pv : parsedNodes )
		{
			tempParentsForValidation.push_back( &pv );
		}

		int missingLinkAt;
		if ( !GmodPath::isValid( tempParentsForValidation, tempEndNode, missingLinkAt ) )
		{
			return std::make_unique<GmodParsePathResult::Err>(
				"GmodPath::parseFullPathInternal: Sequence of nodes is invalid. Path: '" + std::string( item ) + "'" );
		}

		internal::LocationSetsVisitor locationVisitor;
		std::optional<size_t> previousNonNullLocationIdx;

		std::array<std::pair<int, int>, 16> setsArray;
		int setCounter = 0;

		std::vector<GmodNode*> tempParentsForVisit;
		tempParentsForVisit.reserve( parsedNodes.size() );
		for ( GmodNode& node : parsedNodes )
		{
			tempParentsForVisit.push_back( &node );
		}

		for ( size_t i = 0; i < parsedNodes.size() + 1; ++i )
		{
			GmodNode& nodeForVisit = ( i < parsedNodes.size() ) ? parsedNodes[i] : tempEndNode;

			std::optional<std::tuple<size_t, size_t, std::optional<Location>>> setDetails =
				locationVisitor.visit( nodeForVisit, i, tempParentsForVisit, tempEndNode );

			if ( !setDetails.has_value() )
			{
				if ( !previousNonNullLocationIdx.has_value() && nodeForVisit.location().has_value() )
				{
					previousNonNullLocationIdx = i;
				}
				continue;
			}

			const auto& setTuple = setDetails.value();
			size_t setStartIdx = std::get<0>( setTuple );
			size_t setEndIdx = std::get<1>( setTuple );
			std::optional<Location> setCommonLocation = std::get<2>( setTuple );

			if ( previousNonNullLocationIdx.has_value() )
			{
				for ( size_t j = previousNonNullLocationIdx.value(); j < setStartIdx; ++j )
				{
					GmodNode& previousNode = ( j < parsedNodes.size() )
												 ? parsedNodes[j]
												 : tempEndNode;

					if ( previousNode.location().has_value() )
					{
						return std::make_unique<GmodParsePathResult::Err>(
							fmt::format( "Expected all nodes in the set to be without individualization. Found {}", std::string( previousNode.code() ) ) );
					}
				}
			}
			previousNonNullLocationIdx.reset();

			if ( setCounter < 16 )
			{
				setsArray[static_cast<size_t>( setCounter++ )] = { static_cast<int>( setStartIdx ), static_cast<int>( setEndIdx ) };
			}
			else
			{
				throw std::out_of_range( fmt::format( "Exceeded maximum of 16 location sets supported while parsing path: '{}'", std::string( item ) ) );
			}

			if ( setStartIdx == setEndIdx )
				continue;

			for ( size_t j = setStartIdx; j <= setEndIdx; ++j )
			{
				if ( j < parsedNodes.size() )
				{
					parsedNodes[j] = parsedNodes[j].tryWithLocation( setCommonLocation );
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
					throw std::logic_error(
						fmt::format( "Internal logic error: currentSetIndex ({}) is out of bounds for setsArray (size {}). Path: '{}'", currentSetIndex, setsArray.size(), std::string( item ) ) );
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

					return std::make_unique<GmodParsePathResult::Err>(
						"GmodPath::parseFullPathInternal: Expected nodes in set to be individualized the same. Found " +
						std::string( nodeCheck.code().data() ) + " with location " +
						formatLocation( nodeCheck.location() ) + ", expected " +
						formatLocation( expectedLocationNode ? expectedLocationNode->location() : std::optional<Location>() ) );
				}
			}
			else
			{
				if ( nodeCheck.location().has_value() )
				{
					return std::make_unique<GmodParsePathResult::Err>(
						fmt::format( "Expected all nodes outside set to be without individualization. Found {}", std::string( nodeCheck.code() ) ) );
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

		return std::make_unique<GmodParsePathResult::Ok>( std::move( *finalPath ) );
	}

	//----------------------------------------------
	// GmodPath enumerator
	//----------------------------------------------

	//----------------------------
	// Construction / destruction
	//----------------------------

	GmodPath::Enumerator::Enumerator( const GmodPath* pathInst, size_t startIndex )
		: m_pathInstance( pathInst ),
		  m_currentIndex( std::numeric_limits<size_t>::max() ),
		  m_current( std::numeric_limits<size_t>::max(), nullptr )
	{
		if ( startIndex != std::numeric_limits<size_t>::max() )
		{
			if ( startIndex > 0 )
			{
				m_currentIndex = startIndex - 1;
			}
		}
	}

	//----------------------------
	// Enumeration methods
	//----------------------------

	bool GmodPath::Enumerator::next()
	{
		if ( !m_pathInstance )
		{
			return false;
		}

		if ( m_currentIndex == std::numeric_limits<size_t>::max() )
		{
			m_currentIndex = 0;
		}
		else
		{
			m_currentIndex++;
		}

		return m_currentIndex < m_pathInstance->length();
	}

	const GmodPath::Enumerator::PathElement& GmodPath::Enumerator::current() const
	{
		if ( !m_pathInstance || m_currentIndex >= m_pathInstance->length() )
		{
			static PathElement invalidElement{ std::numeric_limits<size_t>::max(), nullptr };
			return invalidElement;
		}

		m_current.first = m_currentIndex;
		m_current.second = ( *m_pathInstance )[m_currentIndex];
		return m_current;
	}

	void GmodPath::Enumerator::reset()
	{
		m_currentIndex = std::numeric_limits<size_t>::max();
		m_current = { std::numeric_limits<size_t>::max(), nullptr };
	}

	//=====================================================================
	// GmodIndividualizableSet class
	//=====================================================================

	GmodIndividualizableSet::GmodIndividualizableSet( const std::vector<int>& nodeIndices, const GmodPath& sourcePath )
		: m_nodeIndices{ nodeIndices },
		  m_path{ sourcePath },
		  m_isBuilt{ false }
	{
		if ( m_nodeIndices.empty() )
		{
			throw std::invalid_argument( "GmodIndividualizableSet constructor: nodeIndices cannot be empty." );
		}

		for ( int nodeIdx : m_nodeIndices )
		{
			if ( static_cast<size_t>( nodeIdx ) >= m_path.length() || nodeIdx < 0 )
			{
				throw std::out_of_range( "GmodIndividualizableSet constructor: Node index " +
										 std::to_string( nodeIdx ) + " is out of bounds for path length " +
										 std::to_string( m_path.length() ) + "." );
			}

			GmodNode* currentNode = m_path[static_cast<size_t>( nodeIdx )];
			if ( !currentNode )
			{
				throw std::runtime_error( "GmodIndividualizableSet constructor: Node at index " +
										  std::to_string( nodeIdx ) + " is null in the source path." );
			}

			bool isTargetNode = ( static_cast<size_t>( nodeIdx ) == m_path.length() - 1 );
			bool isInSet = ( m_nodeIndices.size() > 1 );
			if ( !currentNode->isIndividualizable( isTargetNode, isInSet ) )
			{
				throw std::invalid_argument( "GmodIndividualizableSet constructor: Node '" +
											 std::string( currentNode->code().data() ) + "' (at index " +
											 std::to_string( nodeIdx ) + ") is not individualizable in the given context." );
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
					throw std::invalid_argument( "GmodIndividualizableSet constructor: Nodes have different locations. Node '" +
												 std::string( currentNode->code().data() ) + "' (at index " +
												 std::to_string( currentIdx ) + ") has location while first node in set had different or no location." );
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
			throw std::invalid_argument( "GmodIndividualizableSet constructor: No nodes in the set are part of the short path (final node or leaf node)." );
		}
	}

	//----------------------------------------------
	// Build
	//----------------------------------------------

	GmodPath GmodIndividualizableSet::build()
	{
		if ( m_isBuilt )
		{
			throw std::runtime_error( "Tried to build individualizable set twice" );
		}

		m_isBuilt = true;
		return std::move( m_path );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	std::vector<GmodNode*> GmodIndividualizableSet::nodes() const
	{
		std::vector<GmodNode*> result;
		result.reserve( m_nodeIndices.size() );

		for ( int nodeIdx : m_nodeIndices )
		{
			if ( nodeIdx < 0 || static_cast<size_t>( nodeIdx ) >= m_path.length() )
			{
				throw std::out_of_range( "Node index out of bounds" );
			}

			GmodNode* node = m_path[static_cast<size_t>( nodeIdx )];
			result.push_back( node );
		}

		return result;
	}

	const std::vector<int>& GmodIndividualizableSet::nodeIndices() const noexcept
	{
		return m_nodeIndices;
	}

	std::optional<Location> GmodIndividualizableSet::location() const
	{
		if ( m_nodeIndices.empty() )
		{
			return std::nullopt;
		}

		int firstNodeIdx = m_nodeIndices[0];
		if ( firstNodeIdx < 0 || static_cast<size_t>( firstNodeIdx ) >= m_path.length() )
		{
			return std::nullopt;
		}

		const GmodNode* firstNode = m_path[static_cast<size_t>( firstNodeIdx )];
		if ( !firstNode )
		{
			return std::nullopt;
		}

		return firstNode->location();
	}

	void GmodIndividualizableSet::setLocation( const std::optional<Location>& location )
	{
		for ( size_t i = 0; i < m_nodeIndices.size(); ++i )
		{
			int nodeIdx = m_nodeIndices[i];

			if ( nodeIdx < 0 || static_cast<size_t>( nodeIdx ) >= m_path.length() )
			{
				continue;
			}

			GmodNode*& nodeInPath = m_path[static_cast<size_t>( nodeIdx )];
			if ( !nodeInPath )
			{
				continue;
			}

			if ( !location.has_value() )
			{
				GmodNode newNode = nodeInPath->withoutLocation();
				*nodeInPath = std::move( newNode );
			}
			else
			{
				GmodNode newNode = nodeInPath->withLocation( location.value() );
				*nodeInPath = std::move( newNode );
			}
		}
	}

	std::string GmodIndividualizableSet::toString() const
	{
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
			}
		}
		std::string result = ss.str();
		return result;
	}

	//=====================================================================
	// GmodParsePathResult class
	//=====================================================================

	GmodParsePathResult::Ok::Ok( GmodPath path )
		: GmodParsePathResult{},
		  path{ std::move( path ) }
	{
	}

	GmodParsePathResult::Err::Err( std::string errorString )
		: GmodParsePathResult{},
		  error{ std::move( errorString ) }
	{
	}
}
