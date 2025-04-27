/**
 * @file GmodDto.cpp
 * @brief Implementation of Generic Product Model (GMOD) data transfer objects
 */

#include "pch.h"
#include "dnv/vista/sdk/GmodDto.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------

	static constexpr const char* CATEGORY_KEY = "category";
	static constexpr const char* TYPE_KEY = "type";
	static constexpr const char* CODE_KEY = "code";
	static constexpr const char* NAME_KEY = "name";
	static constexpr const char* COMMON_NAME_KEY = "commonName";
	static constexpr const char* DEFINITION_KEY = "definition";
	static constexpr const char* COMMON_DEFINITION_KEY = "commonDefinition";
	static constexpr const char* INSTALL_SUBSTRUCTURE_KEY = "installSubstructure";
	static constexpr const char* NORMAL_ASSIGNMENT_NAMES_KEY = "normalAssignmentNames";

	static constexpr const char* VIS_RELEASE_KEY = "visRelease";
	static constexpr const char* ITEMS_KEY = "items";
	static constexpr const char* RELATIONS_KEY = "relations";

	namespace
	{
		const std::string& internString( const std::string& value )
		{
			static std::unordered_map<std::string, std::string> cache;
			if ( value.length() > 30 )
			{
				return value;
			}

			auto it = cache.find( value );
			if ( it != cache.end() )
			{
				return it->second;
			}

			return cache.emplace( value, value ).first->first;
		}
	}

	//-------------------------------------------------------------------
	// GmodNodeDto Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	GmodNodeDto::GmodNodeDto(
		std::string category,
		std::string type,
		std::string code,
		std::string name,
		std::optional<std::string> commonName,
		std::optional<std::string> definition,
		std::optional<std::string> commonDefinition,
		std::optional<bool> installSubstructure,
		std::optional<std::unordered_map<std::string, std::string>> normalAssignmentNames )
		: m_category{ std::move( category ) },
		  m_type{ std::move( type ) },
		  m_code{ std::move( code ) },
		  m_name{ std::move( name ) },
		  m_commonName{ std::move( commonName ) },
		  m_definition{ std::move( definition ) },
		  m_commonDefinition{ std::move( commonDefinition ) },
		  m_installSubstructure{ installSubstructure },
		  m_normalAssignmentNames{ std::move( normalAssignmentNames ) }
	{
		SPDLOG_DEBUG( "Creating GmodNodeDto: code={}, category={}, type={}", m_code, m_category, m_type );
	}

	//-------------------------------------------------------------------
	// Accessor Methods
	//-------------------------------------------------------------------

	const std::string& GmodNodeDto::category() const
	{
		return m_category;
	}

	const std::string& GmodNodeDto::type() const
	{
		return m_type;
	}

	const std::string& GmodNodeDto::code() const
	{
		return m_code;
	}

	const std::string& GmodNodeDto::name() const
	{
		return m_name;
	}

	std::optional<std::string> GmodNodeDto::commonName() const
	{
		return m_commonName;
	}

	std::optional<std::string> GmodNodeDto::definition() const
	{
		return m_definition;
	}

	std::optional<std::string> GmodNodeDto::commonDefinition() const
	{
		return m_commonDefinition;
	}

	std::optional<bool> GmodNodeDto::installSubstructure() const
	{
		return m_installSubstructure;
	}

	std::optional<std::unordered_map<std::string, std::string>> GmodNodeDto::normalAssignmentNames() const
	{
		return m_normalAssignmentNames;
	}

	//-------------------------------------------------------------------
	// Serialization Methods
	//-------------------------------------------------------------------

	GmodNodeDto GmodNodeDto::fromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();

		SPDLOG_DEBUG( "Parsing GmodNodeDto from JSON" );
		GmodNodeDto node;

		node.m_category = "UNKNOWN";
		node.m_type = "UNKNOWN";
		node.m_code = "UNKNOWN";
		node.m_name = "UNKNOWN";

		if ( json.HasMember( CATEGORY_KEY ) && json[CATEGORY_KEY].IsString() )
		{
			std::string categoryStr = json[CATEGORY_KEY].GetString();
			if ( categoryStr.empty() )
			{
				SPDLOG_WARN( "Empty category field found in GMOD node" );
			}
			else
			{
				node.m_category = internString( categoryStr );
			}
		}

		if ( json.HasMember( TYPE_KEY ) && json[TYPE_KEY].IsString() )
		{
			std::string typeStr = json[TYPE_KEY].GetString();
			if ( typeStr.empty() )
			{
				SPDLOG_WARN( "Empty type field found in GMOD node" );
			}
			else
			{
				node.m_type = internString( typeStr );
			}
		}

		if ( json.HasMember( CODE_KEY ) && json[CODE_KEY].IsString() )
		{
			std::string codeStr = json[CODE_KEY].GetString();
			if ( codeStr.empty() )
			{
				SPDLOG_WARN( "Empty code field found in GMOD node" );
			}
			else
			{
				node.m_code = std::move( codeStr );
			}
		}

		if ( json.HasMember( NAME_KEY ) && json[NAME_KEY].IsString() )
		{
			std::string nameStr = json[NAME_KEY].GetString();
			if ( nameStr.empty() )
			{
				SPDLOG_WARN( "Empty name field found in GMOD node" );
			}
			else
			{
				node.m_name = std::move( nameStr );
			}
		}

		SPDLOG_DEBUG( "Parsed required GMOD node fields: category={}, type={}, code={}, name={}",
			node.m_category, node.m_type, node.m_code, node.m_name );

		if ( json.HasMember( COMMON_NAME_KEY ) && json[COMMON_NAME_KEY].IsString() )
			node.m_commonName = json[COMMON_NAME_KEY].GetString();

		if ( json.HasMember( DEFINITION_KEY ) && json[DEFINITION_KEY].IsString() )
			node.m_definition = json[DEFINITION_KEY].GetString();

		if ( json.HasMember( COMMON_DEFINITION_KEY ) && json[COMMON_DEFINITION_KEY].IsString() )
			node.m_commonDefinition = json[COMMON_DEFINITION_KEY].GetString();

		if ( json.HasMember( INSTALL_SUBSTRUCTURE_KEY ) && json[INSTALL_SUBSTRUCTURE_KEY].IsBool() )
			node.m_installSubstructure = json[INSTALL_SUBSTRUCTURE_KEY].GetBool();

		if ( json.HasMember( NORMAL_ASSIGNMENT_NAMES_KEY ) && json[NORMAL_ASSIGNMENT_NAMES_KEY].IsObject() )
		{
			std::unordered_map<std::string, std::string> assignmentNames;
			auto& namesObj = json[NORMAL_ASSIGNMENT_NAMES_KEY];
			assignmentNames.reserve( namesObj.MemberCount() );

			for ( auto it = namesObj.MemberBegin(); it != namesObj.MemberEnd(); ++it )
			{
				if ( it->name.IsString() && it->value.IsString() )
					assignmentNames[it->name.GetString()] = it->value.GetString();
			}

			if ( !assignmentNames.empty() )
			{
				node.m_normalAssignmentNames = std::move( assignmentNames );
				SPDLOG_DEBUG( "Parsed {} normal assignment name mappings", assignmentNames.size() );
			}
		}

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Successfully parsed GmodNodeDto: code={} in {} µs", node.m_code, duration.count() );

		return node;
	}

	bool GmodNodeDto::tryFromJson( const rapidjson::Value& json, GmodNodeDto& dto )
	{
		try
		{
			dto = fromJson( json );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing GmodNodeDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value GmodNodeDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		SPDLOG_DEBUG( "Serializing GmodNodeDto: code={}", m_code );
		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( rapidjson::StringRef( CATEGORY_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_category.c_str() ), allocator ),
			allocator );
		obj.AddMember( rapidjson::StringRef( TYPE_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_type.c_str() ), allocator ),
			allocator );
		obj.AddMember( rapidjson::StringRef( CODE_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_code.c_str() ), allocator ),
			allocator );
		obj.AddMember( rapidjson::StringRef( NAME_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_name.c_str() ), allocator ),
			allocator );

		if ( m_commonName.has_value() )
			obj.AddMember( rapidjson::StringRef( COMMON_NAME_KEY ),
				rapidjson::Value( rapidjson::StringRef( m_commonName.value().c_str() ), allocator ),
				allocator );

		if ( m_definition.has_value() )
			obj.AddMember( rapidjson::StringRef( DEFINITION_KEY ),
				rapidjson::Value( rapidjson::StringRef( m_definition.value().c_str() ), allocator ),
				allocator );

		if ( m_commonDefinition.has_value() )
			obj.AddMember( rapidjson::StringRef( COMMON_DEFINITION_KEY ),
				rapidjson::Value( rapidjson::StringRef( m_commonDefinition.value().c_str() ), allocator ),
				allocator );

		if ( m_installSubstructure.has_value() )
			obj.AddMember( rapidjson::StringRef( INSTALL_SUBSTRUCTURE_KEY ),
				m_installSubstructure.value(), allocator );

		if ( m_normalAssignmentNames.has_value() && !m_normalAssignmentNames.value().empty() )
		{
			rapidjson::Value assignmentObj( rapidjson::kObjectType );
			for ( const auto& [key, value] : m_normalAssignmentNames.value() )
			{
				assignmentObj.AddMember(
					rapidjson::Value( rapidjson::StringRef( key.c_str() ), allocator ),
					rapidjson::Value( rapidjson::StringRef( value.c_str() ), allocator ),
					allocator );
			}
			obj.AddMember( rapidjson::StringRef( NORMAL_ASSIGNMENT_NAMES_KEY ), assignmentObj, allocator );
		}

		return obj;
	}

	//-------------------------------------------------------------------
	// GmodDto Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	GmodDto::GmodDto( std::string visVersion, std::vector<GmodNodeDto> items, std::vector<std::vector<std::string>> relations )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) },
		  m_relations{ std::move( relations ) }
	{
	}

	//-------------------------------------------------------------------
	// Accessor Methods
	//-------------------------------------------------------------------

	const std::string& GmodDto::visVersion() const
	{
		return m_visVersion;
	}

	const std::vector<GmodNodeDto>& GmodDto::items() const
	{
		return m_items;
	}

	const std::vector<std::vector<std::string>>& GmodDto::relations() const
	{
		return m_relations;
	}

	//-------------------------------------------------------------------
	// Serialization Methods
	//-------------------------------------------------------------------

	GmodDto GmodDto::fromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();

		SPDLOG_INFO( "Parsing GMOD from JSON" );
		GmodDto dto;

		dto.m_visVersion = "unknown";

		if ( json.HasMember( VIS_RELEASE_KEY ) && json[VIS_RELEASE_KEY].IsString() )
		{
			dto.m_visVersion = json[VIS_RELEASE_KEY].GetString();
			SPDLOG_INFO( "GMOD VIS version: {}", dto.m_visVersion );
		}

		if ( json.HasMember( ITEMS_KEY ) && json[ITEMS_KEY].IsArray() )
		{
			size_t totalItems = json[ITEMS_KEY].Size();
			SPDLOG_INFO( "Found {} GMOD node items to parse", totalItems );
			dto.m_items.reserve( totalItems );

			size_t successCount = 0;
			for ( rapidjson::SizeType i = 0; i < json[ITEMS_KEY].Size(); ++i )
			{
				try
				{
					dto.m_items.emplace_back( GmodNodeDto::fromJson( json[ITEMS_KEY][i] ) );
					successCount++;
				}
				catch ( const std::exception& e )
				{
					SPDLOG_ERROR( "Skipping malformed GMOD node at index {}: {}", i, e.what() );
				}
			}

			SPDLOG_INFO( "Successfully parsed {}/{} GMOD nodes", successCount, totalItems );

			if ( static_cast<double>( successCount ) < static_cast<double>( totalItems ) * 0.9 )
			{
				dto.m_items.shrink_to_fit();
			}
		}
		else
		{
			SPDLOG_WARN( "GMOD missing 'items' array or not in array format" );
		}

		if ( json.HasMember( RELATIONS_KEY ) && json[RELATIONS_KEY].IsArray() )
		{
			size_t relationCount = json[RELATIONS_KEY].Size();
			SPDLOG_INFO( "Found {} GMOD relation entries to parse", relationCount );
			dto.m_relations.reserve( relationCount );

			size_t validRelationCount = 0;
			for ( const auto& relation : json[RELATIONS_KEY].GetArray() )
			{
				if ( relation.IsArray() )
				{
					std::vector<std::string> relationPair;
					relationPair.reserve( relation.Size() );

					for ( const auto& rel : relation.GetArray() )
					{
						if ( rel.IsString() )
							relationPair.emplace_back( rel.GetString() );
					}

					if ( !relationPair.empty() )
					{
						dto.m_relations.emplace_back( std::move( relationPair ) );
						validRelationCount++;
					}
				}
			}

			SPDLOG_DEBUG( "Added {} valid relations to GMOD", validRelationCount );

			if ( static_cast<double>( validRelationCount ) < static_cast<double>( relationCount ) * 0.9 )
			{
				dto.m_relations.shrink_to_fit();
			}
		}
		else
		{
			SPDLOG_INFO( "GMOD has no relations or 'relations' is not an array" );
		}

		if ( dto.m_items.size() > 10000 )
		{
			const size_t approxMemoryUsage =
				( dto.m_items.size() * sizeof( GmodNodeDto ) +
					dto.m_relations.size() * 24 ) /
				( 1024 * 1024 );
			SPDLOG_INFO( "Large GMOD model loaded: ~{} MB estimated memory usage",
				approxMemoryUsage );
		}

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Parsed GmodDto with {} nodes, {} relations and VIS version {} in {} ms", dto.m_items.size(), dto.m_relations.size(), dto.m_visVersion, duration.count() );

		return dto;
	}

	bool GmodDto::tryFromJson( const rapidjson::Value& json, GmodDto& dto )
	{
		try
		{
			dto = fromJson( json );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing GmodDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value GmodDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		auto startTime = std::chrono::steady_clock::now();

		SPDLOG_INFO( "Serializing GmodDto with {} nodes and {} relations", m_items.size(), m_relations.size() );
		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( rapidjson::StringRef( VIS_RELEASE_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_visVersion.c_str() ), allocator ),
			allocator );

		rapidjson::Value itemsArray( rapidjson::kArrayType );
		const auto itemsSize = m_items.size();
		if ( itemsSize > 0 )
		{
			if ( itemsSize > std::numeric_limits<rapidjson::SizeType>::max() )
			{
				SPDLOG_WARN( "Items array size {} exceeds maximum RapidJSON capacity", itemsSize );
				itemsArray.Reserve( std::numeric_limits<rapidjson::SizeType>::max(), allocator );
			}
			else
			{
				itemsArray.Reserve( static_cast<rapidjson::SizeType>( itemsSize ), allocator );
			}

			for ( const auto& item : m_items )
			{
				itemsArray.PushBack( item.toJson( allocator ), allocator );
			}
		}
		obj.AddMember( rapidjson::StringRef( ITEMS_KEY ), itemsArray, allocator );

		rapidjson::Value relationsArray( rapidjson::kArrayType );
		const auto relSize = m_relations.size();
		if ( relSize > 0 )
		{
			if ( relSize > std::numeric_limits<rapidjson::SizeType>::max() )
			{
				SPDLOG_WARN( "Relations array size {} exceeds maximum RapidJSON capacity", relSize );
				relationsArray.Reserve( std::numeric_limits<rapidjson::SizeType>::max(), allocator );
			}
			else
			{
				relationsArray.Reserve( static_cast<rapidjson::SizeType>( relSize ), allocator );
			}

			for ( const auto& relation : m_relations )
			{
				rapidjson::Value relationArray( rapidjson::kArrayType );
				const auto relationSize = relation.size();
				if ( relationSize > 0 )
				{
					relationArray.Reserve( static_cast<rapidjson::SizeType>( relationSize ), allocator );

					for ( const auto& rel : relation )
					{
						relationArray.PushBack(
							rapidjson::Value( rapidjson::StringRef( rel.c_str() ), allocator ),
							allocator );
					}
				}
				relationsArray.PushBack( relationArray, allocator );
			}
		}
		obj.AddMember( rapidjson::StringRef( RELATIONS_KEY ), relationsArray, allocator );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Serialized GmodDto with {} items for VIS version {} in {} ms", m_items.size(), m_visVersion, duration.count() );

		return obj;
	}
}
