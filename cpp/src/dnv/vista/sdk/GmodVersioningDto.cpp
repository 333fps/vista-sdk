/**
 * @file GmodVersioningDto.cpp
 * @brief Implementation of GMOD versioning data transfer objects
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodVersioningDto.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr std::string_view VIS_RELEASE_KEY = "visRelease";
		static constexpr std::string_view ITEMS_KEY = "items";

		static constexpr std::string_view OLD_ASSIGNMENT_KEY = "oldAssignment";
		static constexpr std::string_view CURRENT_ASSIGNMENT_KEY = "currentAssignment";
		static constexpr std::string_view NEW_ASSIGNMENT_KEY = "newAssignment";
		static constexpr std::string_view DELETE_ASSIGNMENT_KEY = "deleteAssignment";

		static constexpr std::string_view OPERATIONS_KEY = "operations";
		static constexpr std::string_view SOURCE_KEY = "source";
		static constexpr std::string_view TARGET_KEY = "target";

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
		 * @brief String interning for memory optimization
		 */
		const std::string& internString( const std::string& value )
		{
			static std::unordered_map<std::string, std::string> cache;
			static size_t hits = 0, misses = 0, calls = 0;
			calls++;

			if ( value.size() > 30 )
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
	// GmodVersioningAssignmentChangeDto Implementation
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	GmodVersioningAssignmentChangeDto::GmodVersioningAssignmentChangeDto( std::string oldAssignment, std::string currentAssignment )
		: m_oldAssignment{ std::move( oldAssignment ) },
		  m_currentAssignment{ std::move( currentAssignment ) }
	{
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& GmodVersioningAssignmentChangeDto::oldAssignment() const
	{
		return m_oldAssignment;
	}

	const std::string& GmodVersioningAssignmentChangeDto::currentAssignment() const
	{
		return m_currentAssignment;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<GmodVersioningAssignmentChangeDto> GmodVersioningAssignmentChangeDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "GmodVersioningAssignmentChangeDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto oldElement = obj[OLD_ASSIGNMENT_KEY];
			if ( oldElement.error() )
			{
				SPDLOG_ERROR( "GmodVersioningAssignmentChangeDto: Missing '{}' field", OLD_ASSIGNMENT_KEY );
				return std::nullopt;
			}

			auto oldAssignmentOpt = safeGetString( oldElement.value() );
			if ( !oldAssignmentOpt )
			{
				SPDLOG_ERROR( "GmodVersioningAssignmentChangeDto: '{}' field is not a string", OLD_ASSIGNMENT_KEY );
				return std::nullopt;
			}

			auto currentElement = obj[CURRENT_ASSIGNMENT_KEY];
			if ( currentElement.error() )
			{
				SPDLOG_ERROR( "GmodVersioningAssignmentChangeDto: Missing '{}' field", CURRENT_ASSIGNMENT_KEY );
				return std::nullopt;
			}

			auto currentAssignmentOpt = safeGetString( currentElement.value() );
			if ( !currentAssignmentOpt )
			{
				SPDLOG_ERROR( "GmodVersioningAssignmentChangeDto: '{}' field is not a string", CURRENT_ASSIGNMENT_KEY );
				return std::nullopt;
			}

			std::string oldAssignment = internString( *oldAssignmentOpt );
			std::string currentAssignment = internString( *currentAssignmentOpt );

			if ( oldAssignment.empty() )
			{
				SPDLOG_WARN( "Empty 'oldAssignment' field found in GmodVersioningAssignmentChangeDto" );
			}
			if ( currentAssignment.empty() )
			{
				SPDLOG_WARN( "Empty 'currentAssignment' field found in GmodVersioningAssignmentChangeDto" );
			}

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed assignment change: {} → {} in {} µs", oldAssignment, currentAssignment, duration.count() );

			return GmodVersioningAssignmentChangeDto( std::move( oldAssignment ), std::move( currentAssignment ) );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during GmodVersioningAssignmentChangeDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<GmodVersioningAssignmentChangeDto> GmodVersioningAssignmentChangeDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	GmodVersioningAssignmentChangeDto GmodVersioningAssignmentChangeDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodVersioningAssignmentChangeDto from simdjson element" );
		}
		return std::move( *result );
	}

	GmodVersioningAssignmentChangeDto GmodVersioningAssignmentChangeDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodVersioningAssignmentChangeDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string GmodVersioningAssignmentChangeDto::toJsonString() const
	{
		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"" << OLD_ASSIGNMENT_KEY << "\": \"" << escapeJsonString( m_oldAssignment ) << "\",\n";
		oss << "  \"" << CURRENT_ASSIGNMENT_KEY << "\": \"" << escapeJsonString( m_currentAssignment ) << "\"\n";
		oss << "}";
		return oss.str();
	}

	//=====================================================================
	// GmodNodeConversionDto Implementation
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	GmodNodeConversionDto::GmodNodeConversionDto(
		OperationSet operations, std::string source,
		std::string target, std::string oldAssignment,
		std::string newAssignment, bool deleteAssignment )
		: m_operations{ std::move( operations ) },
		  m_source{ std::move( source ) },
		  m_target{ std::move( target ) },
		  m_oldAssignment{ std::move( oldAssignment ) },
		  m_newAssignment{ std::move( newAssignment ) },
		  m_deleteAssignment{ deleteAssignment }
	{
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const GmodNodeConversionDto::OperationSet& GmodNodeConversionDto::operations() const
	{
		return m_operations;
	}

	const std::string& GmodNodeConversionDto::source() const
	{
		return m_source;
	}

	const std::string& GmodNodeConversionDto::target() const
	{
		return m_target;
	}

	const std::string& GmodNodeConversionDto::oldAssignment() const
	{
		return m_oldAssignment;
	}

	const std::string& GmodNodeConversionDto::newAssignment() const
	{
		return m_newAssignment;
	}

	bool GmodNodeConversionDto::deleteAssignment() const
	{
		return m_deleteAssignment;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<GmodNodeConversionDto> GmodNodeConversionDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "GmodNodeConversionDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			OperationSet operations;
			auto operationsElement = obj[OPERATIONS_KEY];
			if ( !operationsElement.error() )
			{
				auto operationsArrayOpt = safeGetArray( operationsElement.value() );
				if ( operationsArrayOpt )
				{
					auto operationsArray = *operationsArrayOpt;
					operations.reserve( operationsArray.size() );

					for ( auto opElement : operationsArray )
					{
						auto opStrOpt = safeGetString( opElement );
						if ( opStrOpt )
						{
							operations.insert( internString( *opStrOpt ) );
						}
						else
						{
							SPDLOG_WARN( "Non-string operation in operations array, skipping" );
						}
					}
				}
				else
				{
					SPDLOG_WARN( "Operations field is not an array" );
				}
			}

			std::string source;
			auto sourceElement = obj[SOURCE_KEY];
			if ( !sourceElement.error() )
			{
				auto sourceOpt = safeGetString( sourceElement.value() );
				if ( sourceOpt )
				{
					source = internString( *sourceOpt );
				}
				else
				{
					SPDLOG_WARN( "Source field is not a string" );
				}
			}

			std::string target;
			auto targetElement = obj[TARGET_KEY];
			if ( !targetElement.error() )
			{
				auto targetOpt = safeGetString( targetElement.value() );
				if ( targetOpt )
				{
					target = internString( *targetOpt );
				}
				else
				{
					SPDLOG_WARN( "Target field is not a string" );
				}
			}

			std::string oldAssignment;
			auto oldAssignmentElement = obj[OLD_ASSIGNMENT_KEY];
			if ( !oldAssignmentElement.error() )
			{
				auto oldAssignmentOpt = safeGetString( oldAssignmentElement.value() );
				if ( oldAssignmentOpt )
				{
					oldAssignment = internString( *oldAssignmentOpt );
				}
				else
				{
					SPDLOG_WARN( "OldAssignment field is not a string" );
				}
			}

			std::string newAssignment;
			auto newAssignmentElement = obj[NEW_ASSIGNMENT_KEY];
			if ( !newAssignmentElement.error() )
			{
				auto newAssignmentOpt = safeGetString( newAssignmentElement.value() );
				if ( newAssignmentOpt )
				{
					newAssignment = internString( *newAssignmentOpt );
				}
				else
				{
					SPDLOG_WARN( "NewAssignment field is not a string" );
				}
			}

			bool deleteAssignment = false;
			auto deleteAssignmentElement = obj[DELETE_ASSIGNMENT_KEY];
			if ( !deleteAssignmentElement.error() )
			{
				auto deleteAssignmentOpt = safeGetBool( deleteAssignmentElement.value() );
				if ( deleteAssignmentOpt )
				{
					deleteAssignment = *deleteAssignmentOpt;
				}
				else
				{
					SPDLOG_WARN( "DeleteAssignment field is not a boolean, defaulting to false" );
				}
			}

			if ( operations.empty() )
			{
				SPDLOG_WARN( "Node conversion has no operations: source={}, target={}", source, target );
			}
			if ( source.empty() && target.empty() )
			{
				SPDLOG_WARN( "Node conversion has empty source and target" );
			}

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed node conversion: source={}, target={}, operations={} in {} µs", source, target, operations.size(), duration.count() );

			return GmodNodeConversionDto( std::move( operations ), std::move( source ), std::move( target ),
				std::move( oldAssignment ), std::move( newAssignment ), deleteAssignment );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during GmodNodeConversionDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<GmodNodeConversionDto> GmodNodeConversionDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	GmodNodeConversionDto GmodNodeConversionDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodNodeConversionDto from simdjson element" );
		}
		return std::move( *result );
	}

	GmodNodeConversionDto GmodNodeConversionDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodNodeConversionDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string GmodNodeConversionDto::toJsonString() const
	{
		auto startTime = std::chrono::steady_clock::now();

		std::ostringstream oss;
		oss << "{\n";

		oss << "  \"" << OPERATIONS_KEY << "\": [";
		bool first = true;
		for ( const auto& op : m_operations )
		{
			if ( !first )
				oss << ", ";
			first = false;
			oss << "\"" << escapeJsonString( op ) << "\"";
		}
		oss << "],\n";

		oss << "  \"" << SOURCE_KEY << "\": \"" << escapeJsonString( m_source ) << "\",\n";
		oss << "  \"" << TARGET_KEY << "\": \"" << escapeJsonString( m_target ) << "\",\n";
		oss << "  \"" << OLD_ASSIGNMENT_KEY << "\": \"" << escapeJsonString( m_oldAssignment ) << "\",\n";
		oss << "  \"" << NEW_ASSIGNMENT_KEY << "\": \"" << escapeJsonString( m_newAssignment ) << "\",\n";
		oss << "  \"" << DELETE_ASSIGNMENT_KEY << "\": " << ( m_deleteAssignment ? "true" : "false" ) << "\n";
		oss << "}";

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized node conversion in {} µs", duration.count() );

		return oss.str();
	}

	//=====================================================================
	// GmodVersioningDto Implementation
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	GmodVersioningDto::GmodVersioningDto( std::string visVersion, ItemsMap items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& GmodVersioningDto::visVersion() const
	{
		return m_visVersion;
	}

	const GmodVersioningDto::ItemsMap& GmodVersioningDto::items() const
	{
		return m_items;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<GmodVersioningDto> GmodVersioningDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "GmodVersioningDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto visElement = obj[VIS_RELEASE_KEY];
			if ( visElement.error() )
			{
				SPDLOG_ERROR( "GmodVersioningDto: Missing '{}' field", VIS_RELEASE_KEY );
				return std::nullopt;
			}

			auto visVersionOpt = safeGetString( visElement.value() );
			if ( !visVersionOpt )
			{
				SPDLOG_ERROR( "GmodVersioningDto: '{}' field is not a string", VIS_RELEASE_KEY );
				return std::nullopt;
			}

			std::string visVersion = *visVersionOpt;

			ItemsMap items;
			size_t successCount = 0;
			size_t emptyOperationsCount = 0;

			auto itemsElement = obj[ITEMS_KEY];
			if ( !itemsElement.error() )
			{
				auto itemsObjOpt = safeGetObject( itemsElement.value() );
				if ( itemsObjOpt )
				{
					auto itemsObj = *itemsObjOpt;
					size_t itemCount = itemsObj.size();

					if ( itemCount > 10000 )
					{
						SPDLOG_WARN( "Large versioning dataset detected ({}), consider performance implications", itemCount );
					}

					items.reserve( itemCount );
					auto parseStartTime = std::chrono::steady_clock::now();

					for ( auto [key, value] : itemsObj )
					{
						std::string nodeKey( key );
						auto nodeConversionOpt = GmodNodeConversionDto::tryFromJson( value );
						if ( nodeConversionOpt )
						{
							if ( nodeConversionOpt->operations().empty() )
							{
								emptyOperationsCount++;
							}
							items.emplace( std::move( nodeKey ), std::move( *nodeConversionOpt ) );
							successCount++;
						}
						else
						{
							SPDLOG_ERROR( "Error parsing conversion item '{}'", nodeKey );
						}
					}

					auto parseEndTime = std::chrono::steady_clock::now();
					auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>( parseEndTime - parseStartTime );

					if ( parseDuration.count() > 0 )
					{
						[[maybe_unused]] double parseRatePerSecond = static_cast<double>( successCount ) * 1000.0 / static_cast<double>( parseDuration.count() );
						SPDLOG_DEBUG( "Successfully parsed {}/{} node conversion items ({} with empty operations), rate: {:.1f} items/sec",
							successCount, itemCount, emptyOperationsCount, parseRatePerSecond );
					}
					else if ( itemCount > 0 )
					{
						SPDLOG_DEBUG( "Successfully parsed {}/{} node conversion items ({} with empty operations) very quickly.",
							successCount, itemCount, emptyOperationsCount );
					}

					if ( successCount > 0 && static_cast<double>( successCount ) < static_cast<double>( itemCount ) * 0.9 )
					{
						SPDLOG_DEBUG( "Optimizing memory usage after parsing {} of {} items", successCount, itemCount );
						items.rehash( static_cast<size_t>( static_cast<double>( successCount ) * 1.25 ) );
					}
				}
				else
				{
					SPDLOG_WARN( "'{}' field is not an object for VIS version {}", ITEMS_KEY, visVersion );
				}
			}
			else
			{
				SPDLOG_WARN( "No '{}' object found in GMOD versioning data for VIS version {}", ITEMS_KEY, visVersion );
			}

			if ( items.size() > 1000 )
			{
				[[maybe_unused]] size_t approxMemoryBytes = items.size() * ( sizeof( GmodNodeConversionDto ) + sizeof( std::string ) * 4 + 32 );
				SPDLOG_DEBUG( "Large versioning dataset loaded: {} items, ~{} MB estimated memory", items.size(), approxMemoryBytes / ( 1024 * 1024 ) );
			}

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "GMOD versioning parsing completed in {} ms ({} items)", duration.count(), items.size() );

			return GmodVersioningDto( std::move( visVersion ), std::move( items ) );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during GmodVersioningDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<GmodVersioningDto> GmodVersioningDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	GmodVersioningDto GmodVersioningDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodVersioningDto from simdjson element" );
		}
		return std::move( *result );
	}

	GmodVersioningDto GmodVersioningDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize GmodVersioningDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string GmodVersioningDto::toJsonString() const
	{
		auto startTime = std::chrono::steady_clock::now();

		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"" << VIS_RELEASE_KEY << "\": \"" << escapeJsonString( m_visVersion ) << "\"";

		if ( !m_items.empty() )
		{
			auto serializationStartTime = std::chrono::steady_clock::now();

			oss << ",\n  \"" << ITEMS_KEY << "\": {\n";
			bool first = true;
			size_t emptyOperationsCount = 0;

			for ( const auto& [key, value] : m_items )
			{
				if ( !first )
					oss << ",\n";
				first = false;

				if ( value.operations().empty() )
				{
					emptyOperationsCount++;
				}

				oss << "    \"" << escapeJsonString( key ) << "\": ";

				std::string itemJson = value.toJsonString();
				std::istringstream iss( itemJson );
				std::string line;
				bool firstLine = true;
				while ( std::getline( iss, line ) )
				{
					if ( !firstLine )
						oss << "\n      ";
					firstLine = false;
					oss << line;
				}
			}
			oss << "\n  }";

			auto serializationEndTime = std::chrono::steady_clock::now();
			auto serializationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
				serializationEndTime - serializationStartTime );

			if ( !m_items.empty() && serializationDuration.count() > 0 )
			{
				[[maybe_unused]] double serializationRatePerSecond = static_cast<double>( m_items.size() ) * 1000.0 / static_cast<double>( serializationDuration.count() );
				SPDLOG_DEBUG( "Node serialization rate: {:.1f} items/sec", serializationRatePerSecond );
			}

			if ( emptyOperationsCount > 0 )
			{
				SPDLOG_WARN( "{} nodes have no operations defined during serialization", emptyOperationsCount );
			}
		}
		else
		{
			oss << ",\n  \"" << ITEMS_KEY << "\": {}";
		}

		oss << "\n}";

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Successfully serialized GMOD versioning data for VIS {} in {} ms", m_visVersion, duration.count() );

		return oss.str();
	}
}
