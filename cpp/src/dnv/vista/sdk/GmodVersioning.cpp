/**
 * @file GmodVersioning.cpp
 * @brief Implementation of the GmodVersioning class for converting GMOD objects between VIS versions.
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodVersioning.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodTraversal.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// GmodVersioning Class
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	GmodVersioning::GmodVersioning( const std::unordered_map<std::string, GmodVersioningDto>& dto )
	{
		SPDLOG_INFO( "Creating GmodVersioning with {} version entries", dto.size() );

		for ( const auto& [versionStr, versioningDto] : dto )
		{
			VisVersion version = VisVersionExtensions::parse( versionStr );

			SPDLOG_INFO( "Adding version {} with {} items", versionStr, versioningDto.items().size() );
			m_versioningsMap.emplace( version, GmodVersioningNode( version, versioningDto.items() ) );
		}
	}

	//----------------------------------------------
	// Conversion
	//----------------------------------------------

	std::optional<GmodNode> GmodVersioning::convertNode(
		VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const
	{
		SPDLOG_TRACE( "Converting node {} from version {} to {}",
			sourceNode.code(),
			VisVersionExtensions::toVersionString( sourceVersion ).data(),
			VisVersionExtensions::toVersionString( targetVersion ).data() );

		if ( sourceNode.code().empty() )
		{
			return std::nullopt;
		}

		validateSourceAndTargetVersions( sourceVersion, targetVersion );

		const auto& allVersions = VisVersionExtensions::allVersions();
		auto it = std::find( allVersions.begin(), allVersions.end(), sourceVersion );
		if ( it == allVersions.end() )
		{
			SPDLOG_ERROR( "Source version {} not found in allVersions", static_cast<int>( sourceVersion ) );

			return std::nullopt;
		}

		std::optional<GmodNode> currentNodeOpt = sourceNode;
		VisVersion currentVersion = sourceVersion;

		while ( it != allVersions.end() && currentVersion != targetVersion )
		{
			++it;
			if ( it == allVersions.end() )
			{
				SPDLOG_ERROR( "Target version {} not found in allVersions", static_cast<int>( targetVersion ) );

				return std::nullopt;
			}
			VisVersion nextVersion = *it;
			currentNodeOpt = convertNodeInternal( currentVersion, *currentNodeOpt, nextVersion );
			if ( !currentNodeOpt.has_value() )
			{
				SPDLOG_ERROR( "Node conversion failed going from version {} to {}", static_cast<int>( currentVersion ), static_cast<int>( nextVersion ) );

				return std::nullopt;
			}
			currentVersion = nextVersion;
		}

		if ( currentNodeOpt.has_value() )
		{
			SPDLOG_TRACE( "Node successfully converted to {}", currentNodeOpt->code() );
		}

		return currentNodeOpt;
	}

	std::optional<GmodPath> GmodVersioning::convertPath(
		VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) const
	{
		SPDLOG_DEBUG( "Converting path from version {} to {}",
			VisVersionExtensions::toVersionString( sourceVersion ).data(),
			VisVersionExtensions::toVersionString( targetVersion ).data() );

		validateSourceAndTargetVersions( sourceVersion, targetVersion );

		auto& vis = VIS::instance();
		const auto& targetGmod = vis.gmod( targetVersion );

		auto targetEndNodeOpt = convertNode( sourceVersion, *sourcePath.node(), targetVersion );
		if ( !targetEndNodeOpt.has_value() )
		{
			SPDLOG_ERROR( "Failed to convert end node from {} to {}",
				VisVersionExtensions::toVersionString( sourceVersion ).data(),
				VisVersionExtensions::toVersionString( targetVersion ).data() );
			return std::nullopt;
		}
		const GmodNode& targetEndNode = *targetEndNodeOpt;

		if ( targetEndNode.isRoot() )
		{
			SPDLOG_DEBUG( "Target node is root, returning simple root path" );
			const GmodNode* rootNodePtr = nullptr;
			if ( !targetGmod.tryGetNode( std::string( "VE" ), rootNodePtr ) || !rootNodePtr )
			{
				SPDLOG_ERROR( "Failed to find root node in target GMOD" );
				return std::nullopt;
			}
			return GmodPath( targetGmod, const_cast<GmodNode*>( rootNodePtr ), {} );
		}

		struct QualifyingNodePair
		{
			const GmodNode* sourceNode;
			GmodNode targetNode;
		};

		std::vector<QualifyingNodePair> qualifyingNodes;
		std::vector<const GmodNode*> sourceFullPathNodes;

		for ( const auto& pair : sourcePath.fullPath() )
		{
			sourceFullPathNodes.push_back( pair.second );
		}

		for ( const auto* sourceNodePtr : sourceFullPathNodes )
		{
			auto convertedNodeOpt = convertNode( sourceVersion, *sourceNodePtr, targetVersion );
			if ( !convertedNodeOpt.has_value() )
			{
				SPDLOG_ERROR( "Failed to convert node {} in path", sourceNodePtr->code() );
				return std::nullopt;
			}
			qualifyingNodes.push_back( { sourceNodePtr, *convertedNodeOpt } );
		}

		static std::vector<GmodNode> s_nodesWithLocation;
		s_nodesWithLocation.clear();
		s_nodesWithLocation.reserve( qualifyingNodes.size() );

		std::vector<GmodNode*> potentialParents;
		potentialParents.reserve( qualifyingNodes.size() - 1 );

		for ( size_t i = 0; i < qualifyingNodes.size() - 1; ++i )
		{
			if ( qualifyingNodes[i].targetNode.location().has_value() )
			{
				const GmodNode* nodePtr = nullptr;
				if ( !targetGmod.tryGetNode( qualifyingNodes[i].targetNode.code(), nodePtr ) || !nodePtr )
				{
					SPDLOG_ERROR( "Failed to find node with code {} in target GMOD",
						qualifyingNodes[i].targetNode.code() );
					return std::nullopt;
				}

				s_nodesWithLocation.push_back( nodePtr->tryWithLocation( qualifyingNodes[i].targetNode.location() ) );
				potentialParents.push_back( const_cast<GmodNode*>( &s_nodesWithLocation.back() ) );

				SPDLOG_DEBUG( "Using node {} with location {} in potential parents",
					s_nodesWithLocation.back().code(),
					s_nodesWithLocation.back().location()->value() );
			}
			else
			{
				const GmodNode* nodePtr = nullptr;
				if ( !targetGmod.tryGetNode( qualifyingNodes[i].targetNode.code(), nodePtr ) || !nodePtr )
				{
					SPDLOG_ERROR( "Failed to find node with code {} in target GMOD",
						qualifyingNodes[i].targetNode.code() );
					return std::nullopt;
				}
				potentialParents.push_back( const_cast<GmodNode*>( nodePtr ) );
				SPDLOG_DEBUG( "Using node {} without location in potential parents", nodePtr->code() );
			}
		}

		const GmodNode* targetEndNodePtr = nullptr;
		if ( !targetGmod.tryGetNode( targetEndNode.code(), targetEndNodePtr ) || !targetEndNodePtr )
		{
			SPDLOG_ERROR( "Failed to find target end node with code {} in GMOD", targetEndNode.code() );
			return std::nullopt;
		}

		GmodNode* targetEndNodeWithLocation = const_cast<GmodNode*>( targetEndNodePtr );
		if ( targetEndNode.location().has_value() )
		{
			s_nodesWithLocation.push_back( targetEndNodePtr->tryWithLocation( targetEndNode.location() ) );
			targetEndNodeWithLocation = const_cast<GmodNode*>( &s_nodesWithLocation.back() );
			SPDLOG_DEBUG( "Using end node {} with location {}",
				targetEndNodeWithLocation->code(), targetEndNode.location()->value() );
		}

		size_t missingLinkAt;
		if ( GmodPath::isValid( potentialParents, *targetEndNodeWithLocation, missingLinkAt ) )
		{
			SPDLOG_DEBUG( "Path is valid with {} parent nodes", potentialParents.size() );
			return GmodPath( targetGmod, targetEndNodeWithLocation, potentialParents );
		}

		SPDLOG_DEBUG( "Simple path is not valid, using reconstructed path approach" );
		std::vector<GmodNode> reconstructedPath;

		auto addToPathHelperLambda = [&targetGmod]( std::vector<GmodNode>& path, const GmodNode& node ) {
			SPDLOG_TRACE( "Adding node {} to path", node.code() );

			if ( path.empty() )
			{
				SPDLOG_TRACE( "Path is empty, adding node directly" );
				path.push_back( node );
				return;
			}

			GmodNode& prevNode = path.back();

			if ( prevNode.code() == node.code() && prevNode.location() == node.location() )
			{
				SPDLOG_DEBUG( "Skipping duplicate node: {}", node.code() );
				return;
			}

			if ( !prevNode.isChild( node ) )
			{
				SPDLOG_DEBUG( "Node {} is not a child of {}, repairing path", node.code(), prevNode.code() );

				for ( int j = static_cast<int>( path.size() ) - 1; j >= 0; --j )
				{
					GmodNode& parent = path[j];
					std::vector<const GmodNode*> currentParents;

					for ( int k = 0; k <= j; ++k )
					{
						currentParents.push_back( &path[k] );
					}

					std::vector<const GmodNode*> remaining;
					if ( !GmodTraversal::pathExistsBetween( targetGmod, currentParents, node, remaining ) )
					{
						SPDLOG_DEBUG( "No path exists between current parents and node {}", node.code() );

						if ( parent.isAssetFunctionNode() )
						{
							SPDLOG_DEBUG( "Checking if asset function node can be removed" );
							bool hasOtherAssetFunction = false;

							for ( const auto* n : currentParents )
							{
								if ( n->isAssetFunctionNode() && n->code() != parent.code() )
								{
									hasOtherAssetFunction = true;
									break;
								}
							}

							if ( !hasOtherAssetFunction )
							{
								SPDLOG_ERROR( "Cannot remove last asset function node" );
								throw std::runtime_error( "Tried to remove last asset function node" );
							}

							SPDLOG_DEBUG( "Removing asset function node {}", parent.code() );
							path.erase( path.begin() + j );
						}
					}
					else
					{
						SPDLOG_DEBUG( "Found path between parents and node {}, adding {} intermediate nodes",
							node.code(), remaining.size() );

						if ( node.location().has_value() )
						{
							SPDLOG_DEBUG( "Node has location {}, preserving in intermediate nodes",
								node.location()->value() );

							for ( const auto* n : remaining )
							{
								if ( !n->isIndividualizable( false, true ) )
								{
									SPDLOG_TRACE( "Adding non-individualizable node {}", n->code() );
									path.push_back( *n );
								}
								else
								{
									SPDLOG_TRACE( "Adding individualizable node {} with location {}",
										n->code(), node.location()->value() );
									path.push_back( n->tryWithLocation( *node.location() ) );
								}
							}
						}
						else
						{
							SPDLOG_DEBUG( "Node has no location, adding intermediate nodes as-is" );
							for ( const auto* n : remaining )
							{
								SPDLOG_TRACE( "Adding intermediate node {}", n->code() );
								path.push_back( *n );
							}
						}

						SPDLOG_DEBUG( "Finished adding intermediate nodes" );
						break;
					}
				}
			}

			if ( path.empty() || path.back().code() != node.code() || path.back().location() != node.location() )
			{
				SPDLOG_TRACE( "Adding node {} to path", node.code() );
				path.push_back( node );
			}
			else
			{
				SPDLOG_DEBUG( "Node {} already exists in path, skipping", node.code() );
			}
		};

		for ( size_t i = 0; i < qualifyingNodes.size(); ++i )
		{
			const auto& qualifyingNode = qualifyingNodes[i];

			if ( i > 0 && qualifyingNode.targetNode.code() == qualifyingNodes[i - 1].targetNode.code() )
			{
				SPDLOG_DEBUG( "Skipping duplicate node code {}", qualifyingNode.targetNode.code() );
				continue;
			}

			bool codeChanged = qualifyingNode.sourceNode->code() != qualifyingNode.targetNode.code();

			const GmodNode* sourceNormalAssignment = qualifyingNode.sourceNode->productType();
			const GmodNode* targetNormalAssignment = qualifyingNode.targetNode.productType();

			bool normalAssignmentChanged =
				( sourceNormalAssignment == nullptr && targetNormalAssignment != nullptr ) ||
				( sourceNormalAssignment != nullptr && targetNormalAssignment == nullptr ) ||
				( sourceNormalAssignment != nullptr && targetNormalAssignment != nullptr &&
					sourceNormalAssignment->code() != targetNormalAssignment->code() );

			if ( codeChanged )
			{
				SPDLOG_DEBUG( "Code changed from {} to {}",
					qualifyingNode.sourceNode->code(), qualifyingNode.targetNode.code() );
				addToPathHelperLambda( reconstructedPath, qualifyingNode.targetNode );
			}
			else if ( normalAssignmentChanged )
			{
				SPDLOG_DEBUG( "Normal assignment changed for node {}", qualifyingNode.targetNode.code() );
				bool wasDeleted = sourceNormalAssignment != nullptr && targetNormalAssignment == nullptr;

				if ( !codeChanged )
				{
					addToPathHelperLambda( reconstructedPath, qualifyingNode.targetNode );
				}

				if ( wasDeleted )
				{
					if ( qualifyingNode.targetNode.code() == targetEndNode.code() )
					{
						if ( i + 1 < qualifyingNodes.size() )
						{
							const auto& next = qualifyingNodes[i + 1];
							if ( next.targetNode.code() != qualifyingNode.targetNode.code() )
							{
								SPDLOG_ERROR( "Normal assignment end node was deleted" );
								throw std::runtime_error( "Normal assignment end node was deleted" );
							}
						}
					}
					SPDLOG_DEBUG( "Skipping deleted normal assignment" );
					continue;
				}
				else if ( qualifyingNode.targetNode.code() != targetEndNode.code() )
				{
					SPDLOG_DEBUG( "Adding new target normal assignment node {}", targetNormalAssignment->code() );
					addToPathHelperLambda( reconstructedPath, *targetNormalAssignment );
					++i;
				}
			}

			if ( !codeChanged && !normalAssignmentChanged )
			{
				SPDLOG_DEBUG( "No changes for node {}, adding to path", qualifyingNode.targetNode.code() );
				addToPathHelperLambda( reconstructedPath, qualifyingNode.targetNode );
			}

			if ( !reconstructedPath.empty() && reconstructedPath.back().code() == targetEndNode.code() )
			{
				SPDLOG_DEBUG( "Reached target end node, stopping path reconstruction" );
				break;
			}
		}

		s_nodesWithLocation.clear();
		s_nodesWithLocation.reserve( reconstructedPath.size() );

		std::vector<GmodNode*> finalParentPointers;
		if ( reconstructedPath.size() > 1 )
		{
			SPDLOG_DEBUG( "Building final parent pointers from {} reconstructed nodes", reconstructedPath.size() - 1 );
			finalParentPointers.reserve( reconstructedPath.size() - 1 );

			for ( size_t idx = 0; idx < reconstructedPath.size() - 1; ++idx )
			{
				const GmodNode* nodePtr = nullptr;
				if ( !targetGmod.tryGetNode( reconstructedPath[idx].code(), nodePtr ) || !nodePtr )
				{
					SPDLOG_ERROR( "Failed to find node with code {} in target GMOD",
						reconstructedPath[idx].code() );
					return std::nullopt;
				}

				if ( reconstructedPath[idx].location().has_value() )
				{
					s_nodesWithLocation.push_back( nodePtr->tryWithLocation( reconstructedPath[idx].location() ) );
					finalParentPointers.push_back( const_cast<GmodNode*>( &s_nodesWithLocation.back() ) );

					SPDLOG_DEBUG( "Created and using node {} with location {}",
						s_nodesWithLocation.back().code(),
						s_nodesWithLocation.back().location()->value() );
				}
				else
				{
					finalParentPointers.push_back( const_cast<GmodNode*>( nodePtr ) );
					SPDLOG_DEBUG( "Using node {} without location", nodePtr->code() );
				}
			}
		}

		const GmodNode* finalEndNodePtr = nullptr;
		if ( !targetGmod.tryGetNode( reconstructedPath.back().code(), finalEndNodePtr ) || !finalEndNodePtr )
		{
			SPDLOG_ERROR( "Failed to find final end node with code {} in target GMOD",
				reconstructedPath.back().code() );
			return std::nullopt;
		}

		GmodNode* finalNodeWithLocation = const_cast<GmodNode*>( finalEndNodePtr );
		if ( reconstructedPath.back().location().has_value() )
		{
			s_nodesWithLocation.push_back( finalEndNodePtr->tryWithLocation( reconstructedPath.back().location() ) );
			finalNodeWithLocation = const_cast<GmodNode*>( &s_nodesWithLocation.back() );
			SPDLOG_DEBUG( "Created final node {} with location {}",
				finalNodeWithLocation->code(),
				reconstructedPath.back().location()->value() );
		}

		size_t missingLinkAt2;
		if ( !GmodPath::isValid( finalParentPointers, *finalNodeWithLocation, missingLinkAt2 ) )
		{
			SPDLOG_ERROR( "Final reconstructed path is not valid, missing link at index {}", missingLinkAt2 );
			return std::nullopt;
		}

		SPDLOG_DEBUG( "Successfully created valid path with {} parent nodes", finalParentPointers.size() );
		return GmodPath( targetGmod, finalNodeWithLocation, finalParentPointers );
	}

	std::optional<LocalIdBuilder> GmodVersioning::convertLocalId(
		const LocalIdBuilder& sourceLocalId, VisVersion targetVersion ) const
	{
		SPDLOG_INFO( "Converting LocalIdBuilder to version {}", VisVersionExtensions::toVersionString( targetVersion ).data() );

		if ( !sourceLocalId.visVersion().has_value() )
		{
			SPDLOG_ERROR( "Cannot convert local ID without a specific VIS version" );
			throw std::invalid_argument( "Cannot convert local ID without a specific VIS version" );
		}

		std::optional<GmodPath> primaryItemPathOpt;
		if ( sourceLocalId.primaryItem().has_value() )
		{
			SPDLOG_INFO( "Converting primary item" );
			primaryItemPathOpt = convertPath(
				*sourceLocalId.visVersion(),
				sourceLocalId.primaryItem().value(),
				targetVersion );
			if ( !primaryItemPathOpt.has_value() )
			{
				SPDLOG_ERROR( "Failed to convert primary item for LocalIdBuilder" );
				return std::nullopt;
			}
		}

		std::optional<GmodPath> secondaryItemPathOpt;
		if ( sourceLocalId.secondaryItem().has_value() )
		{
			SPDLOG_INFO( "Converting secondary item" );
			secondaryItemPathOpt = convertPath(
				*sourceLocalId.visVersion(),
				sourceLocalId.secondaryItem().value(),
				targetVersion );
			if ( !secondaryItemPathOpt.has_value() )
			{
				SPDLOG_ERROR( "Failed to convert secondary item for LocalIdBuilder" );
				return std::nullopt;
			}
		}

		SPDLOG_INFO( "Building converted LocalIdBuilder" );
		return LocalIdBuilder::create( targetVersion )
			.tryWithPrimaryItem( std::move( primaryItemPathOpt ) )
			.tryWithSecondaryItem( std::move( secondaryItemPathOpt ) )
			.withVerboseMode( sourceLocalId.isVerboseMode() )
			.tryWithMetadataTag( sourceLocalId.quantity() )
			.tryWithMetadataTag( sourceLocalId.content() )
			.tryWithMetadataTag( sourceLocalId.calculation() )
			.tryWithMetadataTag( sourceLocalId.state() )
			.tryWithMetadataTag( sourceLocalId.command() )
			.tryWithMetadataTag( sourceLocalId.type() )
			.tryWithMetadataTag( sourceLocalId.position() )
			.tryWithMetadataTag( sourceLocalId.detail() );
	}

	std::optional<LocalId> GmodVersioning::convertLocalId(
		const LocalId& sourceLocalId, VisVersion targetVersion ) const
	{
		SPDLOG_INFO( "Converting LocalId to version {}", VisVersionExtensions::toVersionString( targetVersion ) );

		auto builder = convertLocalId( sourceLocalId.builder(), targetVersion );
		if ( !builder.has_value() )
		{
			return std::nullopt;
		}

		return builder->build();
	}

	//----------------------------------------------
	// GmodVersioningNode Class
	//----------------------------------------------

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	GmodVersioning::GmodVersioningNode::GmodVersioningNode(
		VisVersion visVersion,
		const std::unordered_map<std::string, GmodNodeConversionDto>& dto )
		: m_visVersion( visVersion )
	{
		SPDLOG_INFO( "Creating GmodVersioningNode for version {} with {} items",
			VisVersionExtensions::toVersionString( visVersion ), dto.size() );

		for ( const auto& [code, dtoNode] : dto )
		{
			GmodNodeConversion conversion;
			conversion.source = dtoNode.source();
			if ( dtoNode.target().empty() )
			{
				conversion.target = std::nullopt;
			}
			else
			{
				conversion.target = dtoNode.target();
			}
			conversion.oldAssignment = dtoNode.oldAssignment();
			conversion.newAssignment = dtoNode.newAssignment();
			conversion.deleteAssignment = dtoNode.deleteAssignment();

			if ( !dtoNode.operations().empty() )
			{
				for ( const auto& type : dtoNode.operations() )
				{
					conversion.operations.insert( parseConversionType( type ) );
				}
			}

			m_versioningNodeChanges.emplace( code, conversion );
		}
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	VisVersion GmodVersioning::GmodVersioningNode::visVersion() const
	{
		return m_visVersion;
	}

	bool GmodVersioning::GmodVersioningNode::tryGetCodeChanges(
		const std::string& code, GmodNodeConversion& nodeChanges ) const
	{
		SPDLOG_TRACE( "Looking for code changes for node {} in version {}", code, VisVersionExtensions::toVersionString( m_visVersion ) );
		auto it = m_versioningNodeChanges.find( code );
		if ( it != m_versioningNodeChanges.end() )
		{
			nodeChanges = it->second;
			if ( nodeChanges.target.has_value() && nodeChanges.target.value() != code )
			{
				SPDLOG_DEBUG( "Found code change: {} -> {} in version {}", code, *nodeChanges.target, VisVersionExtensions::toVersionString( m_visVersion ) );
			}
			else if ( !nodeChanges.operations.empty() )
			{
				SPDLOG_DEBUG( "Found operational changes for node {} (no direct code change) in version {}", code, VisVersionExtensions::toVersionString( m_visVersion ) );
			}
			else
			{
				SPDLOG_TRACE( "No specific code change or operation found for node {} in version {}, but entry exists.", code, VisVersionExtensions::toVersionString( m_visVersion ) );
			}
			return true;
		}
		else
		{
			SPDLOG_TRACE( "No code changes found for node {} in version {}", code, VisVersionExtensions::toVersionString( m_visVersion ) );
			GmodNodeConversion defaultChanges{};
			nodeChanges = defaultChanges;
			return false;
		}
	}

	//----------------------------------------------
	// Private Helper Methods
	//----------------------------------------------

	std::optional<GmodNode> GmodVersioning::convertNodeInternal(
		[[maybe_unused]] VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const
	{
		SPDLOG_TRACE( "Converting node {} internally from {} to {}",
			sourceNode.code(),
			VisVersionExtensions::toVersionString( sourceNode.visVersion() ).data(),
			VisVersionExtensions::toVersionString( targetVersion ).data() );

		validateSourceAndTargetVersionPair( sourceNode.visVersion(), targetVersion );

		std::string nextCode = sourceNode.code();

		GmodVersioningNode versioningNode;
		if ( tryGetVersioningNode( targetVersion, versioningNode ) )
		{
			GmodNodeConversion change;
			if ( versioningNode.tryGetCodeChanges( sourceNode.code(), change ) && change.target.has_value() )
			{
				SPDLOG_DEBUG( "Code change found: {} -> {}", sourceNode.code().data(), change.target.value().data() );
				nextCode = change.target.value();
			}
		}

		auto& vis = VIS::instance();
		const auto& targetGmod = vis.gmod( targetVersion );

		const GmodNode* targetNodePtr = nullptr;
		if ( !targetGmod.tryGetNode( nextCode, targetNodePtr ) || targetNodePtr == nullptr )
		{
			SPDLOG_ERROR( "Failed to find target node with code {} in GMOD for VIS version {}",
				nextCode, VisVersionExtensions::toVersionString( targetVersion ).data() );
			return std::nullopt;
		}

		GmodNode resultNode = sourceNode.location().has_value()
								  ? targetNodePtr->tryWithLocation( sourceNode.location() )
								  : *targetNodePtr;

		if ( sourceNode.location().has_value() &&
			 ( !resultNode.location().has_value() || resultNode.location() != sourceNode.location() ) )
		{
			SPDLOG_ERROR( "Failed to preserve location {} for node {}",
				sourceNode.location()->value(), resultNode.code() );
		}
		else if ( sourceNode.location().has_value() )
		{
			SPDLOG_DEBUG( "Preserved location {} for node {} during conversion",
				sourceNode.location()->value(), resultNode.code() );
		}

		return resultNode;
	}

	bool GmodVersioning::tryGetVersioningNode(
		VisVersion visVersion,
		GmodVersioningNode& versioningNode ) const
	{
		auto it = m_versioningsMap.find( visVersion );
		if ( it != m_versioningsMap.end() )
		{
			SPDLOG_TRACE( "GmodVersioning::tryGetVersioningNode: GmodVersioningNode for version {} found. Assigning to output parameter.", VisVersionExtensions::toVersionString( visVersion ).data() );
			versioningNode = it->second;
			return true;
		}
		else
		{
			SPDLOG_TRACE( "GmodVersioning::tryGetVersioningNode: GmodVersioningNode for version {} not found. Output parameter 'versioningNode' is reset to default state.", VisVersionExtensions::toVersionString( visVersion ).data() );
			GmodVersioningNode defaultNode{};
			versioningNode = defaultNode;
			return false;
		}
	}

	const GmodVersioning::GmodVersioningNode* GmodVersioning::tryGetVersioningNode( VisVersion visVersion ) const noexcept
	{
		SPDLOG_DEBUG( "Looking for versioning node for version {}", static_cast<int>( visVersion ) );

		auto it = m_versioningsMap.find( visVersion );
		if ( it != m_versioningsMap.end() )
		{
			return &it->second;
		}

		return nullptr;
	}

	//----------------------------------------------
	// Private Validation Methods
	//----------------------------------------------

	void GmodVersioning::validateSourceAndTargetVersions(
		VisVersion sourceVersion, VisVersion targetVersion ) const
	{
		if ( !VisVersionExtensions::isValid( sourceVersion ) )
		{
			std::string errorMsg = "Invalid source VIS Version: " + VisVersionExtensions::toVersionString( sourceVersion );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}

		if ( !VisVersionExtensions::isValid( targetVersion ) )
		{
			std::string errorMsg = "Invalid target VIS Version: " + VisVersionExtensions::toVersionString( targetVersion );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}

		if ( sourceVersion >= targetVersion )
		{
			SPDLOG_ERROR( "Source version {} must be earlier than target version {}",
				VisVersionExtensions::toVersionString( sourceVersion ),
				VisVersionExtensions::toVersionString( targetVersion ) );
			throw std::invalid_argument( "Source version must be earlier than target version" );
		}
	}

	void GmodVersioning::validateSourceAndTargetVersionPair(
		VisVersion sourceVersion, VisVersion targetVersion ) const
	{
		if ( sourceVersion >= targetVersion )
		{
			std::string errorMsg = "Source version " + VisVersionExtensions::toVersionString( sourceVersion ) +
								   " must be less than target version " + VisVersionExtensions::toVersionString( targetVersion );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}

		const auto& allVersions = VisVersionExtensions::allVersions();
		auto itSource = std::find( allVersions.begin(), allVersions.end(), sourceVersion );

		if ( itSource == allVersions.end() || std::next( itSource ) == allVersions.end() )
		{
			std::string errorMsg = "Cannot determine next version for source version " + VisVersionExtensions::toVersionString( sourceVersion ) +
								   " in the defined sequence, or it's the latest version.";
			SPDLOG_ERROR( errorMsg );

			throw std::logic_error( errorMsg );
		}

		VisVersion expectedNextVersion = *std::next( itSource );

		if ( targetVersion != expectedNextVersion )
		{
			std::string errorMsg = "Target version " + VisVersionExtensions::toVersionString( targetVersion ) +
								   " must be exactly one version higher than source version " + VisVersionExtensions::toVersionString( sourceVersion ) +
								   ". Expected next version was " + VisVersionExtensions::toVersionString( expectedNextVersion ) + ".";
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	//----------------------------------------------
	// Private Static Utility Methods
	//----------------------------------------------

	GmodVersioning::ConversionType GmodVersioning::parseConversionType( const std::string& type )
	{
		static const std::unordered_map<std::string, ConversionType> typeMap = {
			{ "changeCode", ConversionType::ChangeCode },
			{ "merge", ConversionType::Merge },
			{ "move", ConversionType::Move },
			{ "assignmentChange", ConversionType::AssignmentChange },
			{ "assignmentDelete", ConversionType::AssignmentDelete } };

		auto it = typeMap.find( type );
		if ( it != typeMap.end() )
		{
			return it->second;
		}
		else
		{
			std::string errorMsg = "Invalid conversion type: " + type;
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}
}
