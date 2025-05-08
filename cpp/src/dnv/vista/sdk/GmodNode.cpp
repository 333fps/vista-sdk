/**
 * @file GmodNode.cpp
 * @brief Implementation of the GmodNode and GmodNodeMetadata classes
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodNode.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr const char* NODE_CATEGORY_PRODUCT = "PRODUCT";
		static constexpr const char* NODE_CATEGORY_ASSET = "ASSET";
		static constexpr const char* NODE_CATEGORY_PRODUCT_FUNCTION = "PRODUCT FUNCTION";
		static constexpr const char* NODE_CATEGORY_ASSET_FUNCTION = "ASSET FUNCTION";

		static constexpr const char* NODE_TYPE_SELECTION = "SELECTION";
		static constexpr const char* NODE_TYPE_GROUP = "GROUP";
		static constexpr const char* NODE_TYPE_COMPOSITION = "COMPOSITION";

		static constexpr const char* NODE_TYPE_VALUE_TYPE = "TYPE";
		static constexpr const char* NODE_CATEGORY_VALUE_FUNCTION = "FUNCTION";
	}

	//=====================================================================
	// GmodNodeMetadata Class
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	GmodNodeMetadata::GmodNodeMetadata(
		const std::string& category,
		const std::string& type,
		const std::string& name,
		const std::optional<std::string>& commonName,
		const std::optional<std::string>& definition,
		const std::optional<std::string>& commonDefinition,
		const std::optional<bool>& installSubstructure,
		const std::unordered_map<std::string, std::string>& normalAssignmentNames )
		: m_category{ category },
		  m_type{ type },
		  m_name{ name },
		  m_commonName{ commonName },
		  m_definition{ definition },
		  m_commonDefinition{ commonDefinition },
		  m_installSubstructure{ installSubstructure },
		  m_normalAssignmentNames{ normalAssignmentNames },
		  m_fullType{ category + " " + type }
	{
		SPDLOG_TRACE( "Created GmodNodeMetadata: {}", m_fullType );
	}

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	bool GmodNodeMetadata::operator==( const GmodNodeMetadata& other ) const
	{
		return m_category == other.m_category &&
			   m_type == other.m_type &&
			   m_name == other.m_name &&
			   m_commonName == other.m_commonName &&
			   m_definition == other.m_definition &&
			   m_commonDefinition == other.m_commonDefinition &&
			   m_installSubstructure == other.m_installSubstructure &&
			   m_normalAssignmentNames == other.m_normalAssignmentNames;
	}

	bool GmodNodeMetadata::operator!=( const GmodNodeMetadata& other ) const
	{
		return !( *this == other );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& GmodNodeMetadata::category() const
	{
		return m_category;
	}

	const std::string& GmodNodeMetadata::type() const
	{
		return m_type;
	}

	const std::string& GmodNodeMetadata::fullType() const
	{
		return m_fullType;
	}

	const std::string& GmodNodeMetadata::name() const
	{
		return m_name;
	}

	const std::optional<std::string>& GmodNodeMetadata::commonName() const
	{
		return m_commonName;
	}

	const std::optional<std::string>& GmodNodeMetadata::definition() const
	{
		return m_definition;
	}

	const std::optional<std::string>& GmodNodeMetadata::commonDefinition() const
	{
		return m_commonDefinition;
	}

	const std::optional<bool>& GmodNodeMetadata::installSubstructure() const
	{
		return m_installSubstructure;
	}

	const std::unordered_map<std::string, std::string>& GmodNodeMetadata::normalAssignmentNames() const
	{
		return m_normalAssignmentNames;
	}

	//=====================================================================
	// GmodNode Class
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	GmodNode::GmodNode()
		: m_code{ "" },
		  m_location{ std::nullopt },
		  m_visVersion{ VisVersion::v3_8a },
		  m_metadata{ "", "", "", std::nullopt, std::nullopt, std::nullopt, std::nullopt, {} },
		  m_children{},
		  m_parents{},
		  m_childrenSet{}
	{
		SPDLOG_TRACE( "Created empty GmodNode" );
	}

	GmodNode::GmodNode( const GmodNode& other, [[maybe_unused]] bool b )
		: m_code{ other.m_code },
		  m_location{ other.m_location },
		  m_visVersion{ other.m_visVersion },
		  m_metadata{ other.m_metadata },
		  m_children{},
		  m_parents{},
		  m_childrenSet{}
	{
		SPDLOG_TRACE( "Copied GmodNode {} for location modification", m_code );
	}

	GmodNode::GmodNode( VisVersion version, const GmodNodeDto& dto )
		: m_code{ dto.code() },
		  m_location{ std::nullopt },
		  m_visVersion{ version },
		  m_metadata{
			  dto.category(),
			  dto.type(),
			  dto.name(),
			  dto.commonName(),
			  dto.definition(),
			  dto.commonDefinition(),
			  dto.installSubstructure(),
			  dto.normalAssignmentNames().has_value() ? *dto.normalAssignmentNames() : std::unordered_map<std::string, std::string>() },
		  m_children{},
		  m_parents{},
		  m_childrenSet{}
	{
		SPDLOG_TRACE( "Created GmodNode with code: {}", m_code );
	}

	//----------------------------------------------
	// Assignment Operators
	//----------------------------------------------

	GmodNode& GmodNode::operator=( const GmodNode& other )
	{
		if ( this == &other )
		{
			return *this;
		}

		m_code = other.m_code;
		m_location = other.m_location;
		m_visVersion = other.m_visVersion;

		if ( m_metadata != other.m_metadata )
		{
			m_metadata = GmodNodeMetadata( other.m_metadata );
		}

		m_children.clear();
		m_parents.clear();
		m_childrenSet.clear();

		return *this;
	}

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	bool GmodNode::operator==( const GmodNode& other ) const
	{
		if ( m_code != other.m_code )
		{
			return false;
		}

		if ( m_location.has_value() )
		{
			if ( !other.m_location.has_value() )
			{
				return false;
			}
			if ( m_location.value() != other.m_location.value() )
			{
				return false;
			}
		}
		else
		{
			if ( other.m_location.has_value() )
			{
				return false;
			}
		}

		return true;
	}

	bool GmodNode::equals( const GmodNode& other ) const
	{
		return ( *this == other );
	}

	bool GmodNode::operator!=( const GmodNode& other ) const
	{
		return !( *this == other );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& GmodNode::code() const
	{
		return m_code;
	}

	const std::optional<Location>& GmodNode::location() const
	{
		return m_location;
	}

	VisVersion GmodNode::visVersion() const
	{
		return m_visVersion;
	}

	const GmodNodeMetadata& GmodNode::metadata() const
	{
		return m_metadata;
	}

	size_t GmodNode::hashCode() const noexcept
	{
		size_t hash = std::hash<std::string>{}( m_code );

		if ( m_location.has_value() )
		{
			hash ^= ( std::hash<std::string>{}( m_location->toString() ) + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 ) );
		}

		return hash;
	}

	//----------------------------------------------
	// Relationship Accessors
	//----------------------------------------------

	const std::vector<GmodNode*>& GmodNode::children() const
	{
		return m_children;
	}

	const std::vector<GmodNode*>& GmodNode::parents() const
	{
		return m_parents;
	}

	const GmodNode* GmodNode::productType() const
	{
		if ( m_children.size() != 1 )
		{
			SPDLOG_WARN( "Product type check failed: expected 1 child, found {}", m_children.size() );
			return nullptr;
		}

		if ( m_metadata.category().find( NODE_CATEGORY_VALUE_FUNCTION ) == std::string::npos )
		{
			SPDLOG_WARN( "Product type check failed: expected FUNCTION category, found {}",
				m_metadata.category() );
			return nullptr;
		}

		const GmodNode* child = m_children[0];
		if ( !child )
		{
			SPDLOG_WARN( "Product type check failed: child is null" );
			return nullptr;
		}

		if ( child->m_metadata.category() != NODE_CATEGORY_PRODUCT )
		{
			SPDLOG_WARN( "Product type check failed: expected PRODUCT category, found {}",
				child->m_metadata.category() );
			return nullptr;
		}

		if ( child->m_metadata.type() != NODE_TYPE_VALUE_TYPE )
		{
			SPDLOG_WARN( "Product type check failed: expected TYPE type, found {}",
				child->m_metadata.type() );
			return nullptr;
		}

		SPDLOG_DEBUG( "Product type check succeeded: {}", child->m_code );
		return child;
	}

	const GmodNode* GmodNode::productSelection() const
	{
		if ( m_children.size() != 1 )
		{
			SPDLOG_WARN( "Product selection check failed: expected 1 child, found {}",
				m_children.size() );

			return nullptr;
		}

		if ( m_metadata.category().find( NODE_CATEGORY_VALUE_FUNCTION ) == std::string::npos )
		{
			SPDLOG_WARN( "Product selection check failed: expected FUNCTION category, found {}",
				m_metadata.category() );

			return nullptr;
		}

		const GmodNode* child = m_children[0];
		if ( !child )
		{
			SPDLOG_WARN( "Product selection check failed: child is null" );

			return nullptr;
		}

		if ( child )
		{
			SPDLOG_INFO( "Child node code: {}", child->code() );

			const auto& childCategory = child->metadata().category();
			SPDLOG_INFO( "Child category: '{}'", childCategory );
			if ( childCategory.empty() )
			{
				SPDLOG_ERROR( "Child node '{}' has an empty category!", child->code() );
				return nullptr;
			}
		}

		if ( child->m_metadata.category().find( NODE_CATEGORY_PRODUCT ) == std::string::npos )
		{
			SPDLOG_CRITICAL( "22" );

			SPDLOG_WARN( "Product selection check failed: expected PRODUCT category, found {}",
				child->m_metadata.category() );

			return nullptr;
		}
		if ( child->m_metadata.type() != NODE_TYPE_SELECTION )
		{
			SPDLOG_WARN( "Product selection check failed: expected SELECTION type, found {}",
				child->m_metadata.type() );

			return nullptr;
		}

		SPDLOG_DEBUG( "Product selection check succeeded: {}", child->m_code );

		return child;
	}

	//----------------------------------------------
	// Node Location Methods
	//----------------------------------------------

	GmodNode GmodNode::withoutLocation() const
	{
		GmodNode result( *this, true );
		if ( !result.m_location.has_value() )
		{
			SPDLOG_DEBUG( "No location to remove from node: {}", m_code );
		}
		else
		{
			SPDLOG_INFO( "Removing location from node: {}", m_code );
			result.m_location = std::nullopt;
		}
		return result;
	}

	GmodNode GmodNode::withLocation( const std::string& locationStr ) const
	{
		SPDLOG_DEBUG( "Adding location '{}' to node: {}", locationStr, m_code );
		Locations locations = VIS::instance().locations( m_visVersion );
		Location location = locations.parse( locationStr );

		GmodNode result( *this, true );
		result.m_location = location;

		return result;
	}

	GmodNode GmodNode::withLocation( const Location& location ) const
	{
		SPDLOG_DEBUG( "Adding location object to node: {}", m_code );

		GmodNode result( *this, true );
		result.m_location = location;
		return result;
	}

	GmodNode GmodNode::tryWithLocation( const std::string& locationStr ) const
	{
		SPDLOG_DEBUG( "Attempting to add location '{}' to node: {}", locationStr, m_code );
		Locations locations = VIS::instance().locations( m_visVersion );
		Location location;

		if ( !locations.tryParse( std::string_view( locationStr ), location ) )
		{
			SPDLOG_ERROR( "Location parsing failed for: {}", locationStr );
			return GmodNode( *this, true );
		}

		return withLocation( location );
	}

	GmodNode GmodNode::tryWithLocation( const std::string& locationStr, ParsingErrors& errors ) const
	{
		SPDLOG_DEBUG( "Attempting to add location '{}' to node: {} with error capture",
			locationStr, m_code );

		Locations locations = VIS::instance().locations( m_visVersion );
		Location location;

		GmodNode result( *this, true );
		if ( !locations.tryParse( std::string_view( locationStr ), location, errors ) )
		{
			SPDLOG_ERROR( "Location parsing failed with {} errors",
				std::distance( errors.begin(), errors.end() ) );
		}
		else
		{
			result.m_location = location;
		}
		return result;
	}

	GmodNode GmodNode::tryWithLocation( const std::optional<Location>& location ) const
	{
		GmodNode result( *this, true );
		if ( !location.has_value() )
		{
			SPDLOG_DEBUG( "No location provided, returning original node copy: {}", m_code );
		}
		else
		{
			result.m_location = *location;
		}
		return result;
	}

	//----------------------------------------------
	// Node Type Checking Methods
	//----------------------------------------------

	bool GmodNode::isIndividualizable( bool isTargetNode, bool isInSet ) const
	{
		if ( m_metadata.type() == NODE_TYPE_GROUP )
		{
			SPDLOG_DEBUG( "Node is a group, not individualizable: {}", m_code );

			return false;
		}
		if ( m_metadata.type() == NODE_TYPE_SELECTION )
		{
			SPDLOG_DEBUG( "Node is a selection, not individualizable: {}", m_code );

			return false;
		}
		if ( isProductType() )
		{
			SPDLOG_DEBUG( "Node is a product type, not individualizable: {}", m_code );

			return false;
		}
		if ( m_metadata.category() == NODE_CATEGORY_ASSET && m_metadata.type() == NODE_TYPE_VALUE_TYPE )
		{
			SPDLOG_DEBUG( "Node is an asset type, not individualizable: {}", m_code );

			return false;
		}
		if ( isFunctionComposition() )
		{
			SPDLOG_DEBUG( "Node is a function composition, checking special conditions" );
			if ( m_code.empty() )
			{
				SPDLOG_WARN( "isIndividualizable: Code is empty, cannot check last character for 'i'. Node: {}", m_code );

				return false;
			}

			return m_code.back() == 'i' || isInSet || isTargetNode;
		}

		SPDLOG_DEBUG( "Node is individualizable: {}", m_code );

		return true;
	}

	bool GmodNode::isFunctionComposition() const
	{
		return ( m_metadata.category() == NODE_CATEGORY_ASSET_FUNCTION ||
				   m_metadata.category() == NODE_CATEGORY_PRODUCT_FUNCTION ) &&
			   m_metadata.type() == NODE_TYPE_COMPOSITION;
	}

	bool GmodNode::isMappable() const noexcept
	{
		if ( productType() != nullptr )
		{
			SPDLOG_DEBUG( "Node is a product type, not mappable: {}", m_code );

			return false;
		}
		if ( productSelection() != nullptr )
		{
			SPDLOG_DEBUG( "Node is a product selection, not mappable: {}", m_code );

			return false;
		}
		if ( isProductSelection() )
		{
			SPDLOG_DEBUG( "Node is a product selection, not mappable: {}", m_code );

			return false;
		}
		if ( isAsset() )
		{
			SPDLOG_DEBUG( "Node is an asset, not mappable: {}", m_code );

			return false;
		}

		if ( m_code.empty() )
		{
			SPDLOG_WARN( "isMappable: Code is empty, cannot check last character. Node: {}", m_code );

			return false;
		}

		char lastChar = m_code.back();
		SPDLOG_DEBUG( "Last character of code: {}", lastChar );

		bool result = ( lastChar != 'a' && lastChar != 's' );
		if ( result )
		{
			SPDLOG_DEBUG( "Node is mappable based on last char: {}", m_code );
		}
		else
		{
			SPDLOG_DEBUG( "Node is NOT mappable based on last char: {}", m_code );
		}

		return result;
	}

	bool GmodNode::isProductSelection() const
	{
		return Gmod::isProductSelection( m_metadata );
	}

	bool GmodNode::isProductType() const
	{
		return Gmod::isProductType( m_metadata );
	}

	bool GmodNode::isAsset() const
	{
		return Gmod::isAsset( m_metadata );
	}

	bool GmodNode::isLeafNode() const
	{
		return Gmod::isLeafNode( m_metadata );
	}

	bool GmodNode::isFunctionNode() const
	{
		return Gmod::isFunctionNode( m_metadata );
	}

	bool GmodNode::isAssetFunctionNode() const
	{
		return Gmod::isAssetFunctionNode( m_metadata );
	}

	bool GmodNode::isRoot() const noexcept
	{
		return m_code == "VE";
	}

	//----------------------------------------------
	// Node Relationship Query Methods
	//----------------------------------------------

	bool GmodNode::isChild( const GmodNode& node ) const
	{
		return isChild( node.m_code );
	}

	bool GmodNode::isChild( const std::string& code ) const
	{
		bool found = m_childrenSet.find( code ) != m_childrenSet.end();
		SPDLOG_DEBUG( "Node {} isChild check for {}: {}", m_code, code, found );

		return found;
	}

	//----------------------------------------------
	// Utility Methods
	//----------------------------------------------

	std::string GmodNode::toString() const
	{
		if ( m_code.empty() )
		{
			SPDLOG_DEBUG( "GmodNode code is empty, returning empty string" );

			return "";
		}

		if ( m_location.has_value() )
		{
			SPDLOG_DEBUG( "Converting GmodNode to string with location" );
			std::string result = m_code;
			result.reserve( m_code.length() + 1 + m_location->toString().length() );
			result += '-';
			result += m_location->toString();

			return result;
		}

		return m_code;
	}

	void GmodNode::toString( std::stringstream& builder ) const
	{
		builder << m_code;

		if ( m_location.has_value() )
		{
			builder << "-" << m_location->toString();
		}

		SPDLOG_DEBUG( "GmodNode string representation: {}", builder.str() );
	}

	//----------------------------------------------
	// Relationship Management Methods
	//----------------------------------------------

	void GmodNode::addChild( GmodNode* child )
	{
		if ( !child )
		{
			SPDLOG_WARN( "Attempt to add null child to node: {}", m_code );
			return;
		}

		if ( m_childrenSet.find( child->code() ) != m_childrenSet.end() )
		{
			SPDLOG_DEBUG( "Child {} already exists for parent {}, skipping", child->code(), m_code );
			return;
		}

		SPDLOG_DEBUG( "Adding child {} to parent {}", child->code(), m_code );
		m_children.push_back( child );
		m_childrenSet.insert( child->code() );
	}

	void GmodNode::addParent( GmodNode* parent )
	{
		if ( !parent )
		{
			SPDLOG_WARN( "Attempt to add null parent to node: {}", m_code );
			return;
		}

		SPDLOG_DEBUG( "Adding parent {} to child {}", parent->code(), m_code );
		m_parents.push_back( parent );
	}

	void GmodNode::trim()
	{
		auto start = std::chrono::high_resolution_clock::now();

		m_children.shrink_to_fit();
		m_parents.shrink_to_fit();

		m_childrenSet.clear();
		m_childrenSet.reserve( m_children.size() );

		for ( const auto* child : m_children )
		{
			m_childrenSet.insert( child->code() );
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( end - start );

		SPDLOG_DEBUG( "GmodNode::trim completed in {} μs for {} children",
			duration.count(), m_children.size() );
	}
}
