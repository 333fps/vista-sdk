/**
 * @file GmodVersioningDto.cpp
 * @brief Implementation of GMOD versioning data transfer objects
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodVersioningDto.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------

	static constexpr const char* VIS_RELEASE_KEY = "visRelease";
	static constexpr const char* ITEMS_KEY = "items";

	static constexpr const char* OLD_ASSIGNMENT_KEY = "oldAssignment";
	static constexpr const char* CURRENT_ASSIGNMENT_KEY = "currentAssignment";
	static constexpr const char* NEW_ASSIGNMENT_KEY = "newAssignment";
	static constexpr const char* DELETE_ASSIGNMENT_KEY = "deleteAssignment";

	static constexpr const char* OPERATIONS_KEY = "operations";
	static constexpr const char* SOURCE_KEY = "source";
	static constexpr const char* TARGET_KEY = "target";

	//-------------------------------------------------------------------
	// Utility Functions
	//-------------------------------------------------------------------

	namespace
	{
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
	}

	//-------------------------------------------------------------------
	// GmodVersioningAssignmentChangeDto Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	GmodVersioningAssignmentChangeDto::GmodVersioningAssignmentChangeDto( std::string oldAssignment, std::string currentAssignment )
		: m_oldAssignment{ std::move( oldAssignment ) },
		  m_currentAssignment{ std::move( currentAssignment ) }
	{
	}

	//-------------------------------------------------------------------
	// Accessor Methods
	//-------------------------------------------------------------------

	const std::string& GmodVersioningAssignmentChangeDto::oldAssignment() const
	{
		return m_oldAssignment;
	}

	const std::string& GmodVersioningAssignmentChangeDto::currentAssignment() const
	{
		return m_currentAssignment;
	}

	//-------------------------------------------------------------------
	// Serialization Methods
	//-------------------------------------------------------------------

	GmodVersioningAssignmentChangeDto GmodVersioningAssignmentChangeDto::fromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Parsing assignment change from JSON" );
		GmodVersioningAssignmentChangeDto dto;

		if ( json.HasMember( OLD_ASSIGNMENT_KEY ) && json[OLD_ASSIGNMENT_KEY].IsString() )
		{
			std::string oldAssignment = json[OLD_ASSIGNMENT_KEY].GetString();
			dto.m_oldAssignment = internString( oldAssignment );
		}

		if ( json.HasMember( CURRENT_ASSIGNMENT_KEY ) && json[CURRENT_ASSIGNMENT_KEY].IsString() )
		{
			std::string currentAssignment = json[CURRENT_ASSIGNMENT_KEY].GetString();
			dto.m_currentAssignment = internString( currentAssignment );
		}

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Parsed assignment change: {} → {} in {:.2f} ms", dto.m_oldAssignment, dto.m_currentAssignment, duration.count() / 1000.0 );

		return dto;
	}

	bool GmodVersioningAssignmentChangeDto::tryFromJson(
		const rapidjson::Value& json, GmodVersioningAssignmentChangeDto& dto )
	{
		try
		{
			dto = fromJson( json );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing GmodVersioningAssignmentChangeDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value GmodVersioningAssignmentChangeDto::toJson(
		rapidjson::Document::AllocatorType& allocator ) const
	{
		SPDLOG_DEBUG( "Serializing assignment change to JSON" );
		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( rapidjson::StringRef( OLD_ASSIGNMENT_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_oldAssignment.c_str() ), allocator ),
			allocator );
		obj.AddMember( rapidjson::StringRef( CURRENT_ASSIGNMENT_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_currentAssignment.c_str() ), allocator ),
			allocator );

		return obj;
	}

	//-------------------------------------------------------------------
	// GmodNodeConversionDto Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	GmodNodeConversionDto::GmodNodeConversionDto(
		std::unordered_set<std::string> operations, std::string source,
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

	//-------------------------------------------------------------------
	// Accessor Methods
	//-------------------------------------------------------------------

	const std::unordered_set<std::string>& GmodNodeConversionDto::operations() const
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

	//-------------------------------------------------------------------
	// Serialization Methods
	//-------------------------------------------------------------------

	GmodNodeConversionDto GmodNodeConversionDto::fromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Parsing node conversion from JSON" );
		GmodNodeConversionDto dto;

		if ( json.HasMember( OPERATIONS_KEY ) && json[OPERATIONS_KEY].IsArray() )
		{
			size_t opCount = json[OPERATIONS_KEY].Size();
			SPDLOG_DEBUG( "Found {} operations", opCount );
			dto.m_operations.reserve( opCount );

			for ( const auto& operation : json[OPERATIONS_KEY].GetArray() )
			{
				if ( operation.IsString() )
				{
					dto.m_operations.insert( internString( operation.GetString() ) );
				}
			}
		}

		if ( json.HasMember( SOURCE_KEY ) && json[SOURCE_KEY].IsString() )
		{
			dto.m_source = internString( json[SOURCE_KEY].GetString() );
		}

		if ( json.HasMember( TARGET_KEY ) && json[TARGET_KEY].IsString() )
		{
			dto.m_target = internString( json[TARGET_KEY].GetString() );
		}

		if ( json.HasMember( OLD_ASSIGNMENT_KEY ) && json[OLD_ASSIGNMENT_KEY].IsString() )
			dto.m_oldAssignment = json[OLD_ASSIGNMENT_KEY].GetString();

		if ( json.HasMember( NEW_ASSIGNMENT_KEY ) && json[NEW_ASSIGNMENT_KEY].IsString() )
			dto.m_newAssignment = json[NEW_ASSIGNMENT_KEY].GetString();

		if ( json.HasMember( DELETE_ASSIGNMENT_KEY ) && json[DELETE_ASSIGNMENT_KEY].IsBool() )
			dto.m_deleteAssignment = json[DELETE_ASSIGNMENT_KEY].GetBool();

		if ( dto.m_operations.empty() )
		{
			SPDLOG_WARN( "Node conversion has no operations: source={}, target={}", dto.m_source, dto.m_target );
		}
		if ( dto.m_source.empty() && dto.m_target.empty() )
		{
			SPDLOG_WARN( "Node conversion has empty source and target" );
		}

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Parsed node conversion: source={}, target={}, operations={} in {:.2f} ms", dto.m_source, dto.m_target, dto.m_operations.size(), duration.count() / 1000.0 );

		return dto;
	}

	bool GmodNodeConversionDto::tryFromJson( const rapidjson::Value& json, GmodNodeConversionDto& dto )
	{
		try
		{
			dto = fromJson( json );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing GmodNodeConversionDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value GmodNodeConversionDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Serializing node conversion to JSON" );
		rapidjson::Value obj( rapidjson::kObjectType );

		rapidjson::Value operationsArray( rapidjson::kArrayType );
		safeReserveArray( operationsArray, m_operations.size(), allocator );

		for ( const auto& operation : m_operations )
		{
			operationsArray.PushBack(
				rapidjson::Value( rapidjson::StringRef( operation.c_str() ), allocator ),
				allocator );
		}
		obj.AddMember( rapidjson::StringRef( OPERATIONS_KEY ), operationsArray, allocator );

		obj.AddMember( rapidjson::StringRef( SOURCE_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_source.c_str() ), allocator ),
			allocator );
		obj.AddMember( rapidjson::StringRef( TARGET_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_target.c_str() ), allocator ),
			allocator );
		obj.AddMember( rapidjson::StringRef( OLD_ASSIGNMENT_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_oldAssignment.c_str() ), allocator ),
			allocator );
		obj.AddMember( rapidjson::StringRef( NEW_ASSIGNMENT_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_newAssignment.c_str() ), allocator ),
			allocator );

		obj.AddMember( rapidjson::StringRef( DELETE_ASSIGNMENT_KEY ), m_deleteAssignment, allocator );

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized node conversion in {:.2f} ms", duration.count() / 1000.0 );

		return obj;
	}

	//-------------------------------------------------------------------
	// GmodVersioningDto Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	GmodVersioningDto::GmodVersioningDto( std::string visVersion, std::unordered_map<std::string, GmodNodeConversionDto> items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
	}

	//-------------------------------------------------------------------
	// Accessor Methods
	//-------------------------------------------------------------------

	const std::string& GmodVersioningDto::visVersion() const
	{
		return m_visVersion;
	}

	const std::unordered_map<std::string, GmodNodeConversionDto>& GmodVersioningDto::items() const
	{
		return m_items;
	}

	//-------------------------------------------------------------------
	// Serialization Methods
	//-------------------------------------------------------------------

	GmodVersioningDto GmodVersioningDto::fromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Parsing GMOD versioning data from JSON" );
		GmodVersioningDto dto;

		if ( !json.IsObject() )
		{
			SPDLOG_ERROR( "JSON value is not an object" );
			throw std::runtime_error( "Invalid JSON: expected an object" );
		}

		if ( json.HasMember( VIS_RELEASE_KEY ) && json[VIS_RELEASE_KEY].IsString() )
		{
			dto.m_visVersion = json[VIS_RELEASE_KEY].GetString();
			SPDLOG_INFO( "GMOD versioning for VIS version: {}", dto.m_visVersion );
		}

		if ( json.HasMember( ITEMS_KEY ) && json[ITEMS_KEY].IsObject() )
		{
			size_t itemCount = json[ITEMS_KEY].MemberCount();
			SPDLOG_INFO( "Found {} node conversion items to parse", itemCount );

			if ( itemCount > 10000 )
			{
				SPDLOG_INFO( "Large versioning dataset detected, consider using parallel parsing implementation" );
			}

			dto.m_items.reserve( itemCount );

			size_t successCount = 0;
			size_t emptyOperationsCount = 0;

			auto parseStartTime = std::chrono::steady_clock::now();

			for ( auto it = json[ITEMS_KEY].MemberBegin(); it != json[ITEMS_KEY].MemberEnd(); ++it )
			{
				if ( it->value.IsObject() )
				{
					try
					{
						auto& nodeDto = dto.m_items[it->name.GetString()] =
							GmodNodeConversionDto::fromJson( it->value );
						successCount++;

						if ( nodeDto.operations().empty() )
						{
							emptyOperationsCount++;
						}
					}
					catch ( const std::exception& e )
					{
						SPDLOG_ERROR( "Error parsing conversion item '{}': {}",
							it->name.GetString(), e.what() );
					}
				}
			}

			auto parseEndTime = std::chrono::steady_clock::now();
			auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>( parseEndTime - parseStartTime );

			double parseRatePerSecond = static_cast<double>( successCount ) * 1000.0 / static_cast<double>( parseDuration.count() );

			SPDLOG_INFO( "Successfully parsed {}/{} node conversion items ({} with empty operations), rate: {:.1f} items/sec",
				successCount, itemCount, emptyOperationsCount, parseRatePerSecond );

			if ( successCount > 0 && static_cast<double>( successCount ) < static_cast<double>( itemCount ) * 0.9 )
			{
				SPDLOG_INFO( "Optimizing memory usage after parsing {} of {} items", successCount, itemCount );
				dto.m_items.rehash( static_cast<size_t>( static_cast<double>( successCount ) * 1.25 ) );
			}
		}
		else
		{
			SPDLOG_WARN( "No 'items' object found in GMOD versioning data" );
		}

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - startTime );

		if ( dto.m_items.size() > 1000 )
		{
			size_t approxMemoryBytes = dto.m_items.size() * ( sizeof( GmodNodeConversionDto ) + sizeof( std::string ) * 4 + 32 ); // estimate
			SPDLOG_INFO( "Large versioning dataset loaded: {} items, ~{} MB estimated memory", dto.m_items.size(), approxMemoryBytes / ( 1024 * 1024 ) );
		}

		SPDLOG_INFO( "GMOD versioning parsing completed in {} ms", duration.count() );

		return dto;
	}

	bool GmodVersioningDto::tryFromJson( const rapidjson::Value& json, GmodVersioningDto& dto )
	{
		try
		{
			dto = fromJson( json );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing GmodVersioningDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value GmodVersioningDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Serializing GMOD versioning data to JSON, {} items", m_items.size() );
		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( rapidjson::StringRef( VIS_RELEASE_KEY ),
			rapidjson::Value( rapidjson::StringRef( m_visVersion.c_str() ), allocator ),
			allocator );

		rapidjson::Value itemsObj( rapidjson::kObjectType );

		size_t emptyOperationsCount = 0;
		auto serializationStartTime = std::chrono::steady_clock::now();

		for ( const auto& [key, value] : m_items )
		{
			itemsObj.AddMember(
				rapidjson::Value( rapidjson::StringRef( key.c_str() ), allocator ),
				value.toJson( allocator ),
				allocator );

			if ( value.operations().empty() )
			{
				emptyOperationsCount++;
			}
		}

		auto serializationEndTime = std::chrono::steady_clock::now();
		auto serializationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
			serializationEndTime - serializationStartTime );

		if ( m_items.size() > 0 )
		{
			double serializationRatePerSecond = static_cast<double>( m_items.size() ) * 1000.0 / static_cast<double>( serializationDuration.count() );
			SPDLOG_INFO( "Node serialization rate: {:.1f} items/sec", serializationRatePerSecond );
		}

		if ( emptyOperationsCount > 0 )
		{
			SPDLOG_WARN( "{} nodes have no operations defined", emptyOperationsCount );
		}

		obj.AddMember( rapidjson::StringRef( ITEMS_KEY ), itemsObj, allocator );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Successfully serialized GMOD versioning data for VIS {} in {} ms", m_visVersion, duration.count() );

		return obj;
	}
}
