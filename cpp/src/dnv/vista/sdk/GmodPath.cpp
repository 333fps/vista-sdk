/**
 * @file GmodPath.cpp
 * @brief Implementation of GmodPath and related classes for representing paths in the Generic Product Model (GMOD).
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodPath.h"

#include "dnv/vista/sdk/Codebook.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Internal Helper Classes
	//=====================================================================

	//----------------------------------------------
	// PathNode Struct
	//----------------------------------------------

	PathNode::PathNode( const std::string& code, const std::optional<Location>& location )
		: code{ code },
		  location{ location }
	{
	}

	//----------------------------------------------
	// LocationSetsVisitor Struct
	//----------------------------------------------

	LocationSetsVisitor::LocationSetsVisitor()
		: m_currentParentStart{ 0 }
	{
	}

	std::optional<std::tuple<size_t, size_t, std::optional<Location>>> LocationSetsVisitor::visit(
		const GmodNode& node,
		size_t i,
		const std::vector<const GmodNode*>& parents,
		const GmodNode& target )
	{
		SPDLOG_DEBUG( "LocationSetsVisitor: Visiting node '{}' at index {}", node.code(), i );

		bool isParent = Gmod::isPotentialParent( node.metadata().type() );
		bool isTargetNode = ( i == parents.size() );

		SPDLOG_DEBUG( "Node '{}': isTargetNode={}, isParent={}, isIndividualizable={}", node.code(), isTargetNode, isParent, node.isIndividualizable( isTargetNode ) );

		if ( m_currentParentStart == 0 )
		{
			if ( isParent )
				m_currentParentStart = i;
			if ( node.isIndividualizable( isTargetNode ) )
			{
				SPDLOG_DEBUG( "Single node is individualizable: [{},{}] with location {}",
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
						SPDLOG_DEBUG( "Found adjacent individualizable nodes: [{},{}] with location {}",
							m_currentParentStart, i, node.location() ? node.location()->toString() : "null" );
					}
				}
				else
				{
					bool skippedOne = false;
					bool hasComposition = false;

					for ( size_t j = m_currentParentStart + 1; j <= i; j++ )
					{
						const GmodNode& setNode = ( j < parents.size() ) ? *( parents[j] ) : target;

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
						const GmodNode& setNode = ( j < parents.size() ) ? *( parents[static_cast<size_t>( j )] ) : target;
						if ( setNode.isLeafNode() || j == parents.size() )
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
			{
				SPDLOG_DEBUG( "Target node forms singleton set: [{},{}] with location {}", i, i, node.location() ? node.location()->toString() : "null" );
				return std::make_tuple( i, i, node.location() );
			}
		}

		return std::nullopt;
	}

	//=====================================================================
	// GmodPath Class
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	GmodPath::GmodPath()
		: m_visVersion{ VisVersion::Unknown },
		  m_targetNode{},
		  m_nodes{},
		  m_isEmpty{ true }
	{
		SPDLOG_DEBUG( "Created empty GmodPath" );
	}

	GmodPath::GmodPath( const std::vector<const GmodNode*>& nodes, GmodNode targetNode, VisVersion visVersion, bool skipVerify )
		: m_visVersion{ visVersion },
		  m_targetNode{ std::move( targetNode ) },
		  m_nodes{ nodes },
		  m_isEmpty{ false }
	{
		SPDLOG_INFO( "Created GmodPath with target node '{}' and {} path nodes (including root)", m_targetNode.code(), m_nodes.size() );

		if ( !skipVerify )
		{
			size_t missingLinkAt = std::numeric_limits<size_t>::max();
			if ( !isValid( m_nodes, m_targetNode, missingLinkAt ) )
			{
				std::stringstream errorMsg;
				errorMsg << "Invalid path structure: ";

				if ( missingLinkAt < m_nodes.size() )
				{
					const GmodNode* parentPtr = m_nodes[missingLinkAt];
					const GmodNode* childPtr = ( missingLinkAt + 1 < m_nodes.size() ) ? m_nodes[missingLinkAt + 1] : &m_targetNode;
					errorMsg << "Parent '" << ( parentPtr ? parentPtr->code() : "null" ) << "' cannot have '"
							 << ( childPtr ? childPtr->code() : "null" ) << "' as a child";
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

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	const GmodNode* GmodPath::operator[]( size_t depth ) const
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

		if ( depth < m_nodes.size() )
			return m_nodes[depth];
		else
			return &m_targetNode;
	}

	bool GmodPath::operator==( const GmodPath& other ) const
	{
		return equals( other );
	}

	bool GmodPath::operator!=( const GmodPath& other ) const
	{
		return !equals( other );
	}

	//----------------------------------------------
	// Public Methods
	//----------------------------------------------

	size_t GmodPath::length() const noexcept
	{
		if ( m_isEmpty )
			return 0;

		return m_nodes.size() + 1;
	}

	bool GmodPath::isMappable() const noexcept
	{
		if ( m_isEmpty )
			return false;

		return m_targetNode.isMappable();
	}

	GmodPath GmodPath::withoutLocations() const
	{
		if ( m_isEmpty )
		{
			SPDLOG_DEBUG( "Called withoutLocations() on empty path" );
			return GmodPath();
		}

		GmodNode newTargetNode = m_targetNode.withoutLocation();

		SPDLOG_WARN( "GmodPath::withoutLocations() creates a copy but cannot modify pointed-to parent nodes." );
		return GmodPath( m_nodes, std::move( newTargetNode ), m_visVersion, true );
	}

	bool GmodPath::equals( const GmodPath& other ) const
	{
		if ( m_isEmpty != other.m_isEmpty )
			return false;
		if ( m_isEmpty )
			return true;
		if ( m_visVersion != other.m_visVersion )
			return false;

		if ( !m_targetNode.equals( other.m_targetNode ) )
			return false;
		if ( m_nodes.size() != other.m_nodes.size() )
			return false;

		for ( size_t i = 0; i < m_nodes.size(); ++i )
		{
			if ( m_nodes[i] == nullptr || other.m_nodes[i] == nullptr )
			{
				if ( m_nodes[i] != other.m_nodes[i] )
					return false;
			}

			else if ( !m_nodes[i]->equals( *other.m_nodes[i] ) )
			{
				return false;
			}
		}
		return true;
	}

	size_t GmodPath::hashCode() const noexcept
	{
		if ( m_isEmpty )
			return 0;

		size_t hash = m_targetNode.hashCode();
		hash = hash * 31 + static_cast<size_t>( m_visVersion );

		for ( const auto* nodePtr : m_nodes )
		{
			hash = hash * 31 + ( nodePtr ? nodePtr->hashCode() : 0 );
		}
		return hash;
	}

	bool GmodPath::isIndividualizable() const noexcept
	{
		if ( m_isEmpty )
			return false;

		if ( m_targetNode.isIndividualizable( true ) )
			return true;

		for ( const auto* nodePtr : m_nodes )
		{
			if ( nodePtr && nodePtr->isIndividualizable( false ) )
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
		for ( size_t i = 0; i < m_nodes.size(); ++i )
		{
			if ( !m_nodes[i] )
				continue;
		}

		SPDLOG_WARN( "GmodPath::individualizableSets() requires LocationSetsVisitor to be updated for pointer usage." );
		return {};
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	VisVersion GmodPath::visVersion() const noexcept
	{
		return m_visVersion;
	}

	const GmodNode& GmodPath::targetNode() const
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "Attempted to access node() on empty path" );
			throw std::logic_error( "Cannot access node of an empty path" );
		}

		return m_targetNode;
	}

	const std::vector<const GmodNode*>& GmodPath::nodes() const noexcept
	{
		return m_nodes;
	}

	//----------------------------------------------
	// Specific Accessors
	//----------------------------------------------

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

		const GmodNode* nodePtr = ( *this )[nodeDepth];
		if ( !nodePtr )
		{
			SPDLOG_ERROR( "Null node pointer encountered at depth {}", nodeDepth );
			return std::nullopt;
		}
		const auto& normalAssignmentNames = nodePtr->metadata().normalAssignmentNames();

		if ( normalAssignmentNames.empty() )
		{
			return std::nullopt;
		}

		for ( size_t i = length(); i > 0; --i )
		{
			size_t currentDepth = i - 1;
			const GmodNode* childPtr = ( *this )[currentDepth];
			if ( !childPtr )
				continue;

			auto it = normalAssignmentNames.find( childPtr->code() );
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

		results.reserve( m_nodes.size() + 1 );

		for ( size_t depth{ 0 }; depth < m_nodes.size(); ++depth )
		{
			const GmodNode* nodePtr = m_nodes[depth];
			if ( !nodePtr )
				continue;

			auto commonName{ nodePtr->metadata().commonName() };

			if ( commonName.has_value() )
			{
				SPDLOG_INFO( "Found common name '{}' for node at depth {}", commonName.value(), depth );
				results.emplace_back( depth, commonName.value() );
			}
		}

		auto targetCommonName{ m_targetNode.metadata().commonName() };
		if ( targetCommonName.has_value() )
		{
			auto depth{ m_nodes.size() };
			SPDLOG_INFO( "Found common name '{}' for target node at depth {}", targetCommonName.value(), depth );
			results.emplace_back( depth, targetCommonName.value() );
		}

		return results;
	}

	//----------------------------------------------
	// String Conversions
	//----------------------------------------------

	std::string GmodPath::toString() const
	{
		std::stringstream ss;
		toString( ss );
		return ss.str();
	}

	void GmodPath::toString( std::stringstream& builder, char separator ) const
	{
		if ( m_isEmpty )
		{
			return;
		}

		bool printedParent = false;
		for ( const auto* nodePtr : m_nodes )
		{
			if ( !nodePtr )
				continue;

			if ( nodePtr->isLeafNode() )
			{
				if ( printedParent )
				{
					builder << separator;
				}
				nodePtr->toString( builder );
				printedParent = true;
			}
		}

		if ( printedParent )
		{
			builder << separator;
		}

		m_targetNode.toString( builder );
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

		for ( size_t i = 0; i < m_nodes.size(); ++i )
		{
			const GmodNode* nodePtr = m_nodes[i];
			if ( !nodePtr )
			{
				builder << "[null]";
			}
			else
			{
				nodePtr->toString( builder );
			}
			builder << '/';
		}

		m_targetNode.toString( builder );
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
		builder << "Nodes (" << m_nodes.size() << "):\n";

		for ( size_t i = 0; i < m_nodes.size(); ++i )
		{
			const GmodNode* nodePtr = m_nodes[i];
			builder << "  [" << i << "] ";
			if ( !nodePtr )
			{
				builder << "[null]";
			}
			else
			{
				builder << nodePtr->code();
				if ( nodePtr->location().has_value() )
				{
					builder << "-" << nodePtr->location().value().toString();
				}
			}
			builder << "\n";
		}

		builder << "Target: " << m_targetNode.code();

		if ( m_targetNode.location().has_value() )
		{
			builder << "-" << m_targetNode.location().value().toString();
		}

		builder << " (Mappable: " << ( m_targetNode.isMappable() ? "Yes" : "No" ) << ")";
	}

	//----------------------------------------------
	// Static Validation Methods
	//----------------------------------------------

	bool GmodPath::isValid( const std::vector<const GmodNode*>& nodes, const GmodNode& targetNode )
	{
		size_t missingLinkAt = std::numeric_limits<size_t>::max();
		return isValid( nodes, targetNode, missingLinkAt );
	}

	bool GmodPath::isValid( const std::vector<const GmodNode*>& nodes, const GmodNode& targetNode, size_t& missingLinkAt )
	{
		SPDLOG_INFO( "Validating path with {} nodes and target node '{}'",
			nodes.size(), targetNode.code() );

		missingLinkAt = std::numeric_limits<size_t>::max();

		if ( nodes.empty() )
		{
			SPDLOG_ERROR( "Invalid path: Nodes list is empty" );
			return false;
		}

		if ( nodes[0] == nullptr || !nodes[0]->isRoot() )
		{
			SPDLOG_ERROR( "Invalid path: First node '{}' is null or not the root node",
				( nodes[0] ? nodes[0]->code() : "null" ) );
			return false;
		}

		std::unordered_set<std::string> codeSet;
		codeSet.insert( nodes[0]->code() );

		for ( size_t i = 0; i < nodes.size(); ++i )
		{
			const GmodNode* parentPtr = nodes[i];
			const GmodNode* childPtr = ( i + 1 < nodes.size() ) ? nodes[i + 1] : &targetNode;

			if ( parentPtr == nullptr || childPtr == nullptr )
			{
				SPDLOG_ERROR( "Invalid path: Null node pointer encountered during validation" );
				missingLinkAt = i;
				return false;
			}

			if ( !parentPtr->isChild( *childPtr ) )
			{
				SPDLOG_ERROR( "Invalid path: '{}' is not a parent of '{}'",
					parentPtr->code(), childPtr->code() );
				missingLinkAt = i;
				return false;
			}

			if ( i + 1 < nodes.size() )
			{
				if ( !codeSet.insert( childPtr->code() ).second )
				{
					SPDLOG_ERROR( "Recursion detected for '{}'", childPtr->code() );
					missingLinkAt = i + 1;
					return false;
				}
			}
		}

		if ( !codeSet.insert( targetNode.code() ).second )
		{
			SPDLOG_ERROR( "Recursion detected: Target node '{}' already exists in parent path", targetNode.code() );
			missingLinkAt = nodes.size();
			return false;
		}

		return true;
	}

	//----------------------------------------------
	// Static Parsing Methods
	//----------------------------------------------

	GmodPath GmodPath::parse(
		std::string_view item,
		VisVersion visVersion )
	{
		SPDLOG_DEBUG( "Parsing path '{}' with VIS version {}", item, static_cast<int>( visVersion ) );

		const Gmod& gmod = VIS::instance().gmod( visVersion );
		const Locations& locations = VIS::instance().locations( visVersion );

		return parse( std::string( item ), gmod, locations );
	}

	bool GmodPath::tryParse( std::string_view item, VisVersion visVersion, GmodPath& path )
	{
		SPDLOG_DEBUG( "Attempting to parse path '{}' with VIS version {}", item, static_cast<int>( visVersion ) );

		try
		{
			const Gmod& gmod = VIS::instance().gmod( visVersion );
			const Locations& locations = VIS::instance().locations( visVersion );

			GmodPath parsedPath;
			if ( tryParse( std::string( item ), gmod, locations, parsedPath ) )
			{
				path = std::move( parsedPath );
				return true;
			}
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_WARN( "Failed to load GMOD/Locations data for parsing: {}", ex.what() );
		}

		return false;
	}

	GmodPath GmodPath::parseFullPath(
		std::string_view pathStr,
		VisVersion visVersion )
	{
		SPDLOG_DEBUG( "Parsing full path '{}' with VIS version {}", pathStr, static_cast<int>( visVersion ) );

		const Gmod& gmod = VIS::instance().gmod( visVersion );
		const Locations& locations = VIS::instance().locations( visVersion );

		GmodPath path;
		if ( !tryParseFullPath( pathStr, gmod, locations, path ) )
		{
			throw std::invalid_argument( "Failed to parse full path string: " + std::string( pathStr ) );
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
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_WARN( "Failed to load GMOD/Locations data for parsing full path: {}", ex.what() );
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
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_WARN( "Failed to load GMOD/Locations data for parsing full path: {}", ex.what() );
			return false;
		}
	}

	GmodPath GmodPath::parse( const std::string& item, const Gmod& gmod, const Locations& locations )
	{
		SPDLOG_INFO( "Parsing path '{}' using provided GMOD and Locations", item );

		auto resultPtr = parseInternal( item, gmod, locations );

		if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( resultPtr.get() ) )
		{
			SPDLOG_INFO( "Successfully parsed path" );
			return std::move( okResult->path() );
		}

		if ( auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( resultPtr.get() ) )
		{
			SPDLOG_ERROR( "Failed to parse path: {}", errResult->error() );
			throw std::invalid_argument( "Failed to parse path: " + errResult->error() );
		}

		SPDLOG_ERROR( "Failed to parse path: unknown internal error" );
		throw std::runtime_error( "Failed to parse path: unknown internal error" );
	}

	bool GmodPath::tryParse( const std::string& item, const Gmod& gmod, const Locations& locations, GmodPath& path )
	{
		SPDLOG_INFO( "Attempting to parse path '{}' using provided GMOD and Locations", item );

		try
		{
			auto result = parseInternal( item, gmod, locations );

			auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() );
			if ( okResult != nullptr )
			{
				path = std::move( okResult->path() );
				SPDLOG_INFO( "Successfully parsed path" );
				return true;
			}

			auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( result.get() );
			if ( errResult != nullptr )
			{
				SPDLOG_ERROR( "Failed to parse path: {}", errResult->error() );
			}
			else
			{
				SPDLOG_ERROR( "Failed to parse path: unknown internal error (result was not Ok or Err)" );
			}
			return false;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during tryParse: {}", ex.what() );
			return false;
		}
	}

	bool GmodPath::tryParseFullPath( std::string_view pathStr, const Gmod& gmod, const Locations& locations, GmodPath& path )
	{
		SPDLOG_INFO( "Attempting to parse full path '{}' using provided GMOD and Locations", pathStr );

		try
		{
			auto result = parseFullPathInternal( pathStr, gmod, locations );

			auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( result.get() );
			if ( okResult != nullptr )
			{
				path = std::move( okResult->path() );
				SPDLOG_INFO( "Successfully parsed full path" );
				return true;
			}

			auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( result.get() );
			if ( errResult != nullptr )
			{
				SPDLOG_ERROR( "Failed to parse full path: {}", errResult->error() );
			}
			else
			{
				SPDLOG_ERROR( "Failed to parse full path: unknown internal error (result was not Ok or Err)" );
			}

			return false;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during tryParseFullPath: {}", ex.what() );
			return false;
		}
	}

	std::unique_ptr<GmodParsePathResult> GmodPath::parseInternal( const std::string& item, const Gmod& gmod, const Locations& locations )
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
				SPDLOG_ERROR( "Cannot parse empty path string: '{}'", item );
				return std::make_unique<GmodParsePathResult::Err>( "Path cannot be empty" + item );
			}

			std::string targetPart = parts.back();
			parts.pop_back();

			std::string targetCode;
			std::optional<Location> targetLocation;
			size_t targetDashPos = targetPart.find( '-' );
			if ( targetDashPos != std::string::npos )
			{
				targetCode = targetPart.substr( 0, targetDashPos );
				std::string locStr = targetPart.substr( targetDashPos + 1 );
				Location parsedLocation;
				if ( !locations.tryParse( locStr, parsedLocation ) )
				{
					SPDLOG_ERROR( "Failed to parse target location: '{}' in path '{}'", locStr, item );
					return std::make_unique<GmodParsePathResult::Err>( "Failed to parse target location: " + locStr );
				}
				targetLocation = parsedLocation;
			}
			else
			{
				targetCode = targetPart;
			}

			const GmodNode* targetNodePtr = nullptr;
			if ( !gmod.tryGetNode( targetCode, targetNodePtr ) )
			{
				SPDLOG_ERROR( "Failed to find target node with code: '{}' in path '{}'", targetCode, item );
				return std::make_unique<GmodParsePathResult::Err>( "Failed to get target node: " + targetCode );
			}
			if ( targetNodePtr == nullptr )
			{
				SPDLOG_ERROR( "Internal error: tryGetNode succeeded but returned null pointer for target code: '{}' in path '{}'", targetCode, item );
				return std::make_unique<GmodParsePathResult::Err>( "Internal error retrieving target node: " + targetCode );
			}

			GmodNode finalTargetNode;
			if ( targetLocation.has_value() )
			{
				finalTargetNode = targetNodePtr->withLocation( *targetLocation );
			}
			else
			{
				if ( targetNodePtr->location().has_value() )
				{
					finalTargetNode = targetNodePtr->withLocation( *targetNodePtr->location() );
				}
				else
				{
					finalTargetNode = targetNodePtr->withoutLocation();
				}
			}

			std::vector<const GmodNode*> parentPathPtrs;
			parentPathPtrs.reserve( parts.size() + 1 );

			const GmodNode* rootPtr = &gmod.rootNode();
			if ( !rootPtr )
			{
				return std::make_unique<GmodParsePathResult::Err>( "Internal error: Could not get root node from GMOD" );
			}
			parentPathPtrs.push_back( rootPtr );

			for ( const auto& partStr : parts )
			{
				std::string nodeCode;
				size_t dashPos = partStr.find( '-' );
				if ( dashPos != std::string::npos )
				{
					nodeCode = partStr.substr( 0, dashPos );
					std::string locStr = partStr.substr( dashPos + 1 );
					Location ignoredLocation;
					if ( !locations.tryParse( locStr, ignoredLocation ) )
					{
						SPDLOG_ERROR( "Failed to parse intermediate node location: '{}' in path '{}'", locStr, item );
						return std::make_unique<GmodParsePathResult::Err>( "Failed to parse intermediate node location: " + locStr );
					}
					SPDLOG_WARN( "Location specified for intermediate node '{}' in path string '{}' is ignored during parsing.", nodeCode, item );
				}
				else
				{
					nodeCode = partStr;
				}

				const GmodNode* nodePtr = nullptr;
				if ( !gmod.tryGetNode( nodeCode, nodePtr ) )
				{
					SPDLOG_ERROR( "Failed to find intermediate node with code: '{}' in path '{}'", nodeCode, item );
					return std::make_unique<GmodParsePathResult::Err>( "Failed to get intermediate node: " + nodeCode );
				}
				if ( nodePtr == nullptr )
				{
					SPDLOG_ERROR( "Internal error: tryGetNode succeeded but returned null pointer for intermediate code: '{}' in path '{}'", nodeCode, item );
					return std::make_unique<GmodParsePathResult::Err>( "Internal error retrieving intermediate node: " + nodeCode );
				}

				parentPathPtrs.push_back( nodePtr );
			}

			std::vector<const GmodNode*> remainingParentPtrs;
			if ( !gmod.pathExistsBetween( parentPathPtrs, finalTargetNode, remainingParentPtrs ) )
			{
				SPDLOG_ERROR( "No valid path exists between specified parents and target node '{}' in path '{}'", finalTargetNode.code(), item );
				return std::make_unique<GmodParsePathResult::Err>( "No valid path exists between specified parents and target node." );
			}

			parentPathPtrs.insert( parentPathPtrs.end(), remainingParentPtrs.begin(), remainingParentPtrs.end() );

			try
			{
				GmodPath path( parentPathPtrs, std::move( finalTargetNode ), gmod.visVersion(), false );
				return std::make_unique<GmodParsePathResult::Ok>( std::move( path ) );
			}
			catch ( const std::invalid_argument& ex )
			{
				SPDLOG_ERROR( "Error creating path object for '{}': {}", item, ex.what() );
				return std::make_unique<GmodParsePathResult::Err>( std::string( "Error creating path object: " ) + ex.what() );
			}
			catch ( const std::exception& ex )
			{
				SPDLOG_ERROR( "Unexpected error creating path object for '{}': {}", item, ex.what() );
				return std::make_unique<GmodParsePathResult::Err>( std::string( "Unexpected error creating path object: " ) + ex.what() );
			}
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during parsing of path '{}': {}", item, ex.what() );
			return std::make_unique<GmodParsePathResult::Err>( std::string( "Exception during parsing: " ) + ex.what() );
		}
	}

	std::unique_ptr<GmodParsePathResult> GmodPath::parseFullPathInternal(
		std::string_view pathStr,
		const Gmod& gmod,
		const Locations& locations )
	{
		SPDLOG_INFO( "Parsing full path '{}' with GMOD version {}", pathStr, static_cast<int>( gmod.visVersion() ) );

		if ( pathStr.empty() || std::all_of( pathStr.begin(), pathStr.end(), []( char c ) { return std::isspace( c ); } ) )
		{
			SPDLOG_ERROR( "Cannot parse empty full path string" );
			return std::make_unique<GmodParsePathResult::Err>( "Path cannot be empty" );
		}

		std::vector<std::string_view> parts;
		size_t start = 0;
		size_t end;
		while ( ( end = pathStr.find( '/', start ) ) != std::string_view::npos )
		{
			if ( end > start )
				parts.push_back( pathStr.substr( start, end - start ) );
			start = end + 1;
		}
		if ( start < pathStr.length() )
			parts.push_back( pathStr.substr( start ) );

		if ( parts.empty() )
		{
			SPDLOG_ERROR( "No parts found after splitting full path string: '{}'", pathStr );
			return std::make_unique<GmodParsePathResult::Err>(
				std::string( "Path cannot be empty or contain only separators: " ) + std::string( pathStr ) );
		}

		std::vector<const GmodNode*> nodePtrs;
		nodePtrs.reserve( parts.size() );
		std::optional<GmodNode> finalTargetNodeOpt;

		for ( size_t i = 0; i < parts.size(); ++i )
		{
			const auto& part = parts[i];

			std::string_view codeView;
			std::optional<Location> locationOpt;
			size_t dashPos = part.find( '-' );

			if ( dashPos != std::string_view::npos )
			{
				codeView = part.substr( 0, dashPos );
				std::string_view locStrView = part.substr( dashPos + 1 );
				std::string locStr( locStrView );

				Location parsedLocation;
				if ( !locations.tryParse( locStr, parsedLocation ) )
				{
					SPDLOG_ERROR( "Failed to parse location '{}' in full path part '{}'", locStr, part );
					return std::make_unique<GmodParsePathResult::Err>( "Failed to parse location: " + locStr );
				}
				locationOpt = parsedLocation;
			}
			else
			{
				codeView = part;
			}
			std::string code( codeView );

			const GmodNode* nodePtr = nullptr;
			if ( !gmod.tryGetNode( code, nodePtr ) )
			{
				SPDLOG_ERROR( "Failed to find node with code '{}' in full path '{}'", code, pathStr );
				return std::make_unique<GmodParsePathResult::Err>( "Failed to get node: " + code );
			}
			if ( nodePtr == nullptr )
			{
				SPDLOG_ERROR( "Internal error: tryGetNode succeeded but returned null pointer for code '{}' in full path '{}'", code, pathStr );
				return std::make_unique<GmodParsePathResult::Err>( "Internal error retrieving node: " + code );
			}

			if ( i < parts.size() - 1 )
			{
				nodePtrs.push_back( nodePtr );
				if ( locationOpt.has_value() )
				{
					SPDLOG_WARN( "Location specified for intermediate node '{}' in full path string '{}' is ignored during parsing.", code, pathStr );
				}
			}
			else
			{
				if ( locationOpt.has_value() )
				{
					finalTargetNodeOpt = nodePtr->withLocation( *locationOpt );
				}
				else
				{
					if ( nodePtr->location().has_value() )
					{
						finalTargetNodeOpt = nodePtr->withLocation( *nodePtr->location() );
					}
					else
					{
						finalTargetNodeOpt = nodePtr->withoutLocation();
					}
				}
			}
		}

		if ( !finalTargetNodeOpt )
		{
			SPDLOG_ERROR( "Internal error: Target node was not created during full path parsing for '{}'", pathStr );
			return std::make_unique<GmodParsePathResult::Err>( std::string( "Error creating path object: " ) + std::string( pathStr ) );
		}

		size_t missingLinkAt = std::numeric_limits<size_t>::max();
		if ( !isValid( nodePtrs, *finalTargetNodeOpt, missingLinkAt ) )
		{
			SPDLOG_ERROR( "Invalid path structure parsed from full path string '{}'. Validation failed.", pathStr );
			std::string errorDetail = "Invalid path structure";
			if ( missingLinkAt < nodePtrs.size() )
			{
				errorDetail += ": Node '" + ( nodePtrs[missingLinkAt] ? nodePtrs[missingLinkAt]->code() : "null" ) + "' validation failed.";
			}
			else if ( missingLinkAt == nodePtrs.size() )
			{
				errorDetail += ": Target node '" + finalTargetNodeOpt->code() + "' validation failed.";
			}
			return std::make_unique<GmodParsePathResult::Err>( errorDetail );
		}

		try
		{
			GmodPath path( nodePtrs, std::move( *finalTargetNodeOpt ), gmod.visVersion(), true );
			return std::make_unique<GmodParsePathResult::Ok>( std::move( path ) );
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Error creating GmodPath object after parsing full path '{}': {}", pathStr, ex.what() );
			return std::make_unique<GmodParsePathResult::Err>( std::string( "Error creating path object: " ) + ex.what() );
		}
	}

	//----------------------------------------------
	// Enumerator Inner Class
	//----------------------------------------------

	//---------------------------
	// Construction / Reset
	//---------------------------

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

	void GmodPath::Enumerator::reset()
	{
		SPDLOG_DEBUG( "Resetting enumerator to start position" );
		m_currentIndex = -1;
	}

	//---------------------------
	// Iteration
	//---------------------------

	std::pair<size_t, std::reference_wrapper<const GmodNode>> GmodPath::Enumerator::current() const
	{
		if ( m_currentIndex < 0 || static_cast<size_t>( m_currentIndex ) + m_startIndex >= m_endIndex )
		{
			SPDLOG_ERROR( "Attempted to access current() when not on a valid element" );
			throw std::runtime_error( "Enumerator not positioned on a valid element" );
		}

		size_t actualDepth = m_startIndex + static_cast<size_t>( m_currentIndex );
		const GmodNode* nodePtr = m_path[actualDepth];

		if ( !nodePtr )
		{
			SPDLOG_ERROR( "Enumerator::current(): Null node pointer encountered at depth {}", actualDepth );
			throw std::runtime_error( "Null node pointer in path during enumeration" );
		}

		return { actualDepth, std::ref( *nodePtr ) };
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

	//---------------------------
	// Enumerator Inner Class
	//---------------------------

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

	//----------------------------------------------
	// Enumerator Accessors
	//----------------------------------------------

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

	//=====================================================================
	// GmodIndividualizableSet Class
	//=====================================================================

	//----------------------------------------------
	// Construction
	//----------------------------------------------

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
			const GmodNode* nodePtr = path[nodeIndex];
			if ( !nodePtr )
			{
				SPDLOG_ERROR( "Null node pointer encountered at index {} during GmodIndividualizableSet construction", nodeIndex );
				throw std::runtime_error( "Invalid path state: null node pointer" );
			}
			bool isTarget = ( nodeIndex == path.length() - 1 );
			bool isInSet = nodeIndices.size() > 1;

			if ( !nodePtr->isIndividualizable( isTarget, isInSet ) )
			{
				throw std::invalid_argument( "Node '" + nodePtr->code() + "' at index " + std::to_string( nodeIndex ) + " is not individualizable in this context" );
			}
		}

		std::optional<Location> firstLocation;
		bool hasSetLocation = false;

		for ( size_t nodeIndex : nodeIndices )
		{
			const GmodNode* nodePtr = path[nodeIndex];
			if ( !nodePtr )
			{
				throw std::runtime_error( "Invalid path state: null node pointer" );
			}
			const auto& nodeLoc = nodePtr->location();

			if ( !hasSetLocation && nodeLoc.has_value() )
			{
				firstLocation = nodeLoc;
				hasSetLocation = true;
			}
			else if ( hasSetLocation && nodeLoc.has_value() && *nodeLoc != *firstLocation )
			{
				throw std::invalid_argument( "GmodIndividualizableSet nodes have different locations ('" +
											 firstLocation->toString() + "' vs '" + nodeLoc->toString() + "')" );
			}
		}

		bool hasLeafOrTarget = false;
		for ( size_t nodeIndex : nodeIndices )
		{
			const GmodNode* nodePtr = path[nodeIndex];
			if ( !nodePtr )
			{
				throw std::runtime_error( "Invalid path state: null node pointer" );
			}
			if ( nodePtr->isLeafNode() || nodeIndex == path.length() - 1 )
			{
				hasLeafOrTarget = true;
				break;
			}
		}

		if ( !hasLeafOrTarget )
		{
			throw std::invalid_argument( "GmodIndividualizableSet has no nodes that are part of short path (no leaf or target node)" );
		}
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

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
			const GmodNode* nodePtr = ( *m_path )[idx];
			if ( !nodePtr )
			{
				SPDLOG_ERROR( "Null node pointer encountered at index {} during GmodIndividualizableSet::nodes()", idx );
				throw std::runtime_error( "Invalid path state: null node pointer" );
			}
			result.emplace_back( std::ref( *nodePtr ) );
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

		const GmodNode* nodePtr = ( *m_path )[m_nodeIndices[0]];
		if ( !nodePtr )
		{
			SPDLOG_ERROR( "Null node pointer encountered at index {} during GmodIndividualizableSet::location()", m_nodeIndices[0] );
			throw std::runtime_error( "Invalid path state: null node pointer" );
		}
		return nodePtr->location();
	}

	//----------------------------------------------
	// Mutators and Operations
	//----------------------------------------------

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

	//----------------------------------------------
	// Conversion
	//----------------------------------------------

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
			try
			{
				if ( auto loc = location() )
					ss << loc->toString();
				else
					ss << "null";
			}
			catch ( const std::exception& e )
			{
				ss << "<error: " << e.what() << ">";
			}
		}

		ss << "]";
		return ss.str();
	}

	//=====================================================================
	// GmodParsePathResult Class
	//=====================================================================

	GmodParsePathResult::Ok::Ok( GmodPath&& path )
		: m_path{ std::move( path ) }
	{
	}

	GmodParsePathResult::Ok::Ok( Ok&& other ) noexcept
		: GmodParsePathResult( std::move( other ) ), m_path( std::move( other.m_path ) )
	{
	}

	GmodPath& GmodParsePathResult::Ok::path()
	{
		return m_path;
	}

	GmodParsePathResult::Err::Err( const std::string& errorMessage ) : m_error( errorMessage )
	{
	}

	GmodParsePathResult::Err::Err( Err&& other ) noexcept
		: GmodParsePathResult( std::move( other ) ), m_error( std::move( other.m_error ) )
	{
	}

	const std::string& GmodParsePathResult::Err::error()
	{
		return m_error;
	}
}
