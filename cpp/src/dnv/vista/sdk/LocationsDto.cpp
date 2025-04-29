/**
 * @file LocationsDto.cpp
 * @brief Implementation of Data Transfer Objects for locations in the VIS standard
 */

#include "pch.h"

#include "dnv/vista/sdk/LocationsDto.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------------

	static constexpr const char* CODE_KEY = "code";
	static constexpr const char* NAME_KEY = "name";
	static constexpr const char* DEFINITION_KEY = "definition";
	static constexpr const char* VIS_RELEASE_KEY = "visRelease";
	static constexpr const char* ITEMS_KEY = "items";

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
	// RelativeLocationsDto Implementation
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	// Constructors / Destructor
	//-------------------------------------------------------------------------

	RelativeLocationsDto::RelativeLocationsDto( char code, std::string name, std::optional<std::string> definition )
		: m_code{ code },
		  m_name{ std::move( name ) },
		  m_definition{ std::move( definition ) }
	{
		SPDLOG_INFO( "RelativeLocationsDto constructed: code={}, name={}, has_definition={}",
			m_code, m_name, m_definition.has_value() );
	}

	//-------------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------------

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

	//-------------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------------

	std::optional<RelativeLocationsDto> RelativeLocationsDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse RelativeLocationsDto from JSON" );

		if ( !json.IsObject() )
		{
			SPDLOG_WARN( "JSON value is not an object" );
			return std::nullopt;
		}

		if ( !json.HasMember( CODE_KEY ) || !json[CODE_KEY].IsString() || json[CODE_KEY].GetStringLength() == 0 )
		{
			SPDLOG_WARN( "Missing or invalid '{}' field in RelativeLocationsDto JSON", CODE_KEY );
			return std::nullopt;
		}

		if ( !json.HasMember( NAME_KEY ) || !json[NAME_KEY].IsString() )
		{
			SPDLOG_WARN( "Missing or invalid '{}' field in RelativeLocationsDto JSON", NAME_KEY );
			return std::nullopt;
		}

		char code = json[CODE_KEY].GetString()[0];
		std::string name = internString( json[NAME_KEY].GetString() );

		std::optional<std::string> definition;
		if ( json.HasMember( DEFINITION_KEY ) && json[DEFINITION_KEY].IsString() )
		{
			definition = internString( json[DEFINITION_KEY].GetString() );
		}

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Successfully parsed relative location: code={}, name={} in {:.2f}ms",
			code, name, static_cast<double>( duration.count() ) / 1000.0 );

		return RelativeLocationsDto(
			code,
			std::move( name ),
			std::move( definition ) );
	}

	RelativeLocationsDto RelativeLocationsDto::fromJson( const rapidjson::Value& json )
	{
		SPDLOG_DEBUG( "Parsing RelativeLocationsDto from JSON" );
		auto dtoOpt = tryFromJson( json );
		if ( !dtoOpt )
		{
			SPDLOG_ERROR( "Failed to parse RelativeLocationsDto from JSON" );
			throw std::invalid_argument( "Failed to parse RelativeLocationsDto from JSON" );
		}
		return std::move( *dtoOpt );
	}

	rapidjson::Value RelativeLocationsDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		SPDLOG_DEBUG( "Serializing RelativeLocationsDto: code={}, name={}", m_code, m_name );
		rapidjson::Value obj( rapidjson::kObjectType );

		char codeStr[2] = { m_code, '\0' };
		obj.AddMember( rapidjson::StringRef( CODE_KEY ), rapidjson::Value( codeStr, allocator ), allocator );
		obj.AddMember( rapidjson::StringRef( NAME_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_name.c_str() ), allocator ), allocator );

		if ( m_definition.has_value() )
		{
			obj.AddMember( rapidjson::StringRef( DEFINITION_KEY ),
				rapidjson::Value( rapidjson::StringRef( m_definition.value().c_str() ), allocator ),
				allocator );
		}

		return obj;
	}

	//-------------------------------------------------------------------------
	// LocationsDto Implementation
	//-------------------------------------------------------------------------

	//-------------------------------------------------------------------------
	// Constructors / Destructor
	//-------------------------------------------------------------------------

	LocationsDto::LocationsDto( std::string visVersion, std::vector<RelativeLocationsDto> items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
		SPDLOG_INFO( "LocationsDto constructed: visVersion={}, items.size={}",
			m_visVersion, m_items.size() );
	}

	//-------------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------------

	const std::string& LocationsDto::visVersion() const
	{
		return m_visVersion;
	}

	const std::vector<RelativeLocationsDto>& LocationsDto::items() const
	{
		return m_items;
	}

	//-------------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------------

	std::optional<LocationsDto> LocationsDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse LocationsDto from JSON" );

		if ( !json.IsObject() )
		{
			SPDLOG_WARN( "JSON value is not an object" );
			return std::nullopt;
		}

		if ( !json.HasMember( VIS_RELEASE_KEY ) || !json[VIS_RELEASE_KEY].IsString() )
		{
			SPDLOG_WARN( "Missing or invalid '{}' field in LocationsDto JSON", VIS_RELEASE_KEY );
			return std::nullopt;
		}

		if ( !json.HasMember( ITEMS_KEY ) || !json[ITEMS_KEY].IsArray() )
		{
			SPDLOG_WARN( "Missing or invalid '{}' field in LocationsDto JSON", ITEMS_KEY );
			return std::nullopt;
		}

		std::string visVersion = internString( json[VIS_RELEASE_KEY].GetString() );
		SPDLOG_INFO( "Parsing locations for VIS version: {}", visVersion );

		const auto& jsonArray = json[ITEMS_KEY].GetArray();
		size_t itemCount = jsonArray.Size();
		SPDLOG_INFO( "Found {} location items to parse", itemCount );

		std::vector<RelativeLocationsDto> items;
		items.reserve( itemCount );

		size_t successCount = 0;
		bool useBatching = itemCount > 5000;

		if ( useBatching )
		{
			SPDLOG_INFO( "Large location list detected, using batched processing" );

			for ( size_t i = 0; i < itemCount; i += BATCH_SIZE )
			{
				auto batchStart = std::chrono::steady_clock::now();
				size_t batchEnd = std::min( i + BATCH_SIZE, itemCount );
				SPDLOG_DEBUG( "Processing batch {}-{}", i, batchEnd - 1 );

				size_t batchSuccess = 0;
				for ( size_t j = i; j < batchEnd; j++ )
				{
					auto itemOpt = RelativeLocationsDto::tryFromJson(
						jsonArray[static_cast<rapidjson::SizeType>( j )] );
					if ( itemOpt )
					{
						items.emplace_back( std::move( *itemOpt ) );
						batchSuccess++;
						successCount++;
					}
					else
					{
						SPDLOG_WARN( "Skipping malformed location item at index {}", j );
					}
				}

				auto batchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::steady_clock::now() - batchStart );
				SPDLOG_DEBUG( "Batch {}-{} processed: {}/{} items in {}ms ({:.1f} items/sec)",
					i, batchEnd - 1, batchSuccess, batchEnd - i, batchDuration.count(),
					batchDuration.count() > 0 ? static_cast<double>( batchSuccess ) * 1000.0 /
													static_cast<double>( batchDuration.count() )
											  : 0 );
			}
		}
		else
		{
			for ( size_t i = 0; i < itemCount; i++ )
			{
				auto itemOpt = RelativeLocationsDto::tryFromJson(
					jsonArray[static_cast<rapidjson::SizeType>( i )] );
				if ( itemOpt )
				{
					items.emplace_back( std::move( *itemOpt ) );
					successCount++;
				}
				else
				{
					SPDLOG_WARN( "Skipping invalid location item at index {}", i );
				}
			}
		}

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - startTime );

		if ( items.size() > 1000 )
		{
			size_t approxBytes = estimateMemoryUsage( items );
			SPDLOG_INFO( "Large location collection loaded: {} items, ~{} KB", items.size(), approxBytes / 1024 );
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

		SPDLOG_INFO( "Successfully parsed {}/{} locations for VIS {} in {}ms ({:.1f} items/sec)",
			successCount, itemCount, visVersion, duration.count(),
			duration.count() > 0 ? static_cast<double>( successCount ) * 1000.0 /
									   static_cast<double>( duration.count() )
								 : 0 );

		return LocationsDto(
			std::move( visVersion ),
			std::move( items ) );
	}

	LocationsDto LocationsDto::fromJson( const rapidjson::Value& json )
	{
		SPDLOG_INFO( "Parsing LocationsDto from JSON" );
		auto dtoOpt = tryFromJson( json );
		if ( !dtoOpt )
		{
			SPDLOG_ERROR( "Failed to parse LocationsDto from JSON" );
			throw std::invalid_argument( "Failed to parse LocationsDto from JSON" );
		}
		return std::move( *dtoOpt );
	}

	rapidjson::Value LocationsDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Serializing LocationsDto: visVersion={}, items={}",
			m_visVersion, m_items.size() );

		rapidjson::Value obj( rapidjson::kObjectType );
		obj.AddMember( rapidjson::StringRef( VIS_RELEASE_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_visVersion.c_str() ), allocator ),
			allocator );

		rapidjson::Value itemsArray( rapidjson::kArrayType );
		safeReserveArray( itemsArray, m_items.size(), allocator );

		bool useBatching = m_items.size() > 5000;
		if ( useBatching )
		{
			SPDLOG_INFO( "Large collection detected, using batched serialization" );

			for ( size_t i = 0; i < m_items.size(); i += BATCH_SIZE )
			{
				auto batchStart = std::chrono::steady_clock::now();
				size_t batchEnd = std::min( i + BATCH_SIZE, m_items.size() );

				for ( size_t j = i; j < batchEnd; j++ )
				{
					itemsArray.PushBack( m_items[j].toJson( allocator ), allocator );
				}

				auto batchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::steady_clock::now() - batchStart );
				SPDLOG_DEBUG( "Serialized items {}-{} in {}ms", i, batchEnd - 1, batchDuration.count() );
			}
		}
		else
		{
			for ( const auto& item : m_items )
			{
				itemsArray.PushBack( item.toJson( allocator ), allocator );
			}
		}

		obj.AddMember( rapidjson::StringRef( ITEMS_KEY ), itemsArray, allocator );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - startTime );

		SPDLOG_INFO( "Serialized {} locations in {}ms ({:.1f} items/sec)",
			m_items.size(), duration.count(),
			duration.count() > 0 ? static_cast<double>( m_items.size() ) * 1000.0 /
									   static_cast<double>( duration.count() )
								 : 0 );

		return obj;
	}
}
