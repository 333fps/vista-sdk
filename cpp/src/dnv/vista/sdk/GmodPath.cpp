#include "pch.h"

#include "dnv/vista/sdk/GmodPath.h"

#include "dnv/vista/sdk/Codebook.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/Gmodnode.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	std::optional<std::tuple<size_t, size_t, std::optional<Location>>> LocationSetsVisitor::visit(
		const GmodNode& node,
		size_t i,
		const std::vector<GmodNode>& parents,
		const GmodNode& target )
	{
		SPDLOG_INFO( "LocationSetsVisitor: Visiting node '{}' at index {}", node.code(), i );

		bool isParent = Gmod::isPotentialParent( node.metadata().type() );
		bool isTargetNode = ( i == parents.size() );

		SPDLOG_INFO( "Node '{}': isTargetNode={}, isParent={}, isIndividualizable={}", node.code(), isTargetNode, isParent, node.isIndividualizable( isTargetNode ) );

		if ( m_currentParentStart == 0 )
		{
			if ( isParent )
				m_currentParentStart = i;
			if ( node.isIndividualizable( isTargetNode ) )
			{
				SPDLOG_INFO( "Single node is individualizable: [{},{}] with location {}",
					i, i, node.location() ? node.location()->toString() : "null" );
				return std::make_tuple( i, i, node.location() );
			}
		}
		else
		{
			if ( isParent || isTargetNode )
			{
				std::optional<std::tuple<size_t, size_t, std::optional<Location>>> nodes = std::nullopt;
				if ( m_currentParentStart + 1 == i )
				{
					if ( node.isIndividualizable( isTargetNode ) )
					{
						nodes = std::make_tuple( m_currentParentStart, i, node.location() );
						SPDLOG_INFO( "Found adjacent individualizable nodes: [{},{}] with location {}",
							m_currentParentStart, i, node.location() ? node.location()->toString() : "null" );
					}
				}
				else
				{
					bool skippedOne = false;
					bool hasComposition = false;

					for ( size_t j = m_currentParentStart + 1; j <= i; j++ )
					{
						const auto& setNode = j < static_cast<int>( parents.size() ) ? parents[j] : target;

						if ( !setNode.isIndividualizable( j == parents.size(), true ) )
						{
							if ( nodes.has_value() )
								skippedOne = true;
							continue;
						}

						if ( nodes.has_value() && std::get<2>( *nodes ).has_value() &&
							 setNode.location().has_value() &&
							 std::get<2>( *nodes ).value() != *setNode.location() )
						{
							SPDLOG_ERROR( "Location mismatch: {} vs {}",
								std::get<2>( *nodes ).value().toString(), setNode.location()->toString() );
							throw std::runtime_error( "Mapping error: different locations in the same nodeset" );
						}

						if ( skippedOne )
						{
							throw std::runtime_error( "Can't skip in the middle of individualizable set" );
						}

						if ( setNode.isFunctionComposition() )
						{
							hasComposition = true;
						}

						auto location = !nodes.has_value() || !std::get<2>( *nodes ).has_value() ? setNode.location() : std::get<2>( *nodes );
						auto start = nodes.has_value() ? std::get<0>( *nodes ) : j;
						auto end = j;
						nodes = std::make_tuple( start, end, location );
					}

					if ( nodes.has_value() && std::get<0>( *nodes ) == std::get<1>( *nodes ) && hasComposition )
						nodes = std::nullopt;
				}

				m_currentParentStart = i;
				if ( nodes.has_value() )
				{
					bool hasLeafNode = false;
					for ( size_t j = std::get<0>( *nodes ); j <= std::get<1>( *nodes ); j++ )
					{
						const auto& setNode = j < static_cast<int>( parents.size() ) ? parents[static_cast<size_t>( j )] : target;
						if ( setNode.isLeafNode() || j == static_cast<int>( parents.size() ) )
						{
							hasLeafNode = true;
							break;
						}
					}

					if ( hasLeafNode )
						return nodes;
				}
			}

			if ( isTargetNode && node.isIndividualizable( isTargetNode ) )
			{
				SPDLOG_INFO( "Target node forms singleton set: [{},{}] with location {}", i, i, node.location() ? node.location()->toString() : "null" );
				return std::make_tuple( i, i, node.location() );
			}
		}

		return std::nullopt;
	}

	GmodPath::GmodPath()
		: m_visVersion( VisVersion::Unknown ),
		  m_node( GmodNode() ),
		  m_parents()
	{
		SPDLOG_WARN( "Creating default-constructed GmodPath - this is invalid until assigned" );
	}

	GmodPath::GmodPath( std::vector<GmodNode> parents, GmodNode node, bool skipVerify )
		: m_visVersion{ node.visVersion() },
		  m_node{ std::move( node ) },
		  m_parents{ std::move( parents ) }
	{
		SPDLOG_INFO( "Creating GmodPath with {} parent nodes, target node '{}' (skipVerify={})", m_parents.size(), m_node.code(), skipVerify );

		if ( !skipVerify )
		{
			SPDLOG_INFO( "Validating GmodPath parent-child relationships" );

			if ( m_parents.empty() )
			{
				SPDLOG_ERROR( "Invalid path: Parents list is empty" );
				throw std::invalid_argument(
					"Invalid gmod path - no parents, and " + m_node.code() + " is not the root of gmod" );
			}

			if ( !m_parents[0].isRoot() )
			{
				SPDLOG_ERROR( "Invalid path: First parent '{}' is not the root node", m_parents[0].code() );
				throw std::invalid_argument(
					"Invalid gmod path - first parent should be root of gmod (VE), but was " +
					m_parents[0].code() );
			}

			std::unordered_set<std::string> set;
			set.insert( "VE" );

			for ( size_t i = 0; i < m_parents.size(); i++ )
			{
				const auto& parent = m_parents[i];
				size_t nextIndex = i + 1;
				const auto& child = nextIndex < m_parents.size() ? m_parents[nextIndex] : m_node;

				if ( !parent.isChild( child ) )
				{
					SPDLOG_ERROR( "Invalid path: '{}' is not a parent of '{}'",
						parent.code(), child.code() );
					throw std::invalid_argument(
						"Invalid gmod path - " + child.code() + " not child of " + parent.code() );
				}

				if ( !set.insert( child.code() ).second )
				{
					throw std::invalid_argument( "Recursion in gmod path argument for code: " + child.code() );
				}
			}

			LocationSetsVisitor visitor;
			for ( size_t i = 0; i < m_parents.size() + 1; ++i )
			{
				const auto& n = i < m_parents.size() ? m_parents[i] : m_node;
				auto _ = visitor.visit( n, i, m_parents, m_node );
			}
		}
	}

	bool GmodPath::operator==( const GmodPath& other ) const
	{
		return equals( other );
	}

	bool GmodPath::operator!=( const GmodPath& other ) const
	{
		return !equals( other );
	}

	const std::vector<GmodNode>& GmodPath::parents() const
	{
		return m_parents;
	}

	VisVersion GmodPath::visVersion() const
	{
		return m_visVersion;
	}

	const GmodNode& GmodPath::node() const
	{
		return m_node;
	}

	void GmodPath::setNode( const GmodNode& node )
	{
		m_node = node;
	}

	size_t GmodPath::length() const
	{
		return m_parents.size() + 1;
	}

	bool GmodPath::isMappable() const
	{
		return m_node.isMappable();
	}

	bool GmodPath::equals( const GmodPath& other ) const
	{
		SPDLOG_INFO( "Comparing GmodPaths for equality (this: {}, other: {})",
			m_node.code(), other.m_node.code() );

		if ( m_visVersion != other.m_visVersion )
			return false;

		if ( m_node != other.m_node )
			return false;

		if ( m_parents.size() != other.m_parents.size() )
			return false;

		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			if ( m_parents[i] != other.m_parents[i] )
				return false;
		}

		return true;
	}

	size_t GmodPath::hashCode() const
	{
		SPDLOG_DEBUG( "Calculating hash code for GmodPath" );

		size_t hash = 17;

		for ( const auto& parent : m_parents )
			hash = hash * 31 + std::hash<std::string>{}( parent.code() );

		hash = hash * 31 + std::hash<std::string>{}( m_node.code() );

		return hash;
	}

	const GmodNode& GmodPath::operator[]( size_t depth ) const
	{
		if ( depth > m_parents.size() )
		{
			SPDLOG_ERROR( "GmodPath::operator[]: Depth {} exceeds path length {}", depth, m_parents.size() + 1 );
			throw std::out_of_range( "Index out of range for GmodPath indexer" );
		}

		return depth < m_parents.size() ? m_parents[depth] : m_node;
	}

	GmodNode& GmodPath::operator[]( size_t depth )
	{
		if ( depth > m_parents.size() )
		{
			SPDLOG_ERROR( "GmodPath::operator[]: Depth {} exceeds path length {}", depth, m_parents.size() + 1 );
			throw std::out_of_range( "Index out of range for GmodPath indexer" );
		}

		return depth < m_parents.size() ? m_parents[depth] : m_node;
	}

	GmodPath GmodPath::withoutLocations() const
	{
		SPDLOG_INFO( "Creating copy of path without locations (original path has {} parents)", m_parents.size() );

		std::vector<GmodNode> parentsWithoutLocations;
		parentsWithoutLocations.reserve( m_parents.size() );

		for ( const auto& parent : m_parents )
		{
			parentsWithoutLocations.push_back( parent.withoutLocation() );
		}

		GmodNode nodeWithoutLocation = m_node.withoutLocation();

		return GmodPath{ parentsWithoutLocations, nodeWithoutLocation };
	}

	std::vector<GmodIndividualizableSet> GmodPath::individualizableSets() const
	{
		SPDLOG_DEBUG( "Getting individualizable sets for path" );
		std::vector<GmodIndividualizableSet> result;

		LocationSetsVisitor visitor;
		for ( size_t i{ 0 }; i < length(); i++ )
		{
			const auto& node = ( *this )[i];
			auto set = visitor.visit( node, i, m_parents, m_node );

			if ( !set.has_value() )
				continue;

			auto [start, end, location] = *set;

			if ( start == end )
			{
				std::vector<size_t> singleNodeSet{ static_cast<size_t>( start ) };
				GmodPath pathCopy = *this;
				result.emplace_back( singleNodeSet, pathCopy );
				SPDLOG_DEBUG( "Added single-node individualizable set at index {}", start );
				continue;
			}

			std::vector<size_t> indices;
			indices.reserve( static_cast<size_t>( end - start + 1 ) );
			for ( size_t j{ start }; j <= end; j++ )
			{
				indices.push_back( j );
			}

			GmodPath pathCopy = *this;
			result.emplace_back( indices, pathCopy );
			SPDLOG_DEBUG( "Added individualizable set from {} to {}", start, end );
		}

		SPDLOG_DEBUG( "Found {} individualizable sets", result.size() );
		return result;
	}

	bool GmodPath::isIndividualizable() const
	{
		SPDLOG_INFO( "Checking if path is individualizable" );

		LocationSetsVisitor visitor;
		for ( size_t i = 0; i < length(); i++ )
		{
			const auto& node = ( *this )[i];
			auto set = visitor.visit( node, i, m_parents, m_node );

			if ( set.has_value() )
			{
				SPDLOG_INFO( "Path is individualizable: Yes" );
				return true;
			}
		}

		SPDLOG_INFO( "Path is individualizable: No" );
		return false;
	}

	std::string GmodPath::toString() const
	{
		SPDLOG_INFO( "Converting path to string (node='{}')", m_node.code() );
		std::stringstream builder;
		toString( builder );
		return builder.str();
	}

	void GmodPath::toString( std::stringstream& builder, char separator ) const
	{
		SPDLOG_INFO( "Building path string with separator '{}'", separator );

		bool isFirst = true;

		for ( const auto& parent : m_parents )
		{
			if ( !Gmod::isLeafNode( parent.metadata() ) )
				continue;

			if ( !isFirst )
				builder << separator;
			else
				isFirst = false;

			parent.toString( builder );
		}

		if ( !isFirst )
			builder << separator;

		m_node.toString( builder );
	}

	std::string GmodPath::toFullPathString() const
	{
		SPDLOG_INFO( "Converting path to full path string (target='{}')", m_node.code() );
		std::stringstream builder;
		toFullPathString( builder );
		return builder.str();
	}

	void GmodPath::toFullPathString( std::stringstream& builder ) const
	{
		SPDLOG_INFO( "Building full path string including all parent nodes" );

		bool isFirst = true;

		for ( const auto& parent : m_parents )
		{
			if ( !isFirst )
				builder << '/';
			else
				isFirst = false;

			builder << parent.code();

			if ( parent.location().has_value() )
			{
				builder << "-" << parent.location().value().toString();
			}
		}

		if ( !isFirst )
			builder << '/';

		builder << m_node.code();

		if ( m_node.location().has_value() )
		{
			builder << "-" << m_node.location().value().toString();
		}
	}

	std::string GmodPath::toStringDump() const
	{
		SPDLOG_INFO( "Creating detailed string dump of path" );
		std::stringstream builder;
		toStringDump( builder );
		return builder.str();
	}

	void GmodPath::toStringDump( std::stringstream& builder ) const
	{
		SPDLOG_INFO( "Building detailed path dump" );

		builder << "GmodPath [VIS Version: " << static_cast<int>( m_visVersion ) << "]\n";
		builder << "Parents (" << m_parents.size() << "):\n";

		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			builder << "  [" << i << "] " << m_parents[i].code();

			if ( m_parents[i].location().has_value() )
			{
				builder << "-" << m_parents[i].location().value().toString();
			}

			builder << "\n";
		}

		builder << "Target: " << m_node.code();

		if ( m_node.location().has_value() )
		{
			builder << "-" << m_node.location().value().toString();
		}

		builder << " (Mappable: " << ( m_node.isMappable() ? "Yes" : "No" ) << ")";
	}

	std::optional<std::string> GmodPath::normalAssignmentName( size_t nodeDepth ) const
	{
		const GmodNode& node = ( *this )[nodeDepth];
		auto normalAssignmentNames = node.metadata().normalAssignmentNames();

		if ( normalAssignmentNames.empty() )
			return std::nullopt;

		for ( size_t i = length() - 1; i != 0; --i )
		{
			const auto& child = ( *this )[i];
			auto it = normalAssignmentNames.find( child.code() );
			if ( it != normalAssignmentNames.end() )
				return it->second;
		}

		return std::nullopt;
	}

	std::vector<std::pair<size_t, std::string>> GmodPath::commonNames() const
	{
		SPDLOG_INFO( "Getting all common names in path" );
		std::vector<std::pair<size_t, std::string>> results;

		results.reserve( m_parents.size() + 1 );

		for ( size_t depth{ 0 }; depth < m_parents.size(); ++depth )
		{
			const GmodNode& node{ m_parents[depth] };
			auto commonName{ node.metadata().commonName() };

			if ( commonName.has_value() )
			{
				SPDLOG_INFO( "Found common name '{}' for parent node at depth {}", commonName.value(), depth );
				results.emplace_back( depth, commonName.value() );
			}
		}

		auto targetCommonName{ m_node.metadata().commonName() };
		if ( targetCommonName.has_value() )
		{
			auto depth{ m_parents.size() };
			SPDLOG_INFO( "Found common name '{}' for target node at depth {}", targetCommonName.value(), depth );
			results.emplace_back( depth, targetCommonName.value() );
		}

		return results;
	}

	GmodPath::Enumerator GmodPath::fullPath() const
	{
		return Enumerator( *this );
	}

	GmodPath::Enumerator GmodPath::fullPathFrom( size_t fromDepth ) const
	{
		return Enumerator( *this, fromDepth );
	}

	bool GmodPath::isValid( const std::vector<GmodNode>& parents, const GmodNode& node )
	{
		int missingLinkAt;
		return isValid( parents, node, missingLinkAt );
	}

	bool GmodPath::isValid( const std::vector<GmodNode>& parents, const GmodNode& node, int& missingLinkAt )
	{
		SPDLOG_INFO( "Validating path with {} parents and target node '{}'",
			parents.size(), node.code() );

		missingLinkAt = -1;

		if ( parents.empty() )
		{
			SPDLOG_ERROR( "Invalid path: Parents list is empty" );
			return false;
		}

		if ( !parents[0].isRoot() )
		{
			SPDLOG_ERROR( "Invalid path: First parent '{}' is not the root node",
				parents[0].code() );
			return false;
		}

		std::unordered_set<std::string> set;
		set.insert( "VE" );

		for ( size_t i = 0; i < parents.size(); i++ )
		{
			const auto& parent = parents[i];
			size_t nextIndex = i + 1;
			const auto& child = nextIndex < parents.size() ? parents[nextIndex] : node;

			if ( !parent.isChild( child ) )
			{
				SPDLOG_ERROR( "Invalid path: '{}' is not a parent of '{}'",
					parent.code(), child.code() );
				missingLinkAt = static_cast<int>( i );
				return false;
			}

			if ( !set.insert( child.code() ).second )
			{
				SPDLOG_ERROR( "Recursion detected for '{}'", child.code() );
				return false;
			}
		}

		return true;
	}

	GmodParsePathResult::Ok::Ok( const GmodPath& path )
		: path{ path }
	{
	}

	GmodParsePathResult::Ok::Ok( GmodPath&& path )
		: path{ std::move( path ) }
	{
	}

	GmodParsePathResult::Ok::Ok( Ok&& other ) noexcept
		: GmodParsePathResult( std::move( other ) ), path( std::move( other.path ) )
	{
	}

	GmodParsePathResult::Err::Err( const std::string& errorMessage ) : error( errorMessage )
	{
	}

	GmodParsePathResult::Err::Err( Err&& other ) noexcept
		: GmodParsePathResult( std::move( other ) ), error( std::move( other.error ) )
	{
	}

	GmodPath::Enumerator::Enumerator( const GmodPath& path, std::optional<size_t> fromDepth )
		: m_path( path ),
		  m_current{ std::numeric_limits<size_t>::max() },
		  m_depth( fromDepth.value_or( 0 ) - 1 ),
		  m_fromDepth( fromDepth )
	{
		SPDLOG_INFO( "Creating GmodPath::Enumerator with fromDepth={}",
			fromDepth.has_value() ? std::to_string( fromDepth.value() ) : "null" );
	}

	bool GmodPath::Enumerator::next()
	{
		if ( m_current == std::numeric_limits<size_t>::max() )
		{
			m_current = 0;
			m_depth = m_fromDepth.value_or( 0 );

			if ( m_fromDepth.has_value() && m_fromDepth.value() > 0 )
			{
				m_current = m_fromDepth.value();
			}
		}
		else
		{
			m_current++;
			m_depth++;
		}

		if ( m_current >= m_path.length() )
		{
			SPDLOG_INFO( "Enumerator reached end of path (length={})", m_path.length() );
			return false;
		}

		SPDLOG_INFO( "Enumerator advanced to position {} at depth {}", m_current, m_depth );
		return true;
	}

	GmodPath::Enumerator& GmodPath::Enumerator::operator=( const Enumerator& other )
	{
		if ( this != &other )
		{
			if ( &m_path != &other.m_path )
			{
				SPDLOG_ERROR( "Cannot assign enumerators with different path references" );
				throw std::invalid_argument( "Cannot assign enumerators with different path references" );
			}

			m_current = other.m_current;
			m_depth = other.m_depth;
			m_fromDepth = other.m_fromDepth;
		}
		return *this;
	}

	std::pair<size_t, std::reference_wrapper<const GmodNode>> GmodPath::Enumerator::current() const
	{
		if ( m_current >= m_path.length() )
		{
			SPDLOG_ERROR( "Enumerator::current called with invalid position (m_current={})", m_current );
			throw std::runtime_error( "Enumerator position is invalid" );
		}

		return std::make_pair( m_depth, std::cref( m_path[m_current] ) );
	}

	void GmodPath::Enumerator::reset()
	{
		SPDLOG_INFO( "Resetting enumerator" );
		m_current = std::numeric_limits<size_t>::max();
		m_depth = m_fromDepth.value_or( 0 ) - 1;
	}

	GmodPath::Enumerator::Iterator GmodPath::Enumerator::begin()
	{
		return Iterator( *this, false );
	}

	GmodPath::Enumerator::Iterator GmodPath::Enumerator::end()
	{
		return Iterator( *this, true );
	}

	GmodPath::Enumerator::Iterator::Iterator( Enumerator& enumerator, bool end )
		: m_enumerator( enumerator ),
		  m_end( end ),
		  m_cachedValue( std::nullopt )
	{
		if ( !m_end )
		{
			if ( !m_enumerator.next() )
			{
				m_end = true;
			}
		}
	}

	GmodPath::Enumerator::Iterator::Iterator( const Iterator& other )
		: m_enumerator( other.m_enumerator ),
		  m_end( other.m_end ),
		  m_current( other.m_current ),
		  m_cachedValue( other.m_cachedValue )
	{
	}

	GmodPath::Enumerator::Iterator& GmodPath::Enumerator::Iterator::operator++()
	{
		if ( !m_end && !m_enumerator.next() )
		{
			m_end = true;
		}

		m_cachedValue.reset();

		return *this;
	}

	GmodPath::Enumerator::Iterator GmodPath::Enumerator::Iterator::operator++( int )
	{
		Iterator tmp = *this;
		++( *this );
		return tmp;
	}

	bool GmodPath::Enumerator::Iterator::operator==( const Iterator& other ) const
	{
		return m_end == other.m_end;
	}

	bool GmodPath::Enumerator::Iterator::operator!=( const Iterator& other ) const
	{
		return m_end != other.m_end;
	}

	GmodPath::Enumerator::Iterator::reference GmodPath::Enumerator::Iterator::operator*() const
	{
		if ( m_end )
		{
			SPDLOG_ERROR( "Attempting to dereference end iterator" );
			throw std::out_of_range( "Cannot dereference end iterator" );
		}

		if ( !m_cachedValue )
		{
			m_cachedValue = m_enumerator.current();
		}

		return *m_cachedValue;
	}

	GmodPath::Enumerator::Iterator& GmodPath::Enumerator::Iterator::operator=( const Iterator& other )
	{
		if ( this != &other )
		{
			if ( &m_enumerator != &other.m_enumerator )
			{
				throw std::invalid_argument( "Cannot assign iterators with different enumerator references" );
			}

			m_end = other.m_end;
			m_current = other.m_current;
			m_cachedValue = other.m_cachedValue;
		}
		return *this;
	}
	GmodIndividualizableSet::GmodIndividualizableSet( const std::vector<size_t>& nodes, GmodPath& path )
		: m_nodes( nodes ), m_path( &path )
	{
		SPDLOG_INFO( "Creating individualizable set with {} nodes", nodes.size() );

		if ( nodes.empty() )
		{
			SPDLOG_ERROR( "GmodIndividualizableSet cannot be empty" );
			throw std::invalid_argument( "GmodIndividualizableSet cant be empty" );
		}

		for ( size_t nodeIndex : nodes )
		{
			const auto& node = path[nodeIndex];
			bool isTarget = ( nodeIndex == path.length() - 1 );
			bool isInSet = nodes.size() > 1;

			if ( !node.isIndividualizable( isTarget, isInSet ) )
			{
				throw std::invalid_argument( "Node at index " + std::to_string( nodeIndex ) + " is not individualizable" );
			}
		}

		std::optional<Location> firstLocation;
		bool hasSetLocation = false;

		for ( size_t nodeIndex : nodes )
		{
			const auto& nodeLoc = path[nodeIndex].location();

			if ( !hasSetLocation && nodeLoc.has_value() )
			{
				firstLocation = nodeLoc;
				hasSetLocation = true;
			}
			else if ( hasSetLocation && nodeLoc.has_value() && *nodeLoc != *firstLocation )
			{
				throw std::invalid_argument( "GmodIndividualizableSet nodes have different locations" );
			}
		}

		bool hasLeafOrTarget = false;
		for ( size_t nodeIndex : nodes )
		{
			const auto& node = path[nodeIndex];
			if ( node.isLeafNode() || nodeIndex == path.length() - 1 )
			{
				hasLeafOrTarget = true;
				break;
			}
		}

		if ( !hasLeafOrTarget )
		{
			throw std::invalid_argument( "GmodIndividualizableSet has no nodes that are part of short path" );
		}
	}

	std::vector<GmodNode> GmodIndividualizableSet::nodes() const
	{
		SPDLOG_INFO( "Getting nodes for individualizable set with {} indices", m_nodes.size() );

		std::vector<GmodNode> result;
		result.reserve( m_nodes.size() );

		for ( size_t index : m_nodes )
		{
			try
			{
				result.push_back( ( *m_path )[index] );
				SPDLOG_INFO( "Added node '{}' at index {}", ( *m_path )[index].code(), index );
			}
			catch ( const std::exception& e )
			{
				SPDLOG_ERROR( "Failed to get node at index {}: {}", index, e.what() );
			}
		}

		SPDLOG_INFO( "Returning {} nodes from individualizable set", result.size() );
		return result;
	}

	const std::vector<size_t>& GmodIndividualizableSet::nodeIndices() const
	{
		return m_nodes;
	}

	std::optional<Location> GmodIndividualizableSet::location() const
	{
		SPDLOG_INFO( "Getting location for individualizable set with {} indices", m_nodes.size() );

		if ( m_nodes.empty() )
		{
			SPDLOG_INFO( "Individualizable set is empty, no location to return" );
			return std::nullopt;
		}

		return ( *m_path )[m_nodes[0]].location();
	}

	void GmodIndividualizableSet::setLocation( const std::optional<Location>& location )
	{
		SPDLOG_INFO( "Setting location {} for individualizable set with {} indices",
			location.has_value() ? location.value().toString() : "null",
			m_nodes.size() );

		if ( !m_path )
		{
			SPDLOG_ERROR( "Cannot set location: path is null" );
			throw std::runtime_error( "Tried to modify individualizable set after it was built" );
		}

		for ( size_t index : m_nodes )
		{
			try
			{
				if ( location.has_value() )
					( *m_path )[index] = ( *m_path )[index].withLocation( location.value() );
				else
					( *m_path )[index] = ( *m_path )[index].withoutLocation();

				SPDLOG_INFO( "Set location for node '{}' at index {}", ( *m_path )[index].code(), index );
			}
			catch ( const std::exception& e )
			{
				SPDLOG_ERROR( "Failed to set location for node at index {}: {}",
					index, e.what() );
			}
		}
	}

	GmodPath GmodIndividualizableSet::build()
	{
		SPDLOG_INFO( "Building path from individualizable set" );

		if ( !m_path )
		{
			SPDLOG_ERROR( "Cannot build path: m_path is null" );
			throw std::runtime_error( "Tried to build individualizable set twice" );
		}

		return *m_path;
	}

	std::string GmodIndividualizableSet::toString() const
	{
		SPDLOG_INFO( "Converting individualizable set to string representation" );

		std::stringstream ss;

		bool first = true;
		for ( size_t i = 0; i < m_nodes.size(); ++i )
		{
			auto idx = m_nodes[i];
			if ( ( *m_path )[idx].isLeafNode() || i == m_nodes.size() - 1 )
			{
				if ( !first )
					ss << "/";
				first = false;

				std::stringstream nodeBuilder;
				( *m_path )[idx].toString( nodeBuilder );
				ss << nodeBuilder.str();
			}
		}

		return ss.str();
	}

	PathNode::PathNode( const std::string& code, const std::optional<Location>& location )
		: code( code ),
		  location( location )
	{
	}

	GmodPath GmodPath::parse( const std::string& item, VisVersion visVersion )
	{
		SPDLOG_INFO( "Parsing path '{}' with VIS version {}", item, static_cast<int>( visVersion ) );

		auto& vis = VIS::instance();
		const auto& gmod = vis.gmod( visVersion );
		auto locations = vis.locations( visVersion );

		return parse( item, gmod, locations );
	}

	bool GmodPath::tryParse( const std::string& item, VisVersion visVersion, std::optional<GmodPath>& path )
	{
		SPDLOG_INFO( "Attempting to parse path '{}' with VIS version {}",
			item, static_cast<int>( visVersion ) );

		try
		{
			auto& vis = VIS::instance();
			const auto& gmod = vis.gmod( visVersion );
			auto locations = vis.locations( visVersion );
			GmodPath parsedPath = parse( item, gmod, locations );
			path = std::move( parsedPath );

			SPDLOG_INFO( "Successfully parsed path" );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Failed to parse path: {}", e.what() );
			path.reset();
			return false;
		}
	}

	GmodPath GmodPath::parseFullPath( const std::string& pathStr, VisVersion visVersion )
	{
		SPDLOG_INFO( "Parsing full path '{}' with VIS version {}", pathStr, static_cast<int>( visVersion ) );

		auto& vis = VIS::instance();
		const auto& gmod = vis.gmod( visVersion );
		auto locations = vis.locations( visVersion );

		auto result = parseFullPathInternal( pathStr, gmod, locations );

		auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( &result );
		if ( okResult != nullptr )
		{
			SPDLOG_INFO( "Successfully parsed full path" );
			return std::move( okResult->path );
		}

		auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( &result );
		SPDLOG_ERROR( "Failed to parse full path: {}", errResult->error );
		throw std::invalid_argument( "Failed to parse full path: " + errResult->error );
	}

	GmodParsePathResult GmodPath::parseFullPathInternal( std::string_view pathStr, const Gmod& gmod, const Locations& locations )
	{
		SPDLOG_INFO( "Parsing full path '{}' with GMOD version {}", pathStr, static_cast<int>( gmod.visVersion() ) );

		if ( pathStr.empty() || std::all_of( pathStr.begin(), pathStr.end(), []( char c ) { return std::isspace( c ); } ) )
		{
			SPDLOG_ERROR( "Cannot parse empty path" );
			return GmodParsePathResult::Err( "Path cannot be empty" );
		}

		std::vector<std::string> parts;
		size_t start = 0;
		size_t end;
		std::string pathStrCopy( pathStr );

		while ( ( end = pathStrCopy.find( '/', start ) ) != std::string::npos )
		{
			if ( end > start )
				parts.push_back( pathStrCopy.substr( start, end - start ) );
			start = end + 1;
		}

		if ( start < pathStrCopy.length() )
			parts.push_back( pathStrCopy.substr( start ) );

		if ( parts.empty() )
		{
			SPDLOG_ERROR( "No parts found in path" );
			return GmodParsePathResult::Err( "Path cannot be empty" );
		}

		std::vector<GmodNode> nodes;
		nodes.reserve( parts.size() );

		for ( const auto& part : parts )
		{
			size_t dashPos = part.find( '-' );
			std::string code;
			std::optional<Location> location;

			if ( dashPos != std::string::npos )
			{
				code = part.substr( 0, dashPos );
				std::string locStr = part.substr( dashPos + 1 );

				Location parsedLocation;
				if ( !locations.tryParse( locStr, parsedLocation ) )
				{
					SPDLOG_ERROR( "Failed to parse location: {}", locStr );
					return GmodParsePathResult::Err( "Failed to parse location: " + locStr );
				}

				location = parsedLocation;
			}
			else
			{
				code = part;
			}

			GmodNode node;
			if ( !gmod.tryGetNode( code, node ) )
			{
				SPDLOG_ERROR( "Failed to find node with code: {}", code );
				return GmodParsePathResult::Err( "Failed to get node: " + code );
			}

			if ( location.has_value() )
				node = node.withLocation( *location );

			nodes.push_back( std::move( node ) );
		}

		if ( nodes.size() < 1 )
		{
			SPDLOG_ERROR( "Path must have at least one node" );
			return GmodParsePathResult::Err( "Path must have at least one node" );
		}

		GmodNode targetNode = nodes.back();
		nodes.pop_back();

		int missingLinkAt;
		if ( !isValid( nodes, targetNode, missingLinkAt ) )
		{
			SPDLOG_ERROR( "Invalid path: missing link at position {}", missingLinkAt );
			return GmodParsePathResult::Err( "Invalid path structure" );
		}

		try
		{
			GmodPath path( nodes, targetNode, false );
			return GmodParsePathResult::Ok( std::move( path ) );
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Error creating path: {}", ex.what() );
			return GmodParsePathResult::Err( std::string( "Error creating path: " ) + ex.what() );
		}
	}

	GmodParsePathResult GmodPath::parseInternal( const std::string& item, const Gmod& gmod, const Locations& locations )
	{
		try
		{
			std::vector<std::string> parts;
			std::stringstream ss( item );
			std::string part;

			while ( std::getline( ss, part, '/' ) )
			{
				if ( !part.empty() )
					parts.push_back( part );
			}

			if ( parts.empty() )
			{
				SPDLOG_ERROR( "No parts found in path" );
				return GmodParsePathResult::Err( "Path cannot be empty" );
			}

			std::string targetPart = parts.back();
			parts.pop_back();

			std::string targetCode;
			std::optional<Location> targetLocation;

			size_t dashPos = targetPart.find( '-' );
			if ( dashPos != std::string::npos )
			{
				targetCode = targetPart.substr( 0, dashPos );
				std::string locStr = targetPart.substr( dashPos + 1 );

				Location parsedLocation;
				if ( !locations.tryParse( locStr, parsedLocation ) )
				{
					SPDLOG_ERROR( "Failed to parse target location: {}", locStr );
					return GmodParsePathResult::Err( "Failed to parse target location: " + locStr );
				}

				targetLocation = parsedLocation;
			}
			else
			{
				targetCode = targetPart;
			}

			GmodNode targetNode;
			if ( !gmod.tryGetNode( targetCode, targetNode ) )
			{
				SPDLOG_ERROR( "Failed to find target node with code: {}", targetCode );
				return GmodParsePathResult::Err( "Failed to get target node: " + targetCode );
			}

			if ( targetLocation.has_value() )
				targetNode = targetNode.withLocation( *targetLocation );

			std::vector<GmodNode> parentPath;
			parentPath.push_back( gmod.rootNode() );

			for ( const auto& partStr : parts )
			{
				std::string nodeCode;
				std::optional<Location> nodeLocation;

				dashPos = partStr.find( '-' );
				if ( dashPos != std::string::npos )
				{
					nodeCode = partStr.substr( 0, dashPos );
					std::string locStr = partStr.substr( dashPos + 1 );

					Location parsedLocation;
					if ( !locations.tryParse( locStr, parsedLocation ) )
					{
						SPDLOG_ERROR( "Failed to parse node location: {}", locStr );
						return GmodParsePathResult::Err( "Failed to parse node location: " + locStr );
					}

					nodeLocation = parsedLocation;
				}
				else
				{
					nodeCode = partStr;
				}

				GmodNode node;
				if ( !gmod.tryGetNode( nodeCode, node ) )
				{
					SPDLOG_ERROR( "Failed to find node with code: {}", nodeCode );
					return GmodParsePathResult::Err( "Failed to get node: " + nodeCode );
				}

				if ( nodeLocation.has_value() )
					node = node.withLocation( *nodeLocation );

				parentPath.push_back( std::move( node ) );
			}

			std::vector<GmodNode> remainingParents;
			if ( !gmod.pathExistsBetween( parentPath, targetNode, remainingParents ) )
			{
				SPDLOG_ERROR( "No path exists between parents and target node" );
				return GmodParsePathResult::Err( "No path exists between parents and target node" );
			}

			for ( const auto& parent : remainingParents )
			{
				parentPath.push_back( parent );
			}

			try
			{
				GmodPath path( parentPath, targetNode, false );
				return GmodParsePathResult::Ok( std::move( path ) );
			}
			catch ( const std::exception& ex )
			{
				SPDLOG_ERROR( "Error creating path: {}", ex.what() );
				return GmodParsePathResult::Err( std::string( "Error creating path: " ) + ex.what() );
			}
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception parsing path: {}", ex.what() );
			return GmodParsePathResult::Err( std::string( "Exception: " ) + ex.what() );
		}
	}

	ParseContext::ParseContext( std::queue<PathNode> parts )
		: parts( std::move( parts ) ),
		  toFind(),
		  locations(),
		  path( std::nullopt )
	{
	}

	GmodPath GmodPath::parse( const std::string& item, const Gmod& gmod, const Locations& locations )
	{
		SPDLOG_INFO( "Parsing path '{}' using provided GMOD and Locations", item );

		auto result = parseInternal( item, gmod, locations );

		auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( &result );
		if ( okResult != nullptr )
		{
			SPDLOG_INFO( "Successfully parsed path" );
			return std::move( okResult->path );
		}

		auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( &result );
		if ( errResult == nullptr )
		{
			SPDLOG_ERROR( "Failed to parse path: unknown error" );
			throw std::invalid_argument( "Failed to parse path: unknown error" );
		}

		SPDLOG_ERROR( "Failed to parse path: {}", errResult->error );
		throw std::invalid_argument( "Failed to parse path: " + errResult->error );
	}

	bool GmodPath::tryParse( const std::string& item, const Gmod& gmod, const Locations& locations, GmodPath& path )
	{
		SPDLOG_INFO( "Attempting to parse path '{}' using provided GMOD and Locations", item );

		try
		{
			auto result = parseInternal( item, gmod, locations );

			auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( &result );
			if ( okResult != nullptr )
			{
				path = std::move( okResult->path );
				SPDLOG_INFO( "Successfully parsed path" );
				return true;
			}

			auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( &result );
			SPDLOG_ERROR( "Failed to parse path: {}", errResult->error );
			return false;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Failed to parse path: {}", e.what() );
			return false;
		}
	}

	bool GmodPath::tryParseFullPath( const std::string& pathStr, VisVersion visVersion, GmodPath& path )
	{
		SPDLOG_INFO( "Attempting to parse full path '{}' with VIS version {}",
			pathStr, static_cast<int>( visVersion ) );

		try
		{
			auto& vis = VIS::instance();
			const auto& gmod = vis.gmod( visVersion );
			auto locations = vis.locations( visVersion );

			return tryParseFullPath( pathStr, gmod, locations, path );
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Failed to parse full path: {}", e.what() );
			return false;
		}
	}

	bool GmodPath::tryParseFullPath( std::string_view pathStr, VisVersion visVersion, GmodPath& path )
	{
		SPDLOG_INFO( "Attempting to parse full path '{}' with VIS version {}",
			pathStr, static_cast<int>( visVersion ) );

		try
		{
			auto& vis = VIS::instance();
			const auto& gmod = vis.gmod( visVersion );
			auto locations = vis.locations( visVersion );

			return tryParseFullPath( pathStr, gmod, locations, path );
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Failed to parse full path: {}", e.what() );
			return false;
		}
	}

	bool GmodPath::tryParseFullPath( std::string_view pathStr, const Gmod& gmod, const Locations& locations, GmodPath& path )
	{
		SPDLOG_INFO( "Attempting to parse full path '{}' using provided GMOD and Locations", pathStr );

		try
		{
			auto result = parseFullPathInternal( pathStr, gmod, locations );

			auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( &result );
			if ( okResult != nullptr )
			{
				path = std::move( okResult->path );
				SPDLOG_INFO( "Successfully parsed full path" );
				return true;
			}

			auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( &result );
			if ( errResult != nullptr )
			{
				SPDLOG_ERROR( "Failed to parse full path: {}", errResult->error );
			}
			else
			{
				SPDLOG_ERROR( "Failed to parse full path: unknown error" );
			}

			return false;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Failed to parse full path: {}", e.what() );
			return false;
		}
	}
}
