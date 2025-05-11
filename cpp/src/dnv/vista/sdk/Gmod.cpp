#include "pch.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/VisVersion.h"
#include <algorithm>

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

	//=====================================================================
	// Gmod Class
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	Gmod::Gmod( VisVersion version, const GmodDto& dto )
		: m_visVersion{ version },
		  m_rootNode{ nullptr }
	{
		SPDLOG_CRITICAL( "Gmod constructor: Starting for VIS version {}", dto.visVersion() );

		std::vector<std::pair<std::string, GmodNode>> nodePairs;
		nodePairs.reserve( dto.items().size() );
		for ( const auto& nodeDto : dto.items() )
		{
			nodePairs.emplace_back( nodeDto.code(), GmodNode( version, nodeDto ) );
		}

		SPDLOG_INFO( "Gmod constructor: Created {} initial GmodNode objects for VIS version {}.", nodePairs.size(), VisVersionExtensions::toVersionString( version ) );

		m_nodeMap = ChdDictionary<GmodNode>( std::move( nodePairs ) );
		SPDLOG_INFO( "Gmod constructor: ChdDictionary m_nodeMap constructed with {} items.", m_nodeMap.size() );

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
			std::vector<std::string> nodeKeysToTrim;
			nodeKeysToTrim.reserve( m_nodeMap.size() );
			for ( auto it_keys = m_nodeMap.begin(); it_keys != m_nodeMap.end(); ++it_keys )
			{
				nodeKeysToTrim.emplace_back( it_keys->first );
			}

			for ( const auto& key : nodeKeysToTrim )
			{
				try
				{
					m_nodeMap[key].trim();
				}
				catch ( [[maybe_unused]] const std::exception& ex )
				{
					SPDLOG_ERROR( "Gmod constructor (trimming): Exception while trimming node '{}'. Error: {}", key, ex.what() );
				}
			}
		}
		SPDLOG_INFO( "Gmod constructor: Trimming nodes complete." );

		SPDLOG_INFO( "Gmod constructor: Initializing m_rootNode..." );
		try
		{
			m_rootNode = &m_nodeMap["VE"];

			SPDLOG_INFO( "Gmod constructor: m_rootNode initialized from m_nodeMap, code: '{}'", m_rootNode->code() );
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
			VisVersionExtensions::toVersionString( version ), m_nodeMap.size(), m_rootNode->code().empty() ? "UNINITIALIZED OR MISSING" : m_rootNode->code() );
	}

	Gmod::Gmod( VisVersion version, const std::unordered_map<std::string, GmodNode>& nodeMap )
		: m_visVersion{ version },
		  m_rootNode{ nullptr },
		  m_nodeMap{ [&nodeMap]() {
			  std::vector<std::pair<std::string, GmodNode>> pairs;
			  pairs.reserve( nodeMap.size() );
			  for ( const auto& [code, node] : nodeMap )
			  {
				  pairs.emplace_back( code, node );
			  }
			  return ChdDictionary<GmodNode>( std::move( pairs ) );
		  }() }
	{
		SPDLOG_CRITICAL( "Creating Gmod from existing map with {} nodes for VIS version {}.", nodeMap.size(), VisVersionExtensions::toVersionString( version ) );

		try
		{
			m_rootNode = &m_nodeMap["VE"];
			SPDLOG_INFO( "Gmod constructor from map: m_rootNode initialized from internal m_nodeMap, code: '{}'", m_rootNode->code() );
		}
		catch ( [[maybe_unused]] const std::out_of_range& oor )
		{
			SPDLOG_ERROR( "Gmod constructor from map: Root node 'VE' not found in internal m_nodeMap for VIS version {}. Error: {}", VisVersionExtensions::toVersionString( version ), oor.what() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Gmod constructor from map: Exception while initializing m_rootNode from internal m_nodeMap for VIS version {}. Error: {}", VisVersionExtensions::toVersionString( version ), ex.what() );
		}

		if ( m_rootNode == nullptr && nodeMap.count( "VE" ) )
		{
			SPDLOG_ERROR( "Failed to correctly initialize root node from provided map for VIS version {}. m_rootNode is null but VE existed in input.", VisVersionExtensions::toVersionString( version ) );
		}
		else if ( m_rootNode != nullptr && m_rootNode->code() != "VE" )
		{
			SPDLOG_ERROR( "m_rootNode code is '{}', expected 'VE'.", m_rootNode->code() );
		}

		if ( m_nodeMap.isEmpty() && !nodeMap.empty() )
		{
			SPDLOG_ERROR( "Failed to initialize node dictionary from provided map for VIS version {}. m_nodeMap is empty.", VisVersionExtensions::toVersionString( version ) );
		}
		if ( !m_nodeMap.isEmpty() && m_nodeMap.size() != nodeMap.size() )
		{
			SPDLOG_WARN( "Gmod constructor from map: m_nodeMap size ({}) does not match input nodeMap size ({}).", m_nodeMap.size(), nodeMap.size() );
		}

		SPDLOG_INFO( "Gmod constructor from map: Consider if pointer fix-up is needed for children/parents in m_nodeMap." );
	}

	//----------------------------------------------
	// Lookup Operators
	//----------------------------------------------

	const GmodNode& Gmod::operator[]( std::string_view key ) const
	{
		const GmodNode* nodePtr = nullptr;
		if ( m_nodeMap.tryGetValue( key, nodePtr ) && nodePtr != nullptr )
		{
			return *nodePtr;
		}
		throw std::out_of_range( "Key not found in Gmod node map: " + std::string( key ) );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	VisVersion Gmod::visVersion() const
	{
		return m_visVersion;
	}

	const GmodNode& Gmod::rootNode() const
	{
		if ( !m_rootNode )
		{
			SPDLOG_ERROR( "Gmod::rootNode() called when m_rootNode is null!" );
			throw std::runtime_error( "Root node is not initialized or 'VE' was not found." );
		}
		return *m_rootNode;
	}

	bool Gmod::isEmpty() const
	{
		return m_nodeMap.size() == 0;
	}

	//----------------------------------------------
	// Node Query Methods
	//----------------------------------------------

	bool Gmod::tryGetNode( std::string_view code, const GmodNode*& node ) const
	{
		node = nullptr;
		try
		{
			if ( code.empty() )
			{
				SPDLOG_WARN( "TryGetNode: Attempted to look up empty node code" );
				return false;
			}

			if ( !m_nodeMap.tryGetValue( code, node ) )
			{
				SPDLOG_WARN( "TryGetNode: Node '{}' not found in GMOD", code );
				return false;
			}

			if ( node == nullptr )
			{
				SPDLOG_ERROR( "TryGetNode: m_nodeMap.tryGetValue succeeded but outNodePtr is null for code '{}'", code );
				return false;
			}

			SPDLOG_INFO( "TryGetNode: Node '{}' found in GMOD", code );
			return true;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception in TryGetNode for '{}': {}", code, ex.what() );
			node = nullptr;
			return false;
		}
	}

	bool Gmod::tryGetNode( const std::string& code, const GmodNode*& node ) const
	{
		return m_nodeMap.tryGetValue( std::string_view( code ), node );
	}

	//----------------------------------------------
	// Path Parsing & Navigation
	//----------------------------------------------

	GmodPath Gmod::parsePath( const std::string& item ) const
	{
		return GmodPath::parse( item, m_visVersion, *this );
	}

	bool Gmod::tryParsePath( const std::string& item, std::optional<GmodPath>& path ) const
	{
		return GmodPath::tryParse( item, m_visVersion, *this, path );
	}

	GmodPath Gmod::parseFromFullPath( const std::string& item ) const
	{
		return GmodPath::parseFullPath( item, m_visVersion, *this );
	}

	bool Gmod::tryParseFromFullPath( const std::string& item, std::optional<GmodPath>& path ) const
	{
		return GmodPath::tryParseFullPath( item, m_visVersion, *this, path );
	}

	//----------------------------------------------
	// Static Utility Methods
	//----------------------------------------------

	bool Gmod::isPotentialParent( const std::string& type )
	{
		return s_potentialParentScopeTypes.find( type ) != s_potentialParentScopeTypes.end();
	}

	bool Gmod::isLeafNode( const GmodNodeMetadata& metadata )
	{
		return s_leafTypesSet.find( metadata.fullType() ) != s_leafTypesSet.end();
	}

	bool Gmod::isFunctionNode( const GmodNodeMetadata& metadata )
	{
		return metadata.category() == NODE_CATEGORY_ASSET_FUNCTION || metadata.category() == NODE_CATEGORY_VALUE_FUNCTION;
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

	bool Gmod::isProductTypeAssignment( const GmodNode* parent, const GmodNode* child )
	{
		if ( parent == nullptr || child == nullptr )
			return false;
		if ( parent->metadata().category().find( NODE_CATEGORY_VALUE_FUNCTION ) == std::string::npos )
			return false;
		if ( child->metadata().category() != NODE_CATEGORY_PRODUCT || child->metadata().type() != NODE_TYPE_VALUE_TYPE )
			return false;
		return true;
	}

	bool Gmod::isProductSelectionAssignment( const GmodNode* parent, const GmodNode* child )
	{
		if ( parent == nullptr || child == nullptr )
			return false;
		if ( parent->metadata().category().find( NODE_CATEGORY_VALUE_FUNCTION ) == std::string::npos )
			return false;
		if ( child->metadata().category().find( NODE_CATEGORY_PRODUCT ) == std::string::npos || child->metadata().type() != NODE_TYPE_SELECTION )
			return false;
		return true;
	}

	//----------------------------------------------
	// Iteration
	//----------------------------------------------

	Gmod::Enumerator Gmod::enumerator() const
	{
		return Enumerator( &m_nodeMap );
	}

	//----------------------------------------------
	// Gmod::Enumerator Class
	//----------------------------------------------

	//-----------------------------
	// Construction / Destruction
	//-----------------------------

	Gmod::Enumerator::Enumerator( const ChdDictionary<GmodNode>* map )
		: m_sourceMapPtr( map ), m_isInitialState( true )
	{
		if ( m_sourceMapPtr )
		{
			m_currentMapIterator = m_sourceMapPtr->begin();
		}
	}

	//-----------------------------
	// Iteration Interface
	//-----------------------------

	const GmodNode& Gmod::Enumerator::current() const
	{
		if ( !m_sourceMapPtr || m_isInitialState || m_currentMapIterator == m_sourceMapPtr->end() )
		{
			throw std::out_of_range( "Gmod::Enumerator::getCurrent() called in an invalid state or past the end." );
		}
		return m_currentMapIterator->second;
	}

	bool Gmod::Enumerator::next()
	{
		if ( !m_sourceMapPtr || m_sourceMapPtr->isEmpty() )
		{
			m_isInitialState = false;
			return false;
		}

		if ( m_isInitialState )
		{
			m_isInitialState = false;
			return m_currentMapIterator != m_sourceMapPtr->end();
		}

		if ( m_currentMapIterator != m_sourceMapPtr->end() )
		{
			++m_currentMapIterator;
			return m_currentMapIterator != m_sourceMapPtr->end();
		}
		return false;
	}

	void Gmod::Enumerator::reset()
	{
		m_isInitialState = true;
		if ( m_sourceMapPtr )
		{
			m_currentMapIterator = m_sourceMapPtr->begin();
		}
	}
}
