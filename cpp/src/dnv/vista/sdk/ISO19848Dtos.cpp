/**
 * @file ISO19848Dtos.cpp
 * @brief Implementation of ISO 19848 data transfer objects
 */

#include "pch.h"

#include "dnv/vista/sdk/ISO19848Dtos.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------------

	static constexpr const char* VALUES_KEY = "values";
	static constexpr const char* TYPE_KEY = "type";
	static constexpr const char* DESCRIPTION_KEY = "description";

	static constexpr size_t BATCH_SIZE = 1000;

	//-------------------------------------------------------------------------
	// Utility Functions
	//-------------------------------------------------------------------------

	namespace
	{
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

		void safeReserveArray( rapidjson::Value& array, size_t size, rapidjson::Document::AllocatorType& allocator )
		{
			if ( size > std::numeric_limits<rapidjson::SizeType>::max() )
			{
				SPDLOG_WARN( "Array size {} exceeds maximum RapidJSON capacity", size );
				array.Reserve( std::numeric_limits<rapidjson::SizeType>::max(), allocator );
			}
			else
			{
				array.Reserve( static_cast<rapidjson::SizeType>( size ), allocator );
			}
		}

		template <typename T>
		size_t estimateMemoryUsage( const std::vector<T>& collection )
		{
			return sizeof( std::vector<T> ) + collection.capacity() * sizeof( T );
		}
	}

	//-------------------------------------------------------------------------
	// DataChannelTypeNameDto Implementation
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	// Constructors / Destructor
	//-------------------------------------------------------------------------

	DataChannelTypeNameDto::DataChannelTypeNameDto( std::string type, std::string description )
		: m_type{ std::move( type ) },
		  m_description{ std::move( description ) }
	{
		SPDLOG_INFO( "Creating DataChannelTypeNameDto: type={}, description={}", m_type, m_description );
	}

	//-------------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------------

	const std::string& DataChannelTypeNameDto::type() const
	{
		return m_type;
	}

	const std::string& DataChannelTypeNameDto::description() const
	{
		return m_description;
	}

	//-------------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------------

	std::optional<DataChannelTypeNameDto> DataChannelTypeNameDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse DataChannelTypeNameDto from JSON" );

		if ( !json.IsObject() )
		{
			SPDLOG_ERROR( "JSON value is not an object" );
			return std::nullopt;
		}

		std::string typeStr;
		std::string descriptionStr;

		if ( json.HasMember( TYPE_KEY ) && json[TYPE_KEY].IsString() )
		{
			typeStr = internString( json[TYPE_KEY].GetString() );
		}
		else
		{
			SPDLOG_DEBUG( "DataChannelTypeNameDto missing required 'type' field" );
		}

		if ( json.HasMember( DESCRIPTION_KEY ) && json[DESCRIPTION_KEY].IsString() )
		{
			descriptionStr = internString( json[DESCRIPTION_KEY].GetString() );
		}

		if ( typeStr.empty() )
		{
			SPDLOG_WARN( "Parsed DataChannelTypeNameDto has empty type field" );
		}

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Parsed DataChannelTypeNameDto: type={}, description={} in {:.2f}ms", typeStr, descriptionStr, duration.count() / 1000.0 );

		return DataChannelTypeNameDto( std::move( typeStr ), std::move( descriptionStr ) );
	}

	DataChannelTypeNameDto DataChannelTypeNameDto::fromJson( const rapidjson::Value& json )
	{
		auto dtoOpt = tryFromJson( json );
		if ( !dtoOpt )
		{
			throw std::invalid_argument( "Failed to parse DataChannelTypeNameDto from JSON" );
		}
		return std::move( *dtoOpt );
	}

	rapidjson::Value DataChannelTypeNameDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		SPDLOG_DEBUG( "Serializing DataChannelTypeNameDto: type={}", m_type );
		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( rapidjson::StringRef( TYPE_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_type.c_str() ), allocator ),
			allocator );
		obj.AddMember( rapidjson::StringRef( DESCRIPTION_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_description.c_str() ), allocator ),
			allocator );

		return obj;
	}

	//-------------------------------------------------------------------------
	// DataChannelTypeNamesDto Implementation
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	// Constructors / Destructor
	//-------------------------------------------------------------------------

	DataChannelTypeNamesDto::DataChannelTypeNamesDto( std::vector<DataChannelTypeNameDto> values )
		: m_values{ std::move( values ) }
	{
		SPDLOG_INFO( "Creating DataChannelTypeNamesDto with {} values", m_values.size() );
	}

	//-------------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------------

	const std::vector<DataChannelTypeNameDto>& DataChannelTypeNamesDto::values() const
	{
		return m_values;
	}

	//-------------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------------

	std::optional<DataChannelTypeNamesDto> DataChannelTypeNamesDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse data channel type names from JSON" );

		if ( !json.IsObject() )
		{
			SPDLOG_ERROR( "JSON value is not an object" );
			return std::nullopt;
		}

		std::vector<DataChannelTypeNameDto> parsedValues;

		if ( json.HasMember( VALUES_KEY ) && json[VALUES_KEY].IsArray() )
		{
			const auto& jsonArray = json[VALUES_KEY].GetArray();
			size_t valueCount = jsonArray.Size();
			SPDLOG_INFO( "Found {} data channel type entries to parse", valueCount );

			parsedValues.reserve( valueCount );
			size_t successCount = 0;
			bool useBatching = valueCount > 5000;

			if ( useBatching )
			{
				SPDLOG_INFO( "Large collection detected, using batched processing" );

				for ( size_t i = 0; i < valueCount; i += BATCH_SIZE )
				{
					auto batchStart = std::chrono::steady_clock::now();
					size_t batchEnd = std::min( i + BATCH_SIZE, valueCount );
					SPDLOG_DEBUG( "Processing batch {}-{}", i, batchEnd - 1 );

					size_t batchSuccess = 0;
					for ( size_t j = i; j < batchEnd; j++ )
					{
						auto itemOpt = DataChannelTypeNameDto::tryFromJson(
							jsonArray[static_cast<rapidjson::SizeType>( j )] );
						if ( itemOpt )
						{
							parsedValues.emplace_back( std::move( *itemOpt ) );
							batchSuccess++;
							successCount++;
						}
						else
						{
							SPDLOG_ERROR( "Skipping malformed data channel type name at index {}", j );
						}
					}

					auto batchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::steady_clock::now() - batchStart );
					SPDLOG_DEBUG( "Batch {}-{} processed: {}/{} items in {}ms",
						i, batchEnd - 1, batchSuccess, batchEnd - i, batchDuration.count() );
				}
			}
			else
			{
				for ( rapidjson::SizeType i = 0; i < jsonArray.Size(); i++ )
				{
					auto itemOpt = DataChannelTypeNameDto::tryFromJson( jsonArray[i] );
					if ( itemOpt )
					{
						parsedValues.emplace_back( std::move( *itemOpt ) );
						successCount++;
					}
					else
					{
						SPDLOG_ERROR( "Skipping malformed data channel type name at index {}", i );
					}
				}
			}

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - startTime );

			if ( parsedValues.size() > 1000 )
			{
				size_t approxBytes = estimateMemoryUsage( parsedValues );
				SPDLOG_INFO( "Large collection loaded: {} items, ~{} KB", parsedValues.size(), approxBytes / 1024 );
			}

			SPDLOG_INFO( "Successfully parsed {}/{} data channel type names in {}ms ({:.1f} items/sec)",
				successCount, valueCount, duration.count(),
				duration.count() > 0 ? static_cast<double>( successCount ) * 1000.0 / static_cast<double>( duration.count() ) : 0 );
		}
		else
		{
			SPDLOG_WARN( "No '{}' array found in data channel type names JSON", VALUES_KEY );
		}

		return DataChannelTypeNamesDto( std::move( parsedValues ) );
	}

	DataChannelTypeNamesDto DataChannelTypeNamesDto::fromJson( const rapidjson::Value& json )
	{
		auto dtoOpt = tryFromJson( json );
		if ( !dtoOpt )
		{
			throw std::invalid_argument( "Failed to parse DataChannelTypeNamesDto from JSON" );
		}
		return std::move( *dtoOpt );
	}

	rapidjson::Value DataChannelTypeNamesDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Serializing {} data channel type names to JSON", m_values.size() );
		rapidjson::Value obj( rapidjson::kObjectType );

		rapidjson::Value valuesArray( rapidjson::kArrayType );
		safeReserveArray( valuesArray, m_values.size(), allocator );

		for ( const auto& value : m_values )
		{
			valuesArray.PushBack( value.toJson( allocator ), allocator );
		}

		obj.AddMember( rapidjson::StringRef( VALUES_KEY ), valuesArray, allocator );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized {} data channel type names in {}ms ({:.1f} items/sec)",
			m_values.size(), duration.count(),
			duration.count() > 0 ? m_values.size() * 1000.0 / duration.count() : 0 );

		return obj;
	}

	//-------------------------------------------------------------------------
	// FormatDataTypeDto Implementation
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	// Constructors / Destructor
	//-------------------------------------------------------------------------

	FormatDataTypeDto::FormatDataTypeDto( std::string type, std::string description )
		: m_type{ std::move( type ) },
		  m_description{ std::move( description ) }
	{
		SPDLOG_INFO( "Creating FormatDataTypeDto: type={}, description={}", m_type, m_description );
	}

	//-------------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------------

	const std::string& FormatDataTypeDto::type() const
	{
		return m_type;
	}

	const std::string& FormatDataTypeDto::description() const
	{
		return m_description;
	}

	//-------------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------------

	std::optional<FormatDataTypeDto> FormatDataTypeDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse FormatDataTypeDto from JSON" );

		if ( !json.IsObject() )
		{
			SPDLOG_ERROR( "JSON value is not an object" );
			return std::nullopt;
		}

		std::string typeStr;
		std::string descriptionStr;

		if ( json.HasMember( TYPE_KEY ) && json[TYPE_KEY].IsString() )
		{
			typeStr = internString( json[TYPE_KEY].GetString() );
		}
		else
		{
			SPDLOG_DEBUG( "FormatDataTypeDto missing required 'type' field" );
		}

		if ( json.HasMember( DESCRIPTION_KEY ) && json[DESCRIPTION_KEY].IsString() )
		{
			descriptionStr = internString( json[DESCRIPTION_KEY].GetString() );
		}

		if ( typeStr.empty() )
		{
			SPDLOG_WARN( "Parsed FormatDataTypeDto has empty type field" );
		}

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Parsed FormatDataTypeDto: type={}, description={} in {:.2f}ms",
			typeStr, descriptionStr, duration.count() / 1000.0 );

		return FormatDataTypeDto( std::move( typeStr ), std::move( descriptionStr ) );
	}

	FormatDataTypeDto FormatDataTypeDto::fromJson( const rapidjson::Value& json )
	{
		auto dtoOpt = tryFromJson( json );
		if ( !dtoOpt )
		{
			throw std::invalid_argument( "Failed to parse FormatDataTypeDto from JSON" );
		}
		return std::move( *dtoOpt );
	}

	rapidjson::Value FormatDataTypeDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		SPDLOG_DEBUG( "Serializing FormatDataTypeDto: type={}", m_type );
		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( rapidjson::StringRef( TYPE_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_type.c_str() ), allocator ),
			allocator );
		obj.AddMember( rapidjson::StringRef( DESCRIPTION_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_description.c_str() ), allocator ),
			allocator );

		return obj;
	}

	//-------------------------------------------------------------------------
	// FormatDataTypesDto Implementation
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	// Constructors / Destructor
	//-------------------------------------------------------------------------

	FormatDataTypesDto::FormatDataTypesDto( std::vector<FormatDataTypeDto> values )
		: m_values{ std::move( values ) }
	{
		SPDLOG_INFO( "Creating FormatDataTypesDto with {} values", m_values.size() );
	}

	//-------------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------------

	const std::vector<FormatDataTypeDto>& FormatDataTypesDto::values() const
	{
		return m_values;
	}

	//-------------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------------

	std::optional<FormatDataTypesDto> FormatDataTypesDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse format data types from JSON" );

		if ( !json.IsObject() )
		{
			SPDLOG_ERROR( "JSON value is not an object" );
			return std::nullopt;
		}

		std::vector<FormatDataTypeDto> parsedValues;

		if ( json.HasMember( VALUES_KEY ) && json[VALUES_KEY].IsArray() )
		{
			const auto& jsonArray = json[VALUES_KEY].GetArray();
			size_t valueCount = jsonArray.Size();
			SPDLOG_INFO( "Found {} format data type entries to parse", valueCount );

			parsedValues.reserve( valueCount );
			size_t successCount = 0;
			bool useBatching = valueCount > 5000;

			if ( useBatching )
			{
				SPDLOG_INFO( "Large collection detected, using batched processing" );

				for ( size_t i = 0; i < valueCount; i += BATCH_SIZE )
				{
					auto batchStart = std::chrono::steady_clock::now();
					size_t batchEnd = std::min( i + BATCH_SIZE, valueCount );
					SPDLOG_DEBUG( "Processing batch {}-{}", i, batchEnd - 1 );

					size_t batchSuccess = 0;
					for ( size_t j = i; j < batchEnd; j++ )
					{
						auto itemOpt = FormatDataTypeDto::tryFromJson(
							jsonArray[static_cast<rapidjson::SizeType>( j )] );
						if ( itemOpt )
						{
							parsedValues.emplace_back( std::move( *itemOpt ) );
							batchSuccess++;
							successCount++;
						}
						else
						{
							SPDLOG_ERROR( "Skipping malformed format data type at index {}", j );
						}
					}

					auto batchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::steady_clock::now() - batchStart );
					SPDLOG_DEBUG( "Batch {}-{} processed: {}/{} items in {}ms",
						i, batchEnd - 1, batchSuccess, batchEnd - i, batchDuration.count() );
				}
			}
			else
			{
				for ( rapidjson::SizeType i = 0; i < jsonArray.Size(); i++ )
				{
					auto itemOpt = FormatDataTypeDto::tryFromJson( jsonArray[i] );
					if ( itemOpt )
					{
						parsedValues.emplace_back( std::move( *itemOpt ) );
						successCount++;
					}
					else
					{
						SPDLOG_ERROR( "Skipping malformed format data type at index {}", i );
					}
				}
			}

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now() - startTime );

			if ( parsedValues.size() > 1000 )
			{
				size_t approxBytes = estimateMemoryUsage( parsedValues );
				SPDLOG_INFO( "Large collection loaded: {} items, ~{} KB", parsedValues.size(), approxBytes / 1024 );
			}

			SPDLOG_INFO( "Successfully parsed {}/{} format data types in {}ms ({:.1f} items/sec)",
				successCount, valueCount, duration.count(),
				duration.count() > 0 ? static_cast<double>( successCount ) * 1000.0 / static_cast<double>( duration.count() ) : 0 );
		}
		else
		{
			SPDLOG_WARN( "No '{}' array found in format data types JSON", VALUES_KEY );
		}

		return FormatDataTypesDto( std::move( parsedValues ) );
	}

	FormatDataTypesDto FormatDataTypesDto::fromJson( const rapidjson::Value& json )
	{
		auto dtoOpt = tryFromJson( json );
		if ( !dtoOpt )
		{
			throw std::invalid_argument( "Failed to parse FormatDataTypesDto from JSON" );
		}
		return std::move( *dtoOpt );
	}

	rapidjson::Value FormatDataTypesDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Serializing {} format data types to JSON", m_values.size() );
		rapidjson::Value obj( rapidjson::kObjectType );

		rapidjson::Value valuesArray( rapidjson::kArrayType );
		safeReserveArray( valuesArray, m_values.size(), allocator );

		for ( const auto& value : m_values )
		{
			valuesArray.PushBack( value.toJson( allocator ), allocator );
		}

		obj.AddMember( rapidjson::StringRef( VALUES_KEY ), valuesArray, allocator );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized {} format data types in {}ms ({:.1f} items/sec)",
			m_values.size(), duration.count(),
			duration.count() > 0 ? m_values.size() * 1000.0 / duration.count() : 0 );

		return obj;
	}
}
