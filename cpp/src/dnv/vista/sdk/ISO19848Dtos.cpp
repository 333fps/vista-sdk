/**
 * @file ISO19848Dtos.cpp
 * @brief Implementation of ISO 19848 data transfer objects
 */

#include "pch.h"

#include "dnv/vista/sdk/ISO19848Dtos.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr const char* VALUES_KEY = "values";
		static constexpr const char* TYPE_KEY = "type";
		static constexpr const char* DESCRIPTION_KEY = "description";
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
	//=====================================================================
	// Single Data Channel Type Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	DataChannelTypeNameDto::DataChannelTypeNameDto( std::string type, std::string description )
		: m_type{ std::move( type ) },
		  m_description{ std::move( description ) }
	{
		SPDLOG_INFO( "Creating DataChannelTypeNameDto: type={}, description={}", m_type, m_description );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& DataChannelTypeNameDto::type() const
	{
		return m_type;
	}

	const std::string& DataChannelTypeNameDto::description() const
	{
		return m_description;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<DataChannelTypeNameDto> DataChannelTypeNameDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse DataChannelTypeNameDto from nlohmann::json" );

		try
		{
			if ( !json.is_object() )
			{
				SPDLOG_ERROR( "JSON value for DataChannelTypeNameDto is not an object" );
				return std::nullopt;
			}

			DataChannelTypeNameDto dto = json.get<DataChannelTypeNameDto>();

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed DataChannelTypeNameDto: type={}, description={} in {} µs", dto.type(), dto.description(), duration.count() );

			return std::optional<DataChannelTypeNameDto>{ std::move( dto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "nlohmann::json exception during DataChannelTypeNameDto parsing: {}", ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Standard exception during DataChannelTypeNameDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	DataChannelTypeNameDto DataChannelTypeNameDto::fromJson( const nlohmann::json& json )
	{
		try
		{
			return json.get<DataChannelTypeNameDto>();
		}
		catch ( const nlohmann::json::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize DataChannelTypeNameDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		catch ( const std::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize DataChannelTypeNameDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	nlohmann::json DataChannelTypeNameDto::toJson() const
	{
		SPDLOG_DEBUG( "Serializing DataChannelTypeNameDto: type={}", m_type );
		nlohmann::json j = *this;
		return j;
	}

	void to_json( nlohmann::json& j, const DataChannelTypeNameDto& dto )
	{
		j = nlohmann::json{
			{ TYPE_KEY, dto.m_type },
			{ DESCRIPTION_KEY, dto.m_description } };
	}

	//----------------------------------------------
	// Private Serialization Methods
	//----------------------------------------------

	void from_json( const nlohmann::json& j, DataChannelTypeNameDto& dto )
	{
		if ( !j.contains( TYPE_KEY ) || !j.at( TYPE_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "DataChannelTypeNameDto JSON missing required '{}' field or not a string", TYPE_KEY ), nullptr );
		}
		if ( j.contains( DESCRIPTION_KEY ) && !j.at( DESCRIPTION_KEY ).is_string() )
		{
			throw nlohmann::json::type_error::create( 302, fmt::format( "DataChannelTypeNameDto JSON field '{}' is not a string", DESCRIPTION_KEY ), nullptr );
		}

		dto.m_type = internString( j.at( TYPE_KEY ).get<std::string>() );

		if ( j.contains( DESCRIPTION_KEY ) )
		{
			dto.m_description = internString( j.at( DESCRIPTION_KEY ).get<std::string>() );
		}
		else
		{
			dto.m_description.clear();
			SPDLOG_DEBUG( "DataChannelTypeNameDto JSON missing optional '{}' field for type '{}'", DESCRIPTION_KEY, dto.m_type );
		}

		if ( dto.m_type.empty() )
		{
			SPDLOG_WARN( "Parsed DataChannelTypeNameDto has empty type field" );
		}
	}

	//=====================================================================
	// Collection of Data Channel Type Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	DataChannelTypeNamesDto::DataChannelTypeNamesDto( std::vector<DataChannelTypeNameDto> values )
		: m_values{ std::move( values ) }
	{
		SPDLOG_INFO( "Creating DataChannelTypeNamesDto with {} values", m_values.size() );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::vector<DataChannelTypeNameDto>& DataChannelTypeNamesDto::values() const
	{
		return m_values;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<DataChannelTypeNamesDto> DataChannelTypeNamesDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse data channel type names from nlohmann::json" );

		try
		{
			if ( !json.is_object() )
			{
				SPDLOG_ERROR( "JSON value for DataChannelTypeNamesDto is not an object" );
				return std::nullopt;
			}

			DataChannelTypeNamesDto dto = json.get<DataChannelTypeNamesDto>();

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_INFO( "Parsed DataChannelTypeNamesDto with {} values in {} ms", dto.values().size(), duration.count() );

			return std::optional<DataChannelTypeNamesDto>{ std::move( dto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "nlohmann::json exception during DataChannelTypeNamesDto parsing: {}", ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Standard exception during DataChannelTypeNamesDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	DataChannelTypeNamesDto DataChannelTypeNamesDto::fromJson( const nlohmann::json& json )
	{
		try
		{
			return json.get<DataChannelTypeNamesDto>();
		}
		catch ( const nlohmann::json::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize DataChannelTypeNamesDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		catch ( const std::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize DataChannelTypeNamesDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	nlohmann::json DataChannelTypeNamesDto::toJson() const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Serializing {} data channel type names to nlohmann::json", m_values.size() );
		nlohmann::json j = *this;
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized {} data channel type names in {}ms", m_values.size(), duration.count() );
		return j;
	}

	//----------------------------------------------
	// Private Serialization Methods
	//----------------------------------------------

	void to_json( nlohmann::json& j, const DataChannelTypeNamesDto& dto )
	{
		j = nlohmann::json{ { VALUES_KEY, dto.m_values } };
	}

	void from_json( const nlohmann::json& j, DataChannelTypeNamesDto& dto )
	{
		dto.m_values.clear();

		if ( j.contains( VALUES_KEY ) )
		{
			if ( !j.at( VALUES_KEY ).is_array() )
			{
				throw nlohmann::json::type_error::create( 302, fmt::format( "DataChannelTypeNamesDto JSON field '{}' is not an array", VALUES_KEY ), nullptr );
			}

			const auto& jsonArray = j.at( VALUES_KEY );
			size_t valueCount = jsonArray.size();
			SPDLOG_INFO( "Found {} data channel type entries to parse", valueCount );

			dto.m_values.reserve( valueCount );
			size_t successCount = 0;
			auto parseStartTime = std::chrono::steady_clock::now();

			for ( const auto& itemJson : jsonArray )
			{
				try
				{
					dto.m_values.emplace_back( itemJson.get<DataChannelTypeNameDto>() );
					successCount++;
				}
				catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
				{
					SPDLOG_ERROR( "Skipping malformed data channel type name at index {}: {}", successCount, ex.what() );
				}
				catch ( [[maybe_unused]] const std::exception& ex )
				{
					SPDLOG_ERROR( "Standard exception parsing data channel type name at index {}: {}", successCount, ex.what() );
				}
			}

			auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - parseStartTime );

			if ( valueCount > 0 && parseDuration.count() > 0 )
			{
				[[maybe_unused]] double rate = static_cast<double>( successCount ) * 1000.0 / static_cast<double>( parseDuration.count() );
				SPDLOG_INFO( "Successfully parsed {}/{} data channel type names in {}ms ({:.1f} items/sec)",
					successCount, valueCount, parseDuration.count(), rate );
			}
			else if ( valueCount > 0 )
			{
				SPDLOG_INFO( "Successfully parsed {}/{} data channel type names very quickly.", successCount, valueCount );
			}

			if ( dto.m_values.size() > 1000 )
			{
				[[maybe_unused]] size_t approxBytes = estimateMemoryUsage( dto.m_values );
				SPDLOG_INFO( "Large collection loaded: {} items, ~{} KB estimated memory", dto.m_values.size(), approxBytes / 1024 );
			}
		}
		else
		{
			SPDLOG_WARN( "No '{}' array found in data channel type names JSON", VALUES_KEY );
		}
	}

	//=====================================================================
	// Single Format Data Type Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	FormatDataTypeDto::FormatDataTypeDto( std::string type, std::string description )
		: m_type{ std::move( type ) },
		  m_description{ std::move( description ) }
	{
		SPDLOG_INFO( "Creating FormatDataTypeDto: type={}, description={}", m_type, m_description );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& FormatDataTypeDto::type() const
	{
		return m_type;
	}

	const std::string& FormatDataTypeDto::description() const
	{
		return m_description;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<FormatDataTypeDto> FormatDataTypeDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse FormatDataTypeDto from nlohmann::json" );

		try
		{
			if ( !json.is_object() )
			{
				SPDLOG_ERROR( "JSON value for FormatDataTypeDto is not an object" );
				return std::nullopt;
			}

			FormatDataTypeDto dto = json.get<FormatDataTypeDto>();

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed FormatDataTypeDto: type={}, description={} in {} µs", dto.type(), dto.description(), duration.count() );

			return std::optional<FormatDataTypeDto>{ std::move( dto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "nlohmann::json exception during FormatDataTypeDto parsing: {}", ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Standard exception during FormatDataTypeDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	FormatDataTypeDto FormatDataTypeDto::fromJson( const nlohmann::json& json )
	{
		try
		{
			return json.get<FormatDataTypeDto>();
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize FormatDataTypeDto from JSON: {}", ex.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize FormatDataTypeDto from JSON: {}", ex.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	nlohmann::json FormatDataTypeDto::toJson() const
	{
		SPDLOG_DEBUG( "Serializing FormatDataTypeDto: type={}", m_type );
		nlohmann::json j = *this;
		return j;
	}

	//----------------------------------------------
	// Private Serialization Methods
	//----------------------------------------------

	void to_json( nlohmann::json& j, const FormatDataTypeDto& dto )
	{
		j = nlohmann::json{
			{ TYPE_KEY, dto.m_type },
			{ DESCRIPTION_KEY, dto.m_description } };
	}

	void from_json( const nlohmann::json& j, FormatDataTypeDto& dto )
	{
		if ( !j.contains( TYPE_KEY ) || !j.at( TYPE_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "FormatDataTypeDto JSON missing required '{}' field or not a string", TYPE_KEY ), nullptr );
		}
		if ( j.contains( DESCRIPTION_KEY ) && !j.at( DESCRIPTION_KEY ).is_string() )
		{
			throw nlohmann::json::type_error::create( 302, fmt::format( "FormatDataTypeDto JSON field '{}' is not a string", DESCRIPTION_KEY ), nullptr );
		}

		dto.m_type = internString( j.at( TYPE_KEY ).get<std::string>() );

		if ( j.contains( DESCRIPTION_KEY ) )
		{
			dto.m_description = internString( j.at( DESCRIPTION_KEY ).get<std::string>() );
		}
		else
		{
			dto.m_description.clear();
			SPDLOG_DEBUG( "FormatDataTypeDto JSON missing optional '{}' field for type '{}'", DESCRIPTION_KEY, dto.m_type );
		}

		if ( dto.m_type.empty() )
		{
			SPDLOG_WARN( "Parsed FormatDataTypeDto has empty type field" );
		}
	}

	//=====================================================================
	// Collection of  Format Data Type Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	FormatDataTypesDto::FormatDataTypesDto( std::vector<FormatDataTypeDto> values )
		: m_values{ std::move( values ) }
	{
		SPDLOG_INFO( "Creating FormatDataTypesDto with {} values", m_values.size() );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::vector<FormatDataTypeDto>& FormatDataTypesDto::values() const
	{
		return m_values;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<FormatDataTypesDto> FormatDataTypesDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse format data types from nlohmann::json" );

		try
		{
			if ( !json.is_object() )
			{
				SPDLOG_ERROR( "JSON value for FormatDataTypesDto is not an object" );
				return std::nullopt;
			}

			FormatDataTypesDto dto = json.get<FormatDataTypesDto>();

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_INFO( "Parsed FormatDataTypesDto with {} values in {} ms", dto.values().size(), duration.count() );

			return std::optional<FormatDataTypesDto>{ std::move( dto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "nlohmann::json exception during FormatDataTypesDto parsing: {}", ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Standard exception during FormatDataTypesDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	FormatDataTypesDto FormatDataTypesDto::fromJson( const nlohmann::json& json )
	{
		try
		{
			return json.get<FormatDataTypesDto>();
		}
		catch ( const nlohmann::json::exception& ex )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize FormatDataTypesDto from JSON: {}", ex.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		catch ( const std::exception& ex )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize FormatDataTypesDto from JSON: {}", ex.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	nlohmann::json FormatDataTypesDto::toJson() const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Serializing {} format data types to nlohmann::json", m_values.size() );
		nlohmann::json j = *this;
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized {} format data types in {}ms", m_values.size(), duration.count() );
		return j;
	}

	//----------------------------------------------
	// Private Serialization Methods
	//----------------------------------------------

	void to_json( nlohmann::json& j, const FormatDataTypesDto& dto )
	{
		j = nlohmann::json{ { VALUES_KEY, dto.m_values } };
	}

	void from_json( const nlohmann::json& j, FormatDataTypesDto& dto )
	{
		dto.m_values.clear();

		if ( j.contains( VALUES_KEY ) )
		{
			if ( !j.at( VALUES_KEY ).is_array() )
			{
				throw nlohmann::json::type_error::create( 302, fmt::format( "FormatDataTypesDto JSON field '{}' is not an array", VALUES_KEY ), nullptr );
			}

			const auto& jsonArray = j.at( VALUES_KEY );
			size_t valueCount = jsonArray.size();
			SPDLOG_INFO( "Found {} format data type entries to parse", valueCount );

			dto.m_values.reserve( valueCount );
			size_t successCount = 0;
			auto parseStartTime = std::chrono::steady_clock::now();

			for ( const auto& itemJson : jsonArray )
			{
				try
				{
					dto.m_values.emplace_back( itemJson.get<FormatDataTypeDto>() );
					successCount++;
				}
				catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
				{
					SPDLOG_ERROR( "Skipping malformed format data type at index {}: {}", successCount, ex.what() );
				}
				catch ( [[maybe_unused]] const std::exception& ex )
				{
					SPDLOG_ERROR( "Standard exception parsing format data type at index {}: {}", successCount, ex.what() );
				}
			}

			auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - parseStartTime );

			if ( valueCount > 0 && parseDuration.count() > 0 )
			{
				[[maybe_unused]] double rate = static_cast<double>( successCount ) * 1000.0 / static_cast<double>( parseDuration.count() );
				SPDLOG_INFO( "Successfully parsed {}/{} format data types in {}ms ({:.1f} items/sec)",
					successCount, valueCount, parseDuration.count(), rate );
			}
			else if ( valueCount > 0 )
			{
				SPDLOG_INFO( "Successfully parsed {}/{} format data types very quickly.", successCount, valueCount );
			}

			if ( dto.m_values.size() > 1000 )
			{
				[[maybe_unused]] size_t approxBytes = estimateMemoryUsage( dto.m_values );
				SPDLOG_INFO( "Large collection loaded: {} items, ~{} KB estimated memory", dto.m_values.size(), approxBytes / 1024 );
			}
		}
		else
		{
			SPDLOG_WARN( "No '{}' array found in format data types JSON", VALUES_KEY );
		}
	}
}
