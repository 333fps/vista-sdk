/**
 * @file LocationsDto.cpp
 * @brief Implementation of Data Transfer Objects for locations in the VIS standard
 */

#include "pch.h"

#include "dnv/vista/sdk/LocationsDto.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr std::string_view CODE_KEY = "code";
		static constexpr std::string_view NAME_KEY = "name";
		static constexpr std::string_view DEFINITION_KEY = "definition";
		static constexpr std::string_view VIS_RELEASE_KEY = "visRelease";
		static constexpr std::string_view ITEMS_KEY = "items";

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
		 * @brief String interning for memory optimization
		 */
		const std::string& internString( const std::string& value )
		{
			static std::unordered_map<std::string, std::string> cache;
			static size_t hits = 0, misses = 0, calls = 0;
			calls++;

			if ( value.size() > 22 ) // Common SSO threshold
			{
				auto it = cache.find( value );
				if ( it != cache.end() )
				{
					hits++;
					if ( calls % 10000 == 0 )
					{
						SPDLOG_DEBUG( "String interning stats: {:.1f}% hit rate ({}/{}), {} unique strings",
							hits * 100.0 / calls, hits, calls, cache.size() );
					}
					return it->second;
				}

				misses++;
				return cache.emplace( value, value ).first->first;
			}

			return value;
		}

		/**
		 * @brief Estimate memory usage of collections
		 */
		template <typename T>
		size_t estimateMemoryUsage( const std::vector<T>& collection )
		{
			return sizeof( std::vector<T> ) + collection.capacity() * sizeof( T );
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
	// Relative Location Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	RelativeLocationsDto::RelativeLocationsDto( char code, std::string name, std::optional<std::string> definition )
		: m_code{ code },
		  m_name{ std::move( name ) },
		  m_definition{ std::move( definition ) }
	{
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	char RelativeLocationsDto::code() const
	{
		return m_code;
	}

	const std::string& RelativeLocationsDto::name() const
	{
		return m_name;
	}

	const std::optional<std::string>& RelativeLocationsDto::definition() const
	{
		return m_definition;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<RelativeLocationsDto> RelativeLocationsDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "RelativeLocationsDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto codeElement = obj[CODE_KEY];
			if ( codeElement.error() )
			{
				SPDLOG_ERROR( "RelativeLocationsDto JSON missing required '{}' field", CODE_KEY );
				return std::nullopt;
			}

			auto codeOpt = safeGetString( codeElement.value() );
			if ( !codeOpt || codeOpt->empty() )
			{
				SPDLOG_ERROR( "RelativeLocationsDto JSON field '{}' is not a string or is empty", CODE_KEY );
				return std::nullopt;
			}

			if ( codeOpt->length() != 1 )
			{
				SPDLOG_ERROR( "RelativeLocationsDto JSON field '{}' must be a single character string", CODE_KEY );
				return std::nullopt;
			}

			char code = ( *codeOpt )[0];

			auto nameElement = obj[NAME_KEY];
			if ( nameElement.error() )
			{
				SPDLOG_ERROR( "RelativeLocationsDto JSON missing required '{}' field", NAME_KEY );
				return std::nullopt;
			}

			auto nameOpt = safeGetString( nameElement.value() );
			if ( !nameOpt )
			{
				SPDLOG_ERROR( "RelativeLocationsDto JSON field '{}' is not a string", NAME_KEY );
				return std::nullopt;
			}

			std::string name = internString( *nameOpt );

			std::optional<std::string> definition;
			auto definitionElement = obj[DEFINITION_KEY];
			if ( !definitionElement.error() )
			{
				auto definitionOpt = safeGetString( definitionElement.value() );
				if ( definitionOpt )
				{
					definition = internString( *definitionOpt );
				}
				else if ( definitionElement.value().type() != simdjson::dom::element_type::NULL_VALUE )
				{
					SPDLOG_WARN( "RelativeLocationsDto has non-string '{}' field", DEFINITION_KEY );
				}
			}

			if ( name.empty() )
			{
				SPDLOG_WARN( "Parsed RelativeLocationsDto has empty name field for code '{}'", code );
			}

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed RelativeLocationsDto: code={}, name={} in {} µs", code, name, duration.count() );

			return RelativeLocationsDto( code, std::move( name ), std::move( definition ) );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during RelativeLocationsDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<RelativeLocationsDto> RelativeLocationsDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	RelativeLocationsDto RelativeLocationsDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize RelativeLocationsDto from simdjson element" );
		}
		return std::move( *result );
	}

	RelativeLocationsDto RelativeLocationsDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize RelativeLocationsDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string RelativeLocationsDto::toJsonString() const
	{
		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"" << CODE_KEY << "\": \"" << escapeJsonString( std::string( 1, m_code ) ) << "\"";
		oss << ",\n  \"" << NAME_KEY << "\": \"" << escapeJsonString( m_name ) << "\"";
		if ( m_definition.has_value() )
		{
			oss << ",\n  \"" << DEFINITION_KEY << "\": \"" << escapeJsonString( *m_definition ) << "\"";
		}
		oss << "\n}";
		return oss.str();
	}

	//=====================================================================
	// Location Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	LocationsDto::LocationsDto( std::string visVersion, std::vector<RelativeLocationsDto> items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& LocationsDto::visVersion() const
	{
		return m_visVersion;
	}

	const std::vector<RelativeLocationsDto>& LocationsDto::items() const
	{
		return m_items;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<LocationsDto> LocationsDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "LocationsDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto visReleaseElement = obj[VIS_RELEASE_KEY];
			if ( visReleaseElement.error() )
			{
				SPDLOG_ERROR( "LocationsDto JSON missing required '{}' field", VIS_RELEASE_KEY );
				return std::nullopt;
			}

			auto visVersionOpt = safeGetString( visReleaseElement.value() );
			if ( !visVersionOpt )
			{
				SPDLOG_ERROR( "LocationsDto JSON field '{}' is not a string", VIS_RELEASE_KEY );
				return std::nullopt;
			}

			std::string visVersion = internString( *visVersionOpt );

			std::vector<RelativeLocationsDto> items;
			auto itemsElement = obj[ITEMS_KEY];
			if ( itemsElement.error() )
			{
				SPDLOG_ERROR( "LocationsDto JSON missing required '{}' field", ITEMS_KEY );
				return std::nullopt;
			}

			auto itemsArrayOpt = safeGetArray( itemsElement.value() );
			if ( !itemsArrayOpt )
			{
				SPDLOG_ERROR( "LocationsDto JSON field '{}' is not an array", ITEMS_KEY );
				return std::nullopt;
			}

			auto itemsArray = *itemsArrayOpt;
			size_t itemCount = itemsArray.size();
			items.reserve( itemCount );
			size_t successCount = 0;
			auto parseStartTime = std::chrono::steady_clock::now();

			for ( auto itemElement : itemsArray )
			{
				auto itemOpt = RelativeLocationsDto::tryFromJson( itemElement );
				if ( itemOpt )
				{
					items.emplace_back( std::move( *itemOpt ) );
					successCount++;
				}
				else
				{
					SPDLOG_ERROR( "Skipping malformed location item at index {}", successCount );
				}
			}

			auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - parseStartTime );

			if ( itemCount > 0 && parseDuration.count() > 0 )
			{
				[[maybe_unused]] double rate = static_cast<double>( successCount ) * 1000.0 / static_cast<double>( parseDuration.count() );
				SPDLOG_DEBUG( "Successfully parsed {}/{} locations in {}ms ({:.1f} items/sec)",
					successCount, itemCount, parseDuration.count(), rate );
			}

			if ( items.size() > 1000 )
			{
				[[maybe_unused]] size_t approxBytes = estimateMemoryUsage( items );
				SPDLOG_DEBUG( "Large location collection loaded: {} items, ~{} KB estimated memory", items.size(), approxBytes / 1024 );
			}

			if ( successCount < itemCount )
			{
				[[maybe_unused]] double errorRate = static_cast<double>( itemCount - successCount ) * 100.0 / static_cast<double>( itemCount );
				SPDLOG_WARN( "Location parsing had {:.1f}% error rate ({} failed items)",
					errorRate, itemCount - successCount );

				if ( errorRate > 20.0 )
				{
					SPDLOG_WARN( "High error rate in location data suggests possible format issue" );
				}
			}

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed LocationsDto for VIS {} with {} items in {} ms", visVersion, items.size(), duration.count() );

			return LocationsDto( std::move( visVersion ), std::move( items ) );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during LocationsDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<LocationsDto> LocationsDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	LocationsDto LocationsDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize LocationsDto from simdjson element" );
		}
		return std::move( *result );
	}

	LocationsDto LocationsDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize LocationsDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string LocationsDto::toJsonString() const
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

		oss << "]\n}";

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized {} locations in {}ms", m_items.size(), duration.count() );

		return oss.str();
	}
}
