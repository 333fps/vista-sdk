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

			SPDLOG_INFO( "Adding version {} with {} items", versionStr, versioningDto.items.size() );
			m_versioningsMap.emplace( version, GmodVersioningNode( version, versioningDto.items ) );
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

		if ( sourceVersion == targetVersion )
			return sourceNode;

		if ( sourceNode.code().empty() )
			return std::nullopt;

		validateSourceAndTargetVersions( sourceVersion, targetVersion );

		VisVersion source = sourceVersion;
		GmodNode node = sourceNode;

		while ( source < targetVersion )
		{
			VisVersion next = static_cast<VisVersion>( static_cast<int>( source ) + 1 );
			node = convertNodeInternal( source, node, next );

			if ( node.code().empty() )
			{
				SPDLOG_ERROR( "Node conversion failed from {} to {}",
					static_cast<int>( source ),
					static_cast<int>( next ) );
				return std::nullopt;
			}

			source = next;
		}

		SPDLOG_INFO( "Node successfully converted to {}", node.code() );
		return node;
	}

	std::optional<GmodPath> GmodVersioning::convertPath( VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) const
	{
		SPDLOG_INFO( "Converting path with end node {} from version {} to {}",
			sourcePath.node().code(),
			static_cast<int>( sourceVersion ),
			static_cast<int>( targetVersion ) );

		validateSourceAndTargetVersions( sourceVersion, targetVersion );

		auto targetEndNode = convertNode( sourceVersion, sourcePath.node(), targetVersion );
		if ( !targetEndNode.has_value() )
		{
			SPDLOG_ERROR( "Failed to convert end node: {}", sourcePath.node().code() );
			return std::nullopt;
		}

		if ( targetEndNode->isRoot() )
		{
			SPDLOG_INFO( "End node is a root node, returning simple path" );
			return GmodPath( {}, *targetEndNode, true );
		}

		const auto& targetGmod = VIS::instance().gmod( targetVersion );

		std::vector<std::pair<GmodNode, GmodNode>> qualifyingNodes;
		for ( const auto& pathNode : sourcePath.fullPath() )
		{
			auto convertedNode = convertNode( sourceVersion, pathNode.second, targetVersion );
			if ( !convertedNode.has_value() )
			{
				// SPDLOG_ERROR( "Failed to convert path node: {}", pathNode.second.code() );
				return std::nullopt;
			}

			qualifyingNodes.emplace_back( pathNode.second, *convertedNode );
		}

		if ( std::any_of( qualifyingNodes.begin(), qualifyingNodes.end(),
				 []( const auto& pair ) { return pair.second.code().empty(); } ) )
		{
			SPDLOG_ERROR( "Failed to convert node forward" );
			throw std::runtime_error( "Failed to convert node forward" );
		}

		SPDLOG_INFO( "Building path for converted node, found {} qualifying nodes",
			qualifyingNodes.size() );

		std::vector<GmodNode> potentialParents;
		for ( size_t i = 0; i < qualifyingNodes.size() - 1; ++i )
		{
			potentialParents.push_back( qualifyingNodes[i].second );
		}

		if ( GmodPath::isValid( potentialParents, *targetEndNode ) )
		{
			SPDLOG_INFO( "Found valid direct path" );
			return GmodPath( potentialParents, *targetEndNode, true );
		}

		auto addToPath = []( const Gmod& targetGmod, std::vector<GmodNode>& path, const GmodNode& node ) -> bool {
			if ( !path.empty() )
			{
				const GmodNode& prev = path.back();
				if ( !prev.isChild( node ) )
				{
					for ( int j = static_cast<int>( path.size() ) - 1; j >= 0; --j )
					{
						const GmodNode& parent = path[static_cast<size_t>( j )];
						std::vector<GmodNode> currentParents( path.begin(), path.begin() + ( j + 1 ) );

						std::vector<GmodNode> remaining;
						if ( !targetGmod.pathExistsBetween( currentParents, node, remaining ) )
						{
							bool hasOtherAssetFunction = false;
							for ( const auto& n : currentParents )
							{
								if ( n.isAssetFunctionNode() && n.code() != parent.code() )
								{
									hasOtherAssetFunction = true;
									break;
								}
							}

							if ( !hasOtherAssetFunction )
							{
								throw std::runtime_error( "Tried to remove last asset function node" );
							}

							path.erase( path.begin() + j );
						}
						else
						{
							std::vector<GmodNode> nodes;
							if ( node.location().has_value() )
							{
								for ( const auto& n : remaining )
								{
									if ( !n.isIndividualizable( false, true ) )
									{
										nodes.push_back( n );
									}
									else
									{
										nodes.push_back( n.withLocation( node.location().value() ) );
									}
								}
							}
							else
							{
								nodes.insert( nodes.end(), remaining.begin(), remaining.end() );
							}

							path.insert( path.end(), nodes.begin(), nodes.end() );
							break;
						}
					}
				}
			}

			path.push_back( node );
			return true;
		};

		std::vector<GmodNode> path;
		for ( size_t i = 0; i < qualifyingNodes.size(); ++i )
		{
			const auto& qualifyingNode = qualifyingNodes[i];
			const auto& sourceNode = qualifyingNode.first;
			const auto& targetNode = qualifyingNode.second;

			if ( i > 0 && targetNode.code() == qualifyingNodes[i - 1].second.code() )
				continue;

			bool codeChanged = sourceNode.code() != targetNode.code();

			const GmodNode* sourceNormalAssignment = sourceNode.productType();
			const GmodNode* targetNormalAssignment = targetNode.productType();

			bool normalAssignmentChanged = false;
			bool wasDeleted = false;

			if ( ( sourceNormalAssignment != nullptr ) != ( targetNormalAssignment != nullptr ) )
			{
				normalAssignmentChanged = true;
				wasDeleted = targetNormalAssignment == nullptr;
			}
			else if ( sourceNormalAssignment != nullptr && targetNormalAssignment != nullptr )
			{
				auto convertedSourceNormalAssignment =
					convertNode( sourceVersion, *sourceNormalAssignment, targetVersion );

				normalAssignmentChanged =
					!convertedSourceNormalAssignment.has_value() ||
					convertedSourceNormalAssignment->code() != targetNormalAssignment->code();
			}

			bool selectionChanged = false;
			if ( sourceNode.isProductSelection() != targetNode.isProductSelection() )
			{
				selectionChanged = true;
			}
			else if ( sourceNode.isProductSelection() && targetNode.isProductSelection() )
			{
				selectionChanged = sourceNode.code() != targetNode.code();
			}

			if ( codeChanged )
			{
				SPDLOG_INFO( "Code changed from {} to {}", sourceNode.code(), targetNode.code() );
				if ( !addToPath( targetGmod, path, targetNode ) )
				{
					SPDLOG_ERROR( "Failed to add node to path: parent-child relationship broken" );
					throw std::runtime_error( "Failed to add node to path: parent-child relationship broken" );
				}
			}
			else if ( normalAssignmentChanged ) // AC || AN || AD
			{
				SPDLOG_INFO( "Normal assignment changed for node {}", targetNode.code() );

				if ( !codeChanged && !path.empty() && path.back().code() == targetNode.code() )
				{
					continue;
				}

				if ( wasDeleted )
				{
					if ( targetNode.code() == targetEndNode->code() )
					{
						bool skipNode = false;
						if ( i + 1 < qualifyingNodes.size() )
						{
							const auto& next = qualifyingNodes[i + 1];
							if ( next.second.code() != qualifyingNode.second.code() )
							{
								skipNode = true;
							}
						}
						if ( !skipNode )
						{
							if ( !addToPath( targetGmod, path, targetNode ) )
							{
								SPDLOG_ERROR( "Failed to add node to path: parent-child relationship broken" );
								throw std::runtime_error( "Failed to add node to path: parent-child relationship broken" );
							}
						}
					}
				}
				else if ( targetNode.code() != targetEndNode->code() )
				{
					if ( !addToPath( targetGmod, path, targetNode ) )
					{
						SPDLOG_ERROR( "Failed to add node to path: parent-child relationship broken" );
						throw std::runtime_error( "Failed to add node to path: parent-child relationship broken" );
					}
				}
			}
			else if ( selectionChanged ) // TODO SC || SN || SD
			{
				SPDLOG_INFO( "Selection changed for node {}", targetNode.code() );

				if ( !codeChanged && !path.empty() && path.back().code() == targetNode.code() )
				{
					continue;
				}

				if ( targetNode.code() == targetEndNode->code() )
				{
					if ( !addToPath( targetGmod, path, targetNode ) )
					{
						SPDLOG_ERROR( "Failed to add node to path: parent-child relationship broken" );
						throw std::runtime_error( "Failed to add node to path: parent-child relationship broken" );
					}
				}
				else if ( !path.empty() && path.back().isChild( targetNode ) )
				{
					if ( !addToPath( targetGmod, path, targetNode ) )
					{
						SPDLOG_ERROR( "Failed to add node to path: parent-child relationship broken" );
						throw std::runtime_error( "Failed to add node to path: parent-child relationship broken" );
					}
				}
			}
			else if ( !codeChanged && !normalAssignmentChanged )
			{
				SPDLOG_INFO( "No changes for node {}, adding to path", targetNode.code() );
				if ( !addToPath( targetGmod, path, targetNode ) )
				{
					SPDLOG_ERROR( "Failed to add node to path: parent-child relationship broken" );
					throw std::runtime_error( "Failed to add node to path: parent-child relationship broken" );
				}
			}

			if ( !path.empty() && path.back().code() == targetEndNode->code() )
			{
				break;
			}
		}

		if ( path.empty() )
		{
			SPDLOG_ERROR( "Failed to build path: no nodes added" );
			throw std::runtime_error( "Failed to build path: no nodes added" );
		}

		potentialParents = std::vector<GmodNode>( path.begin(), path.end() - 1 );
		targetEndNode = path.back();

		int missingLinkAt = -1;
		if ( !GmodPath::isValid( potentialParents, *targetEndNode, missingLinkAt ) )
		{
			SPDLOG_ERROR( "Failed to create a valid path. Missing link at: {}", missingLinkAt );
			throw std::runtime_error( "Failed to create a valid path. Missing link at: " +
									  std::to_string( missingLinkAt ) );
		}

		SPDLOG_INFO( "Successfully created path with {} parents", potentialParents.size() );
		return GmodPath( potentialParents, *targetEndNode );
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
		if ( sourceLocalId.primaryItem().has_value() )
		{
			SPDLOG_INFO( "Converting primary item" );
			auto convertedPath = convertPath(
				*sourceLocalId.visVersion(),
				sourceLocalId.primaryItem().value(),
				targetVersion );

			if ( convertedPath )
			{
				primaryItem = std::move( convertedPath );
			}
			else
			{
				SPDLOG_ERROR( "Failed to convert primary item" );
				return std::nullopt;
			}
		}

		std::optional<GmodPath> secondaryItem;
		if ( sourceLocalId.secondaryItem().has_value() )
		{
			SPDLOG_INFO( "Converting secondary item" );
			auto convertedPath = convertPath(
				*sourceLocalId.visVersion(),
				sourceLocalId.secondaryItem().value(),
				targetVersion );

			if ( convertedPath )
			{
				secondaryItem = std::move( convertedPath );
			}
			else
			{
				SPDLOG_ERROR( "Failed to convert secondary item" );
				return std::nullopt;
			}
		}

		SPDLOG_INFO( "Building converted LocalIdBuilder" );
		return LocalIdBuilder::create( targetVersion )
			.tryWithPrimaryItem( primaryItem )
			.tryWithSecondaryItem( secondaryItem )
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
			conversion.source = dtoNode.source;
			conversion.target = dtoNode.target;
			conversion.oldAssignment = dtoNode.oldAssignment;
			conversion.newAssignment = dtoNode.newAssignment;
			conversion.deleteAssignment = dtoNode.deleteAssignment;

			if ( !dtoNode.operations.empty() )
			{
				for ( const auto& type : dtoNode.operations )
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

	GmodNode GmodVersioning::convertNodeInternal(
		VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const
	{
		SPDLOG_INFO( "Converting node {} internally from {} to {}",
			sourceNode.code(),
			static_cast<int>( sourceVersion ),
			static_cast<int>( targetVersion ) );

		validateSourceAndTargetVersionPair( sourceNode.visVersion(), targetVersion );

		auto& vis = VIS::instance();
		const auto& targetGmod = vis.gmod( targetVersion );

		std::string nextCode = sourceNode.code();

		auto it = m_versioningsMap.find( targetVersion );
		if ( it != m_versioningsMap.end() )
		{
			GmodNodeConversion change;
			if ( it->second.tryGetCodeChanges( sourceNode.code(), change ) && change.target.has_value() )
			{
				SPDLOG_INFO( "Code change found: {} -> {}", sourceNode.code(), *change.target );
				nextCode = *change.target;
			}
		}

		GmodNode targetNode;
		if ( !targetGmod.tryGetNode( nextCode, targetNode ) )
		{
			SPDLOG_ERROR( "Failed to find target node with code {}", nextCode );
			return GmodNode();
		}

		GmodNode result = targetNode;

		if ( sourceNode.location().has_value() )
		{
			SPDLOG_INFO( "Attempting to carry over location information" );
			if ( result.isIndividualizable( false, true ) )
			{
				result = result.withLocation( *sourceNode.location() );

				if ( result.location() != sourceNode.location() )
				{
					SPDLOG_ERROR( "Failed to set location for node {}", sourceNode.code() );
					throw std::runtime_error( "Failed to set location" );
				}
			}
			else
			{
				SPDLOG_ERROR( "Failed to set location for node {} (not individualizable)", sourceNode.code() );
				throw std::runtime_error( "Failed to set location" );
			}
		}

		return result;
	}

	bool GmodVersioning::tryGetVersioningNode(
		VisVersion visVersion, GmodVersioningNode& versioningNode ) const
	{
		SPDLOG_INFO( "Looking for versioning node for version {}", static_cast<int>( visVersion ) );

		auto it = m_versioningsMap.find( visVersion );
		if ( it != m_versioningsMap.end() )
		{
			versioningNode = it->second;
			return true;
		}
		return false;
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
