/**
 * @file LocationsDto.cpp
 * @brief Implementation of Data Transfer Objects for locations in the VIS standard
 */

#include "pch.h"

#include "dnv/vista/sdk/LocationsDto.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	static constexpr const char* CODE_KEY = "code";
	static constexpr const char* NAME_KEY = "name";
	static constexpr const char* DEFINITION_KEY = "definition";
	static constexpr const char* VIS_RELEASE_KEY = "visRelease";
	static constexpr const char* ITEMS_KEY = "items";

	//=====================================================================
	// Helper Functions
	//=====================================================================

	static const std::string& internString( const std::string& value )
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

	template <typename T>
	size_t estimateMemoryUsage( const std::vector<T>& collection )
	{
		return sizeof( std::vector<T> ) + collection.capacity() * sizeof( T );
	}
}

namespace dnv::vista::sdk
{
	//=====================================================================
	// Relative Location Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	RelativeLocationsDto::RelativeLocationsDto( char code, std::string name, std::optional<std::string> definition )
		: m_code{ code },
		  m_name{ std::move( name ) },
		  m_definition{ std::move( definition ) }
	{
		SPDLOG_INFO( "RelativeLocationsDto constructed: code={}, name={}, has_definition={}",
			m_code, m_name, m_definition.has_value() );
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

	std::optional<RelativeLocationsDto> RelativeLocationsDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse RelativeLocationsDto from nlohmann::json" );

		try
		{
			if ( !json.is_object() )
			{
				SPDLOG_ERROR( "JSON value for RelativeLocationsDto is not an object" );
				return std::nullopt;
			}

			RelativeLocationsDto dto = json.get<RelativeLocationsDto>();

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed RelativeLocationsDto: code={}, name={} in {} µs", dto.code(), dto.name(), duration.count() );

			return std::optional<RelativeLocationsDto>{ std::move( dto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "nlohmann::json exception during RelativeLocationsDto parsing: {}", ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Standard exception during RelativeLocationsDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	RelativeLocationsDto RelativeLocationsDto::fromJson( const nlohmann::json& json )
	{
		try
		{
			return json.get<RelativeLocationsDto>();
		}
		catch ( const nlohmann::json::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize RelativeLocationsDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		catch ( const std::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize RelativeLocationsDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	//----------------------------------------------
	// Private Serialization Methods
	//---------------------------------------------

	nlohmann::json RelativeLocationsDto::toJson() const
	{
		SPDLOG_DEBUG( "Serializing RelativeLocationsDto: code={}, name={}", m_code, m_name );
		nlohmann::json j = *this;
		return j;
	}

	void to_json( nlohmann::json& j, const RelativeLocationsDto& dto )
	{
		j = nlohmann::json{
			{ CODE_KEY, std::string( 1, dto.m_code ) },
			{ NAME_KEY, dto.m_name } };
		if ( dto.m_definition.has_value() )
		{
			j[DEFINITION_KEY] = dto.m_definition.value();
		}
	}

	void from_json( const nlohmann::json& j, RelativeLocationsDto& dto )
	{
		if ( !j.contains( CODE_KEY ) || !j.at( CODE_KEY ).is_string() || j.at( CODE_KEY ).get<std::string>().empty() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "RelativeLocationsDto JSON missing required '{}' field, not a string, or empty", CODE_KEY ), nullptr );
		}
		if ( !j.contains( NAME_KEY ) || !j.at( NAME_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "RelativeLocationsDto JSON missing required '{}' field or not a string", NAME_KEY ), nullptr );
		}
		if ( j.contains( DEFINITION_KEY ) && !j.at( DEFINITION_KEY ).is_string() )
		{
			throw nlohmann::json::type_error::create( 302, fmt::format( "RelativeLocationsDto JSON field '{}' is not a string", DEFINITION_KEY ), nullptr );
		}

		std::string codeStr = j.at( CODE_KEY ).get<std::string>();
		if ( codeStr.length() != 1 )
		{
			throw nlohmann::json::type_error::create( 302, fmt::format( "RelativeLocationsDto JSON field '{}' must be a single character string", CODE_KEY ), nullptr );
		}
		dto.m_code = codeStr[0];

		dto.m_name = internString( j.at( NAME_KEY ).get<std::string>() );

		if ( j.contains( DEFINITION_KEY ) )
		{
			dto.m_definition = internString( j.at( DEFINITION_KEY ).get<std::string>() );
		}
		else
		{
			dto.m_definition.reset();
			SPDLOG_DEBUG( "RelativeLocationsDto JSON missing optional '{}' field for code '{}'", DEFINITION_KEY, dto.m_code );
		}

		if ( dto.m_name.empty() )
		{
			SPDLOG_WARN( "Parsed RelativeLocationsDto has empty name field for code '{}'", dto.m_code );
		}
	}

	//=====================================================================
	// Location Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	LocationsDto::LocationsDto( std::string visVersion, std::vector<RelativeLocationsDto> items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
		SPDLOG_INFO( "LocationsDto constructed: visVersion={}, items.size={}",
			m_visVersion, m_items.size() );
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

	std::optional<LocationsDto> LocationsDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse LocationsDto from nlohmann::json" );

		try
		{
			if ( !json.is_object() )
			{
				SPDLOG_ERROR( "JSON value for LocationsDto is not an object" );
				return std::nullopt;
			}

			LocationsDto dto = json.get<LocationsDto>();

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_INFO( "Parsed LocationsDto for VIS {} with {} items in {} ms", dto.visVersion(), dto.items().size(), duration.count() );

			return std::optional<LocationsDto>{ std::move( dto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "nlohmann::json exception during LocationsDto parsing: {}", ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Standard exception during LocationsDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	LocationsDto LocationsDto::fromJson( const nlohmann::json& json )
	{
		try
		{
			return json.get<LocationsDto>();
		}
		catch ( const nlohmann::json::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize LocationsDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		catch ( const std::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize LocationsDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	nlohmann::json LocationsDto::toJson() const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Serializing LocationsDto: visVersion={}, items={}", m_visVersion, m_items.size() );
		nlohmann::json j = *this;
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized {} locations in {}ms", m_items.size(), duration.count() );
		return j;
	}

	//----------------------------------------------
	// Private Serialization Methods
	//----------------------------------------------

	void to_json( nlohmann::json& j, const LocationsDto& dto )
	{
		j = nlohmann::json{
			{ VIS_RELEASE_KEY, dto.m_visVersion },
			{ ITEMS_KEY, dto.m_items } };
	}

	void from_json( const nlohmann::json& j, LocationsDto& dto )
	{
		dto.m_items.clear();

		if ( !j.contains( VIS_RELEASE_KEY ) || !j.at( VIS_RELEASE_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "LocationsDto JSON missing required '{}' field or not a string", VIS_RELEASE_KEY ), nullptr );
		}
		if ( !j.contains( ITEMS_KEY ) || !j.at( ITEMS_KEY ).is_array() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "LocationsDto JSON missing required '{}' field or not an array", ITEMS_KEY ), nullptr );
		}

		dto.m_visVersion = internString( j.at( VIS_RELEASE_KEY ).get<std::string>() );
		SPDLOG_INFO( "Parsing locations for VIS version: {}", dto.m_visVersion );

		const auto& jsonArray = j.at( ITEMS_KEY );
		size_t itemCount = jsonArray.size();
		SPDLOG_INFO( "Found {} location items to parse", itemCount );

		dto.m_items.reserve( itemCount );
		size_t successCount = 0;
		auto parseStartTime = std::chrono::steady_clock::now();

		for ( const auto& itemJson : jsonArray )
		{
			try
			{
				dto.m_items.emplace_back( itemJson.get<RelativeLocationsDto>() );
				successCount++;
			}
			catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
			{
				SPDLOG_ERROR( "Skipping malformed location item at index {}: {}", successCount, ex.what() );
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "Standard exception parsing location item at index {}: {}", successCount, ex.what() );
			}
		}

		auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - parseStartTime );

		if ( itemCount > 0 && parseDuration.count() > 0 )
		{
			[[maybe_unused]] double rate = static_cast<double>( successCount ) * 1000.0 / static_cast<double>( parseDuration.count() );
			SPDLOG_INFO( "Successfully parsed {}/{} locations in {}ms ({:.1f} items/sec)",
				successCount, itemCount, parseDuration.count(), rate );
		}
		else if ( itemCount > 0 )
		{
			SPDLOG_INFO( "Successfully parsed {}/{} locations very quickly.", successCount, itemCount );
		}

		if ( dto.m_items.size() > 1000 )
		{
			[[maybe_unused]] size_t approxBytes = estimateMemoryUsage( dto.m_items );
			SPDLOG_INFO( "Large location collection loaded: {} items, ~{} KB estimated memory", dto.m_items.size(), approxBytes / 1024 );
		}

		if ( successCount < itemCount )
		{
			double errorRate = static_cast<double>( itemCount - successCount ) * 100.0 / static_cast<double>( itemCount );
			SPDLOG_WARN( "Location parsing had {:.1f}% error rate ({} failed items)",
				errorRate, itemCount - successCount );

			if ( errorRate > 20.0 )
			{
				SPDLOG_ERROR( "High error rate in location data suggests possible format issue" );
			}
		}
	}
}
