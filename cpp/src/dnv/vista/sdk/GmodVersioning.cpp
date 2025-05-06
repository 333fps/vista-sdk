#include "pch.h"

#include "dnv/vista/sdk/GmodVersioning.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Constructor
	//-------------------------------------------------------------------------

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

	VisVersion GmodVersioning::GmodVersioningNode::visVersion() const
	{
		return m_visVersion;
	}

	bool GmodVersioning::GmodVersioningNode::tryGetCodeChanges(
		const std::string& code, GmodNodeConversion& nodeChanges ) const
	{
		SPDLOG_INFO( "Looking for code changes for node {}", code );

		auto it = m_versioningNodeChanges.find( code );
		if ( it != m_versioningNodeChanges.end() )
		{
			nodeChanges = it->second;
			if ( nodeChanges.target.has_value() )
			{
				SPDLOG_INFO( "Found code change: {} -> {}", code, *nodeChanges.target );
			}
			return true;
		}
		return false;
	}

	//-------------------------------------------------------------------------
	// Public Conversion Methods
	//-------------------------------------------------------------------------

	std::optional<GmodNode> GmodVersioning::convertNode(
		VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const
	{
		SPDLOG_INFO( "Converting node {} from version {} to {}",
			sourceNode.code(),
			static_cast<int>( sourceVersion ),
			static_cast<int>( targetVersion ) );

		if ( sourceNode.code().empty() )
			return std::nullopt;

		validateSourceAndTargetVersions( sourceVersion, targetVersion );

		VisVersion currentVersion = sourceVersion;
		VisVersion firstStepTargetVersion = static_cast<VisVersion>( static_cast<int>( currentVersion ) + 1 );
		std::optional<GmodNode> currentNodeOpt = convertNodeInternal( currentVersion, sourceNode, firstStepTargetVersion );

		if ( !currentNodeOpt.has_value() )
		{
			SPDLOG_ERROR( "Node conversion failed going from version {} to {}", static_cast<int>( currentVersion ), static_cast<int>( firstStepTargetVersion ) );
			return std::nullopt;
		}

		currentVersion = firstStepTargetVersion;

		while ( currentVersion < targetVersion )
		{
			VisVersion nextVersion = static_cast<VisVersion>( static_cast<int>( currentVersion ) + 1 );
			currentNodeOpt = convertNodeInternal( currentVersion, *currentNodeOpt, nextVersion );

			if ( !currentNodeOpt.has_value() )
			{
				SPDLOG_ERROR( "Node conversion failed going from version {} to {}",
					static_cast<int>( currentVersion ),
					static_cast<int>( nextVersion ) );
				return std::nullopt;
			}
			currentVersion = nextVersion;
		}

		if ( currentNodeOpt.has_value() )
		{
			SPDLOG_INFO( "Node successfully converted to {}", currentNodeOpt->code() );
		}

		return currentNodeOpt;
	}
	std::optional<GmodPath> GmodVersioning::convertPath(
		VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) const
	{
		SPDLOG_INFO( "Converting path with end node {} from version {} to {}",
			sourcePath.targetNode().code(),
			static_cast<int>( sourceVersion ),
			static_cast<int>( targetVersion ) );

		validateSourceAndTargetVersions( sourceVersion, targetVersion );

		auto targetEndNodeOpt = convertNodeInternal( sourceVersion, sourcePath.targetNode(), targetVersion );
		if ( !targetEndNodeOpt.has_value() )
		{
			SPDLOG_ERROR( "Failed to convert end node: {}", sourcePath.targetNode().code() );
			return std::nullopt;
		}
		GmodNode targetEndNode = std::move( *targetEndNodeOpt );

		if ( targetEndNode.isRoot() )
		{
			SPDLOG_INFO( "End node is a root node, returning simple path" );
			return GmodPath( std::vector<const GmodNode*>{}, std::move( targetEndNode ), targetEndNode.visVersion(), true );
		}

		const auto& targetGmod = VIS::instance().gmod( targetVersion );

		std::vector<std::pair<std::reference_wrapper<const GmodNode>, GmodNode>> qualifyingNodes;

		for ( const auto& pathNodePairRef : sourcePath.fullPath() )
		{
			const GmodNode& originalNodeRef = pathNodePairRef.second.get();

			auto convertedNodeOpt = convertNode( sourceVersion, originalNodeRef, targetVersion );
			if ( !convertedNodeOpt.has_value() )
			{
				SPDLOG_ERROR( "Failed to convert path node: {}", originalNodeRef.code() );
				return std::nullopt;
			}

			qualifyingNodes.emplace_back( pathNodePairRef.second, std::move( *convertedNodeOpt ) );
		}

		if ( std::any_of( qualifyingNodes.begin(), qualifyingNodes.end(),
				 []( const auto& pair ) { return pair.second.code().empty(); } ) )
		{
			SPDLOG_ERROR( "Failed to convert node forward (resulted in empty code)" );
			return std::nullopt;
		}

		SPDLOG_INFO( "Building path for converted node, found {} qualifying nodes",
			qualifyingNodes.size() );

		std::vector<const GmodNode*> potentialParentPtrs;
		if ( qualifyingNodes.size() > 1 )
		{
			potentialParentPtrs.reserve( qualifyingNodes.size() - 1 );
			for ( size_t i = 0; i < qualifyingNodes.size() - 1; ++i )
			{
				potentialParentPtrs.push_back( &qualifyingNodes[i].second );
			}
		}

		if ( GmodPath::isValid( potentialParentPtrs, targetEndNode ) )
		{
			SPDLOG_INFO( "Found valid direct path" );
			return GmodPath( std::move( potentialParentPtrs ), std::move( targetEndNode ), targetEndNode.visVersion(), true );
		}

		auto addToPath = [&]( const Gmod& gmod, std::vector<const GmodNode*>& pathPtrs, const GmodNode* nodePtr ) -> bool {
			if ( nodePtr == nullptr )
				return false;

			if ( !pathPtrs.empty() )
			{
				const GmodNode* prevPtr = pathPtrs.back();
				if ( !prevPtr->isChild( *nodePtr ) )
				{
					for ( int j = static_cast<int>( pathPtrs.size() ) - 1; j >= 0; --j )
					{
						const GmodNode* ancestorPtr = pathPtrs[static_cast<size_t>( j )];

						std::vector<const GmodNode*> currentPathPrefixPtrs( pathPtrs.begin(), pathPtrs.begin() + ( j + 1 ) );

						std::vector<const GmodNode*> remainingPathSegmentPtrs;
						if ( !gmod.pathExistsBetween( currentPathPrefixPtrs, *nodePtr, remainingPathSegmentPtrs ) )
						{
							bool isLastAssetFunction = ancestorPtr->isAssetFunctionNode();
							if ( isLastAssetFunction )
							{
								bool otherAssetFunctionExists = std::any_of( currentPathPrefixPtrs.begin(), currentPathPrefixPtrs.end(),
									[&]( const GmodNode* n ) { return n->isAssetFunctionNode() && n->code() != ancestorPtr->code(); } );

								if ( !otherAssetFunctionExists )
								{
									SPDLOG_ERROR( "Path reconstruction failed: Tried to remove last asset function node '{}' while adding '{}'", ancestorPtr->code(), nodePtr->code() );
									return false;
								}
							}
							pathPtrs.erase( pathPtrs.begin() + j );
						}
						else
						{
							SPDLOG_DEBUG( "Found path segment between '{}' and '{}'. Inserting {} nodes.", ancestorPtr->code(), nodePtr->code(), remainingPathSegmentPtrs.size() );
							pathPtrs.insert( pathPtrs.end(), remainingPathSegmentPtrs.begin(), remainingPathSegmentPtrs.end() );
							break;
						}
					}

					if ( pathPtrs.empty() || !pathPtrs.back()->isChild( *nodePtr ) )
					{
						SPDLOG_ERROR( "Path reconstruction failed: Could not find valid parent for node '{}' after checking ancestors.", nodePtr->code() );
						return false;
					}
				}
			}

			pathPtrs.push_back( nodePtr );
			return true;
		};

		std::vector<const GmodNode*> reconstructedPathPtrs;
		for ( size_t i = 0; i < qualifyingNodes.size(); ++i )
		{
			const GmodNode& targetOwnedNode = qualifyingNodes[i].second;

			const GmodNode* targetNodePtr = nullptr;
			if ( !targetGmod.tryGetNode( targetOwnedNode.code(), targetNodePtr ) || targetNodePtr == nullptr )
			{
				SPDLOG_ERROR( "Failed to find node '{}' in target GMOD during reconstruction loop.", targetOwnedNode.code() );
				return std::nullopt;
			}

			if ( i > 0 && !reconstructedPathPtrs.empty() && targetNodePtr->code() == reconstructedPathPtrs.back()->code() )
			{
				SPDLOG_DEBUG( "Skipping duplicate consecutive node: {}", targetNodePtr->code() );
				continue;
			}

			if ( !addToPath( targetGmod, reconstructedPathPtrs, targetNodePtr ) )
			{
				SPDLOG_ERROR( "Failed to add node '{}' to path during reconstruction: parent-child relationship broken or reconstruction failed.", targetNodePtr->code() );
				return std::nullopt;
			}

			if ( !reconstructedPathPtrs.empty() && reconstructedPathPtrs.back()->code() == targetEndNode.code() )
			{
				SPDLOG_DEBUG( "Target node '{}' reached during reconstruction.", targetEndNode.code() );
				break;
			}
		}

		if ( reconstructedPathPtrs.empty() )
		{
			SPDLOG_ERROR( "Failed to build path: no nodes added after reconstruction attempt" );
			return std::nullopt;
		}

		if ( reconstructedPathPtrs.back()->code() != targetEndNode.code() )
		{
			SPDLOG_ERROR( "Path reconstruction failed: final node {} does not match expected target {}", reconstructedPathPtrs.back()->code(), targetEndNode.code() );
			return std::nullopt;
		}

		reconstructedPathPtrs.pop_back();

		size_t missingLinkAt = std::numeric_limits<size_t>::max();
		if ( !GmodPath::isValid( reconstructedPathPtrs, targetEndNode, missingLinkAt ) )
		{
			SPDLOG_ERROR( "Failed to create a valid path after reconstruction. Missing link at: {}", missingLinkAt );
			std::stringstream ss;
			for ( const auto* p : reconstructedPathPtrs )
			{
				ss << "/" << ( p ? p->code() : "[null]" );
			}
			ss << "/" << targetEndNode.code();
			SPDLOG_DEBUG( "Invalid reconstructed path: {}", ss.str() );
			return std::nullopt;
		}

		SPDLOG_INFO( "Successfully created path with {} parents after reconstruction", reconstructedPathPtrs.size() );
		return GmodPath( reconstructedPathPtrs, std::move( targetEndNode ), targetEndNode.visVersion(), true );
	}

	std::optional<LocalIdBuilder> GmodVersioning::convertLocalId(
		const LocalIdBuilder& sourceLocalId, VisVersion targetVersion ) const
	{
		SPDLOG_INFO( "Converting LocalIdBuilder to version {}", static_cast<int>( targetVersion ) );

		if ( !sourceLocalId.visVersion().has_value() )
		{
			SPDLOG_ERROR( "Cannot convert local ID without a specific VIS version" );
			throw std::invalid_argument( "Cannot convert local ID without a specific VIS version" );
		}

		std::optional<GmodPath> primaryItem;
		if ( sourceLocalId.primaryItem().length() > 0 )
		{
			SPDLOG_INFO( "Converting primary item" );
			auto convertedPath = convertPath(
				*sourceLocalId.visVersion(),
				sourceLocalId.primaryItem(),
				targetVersion );

			primaryItem = std::move( convertedPath );
		}

		std::optional<GmodPath> secondaryItem;
		if ( sourceLocalId.secondaryItem().has_value() )
		{
			SPDLOG_INFO( "Converting secondary item" );
			auto convertedPath = convertPath(
				*sourceLocalId.visVersion(),
				sourceLocalId.secondaryItem().value(),
				targetVersion );

			secondaryItem = std::move( convertedPath );
		}

		SPDLOG_INFO( "Building converted LocalIdBuilder" );
		return LocalIdBuilder::create( targetVersion )
			.tryWithPrimaryItem( std::move( primaryItem ) )
			.tryWithSecondaryItem( std::move( secondaryItem ) )
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
		SPDLOG_INFO( "Converting LocalId to version {}", static_cast<int>( targetVersion ) );

		auto builder = convertLocalId( sourceLocalId.builder(), targetVersion );
		if ( !builder.has_value() )
			return std::nullopt;

		return builder->build();
	}

	//-------------------------------------------------------------------------
	// GmodVersioningNode Implementation
	//-------------------------------------------------------------------------

	GmodVersioning::GmodVersioningNode::GmodVersioningNode(
		VisVersion visVersion,
		const std::unordered_map<std::string, GmodNodeConversionDto>& dto )
		: m_visVersion( visVersion )
	{
		SPDLOG_INFO( "Creating GmodVersioningNode for version {} with {} items",
			static_cast<int>( visVersion ), dto.size() );

		for ( const auto& [code, dtoNode] : dto )
		{
			GmodNodeConversion conversion;
			conversion.source = dtoNode.source();
			conversion.target = dtoNode.target();
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

	//-------------------------------------------------------------------------
	// Private Helper Methods
	//-------------------------------------------------------------------------

	std::optional<GmodNode> GmodVersioning::convertNodeInternal(
		[[maybe_unused]] VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const
	{
		SPDLOG_INFO( "Converting node {} internally from {} to {}",
			sourceNode.code(),
			static_cast<int>( sourceVersion ),
			static_cast<int>( targetVersion ) );

		validateSourceAndTargetVersionPair( sourceNode.visVersion(), targetVersion );

		auto& vis = VIS::instance();
		const auto& targetGmod = vis.gmod( targetVersion );

		std::string nextCode = sourceNode.code();

		const GmodVersioningNode* versioningNodePtr = tryGetVersioningNode( targetVersion );
		if ( versioningNodePtr != nullptr )
		{
			GmodNodeConversion change;
			if ( versioningNodePtr->tryGetCodeChanges( sourceNode.code(), change ) && change.target.has_value() )
			{
				SPDLOG_INFO( "Code change found: {} -> {}", sourceNode.code(), *change.target );
				nextCode = *change.target;
			}
		}

		const GmodNode* targetNodePtr = nullptr;
		if ( !targetGmod.tryGetNode( nextCode, targetNodePtr ) )
		{
			SPDLOG_ERROR( "Failed to find target node with code {}", nextCode );
			return std::nullopt;
		}

		if ( sourceNode.location().has_value() )
		{
			SPDLOG_INFO( "Attempting to carry over location information for node {}", targetNodePtr->code() );
			const auto& sourceLocation = *sourceNode.location();

			if ( targetNodePtr->isIndividualizable( false, true ) )
			{
				GmodNode resultWithLocation = targetNodePtr->withLocation( sourceLocation );

				if ( resultWithLocation.location().has_value() && resultWithLocation.location() == sourceLocation )
				{
					SPDLOG_INFO( "Successfully applied location '{}' to node '{}'", sourceLocation.value(), resultWithLocation.code() );
					return resultWithLocation;
				}
				else
				{
					if ( !resultWithLocation.location().has_value() )
					{
						SPDLOG_WARN( "Location '{}' could not be applied to target node '{}'. Returning node without location.", sourceLocation.value(), resultWithLocation.code() );
					}
					else
					{
						SPDLOG_WARN( "Applying location '{}' resulted in different location '{}' on target node '{}'. Returning node with new location.", sourceLocation.value(), resultWithLocation.location()->value(), resultWithLocation.code() );
					}
					return resultWithLocation;
				}
			}
			else
			{
				SPDLOG_WARN( "Target node {} is not individualizable, cannot carry over location {}. Returning node without location.", targetNodePtr->code(), sourceLocation.value() );
				if ( targetNodePtr->location().has_value() )
				{
					return targetNodePtr->withLocation( *targetNodePtr->location() );
				}
				else
				{
					SPDLOG_ERROR( "Cannot create an owned GmodNode instance for non-individualizable node {} without copying and no location.", targetNodePtr->code() );
					return std::nullopt;
				}
			}
		}
		else
		{
			SPDLOG_INFO( "No source location to apply for node {}. Returning base target node.", targetNodePtr->code() );
			if ( targetNodePtr->location().has_value() )
			{
				return targetNodePtr->withLocation( *targetNodePtr->location() );
			}
			else
			{
				SPDLOG_ERROR( "Cannot create an owned GmodNode instance for node {} without copying and no location.", targetNodePtr->code() );
				return std::nullopt;
			}
		}
	}

	const GmodVersioning::GmodVersioningNode* GmodVersioning::tryGetVersioningNode(
		VisVersion visVersion ) const
	{
		SPDLOG_INFO( "Looking for versioning node for version {}", static_cast<int>( visVersion ) );

		auto it = m_versioningsMap.find( visVersion );
		if ( it != m_versioningsMap.end() )
		{
			return &it->second;
		}
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// Private Validation Methods
	//-------------------------------------------------------------------------

	void GmodVersioning::validateSourceAndTargetVersions(
		VisVersion sourceVersion, VisVersion targetVersion ) const
	{
		if ( !VisVersionExtensions::isValid( sourceVersion ) )
		{
			SPDLOG_ERROR( "Invalid source version: {}", static_cast<int>( sourceVersion ) );
			throw std::invalid_argument( "Invalid source version" );
		}

		if ( !VisVersionExtensions::isValid( targetVersion ) )
		{
			SPDLOG_ERROR( "Invalid target version: {}", static_cast<int>( targetVersion ) );
			throw std::invalid_argument( "Invalid target version" );
		}

		if ( sourceVersion >= targetVersion )
		{
			SPDLOG_ERROR( "Source version {} must be earlier than target version {}",
				static_cast<int>( sourceVersion ),
				static_cast<int>( targetVersion ) );
			throw std::invalid_argument( "Source version must be earlier than target version" );
		}
	}

	void GmodVersioning::validateSourceAndTargetVersionPair(
		VisVersion sourceVersion, VisVersion targetVersion ) const
	{
		validateSourceAndTargetVersions( sourceVersion, targetVersion );

		if ( sourceVersion == targetVersion )
		{
			SPDLOG_ERROR( "Source and target versions are the same: {} = {}",
				static_cast<int>( sourceVersion ),
				static_cast<int>( targetVersion ) );
			throw std::invalid_argument( "Source and target versions are the same" );
		}

		if ( static_cast<int>( targetVersion ) - static_cast<int>( sourceVersion ) != 1 )
		{
			SPDLOG_ERROR( "Target version {} must be one version higher than source version {}",
				static_cast<int>( targetVersion ),
				static_cast<int>( sourceVersion ) );
			throw std::invalid_argument( "Target version must be one version higher than source version" );
		}
	}

	//-------------------------------------------------------------------------
	// Private Static Utility Methods
	//-------------------------------------------------------------------------

	GmodVersioning::ConversionType GmodVersioning::parseConversionType( const std::string& type )
	{
		if ( type == "changeCode" )
			return ConversionType::ChangeCode;
		if ( type == "merge" )
			return ConversionType::Merge;
		if ( type == "move" )
			return ConversionType::Move;
		if ( type == "assignmentChange" )
			return ConversionType::AssignmentChange;
		if ( type == "assignmentDelete" )
			return ConversionType::AssignmentDelete;

		SPDLOG_ERROR( "Unknown conversion type: {}", type );
		throw std::invalid_argument( "Unknown conversion type: " + type );
	}
}
