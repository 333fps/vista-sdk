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

	//-------------------------------------------------------------------
	// Helper Functions
	//-------------------------------------------------------------------

	namespace
	{
		/**
		 * @brief Interns short strings to reduce memory usage for commonly repeated values
		 * @param value The string to be interned
		 * @return A reference to the cached string instance
		 */
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
	// Constructors / Destructor
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
		std::optional<NormalAssignmentNamesMap> normalAssignmentNames )
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
	// Public Interface - Accessor Methods
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

	const std::optional<std::string>& GmodNodeDto::commonName() const
	{
		return m_commonName;
	}

	const std::optional<std::string>& GmodNodeDto::definition() const
	{
		return m_definition;
	}

	const std::optional<std::string>& GmodNodeDto::commonDefinition() const
	{
		return m_commonDefinition;
	}

	const std::optional<bool>& GmodNodeDto::installSubstructure() const
	{
		return m_installSubstructure;
	}

	const std::optional<GmodNodeDto::NormalAssignmentNamesMap>& GmodNodeDto::normalAssignmentNames() const
	{
		return m_normalAssignmentNames;
	}

	//-------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------

	std::optional<GmodNodeDto> GmodNodeDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse GmodNodeDto from JSON" );

		std::string code = "[code unavailable]";
		if ( json.HasMember( CODE_KEY ) && json[CODE_KEY].IsString() )
		{
			code = json[CODE_KEY].GetString();
			if ( code.empty() )
			{
				SPDLOG_WARN( "Empty code field found in GMOD node" );
				code = "[empty code]";
			}
		}
		else
		{
			SPDLOG_ERROR( "GMOD Node JSON missing required '{}' field or not a string", CODE_KEY );
			return std::nullopt;
		}

		if ( !json.HasMember( CATEGORY_KEY ) || !json[CATEGORY_KEY].IsString() )
		{
			SPDLOG_ERROR( "GMOD Node JSON (code='{}') missing required '{}' field or not a string", code, CATEGORY_KEY );
			return std::nullopt;
		}
		if ( !json.HasMember( TYPE_KEY ) || !json[TYPE_KEY].IsString() )
		{
			SPDLOG_ERROR( "GMOD Node JSON (code='{}') missing required '{}' field or not a string", code, TYPE_KEY );
			return std::nullopt;
		}
		if ( !json.HasMember( NAME_KEY ) || !json[NAME_KEY].IsString() )
		{
			SPDLOG_ERROR( "GMOD Node JSON (code='{}') missing required '{}' field or not a string", code, NAME_KEY );
			return std::nullopt;
		}

		std::string category = internString( json[CATEGORY_KEY].GetString() );
		std::string type = internString( json[TYPE_KEY].GetString() );
		std::string name = json[NAME_KEY].GetString();

		if ( category.empty() )
			SPDLOG_WARN( "Empty category field found in GMOD node code='{}'", code );
		if ( type.empty() )
			SPDLOG_WARN( "Empty type field found in GMOD node code='{}'", code );
		if ( code.empty() )
			SPDLOG_WARN( "Empty code field found in GMOD node" );
		if ( name.empty() )
			SPDLOG_WARN( "Empty name field found in GMOD node code='{}'", code );

		SPDLOG_DEBUG( "Parsed required GMOD node fields: category={}, type={}, code={}, name={}",
			category, type, code, name );

		std::optional<std::string> commonName = std::nullopt;
		std::optional<std::string> definition = std::nullopt;
		std::optional<std::string> commonDefinition = std::nullopt;
		std::optional<bool> installSubstructure = std::nullopt;
		std::optional<NormalAssignmentNamesMap> normalAssignmentNames = std::nullopt;

		if ( json.HasMember( COMMON_NAME_KEY ) )
		{
			if ( json[COMMON_NAME_KEY].IsString() )
				commonName = json[COMMON_NAME_KEY].GetString();
			else
				SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}'", code, COMMON_NAME_KEY );
		}
		if ( json.HasMember( DEFINITION_KEY ) )
		{
			if ( json[DEFINITION_KEY].IsString() )
				definition = json[DEFINITION_KEY].GetString();
			else
				SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}'", code, DEFINITION_KEY );
		}
		if ( json.HasMember( COMMON_DEFINITION_KEY ) )
		{
			if ( json[COMMON_DEFINITION_KEY].IsString() )
				commonDefinition = json[COMMON_DEFINITION_KEY].GetString();
			else
				SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}'", code, COMMON_DEFINITION_KEY );
		}
		if ( json.HasMember( INSTALL_SUBSTRUCTURE_KEY ) )
		{
			if ( json[INSTALL_SUBSTRUCTURE_KEY].IsBool() )
				installSubstructure = json[INSTALL_SUBSTRUCTURE_KEY].GetBool();
			else
				SPDLOG_WARN( "GMOD Node code='{}' has non-bool '{}'", code, INSTALL_SUBSTRUCTURE_KEY );
		}
		if ( json.HasMember( NORMAL_ASSIGNMENT_NAMES_KEY ) )
		{
			if ( json[NORMAL_ASSIGNMENT_NAMES_KEY].IsObject() )
			{
				NormalAssignmentNamesMap assignmentNames;
				const auto& namesObj = json[NORMAL_ASSIGNMENT_NAMES_KEY];
				assignmentNames.reserve( namesObj.MemberCount() );

				for ( auto it = namesObj.MemberBegin(); it != namesObj.MemberEnd(); ++it )
				{
					if ( it->name.IsString() && it->value.IsString() )
					{
						assignmentNames.emplace( it->name.GetString(), it->value.GetString() );
					}
					else
					{
						SPDLOG_WARN( "GMOD Node code='{}' has invalid entry in '{}'", code, NORMAL_ASSIGNMENT_NAMES_KEY );
					}
				}

				if ( !assignmentNames.empty() )
				{
					normalAssignmentNames = std::move( assignmentNames );
					SPDLOG_DEBUG( "Parsed {} normal assignment name mappings for code='{}'", normalAssignmentNames->size(), code );
				}
			}
			else
			{
				SPDLOG_WARN( "GMOD Node code='{}' has non-object '{}'", code, NORMAL_ASSIGNMENT_NAMES_KEY );
			}
		}

		GmodNodeDto resultDto(
			std::move( category ),
			std::move( type ),
			std::move( code ),
			std::move( name ),
			std::move( commonName ),
			std::move( definition ),
			std::move( commonDefinition ),
			installSubstructure,
			std::move( normalAssignmentNames ) );

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Successfully parsed GmodNodeDto: code={} in {} µs", resultDto.code(), duration.count() );

		return std::optional<GmodNodeDto>{ std::move( resultDto ) };
	}

	GmodNodeDto GmodNodeDto::fromJson( const rapidjson::Value& json )
	{
		auto dtoOpt = GmodNodeDto::tryFromJson( json );
		if ( !dtoOpt.has_value() )
		{
			std::string codeHint = "[unknown code]";
			if ( json.IsObject() && json.HasMember( CODE_KEY ) && json[CODE_KEY].IsString() )
			{
				codeHint = json[CODE_KEY].GetString();
			}
			std::string errorMsg = fmt::format( "Failed to deserialize GmodNodeDto from JSON (hint: code='{}')", codeHint );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		return std::move( dtoOpt.value() );
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
	// Constructors / Destructor
	//-------------------------------------------------------------------

	GmodDto::GmodDto( std::string visVersion, Items items, Relations relations )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) },
		  m_relations{ std::move( relations ) }
	{
		SPDLOG_INFO( "Creating GmodDto: visVersion={}, items={}, relations={}",
			m_visVersion, m_items.size(), m_relations.size() );
	}

	//-------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------

	const std::string& GmodDto::visVersion() const
	{
		return m_visVersion;
	}

	const GmodDto::Items& GmodDto::items() const
	{
		return m_items;
	}

	const GmodDto::Relations& GmodDto::relations() const
	{
		return m_relations;
	}

	//-------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------

	std::optional<GmodDto> GmodDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse GMOD from JSON" );

		if ( !json.HasMember( VIS_RELEASE_KEY ) || !json[VIS_RELEASE_KEY].IsString() )
		{
			SPDLOG_ERROR( "GMOD JSON missing required '{}' field or not a string", VIS_RELEASE_KEY );
			return std::nullopt;
		}
		std::string visVersion = json[VIS_RELEASE_KEY].GetString();
		SPDLOG_INFO( "GMOD VIS version: {}", visVersion );

		Items items;
		if ( json.HasMember( ITEMS_KEY ) )
		{
			if ( !json[ITEMS_KEY].IsArray() )
			{
				SPDLOG_WARN( "GMOD 'items' field is not an array for VIS version {}", visVersion );
			}
			else
			{
				const auto& itemsArray = json[ITEMS_KEY].GetArray();
				size_t totalItems = itemsArray.Size();
				SPDLOG_INFO( "Found {} GMOD node items to parse", totalItems );
				items.reserve( totalItems );
				size_t successCount = 0;

				for ( rapidjson::SizeType i = 0; i < itemsArray.Size(); ++i )
				{
					auto nodeOpt = GmodNodeDto::tryFromJson( itemsArray[i] );
					if ( nodeOpt.has_value() )
					{
						items.emplace_back( std::move( nodeOpt.value() ) );
						successCount++;
					}
					else
					{
						SPDLOG_WARN( "Skipping malformed GMOD node at index {} during GmodDto parsing for VIS version {}", i, visVersion );
					}
				}
				SPDLOG_INFO( "Successfully parsed {}/{} GMOD nodes", successCount, totalItems );
				if ( totalItems > 0 && static_cast<double>( successCount ) < static_cast<double>( totalItems ) * 0.9 )
				{
					SPDLOG_INFO( "Shrinking items vector due to high parsing failure rate ({}/{}) for VIS version {}", successCount, totalItems, visVersion );
					items.shrink_to_fit();
				}
			}
		}
		else
		{
			SPDLOG_WARN( "GMOD missing 'items' array for VIS version {}", visVersion );
		}

		Relations relations;
		if ( json.HasMember( RELATIONS_KEY ) )
		{
			if ( !json[RELATIONS_KEY].IsArray() )
			{
				SPDLOG_WARN( "GMOD 'relations' field is not an array for VIS version {}", visVersion );
			}
			else
			{
				const auto& relationsArray = json[RELATIONS_KEY].GetArray();
				size_t relationCount = relationsArray.Size();
				SPDLOG_INFO( "Found {} GMOD relation entries to parse", relationCount );
				relations.reserve( relationCount );
				size_t validRelationCount = 0;

				for ( const auto& relation : relationsArray )
				{
					if ( relation.IsArray() )
					{
						Relation relationPair;
						relationPair.reserve( relation.Size() );
						bool validPair = true;
						for ( const auto& rel : relation.GetArray() )
						{
							if ( rel.IsString() )
							{
								relationPair.emplace_back( rel.GetString() );
							}
							else
							{
								SPDLOG_WARN( "Non-string value found in relation entry for VIS version {}", visVersion );
								validPair = false;
								break;
							}
						}

						if ( validPair && !relationPair.empty() )
						{
							relations.emplace_back( std::move( relationPair ) );
							validRelationCount++;
						}
					}
					else
					{
						SPDLOG_WARN( "Non-array entry found in 'relations' array for VIS version {}", visVersion );
					}
				}
				SPDLOG_DEBUG( "Added {} valid relations to GMOD", validRelationCount );
				if ( relationCount > 0 && static_cast<double>( validRelationCount ) < static_cast<double>( relationCount ) * 0.9 )
				{
					SPDLOG_INFO( "Shrinking relations vector due to high parsing failure rate ({}/{}) for VIS version {}", validRelationCount, relationCount, visVersion );
					relations.shrink_to_fit();
				}
			}
		}
		else
		{
			SPDLOG_INFO( "GMOD has no 'relations' array for VIS version {}", visVersion );
		}

		if ( items.size() > 10000 )
		{
			const size_t approxMemoryUsage =
				( items.size() * sizeof( GmodNodeDto ) + relations.size() * 24 ) / ( 1024 * 1024 );
			SPDLOG_INFO( "Large GMOD model loaded: ~{} MB estimated memory usage",
				approxMemoryUsage );
		}

		GmodDto resultDto( std::move( visVersion ), std::move( items ), std::move( relations ) );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Parsed GmodDto with {} nodes, {} relations and VIS version {} in {} ms",
			resultDto.items().size(), resultDto.relations().size(), resultDto.visVersion(), duration.count() );

		return std::optional<GmodDto>{ std::move( resultDto ) };
	}

	GmodDto GmodDto::fromJson( const rapidjson::Value& json )
	{
		auto dtoOpt = GmodDto::tryFromJson( json );
		if ( !dtoOpt.has_value() )
		{
			std::string visHint = "[unknown version]";
			if ( json.IsObject() && json.HasMember( VIS_RELEASE_KEY ) && json[VIS_RELEASE_KEY].IsString() )
			{
				visHint = json[VIS_RELEASE_KEY].GetString();
			}
			std::string errorMsg = fmt::format( "Failed to deserialize GmodDto from JSON (hint: visRelease='{}')", visHint );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		return std::move( dtoOpt.value() );
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
		SPDLOG_INFO( "Serialized GmodDto with {} items for VIS version {} in {} ms",
			m_items.size(), m_visVersion, duration.count() );

		return obj;
	}
}
