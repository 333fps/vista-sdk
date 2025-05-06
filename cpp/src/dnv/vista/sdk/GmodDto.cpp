/**
 * @file GmodDto.cpp
 * @brief Implementation of Generic Product Model (GMOD) data transfer objects
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodDto.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

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

	//=====================================================================
	// Helper Functions
	//=====================================================================

	/**
	 * @brief Interns short strings to reduce memory usage for commonly repeated values
	 * @param value The string to be interned
	 * @return A reference to the cached string instance
	 */
	static const std::string& internString( const std::string& value )
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

namespace dnv::vista::sdk
{
	//=====================================================================
	// GMOD Node Data Transfer Object
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

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

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

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

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<GmodNodeDto> GmodNodeDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse GmodNodeDto from nlohmann::json" );

		try
		{
			if ( !json.contains( CODE_KEY ) || !json.at( CODE_KEY ).is_string() )
			{
				SPDLOG_ERROR( "GMOD Node JSON missing required '{}' field or not a string", CODE_KEY );
				return std::nullopt;
			}
			std::string code = json.at( CODE_KEY ).get<std::string>();
			if ( code.empty() )
			{
				SPDLOG_WARN( "Empty code field found in GMOD node" );
			}

			if ( !json.contains( CATEGORY_KEY ) || !json.at( CATEGORY_KEY ).is_string() )
			{
				SPDLOG_ERROR( "GMOD Node JSON (code='{}') missing required '{}' field or not a string", code, CATEGORY_KEY );
				return std::nullopt;
			}
			if ( !json.contains( TYPE_KEY ) || !json.at( TYPE_KEY ).is_string() )
			{
				SPDLOG_ERROR( "GMOD Node JSON (code='{}') missing required '{}' field or not a string", code, TYPE_KEY );
				return std::nullopt;
			}
			if ( !json.contains( NAME_KEY ) || !json.at( NAME_KEY ).is_string() )
			{
				SPDLOG_ERROR( "GMOD Node JSON (code='{}') missing required '{}' field or not a string", code, NAME_KEY );
				return std::nullopt;
			}

			std::string category = internString( json.at( CATEGORY_KEY ).get<std::string>() );
			std::string type = internString( json.at( TYPE_KEY ).get<std::string>() );
			std::string name = json.at( NAME_KEY ).get<std::string>();

			if ( category.empty() )
				SPDLOG_WARN( "Empty category field found in GMOD node code='{}'", code );
			if ( type.empty() )
				SPDLOG_WARN( "Empty type field found in GMOD node code='{}'", code );
			if ( name.empty() )
				SPDLOG_WARN( "Empty name field found in GMOD node code='{}'", code );

			SPDLOG_DEBUG( "Parsed required GMOD node fields: category={}, type={}, code={}, name={}",
				category, type, code, name );

			std::optional<std::string> commonName = std::nullopt;
			std::optional<std::string> definition = std::nullopt;
			std::optional<std::string> commonDefinition = std::nullopt;
			std::optional<bool> installSubstructure = std::nullopt;
			std::optional<NormalAssignmentNamesMap> normalAssignmentNames = std::nullopt;

			if ( json.contains( COMMON_NAME_KEY ) )
			{
				if ( json.at( COMMON_NAME_KEY ).is_string() )
					commonName = json.at( COMMON_NAME_KEY ).get<std::string>();
				else if ( !json.at( COMMON_NAME_KEY ).is_null() )
					SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}'", code, COMMON_NAME_KEY );
			}
			if ( json.contains( DEFINITION_KEY ) )
			{
				if ( json.at( DEFINITION_KEY ).is_string() )
					definition = json.at( DEFINITION_KEY ).get<std::string>();
				else if ( !json.at( DEFINITION_KEY ).is_null() )
					SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}'", code, DEFINITION_KEY );
			}
			if ( json.contains( COMMON_DEFINITION_KEY ) )
			{
				if ( json.at( COMMON_DEFINITION_KEY ).is_string() )
					commonDefinition = json.at( COMMON_DEFINITION_KEY ).get<std::string>();
				else if ( !json.at( COMMON_DEFINITION_KEY ).is_null() )
					SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}'", code, COMMON_DEFINITION_KEY );
			}
			if ( json.contains( INSTALL_SUBSTRUCTURE_KEY ) )
			{
				if ( json.at( INSTALL_SUBSTRUCTURE_KEY ).is_boolean() )
					installSubstructure = json.at( INSTALL_SUBSTRUCTURE_KEY ).get<bool>();
				else if ( !json.at( INSTALL_SUBSTRUCTURE_KEY ).is_null() )
					SPDLOG_WARN( "GMOD Node code='{}' has non-bool '{}'", code, INSTALL_SUBSTRUCTURE_KEY );
			}
			if ( json.contains( NORMAL_ASSIGNMENT_NAMES_KEY ) )
			{
				if ( json.at( NORMAL_ASSIGNMENT_NAMES_KEY ).is_object() )
				{
					try
					{
						auto assignments = json.at( NORMAL_ASSIGNMENT_NAMES_KEY ).get<NormalAssignmentNamesMap>();
						if ( !assignments.empty() )
						{
							normalAssignmentNames = std::move( assignments );
							SPDLOG_DEBUG( "Parsed {} normal assignment name mappings for code='{}'", normalAssignmentNames->size(), code );
						}
					}
					catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
					{
						SPDLOG_WARN( "GMOD Node code='{}' failed to parse '{}' object: {}", code, NORMAL_ASSIGNMENT_NAMES_KEY, ex.what() );
					}
				}
				else if ( !json.at( NORMAL_ASSIGNMENT_NAMES_KEY ).is_null() )
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
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			std::string codeHint = "[unknown code]";
			try
			{
				if ( json.contains( CODE_KEY ) && json.at( CODE_KEY ).is_string() )
				{
					codeHint = json.at( CODE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}

			SPDLOG_ERROR( "nlohmann::json exception during GmodNodeDto parsing (hint: code='{}'): {}", codeHint, ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			std::string codeHint = "[unknown code]";
			try
			{
				if ( json.contains( CODE_KEY ) && json.at( CODE_KEY ).is_string() )
				{
					codeHint = json.at( CODE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}
			SPDLOG_ERROR( "Standard exception during GmodNodeDto parsing (hint: code='{}'): {}", codeHint, ex.what() );
			return std::nullopt;
		}
	}

	GmodNodeDto GmodNodeDto::fromJson( const nlohmann::json& json )
	{
		auto dtoOpt = GmodNodeDto::tryFromJson( json );
		if ( !dtoOpt.has_value() )
		{
			std::string codeHint = "[unknown code]";
			try
			{
				if ( json.is_object() && json.contains( CODE_KEY ) && json.at( CODE_KEY ).is_string() )
				{
					codeHint = json.at( CODE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}
			std::string errorMsg = fmt::format( "Failed to deserialize GmodNodeDto from JSON (hint: code='{}')", codeHint );
			throw std::invalid_argument( errorMsg );
		}
		return std::move( dtoOpt.value() );
	}

	nlohmann::json GmodNodeDto::toJson() const
	{
		nlohmann::json j;
		to_json( j, *this );
		return j;
	}

	//----------------------------------------------
	// Private Serialization Methods
	//----------------------------------------------

	void to_json( nlohmann::json& j, const GmodNodeDto& dto )
	{
		/* ADL hook for nlohmann::json serialization. */
		j = nlohmann::json{ { CATEGORY_KEY, dto.category() }, { TYPE_KEY, dto.type() }, { CODE_KEY, dto.code() }, { NAME_KEY, dto.name() } };

		if ( dto.commonName().has_value() )
		{
			j[COMMON_NAME_KEY] = dto.commonName().value();
		}
		if ( dto.definition().has_value() )
		{
			j[DEFINITION_KEY] = dto.definition().value();
		}
		if ( dto.commonDefinition().has_value() )
		{
			j[COMMON_DEFINITION_KEY] = dto.commonDefinition().value();
		}
		if ( dto.installSubstructure().has_value() )
		{
			j[INSTALL_SUBSTRUCTURE_KEY] = dto.installSubstructure().value();
		}
		if ( dto.normalAssignmentNames().has_value() )
		{
			j[NORMAL_ASSIGNMENT_NAMES_KEY] = dto.normalAssignmentNames().value();
		}
	}

	void from_json( const nlohmann::json& j, GmodNodeDto& dto )
	{
		/* ADL hook for nlohmann::json deserialization. */
		if ( !j.contains( CATEGORY_KEY ) || !j.at( CATEGORY_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "GMOD Node JSON missing required '{}' field or not a string", CATEGORY_KEY ), &j );
		}
		if ( !j.contains( TYPE_KEY ) || !j.at( TYPE_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "GMOD Node JSON missing required '{}' field or not a string", TYPE_KEY ), &j );
		}
		if ( !j.contains( CODE_KEY ) || !j.at( CODE_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "GMOD Node JSON missing required '{}' field or not a string", CODE_KEY ), &j );
		}
		if ( !j.contains( NAME_KEY ) || !j.at( NAME_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "GMOD Node JSON missing required '{}' field or not a string", NAME_KEY ), &j );
		}

		dto.m_category = internString( j.at( CATEGORY_KEY ).get<std::string>() );
		dto.m_type = internString( j.at( TYPE_KEY ).get<std::string>() );
		dto.m_code = j.at( CODE_KEY ).get<std::string>();
		dto.m_name = j.at( NAME_KEY ).get<std::string>();

		dto.m_commonName = std::nullopt;
		if ( j.contains( COMMON_NAME_KEY ) )
		{
			if ( j.at( COMMON_NAME_KEY ).is_string() )
				dto.m_commonName = j.at( COMMON_NAME_KEY ).get<std::string>();
			else if ( !j.at( COMMON_NAME_KEY ).is_null() )
				SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}' in from_json", dto.m_code, COMMON_NAME_KEY );
		}

		dto.m_definition = std::nullopt;
		if ( j.contains( DEFINITION_KEY ) )
		{
			if ( j.at( DEFINITION_KEY ).is_string() )
				dto.m_definition = j.at( DEFINITION_KEY ).get<std::string>();
			else if ( !j.at( DEFINITION_KEY ).is_null() )
				SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}' in from_json", dto.m_code, DEFINITION_KEY );
		}

		dto.m_commonDefinition = std::nullopt;
		if ( j.contains( COMMON_DEFINITION_KEY ) )
		{
			if ( j.at( COMMON_DEFINITION_KEY ).is_string() )
				dto.m_commonDefinition = j.at( COMMON_DEFINITION_KEY ).get<std::string>();
			else if ( !j.at( COMMON_DEFINITION_KEY ).is_null() )
				SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}' in from_json", dto.m_code, COMMON_DEFINITION_KEY );
		}

		dto.m_installSubstructure = std::nullopt;
		if ( j.contains( INSTALL_SUBSTRUCTURE_KEY ) )
		{
			if ( j.at( INSTALL_SUBSTRUCTURE_KEY ).is_boolean() )
				dto.m_installSubstructure = j.at( INSTALL_SUBSTRUCTURE_KEY ).get<bool>();
			else if ( !j.at( INSTALL_SUBSTRUCTURE_KEY ).is_null() )
				SPDLOG_WARN( "GMOD Node code='{}' has non-bool '{}' in from_json", dto.m_code, INSTALL_SUBSTRUCTURE_KEY );
		}

		dto.m_normalAssignmentNames = std::nullopt;
		if ( j.contains( NORMAL_ASSIGNMENT_NAMES_KEY ) )
		{
			if ( j.at( NORMAL_ASSIGNMENT_NAMES_KEY ).is_object() )
			{
				try
				{
					auto assignments = j.at( NORMAL_ASSIGNMENT_NAMES_KEY ).get<GmodNodeDto::NormalAssignmentNamesMap>();
					if ( !assignments.empty() )
					{
						dto.m_normalAssignmentNames = std::move( assignments );
					}
				}
				catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
				{
					SPDLOG_WARN( "GMOD Node code='{}' failed to parse '{}' object in from_json: {}", dto.m_code, NORMAL_ASSIGNMENT_NAMES_KEY, ex.what() );
				}
			}
			else if ( !j.at( NORMAL_ASSIGNMENT_NAMES_KEY ).is_null() )
			{
				SPDLOG_WARN( "GMOD Node code='{}' has non-object '{}' in from_json", dto.m_code, NORMAL_ASSIGNMENT_NAMES_KEY );
			}
		}
	}

	//=====================================================================
	// GMOD Data Transfer Object
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	GmodDto::GmodDto( std::string visVersion, Items items, Relations relations )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) },
		  m_relations{ std::move( relations ) }
	{
		SPDLOG_INFO( "Creating GmodDto: visVersion={}, items={}, relations={}",
			m_visVersion, m_items.size(), m_relations.size() );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

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

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<GmodDto> GmodDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse GMOD from nlohmann::json" );

		try
		{
			if ( !json.contains( VIS_RELEASE_KEY ) || !json.at( VIS_RELEASE_KEY ).is_string() )
			{
				SPDLOG_ERROR( "GMOD JSON missing required '{}' field or not a string", VIS_RELEASE_KEY );
				return std::nullopt;
			}
			std::string visVersion = json.at( VIS_RELEASE_KEY ).get<std::string>();
			SPDLOG_INFO( "GMOD VIS version: {}", visVersion );

			Items items;
			if ( json.contains( ITEMS_KEY ) )
			{
				if ( !json.at( ITEMS_KEY ).is_array() )
				{
					SPDLOG_WARN( "GMOD 'items' field is not an array for VIS version {}", visVersion );
				}
				else
				{
					const auto& itemsArray = json.at( ITEMS_KEY );
					size_t totalItems = itemsArray.size();
					SPDLOG_INFO( "Found {} GMOD node items to parse", totalItems );
					items.reserve( totalItems );
					size_t successCount = 0;

					for ( size_t i = 0; i < itemsArray.size(); ++i )
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
			if ( json.contains( RELATIONS_KEY ) )
			{
				if ( !json.at( RELATIONS_KEY ).is_array() )
				{
					SPDLOG_WARN( "GMOD 'relations' field is not an array for VIS version {}", visVersion );
				}
				else
				{
					const auto& relationsArray = json.at( RELATIONS_KEY );
					size_t relationCount = relationsArray.size();
					SPDLOG_INFO( "Found {} GMOD relation entries to parse", relationCount );
					relations.reserve( relationCount );
					size_t validRelationCount = 0;

					for ( const auto& relationJson : relationsArray )
					{
						if ( relationJson.is_array() )
						{
							Relation relationPair;
							relationPair.reserve( relationJson.size() );
							bool validPair = true;
							for ( const auto& relJson : relationJson )
							{
								if ( relJson.is_string() )
								{
									relationPair.emplace_back( relJson.get<std::string>() );
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
				[[maybe_unused]] const size_t approxMemoryUsage = ( items.size() * sizeof( GmodNodeDto ) + relations.size() * 24 ) / ( 1024 * 1024 );
				SPDLOG_INFO( "Large GMOD model loaded: ~{} MB estimated memory usage", approxMemoryUsage );
			}

			GmodDto resultDto( std::move( visVersion ), std::move( items ), std::move( relations ) );

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_INFO( "Parsed GmodDto with {} nodes, {} relations and VIS version {} in {} ms",
				resultDto.items().size(), resultDto.relations().size(), resultDto.visVersion(), duration.count() );

			return std::optional<GmodDto>{ std::move( resultDto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			std::string visHint = "[unknown version]";
			try
			{
				if ( json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
				{
					visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}
			SPDLOG_ERROR( "nlohmann::json exception during GmodDto parsing (hint: visRelease='{}'): {}", visHint, ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			std::string visHint = "[unknown version]";
			try
			{
				if ( json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
				{
					visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}
			SPDLOG_ERROR( "Standard exception during GmodDto parsing (hint: visRelease='{}'): {}", visHint, ex.what() );
			return std::nullopt;
		}
	}

	GmodDto GmodDto::fromJson( const nlohmann::json& json )
	{
		auto dtoOpt = GmodDto::tryFromJson( json );
		if ( !dtoOpt.has_value() )
		{
			std::string visHint = "[unknown version]";
			try
			{
				if ( json.is_object() && json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
				{
					visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}
			std::string errorMsg = fmt::format( "Failed to deserialize GmodDto from JSON (hint: visRelease='{}')", visHint );
			throw std::invalid_argument( errorMsg );
		}
		return std::move( dtoOpt.value() );
	}

	nlohmann::json GmodDto::toJson() const
	{
		auto startTime = std::chrono::steady_clock::now();

		nlohmann::json j = { { VIS_RELEASE_KEY, m_visVersion }, { ITEMS_KEY, m_items }, { RELATIONS_KEY, m_relations } };

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Serialized GmodDto with {} items, {} relations for VIS version {} in {} ms",
			m_items.size(), m_relations.size(), m_visVersion, duration.count() );

		return j;
	}

	//-------------------------------------------------------------------
	// Private Serialization Methods
	//-------------------------------------------------------------------

	void to_json( nlohmann::json& j, const GmodDto& dto )
	{
		/* ADL hook for nlohmann::json serialization. */
		j = nlohmann::json{ { VIS_RELEASE_KEY, dto.visVersion() }, { ITEMS_KEY, dto.items() }, { RELATIONS_KEY, dto.relations() } };
	}

	void from_json( const nlohmann::json& j, GmodDto& dto )
	{
		if ( !j.contains( VIS_RELEASE_KEY ) || !j.at( VIS_RELEASE_KEY ).is_string() )
		{
			throw std::invalid_argument( "Missing or invalid 'visRelease' in GmodDto JSON" );
		}
		if ( !j.contains( ITEMS_KEY ) || !j.at( ITEMS_KEY ).is_array() )
		{
			throw std::invalid_argument( "Missing or invalid 'items' in GmodDto JSON" );
		}
		if ( !j.contains( RELATIONS_KEY ) || !j.at( RELATIONS_KEY ).is_array() )
		{
			throw std::invalid_argument( "Missing or invalid 'relations' in GmodDto JSON" );
		}

		dto.m_visVersion = j.at( VIS_RELEASE_KEY ).get<std::string>();
		dto.m_items = j.at( ITEMS_KEY ).get<GmodDto::Items>();
		dto.m_relations = j.at( RELATIONS_KEY ).get<GmodDto::Relations>();
	}
}
