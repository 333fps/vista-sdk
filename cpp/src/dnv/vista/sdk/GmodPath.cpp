#include "pch.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	namespace
	{
		std::string buildNodeStringInternal( const GmodNode* node, bool includeNameIfPresent = true )
		{
			if ( !node )
			{
				return "";
			}
			std::string str = node->code();

			if ( includeNameIfPresent && !node->metadata().name().empty() )
			{
				str += "[" + node->metadata().name() + "]";
			}
			return str;
		}

		std::string buildLocationStringInternal( const std::optional<Location>& loc )
		{
			if ( loc.has_value() )
			{
				return "@" + loc->toString();
			}
			return "";
		}
	}

	GmodIndividualizableSet::GmodIndividualizableSet( const std::vector<int>& nodeIndices, const GmodPath& sourcePath )
		: m_nodeIndices( nodeIndices ), m_path( sourcePath )
	{
		if ( m_nodeIndices.empty() )
		{
			throw std::invalid_argument( "GmodIndividualizableSet cannot be empty" );
		}

		bool allNodesIndividualizable = true;
		for ( size_t i = 0; i < m_nodeIndices.size(); ++i )
		{
			int nodeIdxInOriginalPath = m_nodeIndices[i];
			if ( nodeIdxInOriginalPath < 0 || static_cast<size_t>( nodeIdxInOriginalPath ) >= sourcePath.length() )
			{
				throw std::out_of_range( "Node index out of range for GmodIndividualizableSet construction" );
			}
			const GmodNode* node = sourcePath[static_cast<size_t>( nodeIdxInOriginalPath )];
			if ( !node )
			{
				throw std::runtime_error( "Null node encountered in GmodIndividualizableSet construction" );
			}
			bool isLastNodeOfOriginalPath = ( static_cast<size_t>( nodeIdxInOriginalPath ) == sourcePath.length() - 1 );

			if ( !node->isIndividualizable( isLastNodeOfOriginalPath, m_nodeIndices.size() > 1 ) )
			{
				allNodesIndividualizable = false;
				break;
			}
		}
		if ( !allNodesIndividualizable )
		{
			throw std::invalid_argument( "GmodIndividualizableSet nodes must be individualizable" );
		}

		if ( !m_nodeIndices.empty() )
		{
			if ( static_cast<size_t>( m_nodeIndices[0] ) >= sourcePath.length() )
				throw std::out_of_range( "Initial node index out of range for location check." );
			std::optional<Location> firstLocation = sourcePath[static_cast<size_t>( m_nodeIndices[0] )]->location();
			for ( size_t i = 1; i < m_nodeIndices.size(); ++i )
			{
				int nodeIdxInOriginalPath = m_nodeIndices[i];
				if ( static_cast<size_t>( nodeIdxInOriginalPath ) >= sourcePath.length() )
					throw std::out_of_range( "Node index out of range for location check." );

				if ( sourcePath[static_cast<size_t>( nodeIdxInOriginalPath )]->location() != firstLocation )
				{
					throw std::invalid_argument( "GmodIndividualizableSet nodes have different locations" );
				}
			}
		}

		bool partOfShortPathFound = false;
		for ( int nodeIdxInOriginalPath : m_nodeIndices )
		{
			if ( static_cast<size_t>( nodeIdxInOriginalPath ) >= sourcePath.length() )
				throw std::out_of_range( "Node index out of range for short path check." );
			const GmodNode* node = sourcePath[static_cast<size_t>( nodeIdxInOriginalPath )];
			if ( node == sourcePath.node() || node->isLeafNode() )
			{
				partOfShortPathFound = true;
				break;
			}
		}
		if ( !partOfShortPathFound )
		{
			throw std::invalid_argument( "GmodIndividualizableSet has no nodes that are part of short path" );
		}
	}

	std::vector<GmodNode*> GmodIndividualizableSet::nodes() const
	{
		std::vector<GmodNode*> resultNodes;
		resultNodes.reserve( m_nodeIndices.size() );
		for ( int indexInCopiedPath : m_nodeIndices )
		{
			if ( indexInCopiedPath < 0 || static_cast<size_t>( indexInCopiedPath ) >= m_path.length() )
			{
				throw std::out_of_range( "Node index out of range in GmodIndividualizableSet::nodes" );
			}
			resultNodes.push_back( m_path[static_cast<size_t>( indexInCopiedPath )] );
		}
		return resultNodes;
	}

	const std::vector<int>& GmodIndividualizableSet::nodeIndices() const noexcept
	{
		return m_nodeIndices;
	}

	std::optional<Location> GmodIndividualizableSet::lLocation() const
	{
		if ( m_nodeIndices.empty() || static_cast<size_t>( m_nodeIndices[0] ) >= m_path.length() )
		{
			return std::nullopt;
		}
		return m_path[static_cast<size_t>( m_nodeIndices[0] )]->location();
	}

	void GmodIndividualizableSet::setLocation( const std::optional<Location>& location )
	{
		for ( int nodeIndexInSet : m_nodeIndices )
		{
			if ( nodeIndexInSet < 0 || static_cast<size_t>( nodeIndexInSet ) >= m_path.length() )
			{
				throw std::out_of_range( "Node index out of range in GmodIndividualizableSet::setLocation" );
			}

			GmodNode*& nodePtrRef = m_path[static_cast<size_t>( nodeIndexInSet )];
			GmodNode* oldNodePtr = nodePtrRef;

			if ( !oldNodePtr )
			{
				SPDLOG_WARN( "Old node pointer is null in GmodIndividualizableSet::setLocation for index {}", nodeIndexInSet );
				continue;
			}

			GmodNode newGmodNodeValue = oldNodePtr->tryWithLocation( location );

			GmodNode* newHeapAllocatedNode = new GmodNode( newGmodNodeValue );

			nodePtrRef = newHeapAllocatedNode;
		}
	}

	std::string GmodIndividualizableSet::toString() const
	{
		return m_path.toString();
	}

	GmodPath::GmodPath( const Gmod& gmod, GmodNode* node, std::vector<GmodNode*> parents )
		: m_visVersion{ node->visVersion() },
		  m_gmod{ gmod },
		  m_parents{ std::move( parents ) },
		  m_node{ node }
	{
		if ( !m_node )
		{
			throw std::invalid_argument( "GmodPath final node cannot be null" );
		}
	}

	GmodPath::GmodPath( const GmodPath& other )
		: m_visVersion{ other.m_visVersion },
		  m_gmod{ other.m_gmod },
		  m_parents{ other.m_parents },
		  m_node{ other.m_node },
		  m_ownedNodes{ other.m_ownedNodes }
	{
	}

	GmodPath::GmodPath( GmodPath&& other ) noexcept
		: m_visVersion{ other.m_visVersion },
		  m_gmod{ other.m_gmod },
		  m_parents{ std::move( other.m_parents ) },
		  m_node{ other.m_node },
		  m_ownedNodes{ std::move( other.m_ownedNodes ) }
	{
		other.m_node = nullptr;
	}

	GmodPath& GmodPath::operator=( const GmodPath& other )
	{
		if ( this != &other )
		{
			m_parents = other.m_parents;
			m_node = other.m_node;
			m_ownedNodes = other.m_ownedNodes;
			m_visVersion = other.m_visVersion;
		}
		return *this;
	}

	GmodPath& GmodPath::operator=( GmodPath&& other ) noexcept
	{
		if ( this != &other )
		{
			m_parents = std::move( other.m_parents );
			m_node = other.m_node;
			m_ownedNodes = std::move( other.m_ownedNodes );
			m_visVersion = other.m_visVersion;

			other.m_node = nullptr;
		}
		return *this;
	}

	bool GmodPath::operator==( const GmodPath& other ) const noexcept
	{
		if ( this == &other )
		{
			return true;
		}

		if ( &m_gmod != &other.m_gmod )
		{
			return false;
		}

		if ( ( m_node == nullptr && other.m_node != nullptr ) || ( m_node != nullptr && other.m_node == nullptr ) )
		{
			return false;
		}
		if ( m_node != nullptr && other.m_node != nullptr && m_node->code() != other.m_node->code() )
		{
			return false;
		}

		if ( m_parents.size() != other.m_parents.size() )
		{
			return false;
		}
		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			GmodNode* p1 = m_parents[i];
			GmodNode* p2 = other.m_parents[i];
			if ( ( p1 == nullptr && p2 != nullptr ) || ( p1 != nullptr && p2 == nullptr ) )
			{
				return false;
			}
			if ( p1 != nullptr && p2 != nullptr && p1->code() != p2->code() )
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

	VisVersion GmodPath::visVersion() const noexcept
	{
		return m_visVersion;
	}

	size_t GmodPath::hashCode() const noexcept
	{
		size_t hash = 0;
		if ( m_node != nullptr )
		{
			hash ^= std::hash<std::string>{}( m_node->code() ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}

		for ( const auto* parent_node : m_parents )
		{
			if ( parent_node != nullptr )
			{
				hash ^= std::hash<std::string>{}( parent_node->code() ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
			}
		}

		return hash;
	}

	const Gmod& GmodPath::gmod() const noexcept
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

	GmodNode* GmodPath::operator[]( size_t index ) const
	{
		if ( index < m_parents.size() )
		{
			return m_parents[index];
		}
		if ( index == m_parents.size() && m_node != nullptr )
		{
			return m_node;
		}
		throw std::out_of_range( "GmodPath index out of bounds" );
	}

	GmodNode*& GmodPath::operator[]( size_t index )
	{
		if ( index < m_parents.size() )
		{
			return m_parents[index];
		}
		if ( index == m_parents.size() && m_node != nullptr )
		{
			return m_node;
		}
		throw std::out_of_range( "GmodPath non-const operator[]: Index out of bounds" );
	}

	size_t GmodPath::length() const noexcept
	{
		return m_parents.size() + ( m_node != nullptr ? 1 : 0 );
	}

	GmodNode* GmodPath::rootNode() const noexcept
	{
		return m_parents.empty() ? ( length() >= 1 ? m_node : nullptr ) : m_parents.front();
	}

	GmodNode* GmodPath::parentNode() const noexcept
	{
		if ( m_node && !m_parents.empty() )
			return m_parents.back();
		return nullptr;
	}

	std::vector<GmodIndividualizableSet> GmodPath::individualizableNodes() const
	{
		std::vector<GmodIndividualizableSet> sets;
		return sets;
	}

	std::vector<std::pair<size_t, std::string>> GmodPath::commonNames() const
	{
		std::vector<std::pair<size_t, std::string>> result_vector;

		for ( const auto& depth_node_pair : this->fullPath() )
		{
			size_t current_depth = static_cast<size_t>( depth_node_pair.first );
			const GmodNode& iterated_node = depth_node_pair.second.get();

			bool is_path_end_node = ( current_depth == m_parents.size() );

			if ( ( !iterated_node.isLeafNode() && !is_path_end_node ) || !iterated_node.isFunctionNode() )
			{
				continue;
			}

			std::string name_str;
			std::optional<std::string> common_name_opt = iterated_node.metadata().commonName();
			if ( common_name_opt.has_value() && !common_name_opt.value().empty() )
			{
				name_str = common_name_opt.value();
			}
			else
			{
				name_str = iterated_node.metadata().name();
			}

			const auto& iterated_node_nan_map = iterated_node.metadata().normalAssignmentNames();

			if ( !iterated_node_nan_map.empty() )
			{
				std::string name_from_nan = name_str;
				bool nan_applied = false;

				if ( this->m_node )
				{
					auto it_path_end_node = iterated_node_nan_map.find( this->m_node->code() );
					if ( it_path_end_node != iterated_node_nan_map.end() )
					{
						name_from_nan = it_path_end_node->second;
						nan_applied = true;
					}
				}

				for ( int i = static_cast<int>( m_parents.size() ) - 1; i >= static_cast<int>( current_depth ); --i )
				{
					if ( static_cast<size_t>( i ) < m_parents.size() && m_parents[static_cast<size_t>( i )] )
					{
						auto it_path_parent_node = iterated_node_nan_map.find( m_parents[static_cast<size_t>( i )]->code() );
						if ( it_path_parent_node != iterated_node_nan_map.end() )
						{
							name_from_nan = it_path_parent_node->second;
							nan_applied = true;
						}
					}
				}

				if ( nan_applied )
				{
					name_str = name_from_nan;
				}
			}
			result_vector.emplace_back( current_depth, name_str );
		}
		return result_vector;
	}

	std::string GmodPath::fullPathString() const
	{
		std::stringstream ss;
		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			ss << buildNodeStringInternal( m_parents[i] );
			ss << "/";
		}
		if ( m_node )
		{
			ss << buildNodeStringInternal( m_node );
		}
		return ss.str();
	}

	std::vector<std::pair<int, std::reference_wrapper<const GmodNode>>> GmodPath::fullPath() const
	{
		std::vector<std::pair<int, std::reference_wrapper<const GmodNode>>> result;
		result.reserve( m_parents.size() + ( m_node ? 1 : 0 ) );
		int depth = 0;
		for ( GmodNode* parent_node : m_parents )
		{
			if ( parent_node )
			{
				result.emplace_back( depth++, *parent_node );
			}
		}
		if ( m_node )
		{
			result.emplace_back( depth, *m_node );
		}
		return result;
	}

	std::string GmodPath::shortPath() const
	{
		return fullPathString();
	}

	std::string GmodPath::fullPathWithLocation() const
	{
		std::stringstream ss;
		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			ss << buildNodeStringInternal( m_parents[i] );
			ss << buildLocationStringInternal( m_parents[i]->location() );
			ss << "/";
		}
		if ( m_node )
		{
			ss << buildNodeStringInternal( m_node );
			ss << buildLocationStringInternal( m_node->location() );
		}
		return ss.str();
	}

	std::string GmodPath::shortPathWithLocation() const
	{
		return fullPathWithLocation();
	}

	std::string GmodPath::toString() const
	{
		return fullPathString();
	}

	std::string GmodPath::toFullPathString() const
	{
		std::string builder;

		toFullPathString( builder );
		return builder;
	}

	void GmodPath::toFullPathString( std::string& builder ) const
	{
		builder.clear();

		const size_t num_parents = m_parents.size();
		const bool has_final_node = ( m_node != nullptr );
		const size_t total_path_elements = num_parents + ( has_final_node ? 1 : 0 );

		if ( total_path_elements == 0 )
		{
			return;
		}

		auto append_node_representation = [&]( const GmodNode* node_to_append ) {
			if ( !node_to_append )
				return;

			builder.append( node_to_append->code() );
			const auto& loc = node_to_append->location();
			if ( loc.has_value() )
			{
				builder.append( "-" );
				builder.append( loc.value().toString() );
			}
		};

		for ( size_t i = 0; i < num_parents; ++i )
		{
			append_node_representation( m_parents[i] );

			if ( i < total_path_elements - 1 )
			{
				builder.append( "/" );
			}
		}

		if ( has_final_node )
		{
			append_node_representation( m_node );
		}
	}

	GmodPath GmodPath::parse( const Gmod& gmod, std::string_view pathString )
	{
		if ( pathString.empty() )
			throw std::invalid_argument( "Path string cannot be empty for parse." );

		const GmodNode* rootNodePtr = nullptr;
		if ( !gmod.tryGetNode( std::string_view( "VE" ), rootNodePtr ) || !rootNodePtr )
		{
			throw std::runtime_error( "Cannot parse path, GMOD does not contain VE or VE is null." );
		}
		if ( pathString == "VE" )
		{
			return GmodPath( gmod, const_cast<GmodNode*>( rootNodePtr ), {} );
		}

		/* TODO: Fully implement parsing for other path strings. */
		throw std::runtime_error( "GmodPath::parse is not fully implemented beyond VE." );
	}

	bool GmodPath::tryParse( const Gmod& gmod, std::string_view pathString, GmodPath& outPath )
	{
		try
		{
			outPath = parse( gmod, pathString );
			return true;
		}
		catch ( const std::exception& )
		{
			return false;
		}
	}

	GmodPath GmodPath::parseFromFullPath( const Gmod& gmod, std::string_view fullPathString )
	{
		if ( fullPathString.rfind( "VE", 0 ) != 0 )
		{
			throw std::invalid_argument( "Full path must start with VE." );
		}
		return parse( gmod, fullPathString );
	}

	bool GmodPath::tryParseFromFullPath( const Gmod& gmod, std::string_view fullPathString, GmodPath& outPath )
	{
		try
		{
			outPath = parseFromFullPath( gmod, fullPathString );
			return true;
		}
		catch ( const std::exception& )
		{
			return false;
		}
	}

	bool GmodPath::isValid( const std::vector<const GmodNode*>& parents, const GmodNode& node, size_t& missingLinkAt )
	{
		missingLinkAt = 0;
		if ( parents.empty() )
		{
			return node.isRoot();
		}

		const GmodNode* currentParent = parents[0];
		if ( !currentParent->isRoot() )
		{
			missingLinkAt = 0;
			return false;
		}

		for ( size_t i = 1; i < parents.size(); ++i )
		{
			const GmodNode* child = parents[i];
			bool found = false;
			if ( currentParent != nullptr && child != nullptr )
			{
				for ( const auto* pChild : currentParent->children() )
				{
					if ( pChild == child )
					{
						found = true;
						break;
					}
				}
			}
			if ( !found )
			{
				missingLinkAt = i;
				return false;
			}
			currentParent = child;
		}

		bool lastParentConnectedToNode = false;
		if ( currentParent != nullptr )
		{
			for ( const auto* pChild : currentParent->children() )
			{
				if ( pChild == &node )
				{
					lastParentConnectedToNode = true;
					break;
				}
			}
		}
		if ( !lastParentConnectedToNode )
		{
			missingLinkAt = parents.size();
			return false;
		}
		return true;
	}

	bool GmodPath::isValid( const std::vector<const GmodNode*>& parents, const GmodNode& node )
	{
		size_t missingLinkAt;
		return isValid( parents, node, missingLinkAt );
	}

	GmodPath::Enumerator::Enumerator( const GmodPath* pathInst )
		: m_pathInstance( pathInst ), m_currentIndex( 0 ) {}

	GmodNode* GmodPath::Enumerator::current() const
	{
		if ( !m_pathInstance || m_currentIndex == 0 || m_currentIndex > m_pathInstance->length() )
		{
			throw std::out_of_range( "Enumerator current() called in invalid state." );
		}
		return ( *m_pathInstance )[m_currentIndex - 1];
	}

	bool GmodPath::Enumerator::next()
	{
		if ( !m_pathInstance || m_currentIndex >= m_pathInstance->length() )
		{
			return false;
		}
		m_currentIndex++;
		return true;
	}

	void GmodPath::Enumerator::reset()
	{
		m_currentIndex = 0;
	}

	GmodPath::Enumerator GmodPath::enumerator() const
	{
		return Enumerator( this );
	}
}
