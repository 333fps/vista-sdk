/**
 * @file GmodPath.cpp
 * @brief Implementation of GmodPath and related classes for representing paths in the Generic Product Model (GMOD).
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodPath.h"

#include "dnv/vista/sdk/Codebook.h"
#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodTraversal.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/Locations.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	struct ParsedPathNodeFromString
	{
		std::string code;
		std::optional<Location> location;
	};

	//=====================================================================
	// Internal Helper Classes
	//=====================================================================

	struct ParseInternalContext
	{
		std::queue<ParsedPathNodeFromString> partsToFindQueue;
		ParsedPathNodeFromString currentPartToFind;
		std::map<std::string, Location> foundLocationsFromInput;
		std::unique_ptr<GmodPath> resultingPath;
		const Gmod& gmod;
		std::string_view originalInputPath;
		const Locations& locations;
		bool errorOccurred = false;
		std::string errorMessage;

		ParseInternalContext( std::queue<ParsedPathNodeFromString> initialParts, const Gmod& g, const Locations& locs, std::string_view originalInput )
			: partsToFindQueue( std::move( initialParts ) ),
			  gmod( g ),
			  locations( locs ),
			  originalInputPath( originalInput )
		{
			if ( !partsToFindQueue.empty() )
			{
				currentPartToFind = partsToFindQueue.front();
				partsToFindQueue.pop();
			}
		}

		/** @brief Copy constructor */
		ParseInternalContext( const ParseInternalContext& ) = delete;

		/** @brief Copy assignment operator */
		ParseInternalContext& operator=( const ParseInternalContext& ) = delete;
	};

	static std::optional<std::vector<ParsedPathNodeFromString>> splitAndParseSegments(
		std::string_view item_sv, const Gmod& gmod, const Locations& locations, std::string& outErrorMessage )
	{
		std::vector<ParsedPathNodeFromString> parsedNodesVector;
		std::string_view remaining_sv = item_sv;

		do
		{
			size_t slash_pos = remaining_sv.find( '/' );
			std::string_view part_sv = remaining_sv.substr( 0, slash_pos );

			if ( part_sv.empty() )
			{
				outErrorMessage = fmt::format( "Empty path segment encountered in input: {}", item_sv );
				return std::nullopt;
			}

			ParsedPathNodeFromString pn;
			size_t dash_pos = part_sv.find( '-' );
			std::string_view node_code_sv;

			if ( dash_pos != std::string_view::npos )
			{
				node_code_sv = part_sv.substr( 0, dash_pos );
				std::string_view loc_sv = part_sv.substr( dash_pos + 1 );
				if ( loc_sv.empty() )
				{
					outErrorMessage = fmt::format( "Empty location string in part: {}", part_sv );
					return std::nullopt;
				}
				Location parsed_loc;
				if ( !locations.tryParse( std::string( loc_sv ), parsed_loc ) )
				{
					outErrorMessage = fmt::format( "Failed to parse location: {}", loc_sv );
					return std::nullopt;
				}
				pn.location = parsed_loc;
			}
			else
			{
				node_code_sv = part_sv;
			}

			if ( node_code_sv.empty() )
			{
				outErrorMessage = fmt::format( "Empty node code in path segment: {}", part_sv );
				return std::nullopt;
			}
			pn.code = std::string( node_code_sv );

			const GmodNode* tempNodeCheck = nullptr;
			if ( !gmod.tryGetNode( pn.code, tempNodeCheck ) || !tempNodeCheck )
			{
				outErrorMessage = "GMOD node not found: '" + pn.code + "'";
				return std::nullopt;
			}
			parsedNodesVector.push_back( pn );

			if ( slash_pos == std::string_view::npos )
				break;
			remaining_sv.remove_prefix( slash_pos + 1 );
		} while ( true );

		if ( parsedNodesVector.empty() )
		{
			outErrorMessage = "No path segments found in input";
			return std::nullopt;
		}
		return parsedNodesVector;
	}

	static std::optional<std::vector<const GmodNode*>> getAbsolutePrefixTemplates(
		const GmodNode* targetNode, const Gmod& gmod, std::string& outErrorMessage )
	{
		std::vector<const GmodNode*> prefixTemplates;
		const GmodNode* ascendant = targetNode;

		std::unordered_set<std::string> visitedInPrefix;

		while ( ascendant != nullptr && !ascendant->isRoot() )
		{
			if ( !visitedInPrefix.insert( ascendant->code() ).second )
			{
				outErrorMessage = "Cycle detected in GMOD structure while resolving path to root for " + targetNode->code();
				return std::nullopt;
			}

			const auto& parents = ascendant->parents();
			if ( parents.empty() )
			{
				outErrorMessage = "GMOD structure error: Node '" + ascendant->code() + "' (on path to root for '" + targetNode->code() + "') has no parents but is not root.";
				return std::nullopt;
			}
			if ( parents.size() > 1 )
			{
				outErrorMessage = "Path for node '" + targetNode->code() + "' is ambiguous: GMOD node '" + ascendant->code() + "' has multiple parents.";
				return std::nullopt;
			}
			ascendant = parents[0];
			prefixTemplates.push_back( ascendant );
		}

		if ( ascendant == nullptr || !ascendant->isRoot() )
		{
			outErrorMessage = "Could not resolve path to GMOD root 'VE' for node '" + targetNode->code() + "'. Structure might be disconnected.";
			return std::nullopt;
		}

		std::reverse( prefixTemplates.begin(), prefixTemplates.end() );

		if ( !prefixTemplates.empty() && prefixTemplates.back() == targetNode )
		{
			prefixTemplates.pop_back();
		}

		return prefixTemplates;
	}

	//----------------------------------------------
	// LocationSetsVisitor Struct
	//----------------------------------------------

	LocationSetsVisitor::LocationSetsVisitor()
		: m_currentParentStart{ std::numeric_limits<size_t>::max() }
	{
	}

	std::optional<std::tuple<size_t, size_t, std::optional<Location>>> LocationSetsVisitor::visit(
		const GmodNode& visitedNodeInstance,
		size_t visitedNodeIndex,
		const std::vector<GmodNode>& allPathParentInstances,
		const GmodNode& pathTargetInstance )
	{
		bool isParent = Gmod::isPotentialParent( visitedNodeInstance.metadata().type() );

		bool isTargetNode = ( visitedNodeIndex == allPathParentInstances.size() );

		if ( m_currentParentStart == std::numeric_limits<size_t>::max() )
		{
			if ( isParent )
			{
				m_currentParentStart = visitedNodeIndex;
			}
			if ( visitedNodeInstance.isIndividualizable( isTargetNode ) )
			{
				return std::make_tuple( visitedNodeIndex, visitedNodeIndex, visitedNodeInstance.location() );
			}
		}
		else
		{
			if ( isParent || isTargetNode )
			{
				std::optional<std::tuple<size_t, size_t, std::optional<Location>>> identifiedSetOpt = std::nullopt;

				if ( m_currentParentStart + 1 == visitedNodeIndex )
				{
					if ( visitedNodeInstance.isIndividualizable( isTargetNode ) )
					{
						identifiedSetOpt = std::make_tuple( visitedNodeIndex, visitedNodeIndex, visitedNodeInstance.location() );
					}
				}
				else
				{
					size_t skippedOneAtIndex = std::numeric_limits<size_t>::max();
					bool hasComposition = false;

					for ( size_t j = m_currentParentStart + 1; j <= visitedNodeIndex; ++j )
					{
						const GmodNode& setNode = ( j < allPathParentInstances.size() ) ? allPathParentInstances[j] : pathTargetInstance;
						bool isCurrentSetNodeTheTarget = ( j == allPathParentInstances.size() );

						if ( !setNode.isIndividualizable( isCurrentSetNodeTheTarget, true ) )
						{
							if ( identifiedSetOpt.has_value() )
							{
								skippedOneAtIndex = j;
							}
							continue;
						}

						if ( identifiedSetOpt.has_value() && std::get<2>( identifiedSetOpt.value() ).has_value() &&
							 setNode.location().has_value() &&
							 std::get<2>( identifiedSetOpt.value() ).value() != setNode.location().value() )
						{
							SPDLOG_ERROR( "LocationSetsVisitor: Different locations in the same nodeset: {} vs {}",
								std::get<2>( identifiedSetOpt.value() )->toString(), setNode.location()->toString() );
							throw std::runtime_error( "Mapping error: different locations in the same nodeset" );
						}

						if ( skippedOneAtIndex != std::numeric_limits<size_t>::max() )
						{
							SPDLOG_ERROR( "LocationSetsVisitor: Can't skip in the middle of individualizable set. Skipped at {}", skippedOneAtIndex );
							throw std::runtime_error( "Can't skip in the middle of individualizable set" );
						}

						if ( setNode.isFunctionComposition() )
						{
							hasComposition = true;
						}

						std::optional<Location> commonLocation = identifiedSetOpt.has_value() && std::get<2>( identifiedSetOpt.value() ).has_value()
																	 ? std::get<2>( identifiedSetOpt.value() )
																	 : setNode.location();
						size_t startIdx = identifiedSetOpt.has_value() ? std::get<0>( identifiedSetOpt.value() ) : j;
						size_t endIdx = j;
						identifiedSetOpt = std::make_tuple( startIdx, endIdx, commonLocation );
					}

					if ( identifiedSetOpt.has_value() &&
						 std::get<0>( identifiedSetOpt.value() ) == std::get<1>( identifiedSetOpt.value() ) &&
						 hasComposition )
					{
						identifiedSetOpt = std::nullopt;
					}
				}

				m_currentParentStart = visitedNodeIndex;

				if ( identifiedSetOpt.has_value() )
				{
					bool setContainsLeafOrTarget = false;
					for ( size_t k = std::get<0>( identifiedSetOpt.value() ); k <= std::get<1>( identifiedSetOpt.value() ); ++k )
					{
						const GmodNode& nodeInSet = ( k < allPathParentInstances.size() ) ? allPathParentInstances[k] : pathTargetInstance;
						if ( nodeInSet.isLeafNode() || ( k == allPathParentInstances.size() ) )
						{
							setContainsLeafOrTarget = true;
							break;
						}
					}
					if ( setContainsLeafOrTarget )
					{
						return identifiedSetOpt;
					}
				}
			}
			if ( isTargetNode && visitedNodeInstance.isIndividualizable( isTargetNode ) )
			{
				return std::make_tuple( visitedNodeIndex, visitedNodeIndex, visitedNodeInstance.location() );
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
		  m_isEmpty{ true }
	{
		SPDLOG_DEBUG( "GmodPath default constructed (empty)." );
	}

	GmodPath::GmodPath( const std::vector<const GmodNode*>& initialParentNodeTemplates,
		const GmodNode& initialTargetNodeTemplate,
		VisVersion visVersion,
		bool skipVerify )
		: m_visVersion( visVersion )
	{
		std::vector<GmodNode> workingParentNodes;

		for ( const auto* ptr : initialParentNodeTemplates )
		{
			if ( ptr )
				workingParentNodes.push_back( *ptr );
		}
		GmodNode workingTargetNode = initialTargetNodeTemplate;

		if ( !skipVerify )
		{
			LocationSetsVisitor visitor;
			size_t currentPathLength = workingParentNodes.size() + 1;
			for ( size_t i = 0; i < currentPathLength; ++i )
			{
				const GmodNode& nodeToVisit = ( i < workingParentNodes.size() ) ? workingParentNodes[i] : workingTargetNode;

				auto result = visitor.visit( nodeToVisit, i, workingParentNodes, workingTargetNode );

				if ( result.has_value() )
				{
					auto [setStart, setEnd, commonLocationOpt] = *result;
					SPDLOG_DEBUG( "GmodPath Ctor: LocationSetsVisitor found set [{}, {}] with location: {}",
						setStart, setEnd, commonLocationOpt.has_value() ? commonLocationOpt->toString() : "none" );

					if ( commonLocationOpt.has_value() )
					{
						for ( size_t k = setStart; k <= setEnd; ++k )
						{
							if ( k < workingParentNodes.size() )
							{
								workingParentNodes[k] = workingParentNodes[k].withLocation( commonLocationOpt.value() );
							}
							else
							{
								workingTargetNode = workingTargetNode.withLocation( commonLocationOpt.value() );
							}
						}
					}
					else
					{
					}
				}
				else
				{
					const GmodNode& nodeToCheck = ( i < workingParentNodes.size() ) ? workingParentNodes[i] : workingTargetNode;
					if ( nodeToCheck.location().has_value() )
					{
						SPDLOG_WARN( "GmodPath Ctor: Node '{}' at index {} has a location but was not part of any set identified by LocationSetsVisitor.", nodeToCheck.code(), i );
					}
				}
			}
		}

		m_ownedParentNodes = std::move( workingParentNodes );
		m_ownedTargetNode = std::move( workingTargetNode );

		m_isEmpty = false;
		SPDLOG_INFO( "GmodPath constructed. Target: '{}', {} owned parent nodes.", m_ownedTargetNode.code(), m_ownedParentNodes.size() );
	}

	GmodPath::GmodPath( const GmodPath& other )
		: m_visVersion( other.m_visVersion ),
		  m_ownedTargetNode( other.m_ownedTargetNode ),
		  m_ownedParentNodes( other.m_ownedParentNodes ),
		  m_isEmpty( other.m_isEmpty )
	{
		SPDLOG_DEBUG( "GmodPath copy constructed. Target: '{}', {} parents.", m_ownedTargetNode.code(), m_ownedParentNodes.size() );
	}

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	GmodPath& GmodPath::operator=( const GmodPath& other )
	{
		if ( this == &other )
		{
			return *this;
		}
		m_visVersion = other.m_visVersion;
		m_ownedTargetNode = other.m_ownedTargetNode;
		m_ownedParentNodes = other.m_ownedParentNodes;
		m_isEmpty = other.m_isEmpty;

		SPDLOG_DEBUG( "GmodPath copy assigned. Target: '{}', {} parents.", m_ownedTargetNode.code(), m_ownedParentNodes.size() );

		return *this;
	}

	const GmodNode* GmodPath::operator[]( size_t depth ) const
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "GmodPath::operator[]: Array access on empty path." );
			throw std::logic_error( "GmodPath::operator[]: Array access on empty path." );
		}

		if ( depth >= ( m_ownedParentNodes.size() + 1 ) )
		{
			SPDLOG_ERROR( "GmodPath::operator[]: Index {} out of range for path of length {}.", depth, length() );
			throw std::out_of_range(
				fmt::format( "GmodPath::operator[]: Index {} out of range for path of length {}.", depth, length() ) );
		}

		if ( depth < m_ownedParentNodes.size() )
		{
			return &m_ownedParentNodes[depth];
		}
		else
		{
			return &m_ownedTargetNode;
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

	//----------------------------------------------
	// Public Methods
	//----------------------------------------------

	size_t GmodPath::length() const noexcept
	{
		if ( m_isEmpty )
		{
			return 0;
		}

		return m_ownedParentNodes.size() + 1;
	}

	bool GmodPath::isEmpty() const noexcept
	{
		return m_isEmpty;
	}

	bool GmodPath::isMappable() const noexcept
	{
		if ( m_isEmpty )
		{
			return false;
		}

		return m_ownedTargetNode.isMappable();
	}

	GmodPath GmodPath::withoutLocations() const
	{
		if ( m_isEmpty )
		{
			SPDLOG_DEBUG( "Called withoutLocations() on empty path" );
			return GmodPath();
		}

		std::vector<GmodNode> newParentNodesWithoutLoc;
		newParentNodesWithoutLoc.reserve( m_ownedParentNodes.size() );
		for ( const auto& parent : m_ownedParentNodes )
		{
			newParentNodesWithoutLoc.push_back( parent.withoutLocation() );
		}

		GmodNode newTargetNodeWithoutLoc = m_ownedTargetNode.withoutLocation();

		GmodPath newPath;
		newPath.m_visVersion = m_visVersion;
		newPath.m_ownedParentNodes = std::move( newParentNodesWithoutLoc );
		newPath.m_ownedTargetNode = std::move( newTargetNodeWithoutLoc );
		newPath.m_isEmpty = false;
		SPDLOG_DEBUG( "GmodPath::withoutLocations() created a new path with owned nodes without locations." );
		return newPath;
	}

	bool GmodPath::equals( const GmodPath& other ) const
	{
		if ( m_isEmpty != other.m_isEmpty )
			return false;
		if ( m_isEmpty )
			return true;
		if ( m_visVersion != other.m_visVersion )
			return false;

		if ( !m_ownedTargetNode.equals( other.m_ownedTargetNode ) )
			return false;
		if ( m_ownedParentNodes.size() != other.m_ownedParentNodes.size() )
			return false;

		for ( size_t i = 0; i < m_ownedParentNodes.size(); ++i )
		{
			if ( !m_ownedParentNodes[i].equals( other.m_ownedParentNodes[i] ) )
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

		size_t hash = m_ownedTargetNode.hashCode();
		hash = hash * 31 + static_cast<size_t>( m_visVersion );

		for ( const auto& node : m_ownedParentNodes )
		{
			hash = hash * 31 + node.hashCode();
		}
		return hash;
	}

	bool GmodPath::isIndividualizable() const
	{
		if ( m_isEmpty )
			return false;

		LocationSetsVisitor visitor;
		for ( size_t i = 0; i < length(); ++i )
		{
			const GmodNode& currentNodeInstance = ( i < m_ownedParentNodes.size() )
													  ? m_ownedParentNodes[i]
													  : m_ownedTargetNode;

			auto setResult = visitor.visit( currentNodeInstance, i, m_ownedParentNodes, m_ownedTargetNode );
			if ( setResult.has_value() )
			{
				SPDLOG_DEBUG( "GmodPath::isIndividualizable: Found individualizable set at index {}", i );
				return true;
			}
		}
		SPDLOG_DEBUG( "GmodPath::isIndividualizable: No individualizable sets found." );
		return false;
	}

	std::vector<GmodIndividualizableSet> GmodPath::individualizableSets()
	{
		if ( m_isEmpty )
			return {};

		std::vector<GmodIndividualizableSet> result;
		LocationSetsVisitor visitor;
		for ( size_t i = 0; i < length(); ++i )
		{
			const GmodNode& currentNodeInstance = ( i < m_ownedParentNodes.size() )
													  ? m_ownedParentNodes[i]
													  : m_ownedTargetNode;

			auto setResultOpt = visitor.visit( currentNodeInstance, i, m_ownedParentNodes, m_ownedTargetNode );
			if ( !setResultOpt.has_value() )
			{
				continue;
			}

			auto [setStart, setEnd, commonLocationOpt] = setResultOpt.value();

			std::vector<size_t> nodeIndices;
			for ( size_t j = setStart; j <= setEnd; ++j )
			{
				nodeIndices.push_back( j );
			}

			if ( !nodeIndices.empty() )
			{
				try
				{
					result.emplace_back( nodeIndices, *this );
				}
				catch ( const std::exception& ex )
				{
					SPDLOG_ERROR( "Failed to create GmodIndividualizableSet for indices [{}, {}]: {}", setStart, setEnd, ex.what() );
				}
			}
		}

		return result;
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
			SPDLOG_ERROR( "GmodPath::targetNode(): Attempted to access target node of an empty path." );
			throw std::logic_error( "GmodPath::targetNode(): Attempted to access target node of an empty path." );
		}

		return m_ownedTargetNode;
	}

	//----------------------------------------------
	// Specific Accessors
	//----------------------------------------------

	std::optional<std::string> GmodPath::normalAssignmentName( size_t nodeDepth ) const
	{
		if ( m_isEmpty )
		{
			SPDLOG_ERROR( "GmodPath::normalAssignmentName: Called on an empty path." );
			throw std::logic_error( "GmodPath::normalAssignmentName: Called on an empty path." );
		}

		if ( nodeDepth >= length() )
		{
			SPDLOG_ERROR( "GmodPath::normalAssignmentName: Index {} out of range for path of length {}.", nodeDepth, length() );
			throw std::out_of_range( fmt::format(
				"GmodPath::normalAssignmentName: Index {} out of range for path of length {}.", nodeDepth, length() ) );
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

		if ( m_isEmpty )
			return results;

		results.reserve( m_ownedParentNodes.size() + 1 );

		for ( size_t depth{ 0 }; depth < m_ownedParentNodes.size(); ++depth )
		{
			const GmodNode& node = m_ownedParentNodes[depth];

			auto commonName{ node.metadata().commonName() };

			if ( commonName.has_value() )
			{
				SPDLOG_INFO( "Found common name '{}' for node at depth {}", commonName.value(), depth );
				results.emplace_back( depth, commonName.value() );
			}
		}

		auto targetCommonName{ m_ownedTargetNode.metadata().commonName() };
		if ( targetCommonName.has_value() )
		{
			auto depth{ m_ownedParentNodes.size() };
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
		for ( const auto& node : m_ownedParentNodes )
		{
			if ( node.isLeafNode() )
			{
				if ( printedParent )
				{
					builder << separator;
				}
				node.toString( builder );
				printedParent = true;
			}
		}

		if ( printedParent )
		{
			builder << separator;
		}

		m_ownedTargetNode.toString( builder );
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

		for ( size_t i = 0; i < m_ownedParentNodes.size(); ++i )
		{
			const GmodNode& node = m_ownedParentNodes[i];
			node.toString( builder );
			builder << '/';
		}

		m_ownedTargetNode.toString( builder );
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
		if ( m_isEmpty )
		{
			builder << "GmodPath [Empty, VIS Version: " << static_cast<int>( m_visVersion ) << "]\n";
			return;
		}

		builder << "GmodPath [VIS Version: " << static_cast<int>( m_visVersion ) << "]\n";
		builder << "Parent Nodes (" << m_ownedParentNodes.size() << "):\n";

		for ( size_t i = 0; i < m_ownedParentNodes.size(); ++i )
		{
			const GmodNode& node = m_ownedParentNodes[i];
			builder << "  [" << i << "] ";
			builder << node.code();
			if ( node.location().has_value() )
			{
				builder << "-" << node.location().value().toString();
			}
			builder << "\n";
		}

		builder << "Target: " << m_ownedTargetNode.code();

		if ( m_ownedTargetNode.location().has_value() )
		{
			builder << "-" << m_ownedTargetNode.location().value().toString();
		}

		builder << " (Mappable: " << ( m_ownedTargetNode.isMappable() ? "Yes" : "No" ) << ")";
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

	GmodPath GmodPath::parse( std::string_view item, VisVersion visVersion )
	{
		SPDLOG_DEBUG( "Parsing path '{}' with VIS version {}", item, static_cast<int>( visVersion ) );

		const Gmod& gmod = VIS::instance().gmod( visVersion );
		const Locations& locations = VIS::instance().locations( visVersion );

		return parse( std::string( item ), gmod, locations );
	}

	GmodPath GmodPath::parse( const std::string& item, VisVersion visVersion, const Gmod& gmod )
	{
		SPDLOG_DEBUG( "Parsing path '{}' with VIS version {} using provided GMOD (version {})",
			item, static_cast<int>( visVersion ), static_cast<int>( gmod.visVersion() ) );

		if ( gmod.visVersion() != visVersion )
		{
			SPDLOG_WARN(
				"GmodPath::parse called with visVersion {} for Locations, but provided GMOD instance has visVersion {}.",
				static_cast<int>( visVersion ), static_cast<int>( gmod.visVersion() ) );
		}

		const Locations& locations = VIS::instance().locations( visVersion );

		return parse( item, gmod, locations );
	}

	GmodPath GmodPath::parseFullPath(
		std::string_view pathStr,
		VisVersion visVersion )
	{
		SPDLOG_DEBUG( "Parsing full path '{}' with VIS version {}", pathStr, static_cast<int>( visVersion ) );

		const Gmod& gmod = VIS::instance().gmod( visVersion );
		const Locations& locations = VIS::instance().locations( visVersion );

		auto resultPtr = parseFullPathInternal( pathStr, gmod, locations );

		if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( resultPtr.get() ) )
		{
			SPDLOG_INFO( "Successfully parsed full path" );
			return std::move( okResult->path() );
		}

		if ( auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( resultPtr.get() ) )
		{
			SPDLOG_ERROR( "Failed to parse full path: {}", errResult->error() );
			throw std::invalid_argument( "Failed to parse full path: " + errResult->error() );
		}

		SPDLOG_ERROR( "Failed to parse full path: unknown internal error from parseFullPathInternal" );
		throw std::runtime_error( "Failed to parse full path: unknown internal error" );
	}

	GmodPath GmodPath::parseFullPath( const std::string& item, VisVersion visVersion, const Gmod& gmod )
	{
		SPDLOG_DEBUG( "Parsing full path '{}' with VIS version {} using provided GMOD (version {})",
			item, static_cast<int>( visVersion ), static_cast<int>( gmod.visVersion() ) );

		if ( gmod.visVersion() != visVersion )
		{
			SPDLOG_WARN(
				"GmodPath::parseFullPath called with visVersion {} for Locations, but provided GMOD instance has visVersion {}.",
				static_cast<int>( visVersion ), static_cast<int>( gmod.visVersion() ) );
		}

		const Locations& locations = VIS::instance().locations( visVersion );

		auto resultPtr = parseFullPathInternal( item, gmod, locations );

		if ( auto* okResult = dynamic_cast<GmodParsePathResult::Ok*>( resultPtr.get() ) )
		{
			SPDLOG_INFO( "Successfully parsed full path" );
			return std::move( okResult->path() );
		}
		if ( auto* errResult = dynamic_cast<GmodParsePathResult::Err*>( resultPtr.get() ) )
		{
			SPDLOG_ERROR( "Failed to parse full path: {}", errResult->error() );
			throw std::invalid_argument( "Failed to parse full path: " + errResult->error() );
		}
		SPDLOG_ERROR( "Failed to parse full path: unknown internal error from parseFullPathInternal" );
		throw std::runtime_error( "Failed to parse full path: unknown internal error" );
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

	bool GmodPath::tryParseFullPath( std::string_view pathStr, VisVersion visVersion, GmodPath& path )
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

	bool GmodPath::tryParseFullPath( const std::string& item, VisVersion visVersion, const Gmod& gmod, std::optional<GmodPath>& path )
	{
		SPDLOG_DEBUG( "Attempting to parse full path '{}' with VIS version {} using provided GMOD (version {})",
			item, static_cast<int>( visVersion ), static_cast<int>( gmod.visVersion() ) );
		path.reset();

		if ( gmod.visVersion() != visVersion )
		{
			SPDLOG_WARN(
				"GmodPath::tryParseFullPath called with visVersion {} for Locations, but provided GMOD instance has visVersion {}. This might lead to inconsistencies.",
				static_cast<int>( visVersion ), static_cast<int>( gmod.visVersion() ) );
		}

		try
		{
			const Locations& locations = VIS::instance().locations( visVersion );
			GmodPath tempPath;

			if ( GmodPath::tryParseFullPath( item, gmod, locations, tempPath ) )
			{
				path = std::move( tempPath );
				SPDLOG_INFO( "Successfully parsed full path into optional" );
				return true;
			}
			SPDLOG_DEBUG( "GmodPath::tryParseFullPath (delegated) returned false for item '{}'", item );
			return false;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during GmodPath::tryParseFullPath (fetching locations or delegated call): {}", ex.what() );
			return false;
		}
	}

	bool GmodPath::tryParseFullPath( const std::string& pathStr, VisVersion visVersion, GmodPath& path )
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
				SPDLOG_ERROR( "Failed to parse full path (tryParseFullPath): {}", errResult->error() );
			}
			else
			{
				SPDLOG_ERROR( "Failed to parse full path (tryParseFullPath): unknown internal error" );
			}
			return false;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during GmodPath::tryParseFullPath: {}", ex.what() );
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
		SPDLOG_ERROR( "Failed to parse path: unknown internal error from parseInternal" );
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
				SPDLOG_ERROR( "Failed to parse path (tryParse): {}", errResult->error() );
			}
			else
			{
				SPDLOG_ERROR( "Failed to parse path (tryParse): unknown internal error" );
			}
			return false;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during GmodPath::tryParse: {}", ex.what() );
			return false;
		}
	}

	bool GmodPath::tryParse( const std::string& item, VisVersion visVersion, const Gmod& gmod, std::optional<GmodPath>& path )
	{
		SPDLOG_DEBUG( "Attempting to parse path '{}' with VIS version {} using provided GMOD (version {})",
			item, static_cast<int>( visVersion ), static_cast<int>( gmod.visVersion() ) );
		path.reset();

		if ( gmod.visVersion() != visVersion )
		{
			SPDLOG_WARN(
				"GmodPath::tryParse called with visVersion {} for Locations, but provided GMOD instance has visVersion {}. This might lead to inconsistencies.",
				static_cast<int>( visVersion ), static_cast<int>( gmod.visVersion() ) );
		}

		try
		{
			const Locations& locations = VIS::instance().locations( visVersion );
			GmodPath tempPath;
			if ( GmodPath::tryParse( item, gmod, locations, tempPath ) )
			{
				path = std::move( tempPath );
				SPDLOG_INFO( "Successfully parsed path into optional" );
				return true;
			}
			SPDLOG_DEBUG( "GmodPath::tryParse (delegated) returned false for item '{}'", item );
			return false;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during GmodPath::tryParse (fetching locations or delegated call): {}", ex.what() );
			return false;
		}
	}

	std::unique_ptr<GmodParsePathResult> GmodPath::parseInternal(
		std::string_view item_sv, const Gmod& gmod, const Locations& locations )
	{
		if ( gmod.visVersion() != locations.visVersion() )
		{
			return std::make_unique<GmodParsePathResult::Err>( "Got different VIS versions for Gmod and Locations arguments" );
		}

		std::string_view original_item_sv_for_error_messages = item_sv;
		std::string_view current_item_sv = item_sv;

		size_t first_char = current_item_sv.find_first_not_of( " \t\n\r\f\v/" );
		if ( first_char == std::string_view::npos )
		{
			return std::make_unique<GmodParsePathResult::Err>( "Item is empty or consists only of whitespace/slashes" );
		}
		current_item_sv.remove_prefix( first_char );
		size_t last_char = current_item_sv.find_last_not_of( " \t\n\r\f\v/" );
		if ( last_char == std::string_view::npos || last_char < first_char )
		{
			current_item_sv = current_item_sv.substr( 0, 0 );
		}
		else
		{
			current_item_sv = current_item_sv.substr( 0, last_char + 1 );
		}

		if ( current_item_sv.empty() )
		{
			return std::make_unique<GmodParsePathResult::Err>( "Item is empty after trimming" );
		}

		std::string segmentParseErrorMessage;
		std::optional<std::vector<ParsedPathNodeFromString>> parsedSegmentsOpt =
			splitAndParseSegments( current_item_sv, gmod, locations, segmentParseErrorMessage );

		if ( !parsedSegmentsOpt.has_value() )
		{
			return std::make_unique<GmodParsePathResult::Err>( segmentParseErrorMessage );
		}
		std::vector<ParsedPathNodeFromString>& parsedSegments = parsedSegmentsOpt.value();
		if ( parsedSegments.empty() )
		{
			return std::make_unique<GmodParsePathResult::Err>( "No segments parsed from path string." );
		}

		const GmodNode* baseNodeTemplate = nullptr;
		if ( !gmod.tryGetNode( parsedSegments[0].code, baseNodeTemplate ) || !baseNodeTemplate )
		{
			return std::make_unique<GmodParsePathResult::Err>( "GMOD node not found for base segment: '" + parsedSegments[0].code + "'" );
		}

		std::vector<const GmodNode*> relativePathTemplates;

		if ( parsedSegments.size() == 1 )
		{
			relativePathTemplates.push_back( baseNodeTemplate );
		}
		else
		{
			struct RelativePathResolveState
			{
				const std::vector<ParsedPathNodeFromString>& allParsedSegmentsGlobal;
				const GmodNode* const baseNodeTemplateForContext;
				std::vector<const GmodNode*>& outputFinalRelativePath;
				bool pathFullyMatched = false;

				RelativePathResolveState( const std::vector<ParsedPathNodeFromString>& allSegs,
					const GmodNode* baseTpl,
					std::vector<const GmodNode*>& outputVec )
					: allParsedSegmentsGlobal( allSegs ),
					  baseNodeTemplateForContext( baseTpl ),
					  outputFinalRelativePath( outputVec ) {}
			};

			RelativePathResolveState traversalState( parsedSegments, baseNodeTemplate, relativePathTemplates );

			auto relativePathHandler = [&](
										   RelativePathResolveState& state,
										   const std::vector<const GmodNode*>& pathFromGMODRootToCurrentNodesParent,
										   const GmodNode& currentNode ) -> TraversalHandlerResult {
				if ( state.pathFullyMatched )
				{
					return TraversalHandlerResult::Stop;
				}

				std::vector<const GmodNode*> pathFromTraversalStartToCurrentNodeInclusive;
				if ( &currentNode == state.baseNodeTemplateForContext )
				{
					pathFromTraversalStartToCurrentNodeInclusive.push_back( &currentNode );
				}
				else
				{
					auto it_base_in_parent_path = std::find( pathFromGMODRootToCurrentNodesParent.begin(),
						pathFromGMODRootToCurrentNodesParent.end(),
						state.baseNodeTemplateForContext );

					if ( it_base_in_parent_path != pathFromGMODRootToCurrentNodesParent.end() )
					{
						for ( auto it = it_base_in_parent_path; it != pathFromGMODRootToCurrentNodesParent.end(); ++it )
						{
							pathFromTraversalStartToCurrentNodeInclusive.push_back( *it );
						}
						pathFromTraversalStartToCurrentNodeInclusive.push_back( &currentNode );
					}
					else
					{
						return TraversalHandlerResult::SkipSubtree;
					}
				}

				size_t segmentIndexInGlobalList = pathFromTraversalStartToCurrentNodeInclusive.size() - 1;

				if ( segmentIndexInGlobalList >= state.allParsedSegmentsGlobal.size() )
				{
					return TraversalHandlerResult::SkipSubtree;
				}

				if ( currentNode.code() == state.allParsedSegmentsGlobal[segmentIndexInGlobalList].code )
				{
					if ( segmentIndexInGlobalList == state.allParsedSegmentsGlobal.size() - 1 )
					{
						state.pathFullyMatched = true;
						state.outputFinalRelativePath.assign(
							pathFromTraversalStartToCurrentNodeInclusive.begin(),
							pathFromTraversalStartToCurrentNodeInclusive.end() );
						return TraversalHandlerResult::Stop;
					}
					else
					{
						return TraversalHandlerResult::Continue;
					}
				}
				else
				{
					return TraversalHandlerResult::SkipSubtree;
				}
			};

			TraversalOptions travOptions;
			GmodTraversal::traverse<RelativePathResolveState>( gmod, traversalState, *baseNodeTemplate, relativePathHandler, travOptions );

			if ( !traversalState.pathFullyMatched )
			{
				std::string nextExpectedCode = "unknown";
				size_t numFoundIncludingBase = relativePathTemplates.empty() ? 0 : relativePathTemplates.size();
				if ( numFoundIncludingBase < parsedSegments.size() )
				{
					nextExpectedCode = parsedSegments[numFoundIncludingBase].code;
				}
				else if ( !parsedSegments.empty() && numFoundIncludingBase > 0 && relativePathTemplates.back()->code() != parsedSegments.back().code )
				{
					nextExpectedCode = parsedSegments.back().code;
				}

				return std::make_unique<GmodParsePathResult::Err>(
					"Failed to resolve full relative path from base '" + baseNodeTemplate->code() +
					"'. Next expected segment: '" + nextExpectedCode + "'." );
			}
		}

		std::string absPrefixErrorMessage;
		std::optional<std::vector<const GmodNode*>> absolutePrefixTemplatesOpt =
			getAbsolutePrefixTemplates( baseNodeTemplate, gmod, absPrefixErrorMessage );

		if ( !absolutePrefixTemplatesOpt.has_value() )
		{
			return std::make_unique<GmodParsePathResult::Err>( absPrefixErrorMessage );
		}
		const std::vector<const GmodNode*>& absolutePrefixTemplates = absolutePrefixTemplatesOpt.value();

		std::vector<const GmodNode*> final_path_templates;
		final_path_templates.insert( final_path_templates.end(), absolutePrefixTemplates.begin(), absolutePrefixTemplates.end() );
		final_path_templates.insert( final_path_templates.end(), relativePathTemplates.begin(), relativePathTemplates.end() );

		std::vector<GmodNode> fullPathInstances;
		fullPathInstances.reserve( final_path_templates.size() );

		std::unordered_map<std::string, std::optional<Location>> inputLocationsMap;
		for ( const auto& seg : parsedSegments )
		{
			inputLocationsMap[seg.code] = seg.location;
		}

		for ( const GmodNode* tpl : final_path_templates )
		{
			GmodNode instance = *tpl;
			auto it_loc = inputLocationsMap.find( tpl->code() );
			if ( it_loc != inputLocationsMap.end() && it_loc->second.has_value() )
			{
				instance = instance.withLocation( it_loc->second.value() );
			}
			fullPathInstances.push_back( instance );
		}

		if ( fullPathInstances.empty() )
		{
			return std::make_unique<GmodParsePathResult::Err>( "Resulting path instances list is empty prior to visitor." );
		}

		LocationSetsVisitor visitor;
		std::vector<GmodNode> visitorArg_PathParents;
		GmodNode visitorArg_PathTarget = fullPathInstances.back();
		if ( fullPathInstances.size() > 1 )
		{
			visitorArg_PathParents.assign( fullPathInstances.begin(), fullPathInstances.end() - 1 );
		}

		std::vector<GmodNode>& nodesBeingProcessed = fullPathInstances;

		for ( size_t i = 0; i < nodesBeingProcessed.size(); ++i )
		{
			auto visitResult = visitor.visit( nodesBeingProcessed[i], i, visitorArg_PathParents, visitorArg_PathTarget );
			if ( visitResult.has_value() )
			{
				auto [setStart, setEnd, commonLocationOpt] = *visitResult;
				if ( commonLocationOpt.has_value() )
				{
					for ( size_t k = setStart; k <= setEnd; ++k )
					{
						if ( k < nodesBeingProcessed.size() )
						{
							nodesBeingProcessed[k] = nodesBeingProcessed[k].withLocation( commonLocationOpt.value() );
						}
					}
				}
			}
			else
			{
				if ( nodesBeingProcessed[i].location().has_value() )
				{
					return std::make_unique<GmodParsePathResult::Err>(
						"Node " + nodesBeingProcessed[i].toString() + " has a location but is not part of any individualizable set." );
				}
			}
		}

		GmodPath path;
		path.m_visVersion = gmod.visVersion();
		if ( !fullPathInstances.empty() )
		{
			path.m_ownedTargetNode = fullPathInstances.back();
			if ( fullPathInstances.size() > 1 )
			{
				path.m_ownedParentNodes.assign( fullPathInstances.begin(), fullPathInstances.end() - 1 );
			}
			path.m_isEmpty = false;
		}
		else
		{
			return std::make_unique<GmodParsePathResult::Err>( "Path became empty after processing and before final construction." );
		}

		if ( path.length() == 1 && path.targetNode().isRoot() && path.targetNode().code() == std::string( current_item_sv ) )
		{
			return std::make_unique<GmodParsePathResult::Err>( "Input path '" + std::string( original_item_sv_for_error_messages ) + "' (GMOD Root) is not a resolvable path by tryParse." );
		}

		return std::make_unique<GmodParsePathResult::Ok>( std::move( path ) );
	}

	std::unique_ptr<GmodParsePathResult> GmodPath::parseFullPathInternal( std::string_view pathStr, const Gmod& gmod, const Locations& locations )
	{
		SPDLOG_INFO( "Parsing full path '{}' using GMOD version {} and provided Locations", pathStr, static_cast<int>( gmod.visVersion() ) );

		if ( pathStr.empty() || std::all_of( pathStr.begin(), pathStr.end(), []( char c ) { return std::isspace( c ); } ) )
		{
			SPDLOG_ERROR( "Cannot parse empty or whitespace-only full path string." );
			return std::make_unique<GmodParsePathResult::Err>( "Path string cannot be empty or whitespace." );
		}

		try
		{
			std::vector<std::string_view> parts;
			size_t start = 0;
			size_t end;
			while ( ( end = pathStr.find( '/', start ) ) != std::string_view::npos )
			{
				if ( end >= start )

				{
					parts.push_back( pathStr.substr( start, end - start ) );
				}
				start = end + 1;
			}
			if ( start <= pathStr.length() )
			{
				parts.push_back( pathStr.substr( start ) );
			}

			if ( parts.empty() )
			{
				SPDLOG_ERROR( "No parts found after splitting full path string: '{}'", pathStr );
				return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Path string yielded no parts: {}", pathStr ) );
			}

			std::vector<const GmodNode*> parentNodeTemplates;
			parentNodeTemplates.reserve( parts.size() > 0 ? parts.size() - 1 : 0 );
			GmodNode targetNodeInstance;

			for ( size_t i = 0; i < parts.size(); ++i )
			{
				const auto& part = parts[i];
				if ( part.empty() )
				{
					SPDLOG_ERROR( "Empty part encountered in full path string: '{}' at segment {}", pathStr, i );
					return std::make_unique<GmodParsePathResult::Err>( "Empty segment in path string at index " + std::to_string( i ) );
				}

				std::string_view codeView;
				std::optional<Location> locationFromPart;
				size_t dashPos = part.find( '-' );

				if ( dashPos != std::string_view::npos )
				{
					codeView = part.substr( 0, dashPos );
					std::string_view locStrView = part.substr( dashPos + 1 );
					std::string locStr( locStrView );

					Location parsedLocation;
					if ( !locations.tryParse( locStr, parsedLocation ) )
					{
						SPDLOG_ERROR( "Failed to parse location string '{}' from part '{}' in full path '{}'", locStr, part, pathStr );
						return std::make_unique<GmodParsePathResult::Err>( "Failed to parse location: " + locStr );
					}
					locationFromPart = parsedLocation;
				}
				else
				{
					codeView = part;
				}
				std::string code( codeView );

				const GmodNode* nodeTemplatePtr = nullptr;
				if ( !gmod.tryGetNode( code, nodeTemplatePtr ) || !nodeTemplatePtr )
				{
					SPDLOG_ERROR( "Node template with code '{}' not found in GMOD for full path '{}'", code, pathStr );
					return std::make_unique<GmodParsePathResult::Err>( "Node template not found: " + code );
				}

				if ( i < parts.size() - 1 )
				{
					parentNodeTemplates.push_back( nodeTemplatePtr );
					if ( locationFromPart.has_value() )
					{
						SPDLOG_DEBUG( "Location '{}' specified for intermediate parent node '{}' in full path string '{}'. "
									  "The GmodPath constructor will use the node template's original location for its working copy; "
									  "the LocationSetsVisitor will then determine the final location.",
							locationFromPart->toString(), code, pathStr );
					}
				}
				else
				{
					targetNodeInstance = *nodeTemplatePtr;
					if ( locationFromPart.has_value() )
					{
						targetNodeInstance = targetNodeInstance.withLocation( *locationFromPart );
						SPDLOG_DEBUG( "Applied location '{}' from string to target node '{}'", locationFromPart->toString(), code );
					}
				}
			}

			GmodPath path( parentNodeTemplates, targetNodeInstance, gmod.visVersion(), false );
			return std::make_unique<GmodParsePathResult::Ok>( std::move( path ) );
		}
		catch ( const std::invalid_argument& ex )
		{
			SPDLOG_ERROR( "Invalid argument during full path parsing for '{}': {}", pathStr, ex.what() );
			return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Invalid argument: {}", ex.what() ) );
		}
		catch ( const std::runtime_error& ex )
		{
			SPDLOG_ERROR( "Runtime error during full path parsing for '{}': {}", pathStr, ex.what() );
			return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Runtime error: {}", ex.what() ) );
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Generic exception during full path parsing for '{}': {}", pathStr, ex.what() );
			return std::make_unique<GmodParsePathResult::Err>( fmt::format( "Exception: {}", ex.what() ) );
		}
		catch ( ... )
		{
			SPDLOG_ERROR( "Unknown exception during full path parsing for '{}'", pathStr );
			return std::make_unique<GmodParsePathResult::Err>( "Unknown exception during full path parsing." );
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

	GmodPath::Enumerator::Iterator::pointer GmodPath::Enumerator::Iterator::operator->() const
	{
		if ( m_isEnd || !m_enumerator )
		{
			SPDLOG_ERROR( "Attempted to call operator-> on end or invalid iterator" );
			throw std::runtime_error( "Cannot call operator-> on end or invalid iterator" );
		}
		if ( !m_cachedValue )
		{
			updateCache();
			if ( !m_cachedValue )
			{
				SPDLOG_ERROR( "Failed to generate value for iterator operator->" );
				throw std::runtime_error( "Iterator in invalid state for operator->" );
			}
		}
		return &( *m_cachedValue );
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

	GmodIndividualizableSet::GmodIndividualizableSet( const std::vector<size_t>& nodeIndices, const GmodPath& path )
		: m_nodeIndices{ nodeIndices },
		  m_ownedPath{ path }
	{
		SPDLOG_INFO( "Creating individualizable set with {} nodes, copying path with target '{}'", nodeIndices.size(), m_ownedPath.targetNode().code() );

		if ( nodeIndices.empty() )
		{
			SPDLOG_ERROR( "GmodIndividualizableSet cannot be empty" );
			throw std::invalid_argument( "GmodIndividualizableSet cant be empty" );
		}

		for ( size_t nodeIndex : nodeIndices )
		{
			const GmodNode* nodePtr = m_ownedPath[nodeIndex];
			if ( !nodePtr )
			{
				SPDLOG_ERROR( "Null node pointer encountered at index {} during GmodIndividualizableSet construction on owned path", nodeIndex );
				throw std::runtime_error( "Invalid owned path state: null node pointer" );
			}
			bool isTarget = ( nodeIndex == m_ownedPath.length() - 1 );
			bool isInSet = nodeIndices.size() > 1;

			if ( !nodePtr->isIndividualizable( isTarget, isInSet ) )
			{
				throw std::invalid_argument( "Node '" + nodePtr->code() + "' at index " + std::to_string( nodeIndex ) + " is not individualizable in this context (on owned path)" );
			}
		}

		std::optional<Location> firstLocation;
		bool hasSetLocation = false;

		for ( size_t nodeIndex : nodeIndices )
		{
			const GmodNode* nodePtr = m_ownedPath[nodeIndex];

			const auto& nodeLoc = nodePtr->location();

			if ( !hasSetLocation && nodeLoc.has_value() )
			{
				firstLocation = nodeLoc;
				hasSetLocation = true;
			}
			else if ( hasSetLocation && nodeLoc.has_value() && *nodeLoc != *firstLocation )
			{
				throw std::invalid_argument( "GmodIndividualizableSet nodes have different locations ('" +
											 firstLocation->toString() + "' vs '" + nodeLoc->toString() + "') on owned path" );
			}
		}

		if ( !m_nodeIndices.empty() )
		{
			firstLocation = m_ownedPath[m_nodeIndices[0]]->location();
			for ( size_t i = 1; i < m_nodeIndices.size(); ++i )
			{
				const GmodNode* nodePtr = m_ownedPath[m_nodeIndices[i]];
				if ( nodePtr->location() != firstLocation )
				{
					throw std::invalid_argument( "GmodIndividualizableSet nodes have different locations on owned path" );
				}
			}
		}

		bool hasLeafOrTarget = false;
		for ( size_t nodeIndex : nodeIndices )
		{
			const GmodNode* nodePtr = m_ownedPath[nodeIndex];
			if ( nodePtr->isLeafNode() || nodeIndex == m_ownedPath.length() - 1 )
			{
				hasLeafOrTarget = true;
				break;
			}
		}

		if ( !hasLeafOrTarget )
		{
			throw std::invalid_argument( "GmodIndividualizableSet has no nodes that are part of short path (no leaf or target node) on owned path" );
		}
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	std::vector<std::reference_wrapper<const GmodNode>> GmodIndividualizableSet::nodes() const
	{
		std::vector<std::reference_wrapper<const GmodNode>> result;
		result.reserve( m_nodeIndices.size() );
		for ( size_t idx : m_nodeIndices )
		{
			const GmodNode* nodePtr = m_ownedPath[idx];
			if ( !nodePtr )
			{
				SPDLOG_ERROR( "Null node pointer encountered at index {} during GmodIndividualizableSet::nodes() on owned path", idx );
				throw std::runtime_error( "Invalid owned path state: null node pointer" );
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
		if ( m_nodeIndices.empty() )
		{
			return std::nullopt;
		}
		if ( m_ownedPath.isEmpty() && !m_nodeIndices.empty() )
		{
			SPDLOG_ERROR( "Attempted to access location() on GmodIndividualizableSet with an empty owned path but non-empty indices." );
			throw std::runtime_error( "GmodIndividualizableSet has an empty path but expects nodes." );
		}

		const GmodNode* nodePtr = m_ownedPath[m_nodeIndices[0]];
		if ( !nodePtr )
		{
			SPDLOG_ERROR( "Null node pointer encountered at index {} during GmodIndividualizableSet::location()", m_nodeIndices[0] );
			throw std::runtime_error( "Invalid path state: null node pointer" );
		}

		return nodePtr->location();
	}

	void GmodIndividualizableSet::setLocation( std::optional<Location> newLocation )
	{
		SPDLOG_DEBUG( "GmodIndividualizableSet::setLocation to '{}' for {} indices on owned path with target '{}'.",
			newLocation.has_value() ? newLocation->toString() : "null", m_nodeIndices.size(), m_ownedPath.targetNode().code() );

		for ( size_t nodeIndex : m_nodeIndices )
		{
			GmodNode* nodeToModify = nullptr;
			if ( nodeIndex < m_ownedPath.m_ownedParentNodes.size() )
			{
				nodeToModify = &m_ownedPath.m_ownedParentNodes[nodeIndex];
			}
			else if ( nodeIndex == m_ownedPath.m_ownedParentNodes.size() )
			{
				nodeToModify = &m_ownedPath.m_ownedTargetNode;
			}
			else
			{
				SPDLOG_ERROR( "Node index {} out of bounds for GmodIndividualizableSet::setLocation (path length {})", nodeIndex, m_ownedPath.length() );
				throw std::out_of_range( "Node index out of bounds in setLocation" );
			}

			if ( newLocation.has_value() )
			{
				*nodeToModify = nodeToModify->withLocation( *newLocation );
			}
			else
			{
				*nodeToModify = nodeToModify->withoutLocation();
			}
		}
	}

	//----------------------------------------------
	// Mutators and Operations
	//----------------------------------------------

	GmodPath GmodIndividualizableSet::build()
	{
		SPDLOG_INFO( "Building modified path from individualizable set by moving owned path." );

		return std::move( m_ownedPath );
	}

	//----------------------------------------------
	// Conversion
	//----------------------------------------------

	std::string GmodIndividualizableSet::toString() const
	{
		std::stringstream ss;
		bool firstNodePrinted = false;
		for ( size_t i = 0; i < m_nodeIndices.size(); ++i )
		{
			size_t nodePathIndex = m_nodeIndices[i];
			if ( nodePathIndex >= m_ownedPath.length() )
			{
				SPDLOG_ERROR( "Node index {} out of bounds for owned path in GmodIndividualizableSet::toString()", nodePathIndex );
				if ( firstNodePrinted )
					ss << "/";
				ss << "[Error:OOB_Idx:" << nodePathIndex << "]";
				firstNodePrinted = true;
				continue;
			}

			const GmodNode* node = m_ownedPath[nodePathIndex];
			if ( !node )
			{
				SPDLOG_ERROR( "Null node at index {} in GmodIndividualizableSet::toString()", nodePathIndex );
				if ( firstNodePrinted )
					ss << "/";
				ss << "[Error:NullNode:" << nodePathIndex << "]";
				firstNodePrinted = true;
				continue;
			}

			bool isLastNodeInSet = ( i == m_nodeIndices.size() - 1 );

			if ( node->isLeafNode() || isLastNodeInSet )
			{
				if ( firstNodePrinted )
				{
					ss << "/";
				}
				node->toString( ss );
				firstNodePrinted = true;
			}
		}

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
