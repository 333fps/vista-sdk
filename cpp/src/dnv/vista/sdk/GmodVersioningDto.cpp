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
		SPDLOG_INFO( "Created GmodVersioningAssignmentChangeDto: {} → {}", m_oldAssignment, m_currentAssignment );
	}

	//-------------------------------------------------------------------
	// Public Interface - Accessor Methods
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
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------

	std::optional<GmodVersioningAssignmentChangeDto> GmodVersioningAssignmentChangeDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse assignment change from JSON" );

		if ( !json.IsObject() )
		{
			SPDLOG_ERROR( "JSON value is not an object" );
			return std::nullopt;
		}

		std::string oldAssignment;
		std::string currentAssignment;

		if ( json.HasMember( OLD_ASSIGNMENT_KEY ) && json[OLD_ASSIGNMENT_KEY].IsString() )
		{
			oldAssignment = internString( json[OLD_ASSIGNMENT_KEY].GetString() );
		}
		else
		{
			SPDLOG_WARN( "Missing or invalid 'oldAssignment' field" );
		}

		if ( json.HasMember( CURRENT_ASSIGNMENT_KEY ) && json[CURRENT_ASSIGNMENT_KEY].IsString() )
		{
			currentAssignment = internString( json[CURRENT_ASSIGNMENT_KEY].GetString() );
		}
		else
		{
			SPDLOG_WARN( "Missing or invalid 'currentAssignment' field" );
		}

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Parsed assignment change: {} → {} in {:.2f} ms", oldAssignment, currentAssignment, duration.count() / 1000.0 );

		return GmodVersioningAssignmentChangeDto( std::move( oldAssignment ), std::move( currentAssignment ) );
	}

	GmodVersioningAssignmentChangeDto GmodVersioningAssignmentChangeDto::fromJson( const rapidjson::Value& json )
	{
		auto dto = tryFromJson( json );
		if ( !dto )
		{
			SPDLOG_ERROR( "Failed to parse GmodVersioningAssignmentChangeDto from JSON" );
			throw std::invalid_argument( "Invalid JSON for GmodVersioningAssignmentChangeDto" );
		}
		return std::move( *dto );
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
		SPDLOG_INFO( "Created GmodNodeConversionDto: source={}, target={}, operations={}", m_source, m_target, m_operations.size() );
	}

	//-------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------

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

	//-------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------

	std::optional<GmodNodeConversionDto> GmodNodeConversionDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse node conversion from JSON" );

		if ( !json.IsObject() )
		{
			SPDLOG_ERROR( "JSON value for GmodNodeConversionDto is not an object" );
			return std::nullopt;
		}

		OperationSet operations;
		std::string source;
		std::string target;
		std::string oldAssignment;
		std::string newAssignment;
		bool deleteAssignment = false;

		if ( json.HasMember( OPERATIONS_KEY ) && json[OPERATIONS_KEY].IsArray() )
		{
			size_t opCount = json[OPERATIONS_KEY].Size();
			SPDLOG_DEBUG( "Found {} operations", opCount );
			operations.reserve( opCount );

			for ( const auto& operation : json[OPERATIONS_KEY].GetArray() )
			{
				if ( operation.IsString() )
				{
					operations.insert( internString( operation.GetString() ) );
				}
			}
		}

		if ( json.HasMember( SOURCE_KEY ) && json[SOURCE_KEY].IsString() )
		{
			source = internString( json[SOURCE_KEY].GetString() );
		}

		if ( json.HasMember( TARGET_KEY ) && json[TARGET_KEY].IsString() )
		{
			target = internString( json[TARGET_KEY].GetString() );
		}

		if ( json.HasMember( OLD_ASSIGNMENT_KEY ) && json[OLD_ASSIGNMENT_KEY].IsString() )
			oldAssignment = json[OLD_ASSIGNMENT_KEY].GetString();

		if ( json.HasMember( NEW_ASSIGNMENT_KEY ) && json[NEW_ASSIGNMENT_KEY].IsString() )
			newAssignment = json[NEW_ASSIGNMENT_KEY].GetString();

		if ( json.HasMember( DELETE_ASSIGNMENT_KEY ) && json[DELETE_ASSIGNMENT_KEY].IsBool() )
			deleteAssignment = json[DELETE_ASSIGNMENT_KEY].GetBool();

		if ( operations.empty() )
		{
			SPDLOG_WARN( "Node conversion has no operations: source={}, target={}", source, target );
		}
		if ( source.empty() && target.empty() )
		{
			SPDLOG_WARN( "Node conversion has empty source and target" );
		}

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Parsed node conversion: source={}, target={}, operations={} in {:.2f} ms", source, target, operations.size(), duration.count() / 1000.0 );

		return GmodNodeConversionDto(
			std::move( operations ),
			std::move( source ),
			std::move( target ),
			std::move( oldAssignment ),
			std::move( newAssignment ),
			deleteAssignment );
	}

	GmodNodeConversionDto GmodNodeConversionDto::fromJson( const rapidjson::Value& json )
	{
		auto dto = tryFromJson( json );
		if ( !dto )
		{
			SPDLOG_ERROR( "Failed to parse GmodNodeConversionDto from JSON" );
			throw std::invalid_argument( "Invalid JSON for GmodNodeConversionDto" );
		}
		return std::move( *dto );
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

	GmodVersioningDto::GmodVersioningDto( std::string visVersion, ItemsMap items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
		SPDLOG_INFO( "Created GmodVersioningDto for VIS version: {}", m_visVersion );
	}

	//-------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------

	const std::string& GmodVersioningDto::visVersion() const
	{
		return m_visVersion;
	}

	const GmodVersioningDto::ItemsMap& GmodVersioningDto::items() const
	{
		return m_items;
	}

	//-------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------

	std::optional<GmodVersioningDto> GmodVersioningDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse GMOD versioning data from JSON" );

		if ( !json.IsObject() )
		{
			SPDLOG_ERROR( "JSON value is not an object" );
			return std::nullopt;
		}

		std::string visVersion;
		ItemsMap items;

		if ( json.HasMember( VIS_RELEASE_KEY ) && json[VIS_RELEASE_KEY].IsString() )
		{
			visVersion = json[VIS_RELEASE_KEY].GetString();
			SPDLOG_INFO( "GMOD versioning for VIS version: {}", visVersion );
		}
		else
		{
			SPDLOG_ERROR( "Missing or invalid 'visRelease' field" );
			return std::nullopt;
		}

		if ( json.HasMember( ITEMS_KEY ) && json[ITEMS_KEY].IsObject() )
		{
			size_t itemCount = json[ITEMS_KEY].MemberCount();
			SPDLOG_INFO( "Found {} node conversion items to parse", itemCount );

			if ( itemCount > 10000 )
			{
				SPDLOG_INFO( "Large versioning dataset detected, consider using parallel parsing implementation" );
			}

			items.reserve( itemCount );

			size_t successCount = 0;
			size_t emptyOperationsCount = 0;

			auto parseStartTime = std::chrono::steady_clock::now();

			for ( auto it = json[ITEMS_KEY].MemberBegin(); it != json[ITEMS_KEY].MemberEnd(); ++it )
			{
				if ( it->value.IsObject() )
				{
					auto nodeDto = GmodNodeConversionDto::tryFromJson( it->value );
					if ( nodeDto )
					{
						auto& emplaced = items.emplace( it->name.GetString(), std::move( *nodeDto ) ).first->second;
						successCount++;

						if ( emplaced.operations().empty() )
						{
							emptyOperationsCount++;
						}
					}
					else
					{
						SPDLOG_ERROR( "Error parsing conversion item '{}'", it->name.GetString() );
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
				items.rehash( static_cast<size_t>( static_cast<double>( successCount ) * 1.25 ) );
			}
		}
		else
		{
			SPDLOG_WARN( "No 'items' object found in GMOD versioning data" );
		}

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - startTime );

		if ( items.size() > 1000 )
		{
			size_t approxMemoryBytes = items.size() * ( sizeof( GmodNodeConversionDto ) + sizeof( std::string ) * 4 + 32 ); // estimate
			SPDLOG_INFO( "Large versioning dataset loaded: {} items, ~{} MB estimated memory", items.size(), approxMemoryBytes / ( 1024 * 1024 ) );
		}

		SPDLOG_INFO( "GMOD versioning parsing completed in {} ms", duration.count() );

		return GmodVersioningDto( std::move( visVersion ), std::move( items ) );
	}

	GmodVersioningDto GmodVersioningDto::fromJson( const rapidjson::Value& json )
	{
		auto dto = tryFromJson( json );
		if ( !dto )
		{
			SPDLOG_ERROR( "Failed to parse GmodVersioningDto from JSON" );
			throw std::invalid_argument( "Invalid JSON for GmodVersioningDto" );
		}
		return std::move( *dto );
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
