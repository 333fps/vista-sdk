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

		static constexpr std::string_view VALUES_KEY = "values";
		static constexpr std::string_view TYPE_KEY = "type";
		static constexpr std::string_view DESCRIPTION_KEY = "description";

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
	// Single Data Channel Type Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	DataChannelTypeNameDto::DataChannelTypeNameDto( std::string type, std::string description )
		: m_type{ std::move( type ) },
		  m_description{ std::move( description ) }
	{
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

	std::optional<DataChannelTypeNameDto> DataChannelTypeNameDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "DataChannelTypeNameDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto typeElement = obj[TYPE_KEY];
			if ( typeElement.error() )
			{
				SPDLOG_ERROR( "DataChannelTypeNameDto JSON missing required '{}' field", TYPE_KEY );
				return std::nullopt;
			}

			auto typeOpt = safeGetString( typeElement.value() );
			if ( !typeOpt )
			{
				SPDLOG_ERROR( "DataChannelTypeNameDto JSON field '{}' is not a string", TYPE_KEY );
				return std::nullopt;
			}

			std::string type = internString( *typeOpt );

			std::string description;
			auto descriptionElement = obj[DESCRIPTION_KEY];
			if ( !descriptionElement.error() )
			{
				auto descriptionOpt = safeGetString( descriptionElement.value() );
				if ( descriptionOpt )
				{
					description = internString( *descriptionOpt );
				}
				else if ( descriptionElement.value().type() != simdjson::dom::element_type::NULL_VALUE )
				{
					SPDLOG_WARN( "DataChannelTypeNameDto has non-string '{}' field", DESCRIPTION_KEY );
				}
			}
			else
			{
				SPDLOG_DEBUG( "DataChannelTypeNameDto JSON missing optional '{}' field for type '{}'", DESCRIPTION_KEY, type );
			}

			if ( type.empty() )
			{
				SPDLOG_WARN( "Parsed DataChannelTypeNameDto has empty type field" );
			}

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed DataChannelTypeNameDto: type={}, description={} in {} µs", type, description, duration.count() );

			return DataChannelTypeNameDto( std::move( type ), std::move( description ) );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during DataChannelTypeNameDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<DataChannelTypeNameDto> DataChannelTypeNameDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	DataChannelTypeNameDto DataChannelTypeNameDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize DataChannelTypeNameDto from simdjson element" );
		}
		return std::move( *result );
	}

	DataChannelTypeNameDto DataChannelTypeNameDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize DataChannelTypeNameDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string DataChannelTypeNameDto::toJsonString() const
	{
		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"" << TYPE_KEY << "\": \"" << escapeJsonString( m_type ) << "\"";
		if ( !m_description.empty() )
		{
			oss << ",\n  \"" << DESCRIPTION_KEY << "\": \"" << escapeJsonString( m_description ) << "\"";
		}
		oss << "\n}";
		return oss.str();
	}

	//=====================================================================
	// Collection of Data Channel Type Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	DataChannelTypeNamesDto::DataChannelTypeNamesDto( std::vector<DataChannelTypeNameDto> values )
		: m_values{ std::move( values ) }
	{
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

	std::optional<DataChannelTypeNamesDto> DataChannelTypeNamesDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "DataChannelTypeNamesDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			std::vector<DataChannelTypeNameDto> values;
			auto valuesElement = obj[VALUES_KEY];
			if ( !valuesElement.error() )
			{
				auto valuesArrayOpt = safeGetArray( valuesElement.value() );
				if ( valuesArrayOpt )
				{
					auto valuesArray = *valuesArrayOpt;
					size_t valueCount = valuesArray.size();
					values.reserve( valueCount );
					size_t successCount = 0;
					auto parseStartTime = std::chrono::steady_clock::now();

					for ( auto itemElement : valuesArray )
					{
						auto itemOpt = DataChannelTypeNameDto::tryFromJson( itemElement );
						if ( itemOpt )
						{
							values.emplace_back( std::move( *itemOpt ) );
							successCount++;
						}
						else
						{
							SPDLOG_ERROR( "Skipping malformed data channel type name at index {}", successCount );
						}
					}

					auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - parseStartTime );

					if ( valueCount > 0 && parseDuration.count() > 0 )
					{
						[[maybe_unused]] double rate = static_cast<double>( successCount ) * 1000.0 / static_cast<double>( parseDuration.count() );
						SPDLOG_DEBUG( "Successfully parsed {}/{} data channel type names in {}ms ({:.1f} items/sec)",
							successCount, valueCount, parseDuration.count(), rate );
					}

					if ( values.size() > 1000 )
					{
						[[maybe_unused]] size_t approxBytes = estimateMemoryUsage( values );
						SPDLOG_DEBUG( "Large collection loaded: {} items, ~{} KB estimated memory", values.size(), approxBytes / 1024 );
					}
				}
				else
				{
					SPDLOG_WARN( "DataChannelTypeNamesDto field '{}' is not an array", VALUES_KEY );
				}
			}
			else
			{
				SPDLOG_WARN( "No '{}' array found in data channel type names JSON", VALUES_KEY );
			}

			[[maybe_unused]] auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed DataChannelTypeNamesDto with {} values in {} ms", values.size(), duration.count() );

			return DataChannelTypeNamesDto( std::move( values ) );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during DataChannelTypeNamesDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<DataChannelTypeNamesDto> DataChannelTypeNamesDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	DataChannelTypeNamesDto DataChannelTypeNamesDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize DataChannelTypeNamesDto from simdjson element" );
		}
		return std::move( *result );
	}

	DataChannelTypeNamesDto DataChannelTypeNamesDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize DataChannelTypeNamesDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string DataChannelTypeNamesDto::toJsonString() const
	{
		auto startTime = std::chrono::steady_clock::now();

		std::ostringstream oss;
		oss << "{\n  \"" << VALUES_KEY << "\": [";

		if ( !m_values.empty() )
		{
			oss << "\n";
			bool first = true;
			for ( const auto& value : m_values )
			{
				if ( !first )
					oss << ",\n";
				first = false;

				std::string valueJson = value.toJsonString();
				std::istringstream iss( valueJson );
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
		SPDLOG_DEBUG( "Serialized {} data channel type names in {}ms", m_values.size(), duration.count() );

		return oss.str();
	}

	//=====================================================================
	// Single Format Data Type Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	FormatDataTypeDto::FormatDataTypeDto( std::string type, std::string description )
		: m_type{ std::move( type ) },
		  m_description{ std::move( description ) }
	{
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

	std::optional<FormatDataTypeDto> FormatDataTypeDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "FormatDataTypeDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto typeElement = obj[TYPE_KEY];
			if ( typeElement.error() )
			{
				SPDLOG_ERROR( "FormatDataTypeDto JSON missing required '{}' field", TYPE_KEY );
				return std::nullopt;
			}

			auto typeOpt = safeGetString( typeElement.value() );
			if ( !typeOpt )
			{
				SPDLOG_ERROR( "FormatDataTypeDto JSON field '{}' is not a string", TYPE_KEY );
				return std::nullopt;
			}

			std::string type = internString( *typeOpt );

			std::string description;
			auto descriptionElement = obj[DESCRIPTION_KEY];
			if ( !descriptionElement.error() )
			{
				auto descriptionOpt = safeGetString( descriptionElement.value() );
				if ( descriptionOpt )
				{
					description = internString( *descriptionOpt );
				}
				else if ( descriptionElement.value().type() != simdjson::dom::element_type::NULL_VALUE )
				{
					SPDLOG_WARN( "FormatDataTypeDto has non-string '{}' field", DESCRIPTION_KEY );
				}
			}

			if ( type.empty() )
			{
				SPDLOG_WARN( "Parsed FormatDataTypeDto has empty type field" );
			}

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed FormatDataTypeDto: type={}, description={} in {} µs", type, description, duration.count() );

			return FormatDataTypeDto( std::move( type ), std::move( description ) );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during FormatDataTypeDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<FormatDataTypeDto> FormatDataTypeDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	FormatDataTypeDto FormatDataTypeDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize FormatDataTypeDto from simdjson element" );
		}
		return std::move( *result );
	}

	FormatDataTypeDto FormatDataTypeDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize FormatDataTypeDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string FormatDataTypeDto::toJsonString() const
	{
		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"" << TYPE_KEY << "\": \"" << escapeJsonString( m_type ) << "\"";
		if ( !m_description.empty() )
		{
			oss << ",\n  \"" << DESCRIPTION_KEY << "\": \"" << escapeJsonString( m_description ) << "\"";
		}
		oss << "\n}";
		return oss.str();
	}

	//=====================================================================
	// Collection of Format Data Type Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	FormatDataTypesDto::FormatDataTypesDto( std::vector<FormatDataTypeDto> values )
		: m_values{ std::move( values ) }
	{
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

	std::optional<FormatDataTypesDto> FormatDataTypesDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "FormatDataTypesDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			std::vector<FormatDataTypeDto> values;
			auto valuesElement = obj[VALUES_KEY];
			if ( !valuesElement.error() )
			{
				auto valuesArrayOpt = safeGetArray( valuesElement.value() );
				if ( valuesArrayOpt )
				{
					auto valuesArray = *valuesArrayOpt;
					size_t valueCount = valuesArray.size();
					values.reserve( valueCount );
					size_t successCount = 0;
					auto parseStartTime = std::chrono::steady_clock::now();

					for ( auto itemElement : valuesArray )
					{
						auto itemOpt = FormatDataTypeDto::tryFromJson( itemElement );
						if ( itemOpt )
						{
							values.emplace_back( std::move( *itemOpt ) );
							successCount++;
						}
						else
						{
							SPDLOG_ERROR( "Skipping malformed format data type at index {}", successCount );
						}
					}

					auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - parseStartTime );

					if ( valueCount > 0 && parseDuration.count() > 0 )
					{
						double rate = static_cast<double>( successCount ) * 1000.0 / static_cast<double>( parseDuration.count() );
						SPDLOG_DEBUG( "Successfully parsed {}/{} format data types in {}ms ({:.1f} items/sec)",
							successCount, valueCount, parseDuration.count(), rate );
					}

					if ( values.size() > 1000 )
					{
						size_t approxBytes = estimateMemoryUsage( values );
						SPDLOG_DEBUG( "Large collection loaded: {} items, ~{} KB estimated memory", values.size(), approxBytes / 1024 );
					}
				}
				else
				{
					SPDLOG_WARN( "FormatDataTypesDto field '{}' is not an array", VALUES_KEY );
				}
			}
			else
			{
				SPDLOG_WARN( "No '{}' array found in format data types JSON", VALUES_KEY );
			}

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed FormatDataTypesDto with {} values in {} ms", values.size(), duration.count() );

			return FormatDataTypesDto( std::move( values ) );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during FormatDataTypesDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<FormatDataTypesDto> FormatDataTypesDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	FormatDataTypesDto FormatDataTypesDto::fromJson( simdjson::dom::element element )
	{
		auto result = tryFromJson( element );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize FormatDataTypesDto from simdjson element" );
		}
		return std::move( *result );
	}

	FormatDataTypesDto FormatDataTypesDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto result = tryFromJsonString( jsonString, parser );
		if ( !result )
		{
			throw std::invalid_argument( "Failed to deserialize FormatDataTypesDto from JSON string" );
		}
		return std::move( *result );
	}

	std::string FormatDataTypesDto::toJsonString() const
	{
		auto startTime = std::chrono::steady_clock::now();

		std::ostringstream oss;
		oss << "{\n  \"" << VALUES_KEY << "\": [";

		if ( !m_values.empty() )
		{
			oss << "\n";
			bool first = true;
			for ( const auto& value : m_values )
			{
				if ( !first )
					oss << ",\n";
				first = false;

				std::string valueJson = value.toJsonString();
				std::istringstream iss( valueJson );
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
		SPDLOG_DEBUG( "Serialized {} format data types in {}ms", m_values.size(), duration.count() );

		return oss.str();
	}
}
