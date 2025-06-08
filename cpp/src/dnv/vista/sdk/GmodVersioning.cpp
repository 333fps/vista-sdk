/**
 * @file GmodVersioning.cpp
 * @brief Implementation of the GmodVersioning class for converting GMOD objects between VIS versions.
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodVersioning.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodNode.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/GmodTraversal.h"
#include "dnv/vista/sdk/LocalId.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/MetadataTag.h"
#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VISVersion.h"

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
	// GmodVersioning class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	GmodVersioning::GmodVersioning( const std::unordered_map<std::string, GmodVersioningDto>& dto )
	{
		for ( const auto& [versionStr, versioningDto] : dto )
		{
			VisVersion version = VisVersionExtensions::parse( versionStr );

			m_versioningsMap.emplace( version, GmodVersioningNode( version, versioningDto.items() ) );
		}
	}

	//----------------------------------------------
	// Conversion
	//----------------------------------------------

	//----------------------------
	// Node
	//----------------------------

	std::optional<GmodNode> GmodVersioning::convertNode(
		VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const
	{
		if ( sourceNode.code().empty() )
			return std::nullopt;

		validateSourceAndTargetVersions( sourceVersion, targetVersion );

		std::optional<GmodNode> node = sourceNode;
		VisVersion source = sourceVersion;

		while ( static_cast<int>( source ) <= static_cast<int>( targetVersion ) - 100 )
		{
			if ( !node.has_value() )
			{
				break;
			}

			VisVersion target = static_cast<VisVersion>( static_cast<int>( source ) + 100 );

			node = convertNodeInternal( source, *node, target );

			source = static_cast<VisVersion>( static_cast<int>( source ) + 100 );
		}

		return node;
	}

	//----------------------------
	// Path
	//----------------------------

	std::optional<GmodPath> GmodVersioning::convertPath(
		VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) const
	{
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
				throw std::runtime_error( "Failed to get root node from targetGmod during root path conversion for code: " +
										  std::string( overallTargetEndNode.code() ) );
			}

			return GmodPath( targetGmod, const_cast<GmodNode*>( rootNodeInGmodPtr ), {} );
		}

		std::vector<QualifyingNodePair> qualifyingNodesVec;
		auto enumerator = sourcePath.enumerator();
		while ( enumerator.next() )
		{
			const auto& [depth, originalNodeInPath] = enumerator.current();

			std::optional<GmodNode> convertedNodeOpt = convertNode( sourceVersion, *originalNodeInPath, targetVersion );
			if ( !convertedNodeOpt.has_value() )
			{
				throw std::runtime_error( "Could not convert node " +
										  std::string( originalNodeInPath->code() ) + " forward from " +
										  VisVersionExtensions::toVersionString( sourceVersion ) + " to " +
										  VisVersionExtensions::toVersionString( targetVersion ) );
			}

			qualifyingNodesVec.emplace_back( originalNodeInPath, std::move( *convertedNodeOpt ) );
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

			int missingLinkAtInitial;
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
							++i;
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
				throw std::runtime_error( "Node " + pathNodeVal.toString() +
										  " from reconstructed path not found in target GMOD during finalization." );
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
			throw std::runtime_error( "Final end node pointer is null after reconstruction for source: " +
									  sourcePath.toString() );
		}

		int missingLinkAtFinal;
		if ( !GmodPath::isValid( finalParentPtrs, *finalEndNodePtr, missingLinkAtFinal ) )
		{
			throw std::runtime_error( "Did not end up with a valid path for " + sourcePath.toString() );
		}

		return GmodPath( targetGmod, finalEndNodePtr, finalParentPtrs );
	}

	//----------------------------
	// Local Id
	//----------------------------

	std::optional<LocalIdBuilder> GmodVersioning::convertLocalId(
		const LocalIdBuilder& sourceLocalId, VisVersion targetVersion ) const
	{
		if ( !sourceLocalId.visVersion().has_value() ) [[unlikely]]
		{
			throw std::invalid_argument( "Cannot convert local ID without a specific VIS version" );
		}

		LocalIdBuilder targetLocalId = LocalIdBuilder::create( targetVersion );

		if ( sourceLocalId.primaryItem().has_value() )
		{
			auto targetPrimaryItem = convertPath(
				*sourceLocalId.visVersion(),
				sourceLocalId.primaryItem().value(),
				targetVersion );

			if ( !targetPrimaryItem.has_value() ) [[unlikely]]
			{
				return std::nullopt;
			}

			targetLocalId = targetLocalId.withPrimaryItem( std::move( *targetPrimaryItem ) );
		}

		if ( sourceLocalId.secondaryItem().has_value() )
		{
			auto targetSecondaryItem = convertPath(
				*sourceLocalId.visVersion(),
				sourceLocalId.secondaryItem().value(),
				targetVersion );

			if ( !targetSecondaryItem.has_value() ) [[unlikely]]
			{
				return std::nullopt;
			}

			targetLocalId = targetLocalId.withSecondaryItem( std::move( *targetSecondaryItem ) );
		}

		return targetLocalId
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
		auto builder = convertLocalId( sourceLocalId.builder(), targetVersion );

		return builder.has_value()
				   ? std::make_optional( builder->build() )
				   : std::nullopt;
	}

	//----------------------------------------------
	// GmodVersioningNode class
	//----------------------------------------------

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	GmodVersioning::GmodVersioningNode::GmodVersioningNode(
		VisVersion visVersion,
		const std::unordered_map<std::string, GmodNodeConversionDto>& dto )
		: m_visVersion{ visVersion }
	{
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
		auto it = m_versioningNodeChanges.find( code );
		if ( it != m_versioningNodeChanges.end() ) [[likely]]
		{
			nodeChanges = it->second;

			return true;
		}

		return false;
	}

	//----------------------------------------------
	// Private helper methods
	//----------------------------------------------

	std::optional<GmodNode> GmodVersioning::convertNodeInternal(
		[[maybe_unused]] VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const
	{
		validateSourceAndTargetVersionPair( sourceNode.visVersion(), targetVersion );

		std::string nextCode = sourceNode.code();

		auto versioningIt = m_versioningsMap.find( targetVersion );
		if ( versioningIt != m_versioningsMap.end() ) [[likely]]
		{
			const auto& versioningNode = versioningIt->second;

			GmodNodeConversion change{};
			if ( versioningNode.tryGetCodeChanges( sourceNode.code(), change ) && change.target.has_value() ) [[likely]]
			{
				nextCode = change.target.value();
			}
		}

		const auto& targetGmod = VIS::instance().gmod( targetVersion );

		const GmodNode* targetNodePtr = nullptr;
		if ( !targetGmod.tryGetNode( nextCode, targetNodePtr ) ) [[unlikely]]
		{
			return std::nullopt;
		}

		auto resultNode = targetNodePtr->tryWithLocation( sourceNode.location() );
		if ( sourceNode.location().has_value() && resultNode.location() != sourceNode.location() ) [[unlikely]]
		{
			throw std::runtime_error( "Failed to set location" );
		}

		return resultNode;
	}

	bool GmodVersioning::tryGetVersioningNode(
		VisVersion visVersion,
		GmodVersioningNode& versioningNode ) const
	{
		auto it = m_versioningsMap.find( visVersion );
		if ( it != m_versioningsMap.end() ) [[likely]]
		{
			versioningNode = it->second;

			return true;
		}

		return false;
	}

	//----------------------------------------------
	// Private validation methods
	//----------------------------------------------

	void GmodVersioning::validateSourceAndTargetVersions(
		VisVersion sourceVersion, VisVersion targetVersion ) const
	{
		if ( sourceVersion == VisVersion::Unknown ) [[unlikely]]
		{
			throw std::invalid_argument( "Invalid source VIS Version: Unknown" );
		}

		if ( targetVersion == VisVersion::Unknown ) [[unlikely]]
		{
			throw std::invalid_argument( "Invalid target VIS Version: Unknown" );
		}

		if ( sourceVersion >= targetVersion ) [[unlikely]]
		{
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

		if ( static_cast<int>( targetVersion ) - static_cast<int>( sourceVersion ) != 100 )
		{
			throw std::invalid_argument( "Target version must be exactly one version higher than source version" );
		}
	}

	//----------------------------------------------
	// Private static utility methods
	//----------------------------------------------

	GmodVersioning::ConversionType GmodVersioning::parseConversionType( const std::string& type )
	{
		if ( type == "changeCode" )
		{
			return ConversionType::ChangeCode;
		}
		if ( type == "merge" )
		{
			return ConversionType::Merge;
		}
		if ( type == "move" )
		{
			return ConversionType::Move;
		}
		if ( type == "assignmentChange" )
		{
			return ConversionType::AssignmentChange;
		}
		if ( type == "assignmentDelete" )
		{
			return ConversionType::AssignmentDelete;
		}

		throw std::invalid_argument( "Invalid conversion type: " + type );
	}
}
