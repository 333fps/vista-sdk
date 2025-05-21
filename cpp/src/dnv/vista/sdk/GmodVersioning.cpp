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
	namespace
	{
		class QualifyingNodePair
		{
		public:
			QualifyingNodePair( const GmodNode* sourceNode, GmodNode targetNode )
				: m_sourceNode{ sourceNode },
				  m_targetNode{ std::move( targetNode ) }
			{
			}

			QualifyingNodePair() = delete;
			QualifyingNodePair( const QualifyingNodePair& ) = delete;
			QualifyingNodePair( QualifyingNodePair&& other ) noexcept
				: m_sourceNode( other.m_sourceNode ), m_targetNode( std::move( other.m_targetNode ) )
			{
				other.m_sourceNode = nullptr;
			}

			QualifyingNodePair& operator=( const QualifyingNodePair& ) = delete;
			QualifyingNodePair& operator=( QualifyingNodePair&& ) noexcept = delete;

			~QualifyingNodePair() = default;

			const GmodNode* sourceNode() const { return m_sourceNode; }
			const GmodNode& targetNode() const { return m_targetNode; }

		private:
			const GmodNode* m_sourceNode;
			GmodNode m_targetNode;
		};
	}

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
		validateSourceAndTargetVersions( sourceVersion, targetVersion );

		std::optional<GmodNode> targetEndNode = convertNode( sourceVersion, *sourcePath.node(), targetVersion );
		if ( !targetEndNode.has_value() )
		{
			return std::nullopt;
		}
		GmodNode overallTargetEndNode = *targetEndNode;

		auto& vis = VIS::instance();
		const auto& targetGmod = vis.gmod( targetVersion );

		if ( overallTargetEndNode.isRoot() )
		{
			const GmodNode* rootNodeInGmodPtr = nullptr;
			if ( !targetGmod.tryGetNode( overallTargetEndNode.code(), rootNodeInGmodPtr ) || !rootNodeInGmodPtr )
			{
				SPDLOG_ERROR( "Failed to get root node from targetGmod during root path conversion for code: {}", overallTargetEndNode.code() );
				throw std::runtime_error( "Failed to get root node from targetGmod during root path conversion" );
			}

			return GmodPath( targetGmod, const_cast<GmodNode*>( rootNodeInGmodPtr ), {} );
		}

		std::vector<QualifyingNodePair> qualifyingNodesVec;
		for ( const auto& pair : sourcePath.fullPath() )
		{
			const GmodNode* originalNodeInPath = pair.second;
			std::optional<GmodNode> convertedNodeOpt = convertNode( sourceVersion, *originalNodeInPath, targetVersion );
			if ( !convertedNodeOpt.has_value() )
			{
				SPDLOG_ERROR( "Could not convert node {} forward from {} to {}", originalNodeInPath->code(), VisVersionExtensions::toVersionString( sourceVersion ), VisVersionExtensions::toVersionString( targetVersion ) );
				throw std::runtime_error( "Could not convert node forward" );
			}
			qualifyingNodesVec.push_back( { originalNodeInPath, *convertedNodeOpt } );
		}

		static std::list<GmodNode> nodesWithLocationStorage;

		if ( !qualifyingNodesVec.empty() )
		{
			std::vector<GmodNode*> potentialParentPtrsForInitialCheck;
			std::vector<GmodNode> potentialParentNodesForInitialCheck;

			for ( size_t i = 0; i < qualifyingNodesVec.size() - 1; ++i )
			{
				potentialParentNodesForInitialCheck.push_back( qualifyingNodesVec[i].targetNode() );
			}

			for ( const auto& nodeVal : potentialParentNodesForInitialCheck )
			{
				const GmodNode* baseNodePtr = nullptr;
				if ( !targetGmod.tryGetNode( nodeVal.code(), baseNodePtr ) || !baseNodePtr )
				{
					throw std::runtime_error( "Initial check: Base node not found in targetGmod for parent list." );
				}
				if ( nodeVal.location().has_value() )
				{
					nodesWithLocationStorage.push_back( baseNodePtr->tryWithLocation( *nodeVal.location() ) );
					potentialParentPtrsForInitialCheck.push_back( &nodesWithLocationStorage.back() );
				}
				else
				{
					potentialParentPtrsForInitialCheck.push_back( const_cast<GmodNode*>( baseNodePtr ) );
				}
			}

			GmodNode* overallTargetEndNodeForInitialCheckPtr;
			const GmodNode* baseOverallTargetEndNodePtr = nullptr;
			if ( !targetGmod.tryGetNode( overallTargetEndNode.code(), baseOverallTargetEndNodePtr ) || !baseOverallTargetEndNodePtr )
			{
				throw std::runtime_error( "Initial check: Base node for overallTargetEndNode not found." );
			}
			if ( overallTargetEndNode.location().has_value() )
			{
				nodesWithLocationStorage.push_back( baseOverallTargetEndNodePtr->tryWithLocation( *overallTargetEndNode.location() ) );
				overallTargetEndNodeForInitialCheckPtr = &nodesWithLocationStorage.back();
			}
			else
			{
				overallTargetEndNodeForInitialCheckPtr = const_cast<GmodNode*>( baseOverallTargetEndNodePtr );
			}

			size_t missingLinkAtInitial;
			if ( GmodPath::isValid( potentialParentPtrsForInitialCheck, *overallTargetEndNodeForInitialCheckPtr, missingLinkAtInitial ) )
			{
				return GmodPath( targetGmod, overallTargetEndNodeForInitialCheckPtr, potentialParentPtrsForInitialCheck );
			}
		}

		std::vector<GmodNode> reconstructedPath;

		bool selectionChanged = false;

		auto addToPath =
			[&]( const Gmod& targetGmod, std::vector<GmodNode>& path, GmodNode node ) {
				if ( !path.empty() )
				{
					GmodNode& prevNodeInPath = path.back();

					if ( !prevNodeInPath.isChild( node ) )
					{
						for ( int i = static_cast<int>( path.size() ) - 1; i >= 0; --i )
						{
							std::vector<const GmodNode*> currentParentsPtrsForCheck;
							std::vector<GmodNode> currentParentsValuesForCheck;
							for ( size_t j = 0; j <= static_cast<size_t>( i ); ++j )
							{
								currentParentsValuesForCheck.push_back( path[j] );
							}

							std::vector<const GmodNode*> remainingIntermediateGmodNodesPtrs;

							for ( const auto& val_node : currentParentsValuesForCheck )
							{
								const GmodNode* ptr_val_node = nullptr;
								if ( !targetGmod.tryGetNode( val_node.code(), ptr_val_node ) )
									throw std::runtime_error( "Failed to get node for pathExistsBetween parent list" );
								if ( val_node.location().has_value() )
								{
									nodesWithLocationStorage.push_back( ptr_val_node->tryWithLocation( *val_node.location() ) );
									currentParentsPtrsForCheck.push_back( &nodesWithLocationStorage.back() );
								}
								else
								{
									currentParentsPtrsForCheck.push_back( ptr_val_node );
								}
							}
							const GmodNode* nodeToAddAsGmodPtr = nullptr;
							if ( !targetGmod.tryGetNode( node.code(), nodeToAddAsGmodPtr ) )
								throw std::runtime_error( "Failed to get node for pathExistsBetween toNode" );
							GmodNode locatednodeForPathExists = node;
							if ( node.location().has_value() )
							{
								nodesWithLocationStorage.push_back( nodeToAddAsGmodPtr->tryWithLocation( *node.location() ) );
								locatednodeForPathExists = nodesWithLocationStorage.back();
							}
							else
							{
								locatednodeForPathExists = *nodeToAddAsGmodPtr;
							}

							bool pathFound = GmodTraversal::pathExistsBetween(
								targetGmod, currentParentsPtrsForCheck,
								locatednodeForPathExists, remainingIntermediateGmodNodesPtrs );

							if ( !pathFound )
							{

								const GmodNode& currentParentCandidateForRemoval = path[static_cast<size_t>( i )];
								bool otherAssetFunctionExists = false;
								for ( size_t j = 0; j <= static_cast<size_t>( i ); ++j )
								{
									if ( path[j].isAssetFunctionNode() && path[j].code() != currentParentCandidateForRemoval.code() )
									{
										otherAssetFunctionExists = true;
										break;
									}
								}
								if ( currentParentCandidateForRemoval.isAssetFunctionNode() && !otherAssetFunctionExists )
								{
									throw std::runtime_error( "Tried to remove last asset function node" );
								}
								path.erase( path.begin() + i );
							}
							else
							{
								std::vector<GmodNode> nodesToInsertInPathValues;
								if ( node.location().has_value() )
								{
									for ( const GmodNode* interGmodNodePtr : remainingIntermediateGmodNodesPtrs )
									{
										if ( !interGmodNodePtr->isIndividualizable( false, true ) )
										{
											nodesToInsertInPathValues.push_back( *interGmodNodePtr );
										}
										else
										{
											nodesToInsertInPathValues.push_back( interGmodNodePtr->tryWithLocation( *node.location() ) );
										}
									}
								}
								else
								{
									for ( const GmodNode* interGmodNodePtr : remainingIntermediateGmodNodesPtrs )
									{
										nodesToInsertInPathValues.push_back( *interGmodNodePtr );
									}
								}
								path.insert( path.end(), nodesToInsertInPathValues.begin(), nodesToInsertInPathValues.end() );
								break;
							}
						}
					}
				}
				path.push_back( node );
			};

		for ( size_t i = 0; i < qualifyingNodesVec.size(); ++i )
		{
			const auto& qualifyingNodeEntry = qualifyingNodesVec[i];

			if ( i > 0 && qualifyingNodeEntry.targetNode().code() == qualifyingNodesVec[i - 1].targetNode().code() )
			{
				continue;
			}

			bool codeChanged = qualifyingNodeEntry.sourceNode()->code() != qualifyingNodeEntry.targetNode().code();

			const GmodNode* sourceNormalAssignment = qualifyingNodeEntry.sourceNode()->productType();
			const GmodNode* targetNormalAssignment = qualifyingNodeEntry.targetNode().productType();

			bool normalAssignmentChanged =
				( sourceNormalAssignment == nullptr && targetNormalAssignment != nullptr ) ||
				( sourceNormalAssignment != nullptr && targetNormalAssignment == nullptr ) ||
				( sourceNormalAssignment != nullptr && targetNormalAssignment != nullptr &&
					sourceNormalAssignment->code() != targetNormalAssignment->code() );

			if ( codeChanged )
			{
				addToPath( targetGmod, reconstructedPath, qualifyingNodeEntry.targetNode() );
			}
			else if ( normalAssignmentChanged )
			{ /* AC || AN || AD */
				bool wasDeleted = sourceNormalAssignment != nullptr && targetNormalAssignment == nullptr;

				if ( !codeChanged )
				{
					addToPath( targetGmod, reconstructedPath, qualifyingNodeEntry.targetNode() );
				}

				if ( wasDeleted )
				{
					if ( qualifyingNodeEntry.targetNode().code() == overallTargetEndNode.code() )
					{
						if ( i + 1 < qualifyingNodesVec.size() )
						{
							const auto& nextQualifyingNode = qualifyingNodesVec[i + 1];
							if ( nextQualifyingNode.targetNode().code() != qualifyingNodeEntry.targetNode().code() )
							{
								throw std::runtime_error( "Normal assignment end node was deleted" );
							}
						}
					}
					continue;
				}
				else
				{
					if ( qualifyingNodeEntry.targetNode().code() != overallTargetEndNode.code() )
					{
						if ( targetNormalAssignment != nullptr )
						{
							GmodNode targetNormalAssignmentVal = *targetNormalAssignment;
							if ( qualifyingNodeEntry.targetNode().location().has_value() && targetNormalAssignment->isIndividualizable( false, true ) )
							{
								targetNormalAssignmentVal = targetNormalAssignment->tryWithLocation( *qualifyingNodeEntry.targetNode().location() );
							}
							addToPath( targetGmod, reconstructedPath, targetNormalAssignmentVal );
							i++;
						}
					}
				}
			}

			if ( selectionChanged )
			{ /* SC || SN || SD */
			}

			if ( !codeChanged && !normalAssignmentChanged )
			{
				addToPath( targetGmod, reconstructedPath, qualifyingNodeEntry.targetNode() );
			}

			if ( !reconstructedPath.empty() && reconstructedPath.back().code() == overallTargetEndNode.code() )
			{
				break;
			}
		}

		if ( reconstructedPath.empty() )
		{
			SPDLOG_ERROR( "Path reconstruction resulted in an empty path for source: {}", sourcePath.toString() );
			throw std::runtime_error( "Path reconstruction resulted in an empty path for source: " + sourcePath.toString() );
		}

		std::vector<GmodNode*> finalParentPtrs;
		GmodNode* finalEndNodePtr = nullptr;

		for ( size_t k = 0; k < reconstructedPath.size(); ++k )
		{
			const GmodNode& pathNodeVal = reconstructedPath[k];
			const GmodNode* baseNodeInGmodPtr = nullptr;
			if ( !targetGmod.tryGetNode( pathNodeVal.code(), baseNodeInGmodPtr ) || !baseNodeInGmodPtr )
			{
				SPDLOG_ERROR( "Node {} from reconstructed path not found in target GMOD during finalization.", pathNodeVal.toString() );
				throw std::runtime_error( "Node from reconstructed path not found in target GMOD during finalization." );
			}

			GmodNode* nodeToAddAsPtr;
			if ( pathNodeVal.location().has_value() )
			{
				GmodNode nodeWithLocation = baseNodeInGmodPtr->tryWithLocation( *pathNodeVal.location() );

				if ( nodeWithLocation.code().empty() )
				{
					SPDLOG_WARN( "tryWithLocation() resulted in empty code for node '{}' with location '{}', using base node instead",
						baseNodeInGmodPtr->code().data(), pathNodeVal.location()->toString() );

					nodeToAddAsPtr = const_cast<GmodNode*>( baseNodeInGmodPtr );
				}
				else
				{
					nodesWithLocationStorage.push_back( nodeWithLocation );
					nodeToAddAsPtr = &nodesWithLocationStorage.back();
				}
			}
			else
			{
				nodeToAddAsPtr = const_cast<GmodNode*>( baseNodeInGmodPtr );
			}

			if ( k < reconstructedPath.size() - 1 )
			{
				finalParentPtrs.push_back( nodeToAddAsPtr );
			}
			else
			{
				finalEndNodePtr = nodeToAddAsPtr;
			}
		}

		if ( !finalEndNodePtr )
		{
			SPDLOG_ERROR( "Final end node pointer is null after reconstruction for source: {}", sourcePath.toString() );
			throw std::runtime_error( "Final end node pointer is null after reconstruction." );
		}

		size_t missingLinkAtFinal;
		if ( !GmodPath::isValid( finalParentPtrs, *finalEndNodePtr, missingLinkAtFinal ) )
		{
			std::string errorMsg = "Did not end up with a valid path for " + sourcePath.toString();
			SPDLOG_ERROR( errorMsg );
			throw std::runtime_error( errorMsg );
		}

		return GmodPath( targetGmod, finalEndNodePtr, finalParentPtrs );
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
		: m_visVersion{ visVersion }
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

		GmodNode resultNode = targetNodePtr->tryWithLocation( sourceNode.location() );

		if ( sourceNode.location().has_value() &&
			 ( !resultNode.location().has_value() || resultNode.location().value() != sourceNode.location().value() ) )
		{
			std::string errorMsg = "Failed to set location for node " + sourceNode.code() +
								   " (source location: " + ( sourceNode.location() ? sourceNode.location()->toString() : "none" ) +
								   ", result location: " + ( resultNode.location() ? resultNode.location()->toString() : "none" ) + ")";
			SPDLOG_ERROR( errorMsg );
			throw std::runtime_error( errorMsg );
		}
		else if ( sourceNode.location().has_value() )
		{
			SPDLOG_DEBUG( "Successfully preserved/set location {} for node {} during conversion",
				sourceNode.location()->value().toString(), resultNode.code() );
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
			throw std::invalid_argument( "Source version must be less than target version" );
		}

		const auto& allVersions = VisVersionExtensions::allVersions();
		auto itSource = std::find( allVersions.begin(), allVersions.end(), sourceVersion );

		bool isTargetTheExactNextVersion = false;
		if ( itSource != allVersions.end() )
		{
			auto itNext = std::next( itSource );
			if ( itNext != allVersions.end() )
			{
				if ( *itNext == targetVersion )
				{
					isTargetTheExactNextVersion = true;
				}
			}
		}

		if ( !isTargetTheExactNextVersion )
		{
			throw std::invalid_argument( "Target version must be exactly one version higher than source version" );
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
