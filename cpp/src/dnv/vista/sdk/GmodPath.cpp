/**
 * @file GmodPath.cpp
 * @brief Implementation of GmodPath and related classes for representing paths in the Generic Product Model (GMOD).
 */

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
	//-------------------------------------------------------------------------
	// Result Class for Parsing Operations (Implementation)
	//-------------------------------------------------------------------------

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

	//-------------------------------------------------------------------------
	// Helper Classes (Implementations)
	//-------------------------------------------------------------------------

	PathNode::PathNode( const std::string& code, const std::optional<Location>& location )
		: code( code ),
		  location( location )
	{
	}

	LocationSetsVisitor::LocationSetsVisitor()
		: m_currentParentStart{ 0 }
	{
	}

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

	ParseContext::ParseContext( std::queue<PathNode> initialParts )
		: parts{ std::move( initialParts ) },
		  toFind{},
		  locations{},
		  path{ std::nullopt }
	{
	}

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Constructors and Special Member Functions
	//-------------------------------------------------------------------------

	GmodPath::GmodPath()
		: m_visVersion{ VisVersion::Unknown },
		  m_node{},
		  m_parents{},
		  m_isEmpty{ true }
	{
		SPDLOG_DEBUG( "Created empty GmodPath" );
	}

	GmodPath::GmodPath( const std::vector<GmodNode>& parents, GmodNode node, VisVersion visVersion, bool skipVerify )
		: m_visVersion{ visVersion },
		  m_node{ std::move( node ) },
		  m_parents{ std::move( parents ) },
		  m_isEmpty{ false }
	{
		SPDLOG_DEBUG( "Created GmodPath with target node '{}' and {} parent nodes", m_node.code(), m_parents.size() );

		if ( !skipVerify )
		{
			size_t missingLinkAt = std::numeric_limits<size_t>::max();
			if ( !isValid( m_parents, m_node, missingLinkAt ) )
			{
				std::stringstream errorMsg;
				errorMsg << "Invalid path structure: ";

				if ( missingLinkAt < m_parents.size() )
				{
					errorMsg << "Parent '" << m_parents[missingLinkAt].code() << "' cannot have '"
							 << ( ( missingLinkAt + 1 < m_parents.size() ) ? m_parents[missingLinkAt + 1].code() : m_node.code() )
							 << "' as a child";
				}
				else
				{
					errorMsg << "Unknown validation error";
				}

				SPDLOG_ERROR( "{}", errorMsg.str() );
				throw std::invalid_argument( errorMsg.str() );
			}
		}
	}

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Core Properties & Operations
	//-------------------------------------------------------------------------

	size_t GmodPath::length() const noexcept
	{
		if ( m_isEmpty )
			return 0;

		return m_parents.size() + 1;
	}

	bool GmodPath::isMappable() const noexcept
	{
		if ( m_isEmpty )
			return false;

		return m_node.isMappable();
	}

	GmodPath GmodPath::withoutLocations() const
	{
		if ( m_isEmpty )
		{
			SPDLOG_DEBUG( "Called withoutLocations() on empty path" );
			return GmodPath();
		}

		GmodNode newNode = m_node.withoutLocation();

		std::vector<GmodNode> newParents;
		newParents.reserve( m_parents.size() );

		for ( const auto& parent : m_parents )
		{
			newParents.push_back( parent.withoutLocation() );
		}

		return GmodPath( std::move( newParents ), std::move( newNode ), m_visVersion );
	}

	bool GmodPath::equals( const GmodPath& other ) const
	{
		if ( m_isEmpty && other.m_isEmpty )
			return true;

		if ( m_isEmpty != other.m_isEmpty )
			return false;

		if ( m_visVersion != other.m_visVersion )
			return false;

		if ( !m_node.equals( other.m_node ) )
			return false;

		if ( m_parents.size() != other.m_parents.size() )
			return false;

		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			if ( !m_parents[i].equals( other.m_parents[i] ) )
				return false;
		}

		return true;
	}

	size_t GmodPath::hashCode() const noexcept
	{
		if ( m_isEmpty )
			return 0;

		size_t hash = m_node.hashCode();

		hash = hash * 31 + static_cast<size_t>( m_visVersion );

		for ( const auto& parent : m_parents )
		{
			hash = hash * 31 + parent.hashCode();
		}

		return hash;
	}

	bool GmodPath::isIndividualizable() const noexcept
	{
		if ( m_isEmpty )
			return false;

		if ( m_node.isIndividualizable( true ) )
			return true;

		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			if ( m_parents[i].isIndividualizable( false ) )
				return true;
		}

		return false;
	}

	std::vector<GmodIndividualizableSet> GmodPath::individualizableSets()
	{
		if ( m_isEmpty || !isIndividualizable() )
		{
			SPDLOG_DEBUG( "No individualizable sets found in path" );
			return {};
		}

		std::vector<GmodIndividualizableSet> result;
		LocationSetsVisitor visitor;

		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			auto setInfo = visitor.visit( m_parents[i], i, m_parents, m_node );
			if ( setInfo )
			{
				size_t start = std::get<0>( *setInfo );
				size_t end = std::get<1>( *setInfo );

				std::vector<size_t> indices;
				for ( size_t idx = start; idx <= end; ++idx )
				{
					indices.push_back( idx );
				}

				SPDLOG_INFO( "Creating individualizable set with indices [{}-{}]", start, end );
				result.emplace_back( indices, *this );
			}
		}

		auto setInfo = visitor.visit( m_node, m_parents.size(), m_parents, m_node );
		if ( setInfo )
		{
			size_t start = std::get<0>( *setInfo );
			size_t end = std::get<1>( *setInfo );

			std::vector<size_t> indices;
			for ( size_t idx = start; idx <= end; ++idx )
			{
				indices.push_back( idx );
			}

			SPDLOG_INFO( "Creating individualizable set with indices [{}-{}] (includes target node)", start, end );
			result.emplace_back( indices, *this );
		}

		return result;
	}

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Accessors
	//-------------------------------------------------------------------------

	VisVersion GmodPath::visVersion() const noexcept
	{
		return m_visVersion;
	}

	const GmodNode& GmodPath::node() const
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "Attempted to access node() on empty path" );
			throw std::logic_error( "Cannot access node of an empty path" );
		}

		return m_node;
	}

	const std::vector<GmodNode>& GmodPath::parents() const noexcept
	{
		return m_parents;
	}

	std::optional<std::string> GmodPath::normalAssignmentName( size_t nodeDepth ) const
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "Called normalAssignmentName() on empty path" );
			throw std::logic_error( "Cannot get normal assignment name from an empty path" );
		}

		if ( nodeDepth >= length() )
		{
			SPDLOG_ERROR( "Invalid node depth: {} (path length: {})", nodeDepth, length() );
			throw std::out_of_range( "Node depth out of range" );
		}

		const GmodNode& node = ( nodeDepth < m_parents.size() ) ? m_parents[nodeDepth] : m_node;
		const auto& normalAssignmentNames = node.metadata().normalAssignmentNames();

		if ( normalAssignmentNames.empty() )
		{
			return std::nullopt;
		}

		for ( size_t i = length(); i-- > 0; )
		{
			const GmodNode& child = ( *this )[i];

			auto it = normalAssignmentNames.find( child.code() );
			if ( it != normalAssignmentNames.end() )
			{
				return it->second;
			}
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

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Mutators
	//-------------------------------------------------------------------------

	void GmodPath::setNode( GmodNode newNode )
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "Called setNode() on empty path" );
			throw std::logic_error( "Cannot set node on an empty path" );
		}

		SPDLOG_INFO( "Replacing target node '{}' with '{}'", m_node.code(), newNode.code() );
		m_node = std::move( newNode );
	}

	//-------------------------------------------------------------------------
	// GmodPath Implementation - String Conversions
	//-------------------------------------------------------------------------

	std::string GmodPath::toString() const
	{
		std::stringstream ss;
		toString( ss );
		return ss.str();
	}

	void GmodPath::toString( std::stringstream& builder, char separator ) const
	{
		if ( m_isEmpty )
			return;

		bool hasNodeWithLocation = false;

		for ( const auto& parent : m_parents )
		{
			if ( parent.location() )
			{
				hasNodeWithLocation = true;
				break;
			}
		}

		if ( !hasNodeWithLocation )
		{
			m_node.toString( builder );
			return;
		}

		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			if ( m_parents[i].location() )
			{
				builder << m_parents[i].code();

				if ( m_parents[i].location() )
				{
					builder << '.' << m_parents[i].location()->toString();
				}

				builder << separator;
			}
		}

		m_node.toString( builder );
	}

	std::string GmodPath::toFullPathString() const
	{
		std::stringstream ss;
		toFullPathString( ss );
		return ss.str();
	}

	void GmodPath::toFullPathString( std::stringstream& builder ) const
	{
		if ( m_isEmpty )
			return;

		for ( size_t i = 0; i < m_parents.size(); ++i )
		{
			m_parents[i].toString( builder );
			builder << '/';
		}

		m_node.toString( builder );
	}

	std::string GmodPath::toStringDump() const
	{
		std::stringstream ss;
		toStringDump( ss );
		return ss.str();
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

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Operators
	//-------------------------------------------------------------------------

	const GmodNode& GmodPath::operator[]( size_t depth ) const
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "Array access on empty path" );
			throw std::logic_error( "Cannot access nodes of an empty path" );
		}

		if ( depth >= length() )
		{
			SPDLOG_ERROR( "Array access out of bounds: index {} in path of length {}", depth, length() );
			throw std::out_of_range( "Node depth out of range" );
		}

		if ( depth < m_parents.size() )
			return m_parents[depth];
		else
			return m_node;
	}

	GmodNode& GmodPath::operator[]( size_t depth )
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "Mutable array access on empty path" );
			throw std::logic_error( "Cannot access nodes of an empty path" );
		}

		if ( depth >= length() )
		{
			SPDLOG_ERROR( "Mutable array access out of bounds: index {} in path of length {}", depth, length() );
			throw std::out_of_range( "Node depth out of range" );
		}

		if ( depth < m_parents.size() )
			return m_parents[depth];
		else
			return m_node;
	}

	bool GmodPath::operator==( const GmodPath& other ) const
	{
		return equals( other );
	}

	bool GmodPath::operator!=( const GmodPath& other ) const
	{
		return !equals( other );
	}

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Static Methods - Validation
	//-------------------------------------------------------------------------

	bool GmodPath::isValid( const std::vector<GmodNode>& parents, const GmodNode& node )
	{
		size_t missingLinkAt = std::numeric_limits<size_t>::max();

		return isValid( parents, node, missingLinkAt );
	}

	bool GmodPath::isValid( const std::vector<GmodNode>& parents, const GmodNode& node, size_t& missingLinkAt )
	{
		SPDLOG_INFO( "Validating path with {} parents and target node '{}'",
			parents.size(), node.code() );

		missingLinkAt = std::numeric_limits<size_t>::max();

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
				missingLinkAt = i;
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

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Static Methods - Parsing (Default GMOD/Locations)
	//-------------------------------------------------------------------------

	GmodPath GmodPath::parse(
		const std::string& item,
		VisVersion visVersion )
	{
		SPDLOG_DEBUG( "Parsing path '{}' with VIS version {}", item, static_cast<int>( visVersion ) );

		const Gmod& gmod = VIS::instance().gmod( visVersion );
		const Locations& locations = VIS::instance().locations( visVersion );

		return parse( item, gmod, locations );
	}

	bool GmodPath::tryParse( const std::string& item, VisVersion visVersion, GmodPath& path )
	{
		SPDLOG_DEBUG( "Attempting to parse path '{}' with VIS version {}", item, static_cast<int>( visVersion ) );

		try
		{
			const Gmod& gmod = VIS::instance().gmod( visVersion );
			const Locations& locations = VIS::instance().locations( visVersion );

			GmodPath parsedPath;
			if ( tryParse( item, gmod, locations, parsedPath ) )
			{
				path = std::move( parsedPath );
				return true;
			}
		}
		catch ( const std::exception& e )
		{
			SPDLOG_WARN( "Failed to load GMOD/Locations data for parsing: {}", e.what() );
		}

		return false;
	}

	GmodPath GmodPath::parseFullPath(
		const std::string& pathStr,
		VisVersion visVersion )
	{
		SPDLOG_DEBUG( "Parsing full path '{}' with VIS version {}", pathStr, static_cast<int>( visVersion ) );

		const Gmod& gmod = VIS::instance().gmod( visVersion );
		const Locations& locations = VIS::instance().locations( visVersion );

		GmodPath path;
		if ( !tryParseFullPath( pathStr, gmod, locations, path ) )
		{
			throw std::invalid_argument( "Failed to parse full path string: " + pathStr );
		}

		return path;
	}

	bool GmodPath::tryParseFullPath(
		const std::string& pathStr,
		VisVersion visVersion,
		GmodPath& path )
	{
		SPDLOG_DEBUG( "Attempting to parse full path '{}' with VIS version {}", pathStr, static_cast<int>( visVersion ) );

		try
		{
			const Gmod& gmod = VIS::instance().gmod( visVersion );
			const Locations& locations = VIS::instance().locations( visVersion );

			return tryParseFullPath( pathStr, gmod, locations, path );
		}
		catch ( const std::exception& e )
		{
			SPDLOG_WARN( "Failed to load GMOD/Locations data for parsing full path: {}", e.what() );
			return false;
		}
	}

	bool GmodPath::tryParseFullPath(
		std::string_view pathStr,
		VisVersion visVersion,
		GmodPath& path )
	{
		SPDLOG_DEBUG( "Attempting to parse full path (string_view) with VIS version {}", static_cast<int>( visVersion ) );

		try
		{
			const Gmod& gmod = VIS::instance().gmod( visVersion );
			const Locations& locations = VIS::instance().locations( visVersion );

			return tryParseFullPath( pathStr, gmod, locations, path );
		}
		catch ( const std::exception& e )
		{
			SPDLOG_WARN( "Failed to load GMOD/Locations data for parsing full path: {}", e.what() );
			return false;
		}
	}

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Static Methods - Parsing (Explicit GMOD/Locations)
	//-------------------------------------------------------------------------

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

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Private Static Methods - Parsing Internals
	//-------------------------------------------------------------------------

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
				GmodPath path( parentPath, std::move( targetNode ), gmod.visVersion(), false );
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

	GmodParsePathResult GmodPath::parseFullPathInternal(
		std::string_view pathStr,
		const Gmod& gmod,
		const Locations& locations )
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

		GmodNode targetNode = std::move( nodes.back() );
		nodes.pop_back();

		size_t missingLinkAt = std::numeric_limits<size_t>::max();
		if ( !isValid( nodes, targetNode, missingLinkAt ) )
		{
			SPDLOG_ERROR( "Invalid path: missing link at position {}", missingLinkAt );
			return GmodParsePathResult::Err( "Invalid path structure" );
		}

		try
		{
			GmodPath path( nodes, std::move( targetNode ), gmod.visVersion(), false );
			return GmodParsePathResult::Ok( std::move( path ) );
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Error creating path: {}", ex.what() );
			return GmodParsePathResult::Err( std::string( "Error creating path: " ) + ex.what() );
		}
	}

	//-------------------------------------------------------------------------
	// GmodPath Implementation - Enumerator and Iterator
	//-------------------------------------------------------------------------

	GmodPath::Enumerator::Enumerator( const GmodPath& path, std::optional<size_t> fromDepth )
		: m_path{ path },
		  m_currentIndex{ -1 },
		  m_endIndex{ path.length() },
		  m_startIndex{ fromDepth.value_or( 0 ) }
	{
		SPDLOG_DEBUG( "Creating GmodPath::Enumerator for path{}, starting from depth {}",
			path.isEmpty() ? " [empty]" : "", m_startIndex );

		if ( path.m_isEmpty )
		{
			SPDLOG_ERROR( "Attempted to create Enumerator for empty path" );
			throw std::logic_error( "Cannot enumerate over an empty path" );
		}

		if ( m_startIndex >= m_endIndex )
		{
			SPDLOG_ERROR( "Invalid fromDepth: {} (path length: {})", m_startIndex, m_endIndex );
			throw std::out_of_range( "Starting depth out of range for path" );
		}
	}

	std::pair<size_t, std::reference_wrapper<const GmodNode>> GmodPath::Enumerator::current() const
	{
		if ( m_currentIndex < 0 || static_cast<size_t>( m_currentIndex ) + m_startIndex >= m_endIndex )
		{
			SPDLOG_ERROR( "Attempted to access current() when not on a valid element" );
			throw std::runtime_error( "Enumerator not positioned on a valid element" );
		}

		size_t actualDepth = m_startIndex + static_cast<size_t>( m_currentIndex );
		return { actualDepth, std::ref( m_path[actualDepth] ) };
	}

	bool GmodPath::Enumerator::next()
	{
		if ( static_cast<size_t>( m_currentIndex + 1 ) + m_startIndex >= m_endIndex )
		{
			return false;
		}

		++m_currentIndex;
		SPDLOG_DEBUG( "Advanced enumerator to index {} (depth {})",
			m_currentIndex, m_startIndex + static_cast<size_t>( m_currentIndex ) );
		return true;
	}

	void GmodPath::Enumerator::reset()
	{
		SPDLOG_DEBUG( "Resetting enumerator to start position" );
		m_currentIndex = -1;
	}

	void GmodPath::Enumerator::Iterator::updateCache() const
	{
		if ( !m_isEnd && m_enumerator )
		{
			try
			{
				m_cachedValue = m_enumerator->current();
			}
			catch ( const std::exception& )
			{
				m_cachedValue = std::nullopt;
				SPDLOG_ERROR( "Failed to cache current value in iterator" );
			}
		}
		else
		{
			m_cachedValue = std::nullopt;
		}
	}

	GmodPath::Enumerator::Iterator::Iterator( Enumerator* enumerator, bool isEnd )
		: m_enumerator{ enumerator },
		  m_isEnd{ isEnd },
		  m_cachedValue{ std::nullopt }
	{
		if ( !m_isEnd && m_enumerator )
		{
			if ( !m_enumerator->next() )
			{
				m_isEnd = true;
			}
			else
			{
				updateCache();
			}
		}
	}

	GmodPath::Enumerator::Iterator::reference GmodPath::Enumerator::Iterator::operator*() const
	{
		if ( m_isEnd || !m_enumerator )
		{
			SPDLOG_ERROR( "Attempted to dereference end iterator" );
			throw std::runtime_error( "Cannot dereference end iterator" );
		}

		if ( !m_cachedValue )
		{
			updateCache();
			if ( !m_cachedValue )
			{
				SPDLOG_ERROR( "Failed to generate value for iterator dereference" );
				throw std::runtime_error( "Iterator in invalid state" );
			}
		}

		return *m_cachedValue;
	}

	GmodPath::Enumerator::Iterator& GmodPath::Enumerator::Iterator::operator++()
	{
		if ( m_isEnd || !m_enumerator )
		{
			SPDLOG_ERROR( "Attempted to increment end iterator" );
			throw std::runtime_error( "Cannot increment end iterator" );
		}

		if ( !m_enumerator->next() )
		{
			m_isEnd = true;
			m_cachedValue = std::nullopt;
		}
		else
		{
			updateCache();
		}

		return *this;
	}

	GmodPath::Enumerator::Iterator GmodPath::Enumerator::Iterator::operator++( int )
	{
		Iterator temp = *this;
		++( *this );
		return temp;
	}

	bool GmodPath::Enumerator::Iterator::operator==( const Iterator& other ) const
	{
		if ( m_isEnd && other.m_isEnd )
			return true;

		if ( m_isEnd != other.m_isEnd )
			return false;

		return m_enumerator == other.m_enumerator;
	}

	bool GmodPath::Enumerator::Iterator::operator!=( const Iterator& other ) const
	{
		return !( *this == other );
	}

	GmodPath::Enumerator::Iterator GmodPath::Enumerator::begin()
	{
		reset();
		return Iterator( this, false );
	}

	GmodPath::Enumerator::Iterator GmodPath::Enumerator::end()
	{
		return Iterator( nullptr, true );
	}

	GmodPath::Enumerator GmodPath::fullPath() const
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "Called fullPath() on empty path" );
			throw std::logic_error( "Cannot create enumerator for empty path" );
		}

		return Enumerator( *this );
	}

	GmodPath::Enumerator GmodPath::fullPathFrom( size_t fromDepth ) const
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "Called fullPathFrom() on empty path" );
			throw std::logic_error( "Cannot create enumerator for empty path" );
		}

		if ( fromDepth >= length() )
		{
			SPDLOG_ERROR( "Invalid fromDepth: {} (path length: {})", fromDepth, length() );
			throw std::out_of_range( "Starting depth out of range for path" );
		}

		return Enumerator( *this, fromDepth );
	}

	//-------------------------------------------------------------------------
	// GmodIndividualizableSet Implementation
	//-------------------------------------------------------------------------

	GmodIndividualizableSet::GmodIndividualizableSet( const std::vector<size_t>& nodeIndices, GmodPath& path )
		: m_nodeIndices{ nodeIndices },
		  m_path{ &path }
	{
		SPDLOG_INFO( "Creating individualizable set with {} nodes", nodeIndices.size() );

		if ( nodeIndices.empty() )
		{
			SPDLOG_ERROR( "GmodIndividualizableSet cannot be empty" );
			throw std::invalid_argument( "GmodIndividualizableSet cant be empty" );
		}

		for ( size_t nodeIndex : nodeIndices )
		{
			const auto& node = path[nodeIndex];
			bool isTarget = ( nodeIndex == path.length() - 1 );
			bool isInSet = nodeIndices.size() > 1;

			if ( !node.isIndividualizable( isTarget, isInSet ) )
			{
				throw std::invalid_argument( "Node at index " + std::to_string( nodeIndex ) + " is not individualizable" );
			}
		}

		std::optional<Location> firstLocation;
		bool hasSetLocation = false;

		for ( size_t nodeIndex : nodeIndices )
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
		for ( size_t nodeIndex : nodeIndices )
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

	std::vector<std::reference_wrapper<const GmodNode>> GmodIndividualizableSet::nodes() const
	{
		if ( !m_path )
		{
			SPDLOG_ERROR( "Attempted to access nodes() on invalid GmodIndividualizableSet (after build)" );
			throw std::runtime_error( "GmodIndividualizableSet is no longer valid" );
		}

		std::vector<std::reference_wrapper<const GmodNode>> result;
		result.reserve( m_nodeIndices.size() );

		for ( size_t idx : m_nodeIndices )
		{
			result.emplace_back( std::ref( ( *m_path )[idx] ) );
		}

		return result;
	}

	const std::vector<size_t>& GmodIndividualizableSet::nodeIndices() const noexcept
	{
		return m_nodeIndices;
	}

	std::optional<Location> GmodIndividualizableSet::location() const
	{
		if ( !m_path )
		{
			SPDLOG_ERROR( "Attempted to access location() on invalid GmodIndividualizableSet (after build)" );
			throw std::runtime_error( "GmodIndividualizableSet is no longer valid" );
		}

		if ( m_nodeIndices.empty() )
			return std::nullopt;

		return ( *m_path )[m_nodeIndices[0]].location();
	}

	void GmodIndividualizableSet::setLocation( const std::optional<Location>& newLocation )
	{
		if ( !m_path )
		{
			SPDLOG_ERROR( "Attempted to call setLocation() on invalid GmodIndividualizableSet (after build)" );
			throw std::runtime_error( "GmodIndividualizableSet is no longer valid" );
		}

		SPDLOG_INFO( "Setting location {} for {} nodes in individualizable set",
			newLocation ? newLocation->toString() : "null", m_nodeIndices.size() );

		for ( size_t idx : m_nodeIndices )
		{
			if ( newLocation )
			{
				( *m_path )[idx] = ( *m_path )[idx].withLocation( *newLocation );
			}
			else
			{
				( *m_path )[idx] = ( *m_path )[idx].withoutLocation();
			}
		}
	}

	GmodPath GmodIndividualizableSet::build()
	{
		if ( !m_path )
		{
			SPDLOG_ERROR( "Attempted to call build() on invalid GmodIndividualizableSet (after build)" );
			throw std::runtime_error( "GmodIndividualizableSet is no longer valid" );
		}

		SPDLOG_INFO( "Building modified path from individualizable set" );

		GmodPath result = std::move( *m_path );
		m_path = nullptr;

		return result;
	}

	std::string GmodIndividualizableSet::toString() const
	{
		std::stringstream ss;

		ss << "GmodIndividualizableSet [";

		if ( !m_path )
		{
			ss << "invalid/consumed";
		}
		else
		{
			ss << "indices: ";
			for ( size_t i = 0; i < m_nodeIndices.size(); ++i )
			{
				if ( i > 0 )
					ss << ", ";
				ss << m_nodeIndices[i];
			}

			ss << "; location: ";
			if ( auto loc = location() )
				ss << loc->toString();
			else
				ss << "null";
		}

		ss << "]";
		return ss.str();
	}
}
