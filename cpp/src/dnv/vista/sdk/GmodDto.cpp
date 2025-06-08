/**
 * @file GmodDto.cpp
 * @brief Implementation of Generic Product Model (GMOD) data transfer objects
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodDto.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr std::string_view CATEGORY_KEY = "category";
		static constexpr std::string_view TYPE_KEY = "type";
		static constexpr std::string_view CODE_KEY = "code";
		static constexpr std::string_view NAME_KEY = "name";
		static constexpr std::string_view COMMON_NAME_KEY = "commonName";
		static constexpr std::string_view DEFINITION_KEY = "definition";
		static constexpr std::string_view COMMON_DEFINITION_KEY = "commonDefinition";
		static constexpr std::string_view INSTALL_SUBSTRUCTURE_KEY = "installSubstructure";
		static constexpr std::string_view NORMAL_ASSIGNMENT_NAMES_KEY = "normalAssignmentNames";

		static constexpr std::string_view VIS_RELEASE_KEY = "visRelease";
		static constexpr std::string_view ITEMS_KEY = "items";
		static constexpr std::string_view RELATIONS_KEY = "relations";

		//=====================================================================
		// Helper Functions
		//=====================================================================

		/**
		 * @brief Safe string extraction from simdjson element
		 */
		std::optional<std::string> safeGetString( simdjson::dom::element element ) noexcept
		{
			if ( element.type() != simdjson::dom::element_type::STRING )
				return std::nullopt;

			std::string_view sv;
			auto error = element.get( sv );
			if ( error )
				return std::nullopt;

			return std::string( sv );
		}

		/**
		 * @brief Safe array extraction from simdjson element
		 */
		std::optional<simdjson::dom::array> safeGetArray( simdjson::dom::element element ) noexcept
		{
			if ( element.type() != simdjson::dom::element_type::ARRAY )
				return std::nullopt;

			simdjson::dom::array arr;
			auto error = element.get( arr );
			if ( error )
				return std::nullopt;

			return arr;
		}

		/**
		 * @brief Safe object extraction from simdjson element
		 */
		std::optional<simdjson::dom::object> safeGetObject( simdjson::dom::element element ) noexcept
		{
			if ( element.type() != simdjson::dom::element_type::OBJECT )
				return std::nullopt;

			simdjson::dom::object obj;
			auto error = element.get( obj );
			if ( error )
				return std::nullopt;

			return obj;
		}

		/**
		 * @brief Safe boolean extraction from simdjson element
		 */
		std::optional<bool> safeGetBool( simdjson::dom::element element ) noexcept
		{
			if ( element.type() != simdjson::dom::element_type::BOOL )
				return std::nullopt;

			bool val;
			auto error = element.get( val );
			if ( error )
				return std::nullopt;

			return val;
		}

		/**
		 * @brief Interns short strings to reduce memory usage for commonly repeated values
		 */
		const std::string& internString( const std::string& value )
		{
			static std::unordered_map<std::string, std::string> cache;
			static size_t hits = 0, misses = 0, calls = 0;
			calls++;

			if ( value.length() > 30 )
			{
				return value;
			}

			auto it = cache.find( value );
			if ( it != cache.end() )
			{
				hits++;
				if ( calls % 10000 == 0 )
				{
					SPDLOG_DEBUG( "String interning stats: {:.1f}% hit rate ({}/{}), {} unique strings", hits * 100.0 / calls, hits, calls, cache.size() );
				}
				return it->second;
			}

			misses++;
			return cache.emplace( value, value ).first->first;
		}

		/**
		 * @brief Escape JSON string for output
		 */
		std::string escapeJsonString( const std::string& str )
		{
			std::ostringstream oss;
			for ( char c : str )
			{
				switch ( c )
				{
					case '"':
						oss << "\\\"";
						break;
					case '\\':
						oss << "\\\\";
						break;
					case '\b':
						oss << "\\b";
						break;
					case '\f':
						oss << "\\f";
						break;
					case '\n':
						oss << "\\n";
						break;
					case '\r':
						oss << "\\r";
						break;
					case '\t':
						oss << "\\t";
						break;
					default:
						if ( c >= 0 && c < 32 )
						{
							oss << "\\u" << std::hex << std::setw( 4 ) << std::setfill( '0' ) << static_cast<int>( c );
						}
						else
						{
							oss << c;
						}
						break;
				}
			}
			return oss.str();
		}
	}

	//=====================================================================
	// GMOD Node Data Transfer Object
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
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

	std::optional<GmodNodeDto> GmodNodeDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "GmodNodeDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto codeElement = obj[CODE_KEY];
			if ( codeElement.error() )
			{
				SPDLOG_ERROR( "GMOD Node JSON missing required '{}' field", CODE_KEY );
				return std::nullopt;
			}

			auto codeOpt = safeGetString( codeElement.value() );
			if ( !codeOpt )
			{
				SPDLOG_ERROR( "GMOD Node JSON '{}' field is not a string", CODE_KEY );
				return std::nullopt;
			}

			std::string code = *codeOpt;
			if ( code.empty() )
			{
				SPDLOG_WARN( "Empty code field found in GMOD node" );
			}

			auto categoryElement = obj[CATEGORY_KEY];
			if ( categoryElement.error() )
			{
				SPDLOG_ERROR( "GMOD Node JSON (code='{}') missing required '{}' field", code, CATEGORY_KEY );
				return std::nullopt;
			}

			auto categoryOpt = safeGetString( categoryElement.value() );
			if ( !categoryOpt )
			{
				SPDLOG_ERROR( "GMOD Node JSON (code='{}') '{}' field is not a string", code, CATEGORY_KEY );
				return std::nullopt;
			}

			auto typeElement = obj[TYPE_KEY];
			if ( typeElement.error() )
			{
				SPDLOG_ERROR( "GMOD Node JSON (code='{}') missing required '{}' field", code, TYPE_KEY );
				return std::nullopt;
			}

			auto typeOpt = safeGetString( typeElement.value() );
			if ( !typeOpt )
			{
				SPDLOG_ERROR( "GMOD Node JSON (code='{}') '{}' field is not a string", code, TYPE_KEY );
				return std::nullopt;
			}

			std::string nameValue;
			auto nameElement = obj[NAME_KEY];
			if ( !nameElement.error() )
			{
				auto nameOpt = safeGetString( nameElement.value() );
				if ( nameOpt )
				{
					nameValue = *nameOpt;
				}
				else
				{
					SPDLOG_ERROR( "GMOD Node JSON (code='{}') field '{}' is present but not a string", code, NAME_KEY );
					return std::nullopt;
				}
			}
			else
			{
				SPDLOG_WARN( "GMOD Node JSON (code='{}') missing '{}' field. Defaulting name to empty string.", code, NAME_KEY );
				nameValue = "";
			}

			std::string category = internString( *categoryOpt );
			std::string type = internString( *typeOpt );

			if ( category.empty() )
			{
				SPDLOG_WARN( "Empty category field found in GMOD node code='{}'", code );
			}
			if ( type.empty() )
			{
				SPDLOG_WARN( "Empty type field found in GMOD node code='{}'", code );
			}
			if ( nameValue.empty() )
			{
				SPDLOG_WARN( "Empty name field used for GMOD node code='{}'", code );
			}

			std::optional<std::string> commonName = std::nullopt;
			auto commonNameElement = obj[COMMON_NAME_KEY];
			if ( !commonNameElement.error() )
			{
				auto commonNameOpt = safeGetString( commonNameElement.value() );
				if ( commonNameOpt )
				{
					commonName = *commonNameOpt;
				}
				else if ( commonNameElement.value().type() != simdjson::dom::element_type::NULL_VALUE )
				{
					SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}'", code, COMMON_NAME_KEY );
				}
			}

			std::optional<std::string> definition = std::nullopt;
			auto definitionElement = obj[DEFINITION_KEY];
			if ( !definitionElement.error() )
			{
				auto definitionOpt = safeGetString( definitionElement.value() );
				if ( definitionOpt )
				{
					definition = *definitionOpt;
				}
				else if ( definitionElement.value().type() != simdjson::dom::element_type::NULL_VALUE )
				{
					SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}'", code, DEFINITION_KEY );
				}
			}

			std::optional<std::string> commonDefinition = std::nullopt;
			auto commonDefinitionElement = obj[COMMON_DEFINITION_KEY];
			if ( !commonDefinitionElement.error() )
			{
				auto commonDefinitionOpt = safeGetString( commonDefinitionElement.value() );
				if ( commonDefinitionOpt )
				{
					commonDefinition = *commonDefinitionOpt;
				}
				else if ( commonDefinitionElement.value().type() != simdjson::dom::element_type::NULL_VALUE )
				{
					SPDLOG_WARN( "GMOD Node code='{}' has non-string '{}'", code, COMMON_DEFINITION_KEY );
				}
			}

			std::optional<bool> installSubstructure = std::nullopt;
			auto installSubstructureElement = obj[INSTALL_SUBSTRUCTURE_KEY];
			if ( !installSubstructureElement.error() )
			{
				auto installSubstructureOpt = safeGetBool( installSubstructureElement.value() );
				if ( installSubstructureOpt )
				{
					installSubstructure = *installSubstructureOpt;
				}
				else if ( installSubstructureElement.value().type() != simdjson::dom::element_type::NULL_VALUE )
				{
					SPDLOG_WARN( "GMOD Node code='{}' has non-bool '{}'", code, INSTALL_SUBSTRUCTURE_KEY );
				}
			}

			std::optional<NormalAssignmentNamesMap> normalAssignmentNames = std::nullopt;
			auto normalAssignmentNamesElement = obj[NORMAL_ASSIGNMENT_NAMES_KEY];
			if ( !normalAssignmentNamesElement.error() )
			{
				auto normalAssignmentNamesObjOpt = safeGetObject( normalAssignmentNamesElement.value() );
				if ( normalAssignmentNamesObjOpt )
				{
					NormalAssignmentNamesMap assignments;
					auto normalAssignmentNamesObj = *normalAssignmentNamesObjOpt;
					assignments.reserve( normalAssignmentNamesObj.size() );

					for ( auto [key, value] : normalAssignmentNamesObj )
					{
						auto valueStrOpt = safeGetString( value );
						if ( valueStrOpt )
						{
							assignments.emplace( std::string( key ), *valueStrOpt );
						}
						else
						{
							SPDLOG_WARN( "GMOD Node code='{}' has non-string value in '{}' for key '{}'", code, NORMAL_ASSIGNMENT_NAMES_KEY, key );
						}
					}

					if ( !assignments.empty() )
					{
						normalAssignmentNames = std::move( assignments );
					}
				}
				else if ( normalAssignmentNamesElement.value().type() != simdjson::dom::element_type::NULL_VALUE )
				{
					SPDLOG_WARN( "GMOD Node code='{}' has non-object '{}'", code, NORMAL_ASSIGNMENT_NAMES_KEY );
				}
			}

			GmodNodeDto resultDto(
				std::move( category ),
				std::move( type ),
				std::move( code ),
				std::move( nameValue ),
				std::move( commonName ),
				std::move( definition ),
				std::move( commonDefinition ),
				installSubstructure,
				std::move( normalAssignmentNames ) );

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed GMOD node '{}' in {} µs", resultDto.code(), duration.count() );

			return std::optional<GmodNodeDto>{ std::move( resultDto ) };
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during GmodNodeDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<GmodNodeDto> GmodNodeDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
	{
		try
		{
			auto parseResult = parser.parse( jsonString );
			if ( parseResult.error() )
			{
				SPDLOG_ERROR( "JSON parse error: {}", simdjson::error_message( parseResult.error() ) );
				return std::nullopt;
			}

			return tryFromJson( parseResult.value() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during JSON string parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	GmodNodeDto GmodNodeDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodNodeDto from simdjson element" );
		}
		return std::move( *result );
	}

	GmodNodeDto GmodNodeDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodNodeDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string GmodNodeDto::toJsonString() const
	{
		auto startTime = std::chrono::steady_clock::now();

		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"" << CATEGORY_KEY << "\": \"" << escapeJsonString( m_category ) << "\",\n";
		oss << "  \"" << TYPE_KEY << "\": \"" << escapeJsonString( m_type ) << "\",\n";
		oss << "  \"" << CODE_KEY << "\": \"" << escapeJsonString( m_code ) << "\",\n";
		oss << "  \"" << NAME_KEY << "\": \"" << escapeJsonString( m_name ) << "\"";

		if ( m_commonName.has_value() )
		{
			oss << ",\n  \"" << COMMON_NAME_KEY << "\": \"" << escapeJsonString( *m_commonName ) << "\"";
		}
		if ( m_definition.has_value() )
		{
			oss << ",\n  \"" << DEFINITION_KEY << "\": \"" << escapeJsonString( *m_definition ) << "\"";
		}
		if ( m_commonDefinition.has_value() )
		{
			oss << ",\n  \"" << COMMON_DEFINITION_KEY << "\": \"" << escapeJsonString( *m_commonDefinition ) << "\"";
		}
		if ( m_installSubstructure.has_value() )
		{
			oss << ",\n  \"" << INSTALL_SUBSTRUCTURE_KEY << "\": " << ( *m_installSubstructure ? "true" : "false" );
		}
		if ( m_normalAssignmentNames.has_value() )
		{
			oss << ",\n  \"" << NORMAL_ASSIGNMENT_NAMES_KEY << "\": {";
			bool first = true;
			for ( const auto& [key, value] : *m_normalAssignmentNames )
			{
				if ( !first )
					oss << ",";
				first = false;
				oss << "\n    \"" << escapeJsonString( key ) << "\": \"" << escapeJsonString( value ) << "\"";
			}
			oss << "\n  }";
		}

		oss << "\n}";

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized GMOD node '{}' in {} µs", m_code, duration.count() );

		return oss.str();
	}

	//=====================================================================
	// GMOD Data Transfer Object
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	GmodDto::GmodDto( std::string visVersion, Items items, Relations relations )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) },
		  m_relations{ std::move( relations ) }
	{
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

	std::optional<GmodDto> GmodDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "GmodDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto visElement = obj[VIS_RELEASE_KEY];
			if ( visElement.error() )
			{
				SPDLOG_ERROR( "GMOD JSON missing required '{}' field", VIS_RELEASE_KEY );
				return std::nullopt;
			}

			auto visVersionOpt = safeGetString( visElement.value() );
			if ( !visVersionOpt )
			{
				SPDLOG_ERROR( "GMOD JSON '{}' field is not a string", VIS_RELEASE_KEY );
				return std::nullopt;
			}

			std::string visVersion = *visVersionOpt;

			Items items;
			auto itemsElement = obj[ITEMS_KEY];
			if ( !itemsElement.error() )
			{
				auto itemsArrayOpt = safeGetArray( itemsElement.value() );
				if ( itemsArrayOpt )
				{
					auto itemsArray = *itemsArrayOpt;
					size_t totalItems = itemsArray.size();
					items.reserve( totalItems );
					size_t successCount = 0;

					size_t i = 0;
					for ( auto itemElement : itemsArray )
					{
						auto nodeOpt = GmodNodeDto::tryFromJson( itemElement );
						if ( nodeOpt.has_value() )
						{
							items.emplace_back( std::move( nodeOpt.value() ) );
							successCount++;
						}
						else
						{
							SPDLOG_WARN( "Skipping malformed GMOD node at index {} during GmodDto parsing for VIS version {}", i, visVersion );
						}
						++i;
					}

					SPDLOG_DEBUG( "Successfully parsed {}/{} GMOD nodes", successCount, totalItems );
					if ( totalItems > 0 && static_cast<double>( successCount ) < static_cast<double>( totalItems ) * 0.9 )
					{
						SPDLOG_WARN( "Shrinking items vector due to high parsing failure rate ({}/{}) for VIS version {}", successCount, totalItems, visVersion );
						items.shrink_to_fit();
					}
				}
				else
				{
					SPDLOG_WARN( "GMOD 'items' field is not an array for VIS version {}", visVersion );
				}
			}
			else
			{
				SPDLOG_WARN( "GMOD missing 'items' array for VIS version {}", visVersion );
			}

			Relations relations;
			auto relationsElement = obj[RELATIONS_KEY];
			if ( !relationsElement.error() )
			{
				auto relationsArrayOpt = safeGetArray( relationsElement.value() );
				if ( relationsArrayOpt )
				{
					auto relationsArray = *relationsArrayOpt;
					size_t relationCount = relationsArray.size();
					relations.reserve( relationCount );
					size_t validRelationCount = 0;

					for ( auto relationElement : relationsArray )
					{
						auto relationArrayOpt = safeGetArray( relationElement );
						if ( relationArrayOpt )
						{
							auto relationArray = *relationArrayOpt;
							Relation relationPair;
							relationPair.reserve( relationArray.size() );
							bool validPair = true;

							for ( auto relElement : relationArray )
							{
								auto relStrOpt = safeGetString( relElement );
								if ( relStrOpt )
								{
									relationPair.emplace_back( *relStrOpt );
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

					if ( relationCount > 0 && static_cast<double>( validRelationCount ) < static_cast<double>( relationCount ) * 0.9 )
					{
						SPDLOG_WARN( "Shrinking relations vector due to high parsing failure rate ({}/{}) for VIS version {}", validRelationCount, relationCount, visVersion );
						relations.shrink_to_fit();
					}
				}
				else
				{
					SPDLOG_WARN( "GMOD 'relations' field is not an array for VIS version {}", visVersion );
				}
			}

			if ( items.size() > 10000 )
			{
				const size_t approxMemoryUsage = ( items.size() * sizeof( GmodNodeDto ) + relations.size() * 24 ) / ( 1024 * 1024 );
				SPDLOG_DEBUG( "Large GMOD model loaded: ~{} MB estimated memory usage", approxMemoryUsage );
			}

			GmodDto resultDto( std::move( visVersion ), std::move( items ), std::move( relations ) );

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed GmodDto with {} nodes, {} relations and VIS version {} in {} ms",
				resultDto.items().size(), resultDto.relations().size(), resultDto.visVersion(), duration.count() );

			return std::optional<GmodDto>{ std::move( resultDto ) };
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during GmodDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<GmodDto> GmodDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
	{
		try
		{
			auto parseResult = parser.parse( jsonString );
			if ( parseResult.error() )
			{
				SPDLOG_ERROR( "JSON parse error: {}", simdjson::error_message( parseResult.error() ) );
				return std::nullopt;
			}

			return tryFromJson( parseResult.value() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during JSON string parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	GmodDto GmodDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodDto from simdjson element" );
		}
		return std::move( *result );
	}

	GmodDto GmodDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string GmodDto::toJsonString() const
	{
		auto startTime = std::chrono::steady_clock::now();

		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"" << VIS_RELEASE_KEY << "\": \"" << escapeJsonString( m_visVersion ) << "\"";

		oss << ",\n  \"" << ITEMS_KEY << "\": [";
		if ( !m_items.empty() )
		{
			oss << "\n";
			bool first = true;
			for ( const auto& item : m_items )
			{
				if ( !first )
					oss << ",\n";
				first = false;

				std::string itemJson = item.toJsonString();
				std::istringstream iss( itemJson );
				std::string line;
				bool firstLine = true;
				while ( std::getline( iss, line ) )
				{
					if ( !firstLine )
						oss << "\n";
					firstLine = false;
					oss << "    " << line;
				}
			}
			oss << "\n  ";
		}
		oss << "]";

		oss << ",\n  \"" << RELATIONS_KEY << "\": [";
		if ( !m_relations.empty() )
		{
			bool first = true;
			for ( const auto& relation : m_relations )
			{
				if ( !first )
					oss << ",";
				first = false;
				oss << "\n    [";
				bool firstRel = true;
				for ( const auto& rel : relation )
				{
					if ( !firstRel )
						oss << ", ";
					firstRel = false;
					oss << "\"" << escapeJsonString( rel ) << "\"";
				}
				oss << "]";
			}
			if ( !first )
				oss << "\n  ";
		}
		oss << "]";

		oss << "\n}";

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized GmodDto with {} items, {} relations for VIS version {} in {} ms",
			m_items.size(), m_relations.size(), m_visVersion, duration.count() );

		return oss.str();
	}
}
