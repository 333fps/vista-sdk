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
				: m_sourceNode( sourceNode ), m_targetNode( std::move( targetNode ) )
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

		auto& vis = VIS::instance();
		const auto& targetGmod = vis.gmod( targetVersion );

		std::optional<GmodNode> overallConvertedTargetEndNodeOpt = convertNode( sourceVersion, *sourcePath.node(), targetVersion );
		if ( !overallConvertedTargetEndNodeOpt.has_value() )
		{
			return std::nullopt;
		}
		const GmodNode& overallConvertedTargetEndNode = *overallConvertedTargetEndNodeOpt;

		if ( overallConvertedTargetEndNode.isRoot() )
		{
			const GmodNode* rootNodePtr = nullptr;
			if ( !targetGmod.tryGetNode( std::string( "VE" ), rootNodePtr ) || !rootNodePtr )
			{
				return std::nullopt;
			}
			return GmodPath( targetGmod, const_cast<GmodNode*>( rootNodePtr ), {} );
		}

		std::vector<QualifyingNodePair> qualifyingNodes;
		for ( const auto& pair : sourcePath.fullPath() )
		{
			const GmodNode* originalNodeInPath = pair.second;
			std::optional<GmodNode> convertedNodeOpt = convertNode( sourceVersion, *originalNodeInPath, targetVersion );
			if ( !convertedNodeOpt.has_value() )
			{
				return std::nullopt;
			}
			qualifyingNodes.emplace_back( originalNodeInPath, std::move( *convertedNodeOpt ) );
		}

		if ( qualifyingNodes.empty() )
		{
			return std::nullopt;
		}

		std::list<GmodNode> nodesWithLocationList;
		std::vector<GmodNode*> initialPotentialParents;
		if ( qualifyingNodes.size() > 1 )
		{
			initialPotentialParents.reserve( qualifyingNodes.size() - 1 );
			for ( size_t i = 0; i < qualifyingNodes.size() - 1; ++i )
			{
				const GmodNode& qnTargetNode = qualifyingNodes[i].targetNode();
				const GmodNode* nodePtr = nullptr;
				if ( !targetGmod.tryGetNode( qnTargetNode.code(), nodePtr ) || !nodePtr )
					return std::nullopt;

				if ( qnTargetNode.location().has_value() )
				{
					nodesWithLocationList.emplace_back( nodePtr->tryWithLocation( qnTargetNode.location() ) );
					initialPotentialParents.push_back( &nodesWithLocationList.back() );
				}
				else
				{
					initialPotentialParents.push_back( const_cast<GmodNode*>( nodePtr ) );
				}
			}
		}

		const GmodNode& initialEndNodeCandidateFromQualifying = qualifyingNodes.back().targetNode();
		GmodNode* initialTargetEndNodeWithLocationPtr;
		const GmodNode* initialEndNodeBasePtr = nullptr;
		if ( !targetGmod.tryGetNode( initialEndNodeCandidateFromQualifying.code(), initialEndNodeBasePtr ) || !initialEndNodeBasePtr )
			return std::nullopt;

		if ( initialEndNodeCandidateFromQualifying.location().has_value() )
		{
			nodesWithLocationList.emplace_back( initialEndNodeBasePtr->tryWithLocation( initialEndNodeCandidateFromQualifying.location() ) );
			initialTargetEndNodeWithLocationPtr = &nodesWithLocationList.back();
		}
		else
		{
			initialTargetEndNodeWithLocationPtr = const_cast<GmodNode*>( initialEndNodeBasePtr );
		}

		size_t missingLinkAt;
		if ( GmodPath::isValid( initialPotentialParents, *initialTargetEndNodeWithLocationPtr, missingLinkAt ) )
		{
			return GmodPath( targetGmod, initialTargetEndNodeWithLocationPtr, initialPotentialParents );
		}

		std::vector<GmodNode> reconstructedPathNodes;
		auto addToPathHelperLambda =
			[&targetGmod]( std::vector<GmodNode>& currentPath, const GmodNode& nodeToAdd ) {
				if ( currentPath.empty() )
				{
					currentPath.push_back( nodeToAdd );
					return;
				}

				GmodNode& previousNode = currentPath.back();
				if ( previousNode.code() == nodeToAdd.code() && previousNode.location() == nodeToAdd.location() )
				{
					return;
				}

				if ( !previousNode.isChild( nodeToAdd ) )
				{
					for ( size_t i = currentPath.size() - 1;; --i )
					{
						if ( i >= currentPath.size() )
						{
							break;
						}
						GmodNode& parentCandidate = currentPath[i];
						std::vector<const GmodNode*> currentParentChain;
						for ( size_t j = 0; j <= i; ++j )
						{
							currentParentChain.push_back( &currentPath[j] );
						}

						std::vector<const GmodNode*> intermediateNodes;
						if ( !GmodTraversal::pathExistsBetween( targetGmod, currentParentChain, nodeToAdd, intermediateNodes ) )
						{
							if ( parentCandidate.isAssetFunctionNode() )
							{
								bool hasOtherAssetFunction = false;
								for ( const auto* nPtr : currentParentChain )
								{
									if ( nPtr != &parentCandidate && nPtr->isAssetFunctionNode() )
									{
										hasOtherAssetFunction = true;
										break;
									}
								}
								if ( !hasOtherAssetFunction )
								{
									throw std::runtime_error( "Tried to remove last asset function node" );
								}
							}
							currentPath.erase( currentPath.begin() + i );
							if ( i == 0 && currentPath.empty() )
								break;
							if ( i == 0 )
							{
								break;
							}
						}
						else
						{
							for ( const auto* interNodePtr : intermediateNodes )
							{
								if ( nodeToAdd.location().has_value() && interNodePtr->isIndividualizable( false, true ) )
								{
									currentPath.push_back( interNodePtr->tryWithLocation( *nodeToAdd.location() ) );
								}
								else
								{
									currentPath.push_back( *interNodePtr );
								}
							}
							break;
						}
					}
				}

				if ( currentPath.empty() || currentPath.back().code() != nodeToAdd.code() || currentPath.back().location() != nodeToAdd.location() )
				{
					currentPath.push_back( nodeToAdd );
				}
			};

		if ( !qualifyingNodes.empty() )
		{
			addToPathHelperLambda( reconstructedPathNodes, qualifyingNodes[0].targetNode() );
		}

		for ( size_t i = 1; i < qualifyingNodes.size(); ++i )
		{
			const auto& qn = qualifyingNodes[i];
			const GmodNode* sourceNode = qn.sourceNode();
			const GmodNode& convertedTargetNode = qn.targetNode();

			if ( convertedTargetNode.code() == qualifyingNodes[i - 1].targetNode().code() &&
				 convertedTargetNode.location() == qualifyingNodes[i - 1].targetNode().location() )
			{
				continue;
			}

			bool codeChanged = sourceNode->code() != convertedTargetNode.code();
			const GmodNode* sourceNormalAssignment = sourceNode->productType();
			const GmodNode* targetNormalAssignment = convertedTargetNode.productType();

			bool normalAssignmentChanged =
				( sourceNormalAssignment == nullptr && targetNormalAssignment != nullptr ) ||
				( sourceNormalAssignment != nullptr && targetNormalAssignment == nullptr ) ||
				( sourceNormalAssignment != nullptr && targetNormalAssignment != nullptr &&
					sourceNormalAssignment->code() != targetNormalAssignment->code() );

			if ( codeChanged )
			{
				addToPathHelperLambda( reconstructedPathNodes, convertedTargetNode );
			}
			else if ( normalAssignmentChanged )
			{
				addToPathHelperLambda( reconstructedPathNodes, convertedTargetNode );
				bool assignmentWasDeleted = sourceNormalAssignment != nullptr && targetNormalAssignment == nullptr;
				if ( !assignmentWasDeleted && targetNormalAssignment != nullptr )
				{
					addToPathHelperLambda( reconstructedPathNodes, *targetNormalAssignment );
				}
			}
			else
			{
				addToPathHelperLambda( reconstructedPathNodes, convertedTargetNode );
			}

			if ( !reconstructedPathNodes.empty() &&
				 reconstructedPathNodes.back().code() == overallConvertedTargetEndNode.code() &&
				 reconstructedPathNodes.back().location() == overallConvertedTargetEndNode.location() )
			{
				break;
			}
		}

		if ( reconstructedPathNodes.empty() )
		{
			return std::nullopt;
		}

		if ( reconstructedPathNodes.back().code() != overallConvertedTargetEndNode.code() ||
			 reconstructedPathNodes.back().location() != overallConvertedTargetEndNode.location() )
		{
			addToPathHelperLambda( reconstructedPathNodes, overallConvertedTargetEndNode );
		}

		if ( reconstructedPathNodes.empty() ||
			 reconstructedPathNodes.back().code() != overallConvertedTargetEndNode.code() ||
			 reconstructedPathNodes.back().location() != overallConvertedTargetEndNode.location() )
		{
			return std::nullopt;
		}

		std::vector<GmodNode*> finalParentPointers;
		if ( reconstructedPathNodes.size() > 1 )
		{
			finalParentPointers.reserve( reconstructedPathNodes.size() - 1 );
			for ( size_t idx = 0; idx < reconstructedPathNodes.size() - 1; ++idx )
			{
				const GmodNode& reconstructedNode = reconstructedPathNodes[idx];
				const GmodNode* nodePtrFromGmod = nullptr;
				if ( !targetGmod.tryGetNode( reconstructedNode.code(), nodePtrFromGmod ) || !nodePtrFromGmod )
					return std::nullopt;

				if ( reconstructedNode.location().has_value() )
				{
					nodesWithLocationList.emplace_back( nodePtrFromGmod->tryWithLocation( reconstructedNode.location() ) );
					finalParentPointers.push_back( &nodesWithLocationList.back() );
				}
				else
				{
					finalParentPointers.push_back( const_cast<GmodNode*>( nodePtrFromGmod ) );
				}
			}
		}

		const GmodNode& finalEndNodeFromReconstruction = reconstructedPathNodes.back();
		GmodNode* finalEndNodeForPathConstruction;
		const GmodNode* finalEndNodeBasePtrFromGmod = nullptr;
		if ( !targetGmod.tryGetNode( finalEndNodeFromReconstruction.code(), finalEndNodeBasePtrFromGmod ) || !finalEndNodeBasePtrFromGmod )
			return std::nullopt;

		if ( finalEndNodeFromReconstruction.location().has_value() )
		{
			nodesWithLocationList.emplace_back( finalEndNodeBasePtrFromGmod->tryWithLocation( finalEndNodeFromReconstruction.location() ) );
			finalEndNodeForPathConstruction = &nodesWithLocationList.back();
		}
		else
		{
			finalEndNodeForPathConstruction = const_cast<GmodNode*>( finalEndNodeBasePtrFromGmod );
		}

		size_t finalMissingLinkAt;
		if ( !GmodPath::isValid( finalParentPointers, *finalEndNodeForPathConstruction, finalMissingLinkAt ) )
		{
			return std::nullopt;
		}

		return GmodPath( targetGmod, finalEndNodeForPathConstruction, finalParentPointers );
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
