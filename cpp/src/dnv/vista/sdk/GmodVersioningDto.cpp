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

		static constexpr const char* VIS_RELEASE_KEY = "visRelease";
		static constexpr const char* ITEMS_KEY = "items";

		static constexpr const char* OLD_ASSIGNMENT_KEY = "oldAssignment";
		static constexpr const char* CURRENT_ASSIGNMENT_KEY = "currentAssignment";
		static constexpr const char* NEW_ASSIGNMENT_KEY = "newAssignment";
		static constexpr const char* DELETE_ASSIGNMENT_KEY = "deleteAssignment";

		static constexpr const char* OPERATIONS_KEY = "operations";
		static constexpr const char* SOURCE_KEY = "source";
		static constexpr const char* TARGET_KEY = "target";

		//=====================================================================
		// Helper Functions
		//=====================================================================

		static const std::string& internString( const std::string& value )
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
	}

	//=====================================================================
	// GMOD Versioning Data Transfer Objects
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	GmodVersioningAssignmentChangeDto::GmodVersioningAssignmentChangeDto( std::string oldAssignment, std::string currentAssignment )
		: m_oldAssignment{ std::move( oldAssignment ) },
		  m_currentAssignment{ std::move( currentAssignment ) }
	{
		SPDLOG_INFO( "Created GmodVersioningAssignmentChangeDto: {} → {}", m_oldAssignment, m_currentAssignment );
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

	std::optional<GmodVersioningAssignmentChangeDto> GmodVersioningAssignmentChangeDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse GmodVersioningAssignmentChangeDto from nlohmann::json" );

		try
		{
			if ( !json.is_object() )
			{
				SPDLOG_ERROR( "JSON value for GmodVersioningAssignmentChangeDto is not an object" );
				return std::nullopt;
			}

			GmodVersioningAssignmentChangeDto dto = json.get<GmodVersioningAssignmentChangeDto>();

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed assignment change: {} → {} in {} µs", dto.oldAssignment(), dto.currentAssignment(), duration.count() );

			return std::optional<GmodVersioningAssignmentChangeDto>{ std::move( dto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "nlohmann::json exception during GmodVersioningAssignmentChangeDto parsing: {}", ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Standard exception during GmodVersioningAssignmentChangeDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	GmodVersioningAssignmentChangeDto GmodVersioningAssignmentChangeDto::fromJson( const nlohmann::json& json )
	{
		try
		{
			return json.get<GmodVersioningAssignmentChangeDto>();
		}
		catch ( const nlohmann::json::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize GmodVersioningAssignmentChangeDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		catch ( const std::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize GmodVersioningAssignmentChangeDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	nlohmann::json GmodVersioningAssignmentChangeDto::toJson() const
	{
		SPDLOG_DEBUG( "Serializing GmodVersioningAssignmentChangeDto to nlohmann::json: {} -> {}", m_oldAssignment, m_currentAssignment );
		return *this;
	}

	//----------------------------------------------
	// Private Serialization Methods
	//----------------------------------------------

	void to_json( nlohmann::json& j, const GmodVersioningAssignmentChangeDto& dto )
	{
		j = nlohmann::json{
			{ OLD_ASSIGNMENT_KEY, dto.m_oldAssignment },
			{ CURRENT_ASSIGNMENT_KEY, dto.m_currentAssignment } };
	}

	void from_json( const nlohmann::json& j, GmodVersioningAssignmentChangeDto& dto )
	{
		if ( !j.contains( OLD_ASSIGNMENT_KEY ) || !j.at( OLD_ASSIGNMENT_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "GmodVersioningAssignmentChangeDto JSON missing required '{}' field or not a string", OLD_ASSIGNMENT_KEY ), nullptr );
		}
		if ( !j.contains( CURRENT_ASSIGNMENT_KEY ) || !j.at( CURRENT_ASSIGNMENT_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "GmodVersioningAssignmentChangeDto JSON missing required '{}' field or not a string", CURRENT_ASSIGNMENT_KEY ), nullptr );
		}

		dto.m_oldAssignment = internString( j.at( OLD_ASSIGNMENT_KEY ).get<std::string>() );
		dto.m_currentAssignment = internString( j.at( CURRENT_ASSIGNMENT_KEY ).get<std::string>() );

		if ( dto.m_oldAssignment.empty() )
		{
			SPDLOG_WARN( "Empty 'oldAssignment' field found in GmodVersioningAssignmentChangeDto" );
		}
		if ( dto.m_currentAssignment.empty() )
		{
			SPDLOG_WARN( "Empty 'currentAssignment' field found in GmodVersioningAssignmentChangeDto" );
		}
	}

	//=====================================================================
	// GMOD Node Conversion Transfer Object
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
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
		SPDLOG_INFO( "Created GmodNodeConversionDto: source={}, target={}, operations={}", m_source, m_target, m_operations.size() );
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

	std::optional<GmodNodeConversionDto> GmodNodeConversionDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Attempting to parse GmodNodeConversionDto from nlohmann::json" );

		try
		{
			if ( !json.is_object() )
			{
				SPDLOG_ERROR( "JSON value for GmodNodeConversionDto is not an object" );
				return std::nullopt;
			}

			GmodNodeConversionDto dto = json.get<GmodNodeConversionDto>();

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Parsed node conversion: source={}, target={}, operations={} in {} µs", dto.source(), dto.target(), dto.operations().size(), duration.count() );

			return std::optional<GmodNodeConversionDto>{ std::move( dto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "nlohmann::json exception during GmodNodeConversionDto parsing: {}", ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Standard exception during GmodNodeConversionDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	GmodNodeConversionDto GmodNodeConversionDto::fromJson( const nlohmann::json& json )
	{
		try
		{
			return json.get<GmodNodeConversionDto>();
		}
		catch ( const nlohmann::json::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize GmodNodeConversionDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		catch ( const std::exception& e )
		{
			std::string errorMsg = fmt::format( "Failed to deserialize GmodNodeConversionDto from JSON: {}", e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	nlohmann::json GmodNodeConversionDto::toJson() const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_DEBUG( "Serializing GmodNodeConversionDto to nlohmann::json: source={}, target={}", m_source, m_target );
		nlohmann::json j = *this;
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized node conversion in {} µs", duration.count() );
		return j;
	}

	//----------------------------------------------
	// Private Serialization Methods
	//----------------------------------------------

	void to_json( nlohmann::json& j, const GmodNodeConversionDto& dto )
	{
		j = nlohmann::json{
			{ OPERATIONS_KEY, dto.m_operations },
			{ SOURCE_KEY, dto.m_source },
			{ TARGET_KEY, dto.m_target },
			{ OLD_ASSIGNMENT_KEY, dto.m_oldAssignment },
			{ NEW_ASSIGNMENT_KEY, dto.m_newAssignment },
			{ DELETE_ASSIGNMENT_KEY, dto.m_deleteAssignment } };
	}

	void from_json( const nlohmann::json& j, GmodNodeConversionDto& dto )
	{
		if ( j.contains( OPERATIONS_KEY ) )
		{
			if ( !j.at( OPERATIONS_KEY ).is_array() )
			{
				throw nlohmann::json::type_error::create( 302, fmt::format( "GmodNodeConversionDto JSON field '{}' is not an array", OPERATIONS_KEY ), nullptr );
			}
			dto.m_operations.clear();
			const auto& opsArray = j.at( OPERATIONS_KEY );
			dto.m_operations.reserve( opsArray.size() );
			for ( const auto& op : opsArray )
			{
				if ( !op.is_string() )
				{
					throw nlohmann::json::type_error::create( 302, fmt::format( "GmodNodeConversionDto JSON field '{}' contains non-string element", OPERATIONS_KEY ), nullptr );
				}
				dto.m_operations.insert( internString( op.get<std::string>() ) );
			}
		}
		else
		{
			dto.m_operations.clear();
			SPDLOG_WARN( "GmodNodeConversionDto JSON missing optional '{}' field", OPERATIONS_KEY );
		}

		if ( j.contains( SOURCE_KEY ) )
		{
			if ( !j.at( SOURCE_KEY ).is_string() )
			{
				throw nlohmann::json::type_error::create( 302, fmt::format( "GmodNodeConversionDto JSON field '{}' is not a string", SOURCE_KEY ), nullptr );
			}
			dto.m_source = internString( j.at( SOURCE_KEY ).get<std::string>() );
		}
		else
		{
			dto.m_source.clear();
			SPDLOG_WARN( "GmodNodeConversionDto JSON missing optional '{}' field", SOURCE_KEY );
		}

		if ( j.contains( TARGET_KEY ) )
		{
			if ( !j.at( TARGET_KEY ).is_string() )
			{
				throw nlohmann::json::type_error::create( 302, fmt::format( "GmodNodeConversionDto JSON field '{}' is not a string", TARGET_KEY ), nullptr );
			}
			dto.m_target = internString( j.at( TARGET_KEY ).get<std::string>() );
		}
		else
		{
			dto.m_target.clear();
			SPDLOG_WARN( "GmodNodeConversionDto JSON missing optional '{}' field", TARGET_KEY );
		}

		if ( j.contains( OLD_ASSIGNMENT_KEY ) )
		{
			if ( !j.at( OLD_ASSIGNMENT_KEY ).is_string() )
			{
				throw nlohmann::json::type_error::create( 302, fmt::format( "GmodNodeConversionDto JSON field '{}' is not a string", OLD_ASSIGNMENT_KEY ), nullptr );
			}
			dto.m_oldAssignment = internString( j.at( OLD_ASSIGNMENT_KEY ).get<std::string>() );
		}
		else
		{
			dto.m_oldAssignment.clear();
			SPDLOG_WARN( "GmodNodeConversionDto JSON missing optional '{}' field", OLD_ASSIGNMENT_KEY );
		}

		if ( j.contains( NEW_ASSIGNMENT_KEY ) )
		{
			if ( !j.at( NEW_ASSIGNMENT_KEY ).is_string() )
			{
				throw nlohmann::json::type_error::create( 302, fmt::format( "GmodNodeConversionDto JSON field '{}' is not a string", NEW_ASSIGNMENT_KEY ), nullptr );
			}
			dto.m_newAssignment = internString( j.at( NEW_ASSIGNMENT_KEY ).get<std::string>() );
		}
		else
		{
			dto.m_newAssignment.clear();
			SPDLOG_WARN( "GmodNodeConversionDto JSON missing optional '{}' field", NEW_ASSIGNMENT_KEY );
		}

		if ( j.contains( DELETE_ASSIGNMENT_KEY ) )
		{
			if ( !j.at( DELETE_ASSIGNMENT_KEY ).is_boolean() )
			{
				throw nlohmann::json::type_error::create( 302, fmt::format( "GmodNodeConversionDto JSON field '{}' is not a boolean", DELETE_ASSIGNMENT_KEY ), nullptr );
			}
			dto.m_deleteAssignment = j.at( DELETE_ASSIGNMENT_KEY ).get<bool>();
		}
		else
		{
			dto.m_deleteAssignment = false;
			SPDLOG_WARN( "GmodNodeConversionDto JSON missing optional '{}' field, defaulting to false", DELETE_ASSIGNMENT_KEY );
		}

		if ( dto.m_operations.empty() )
		{
			SPDLOG_WARN( "Node conversion has no operations: source={}, target={}", dto.m_source, dto.m_target );
		}
		if ( dto.m_source.empty() && dto.m_target.empty() )
		{
			SPDLOG_WARN( "Node conversion has empty source and target" );
		}
	}

	//=====================================================================
	// GMOD Versioning Data Transfer Object
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	GmodVersioningDto::GmodVersioningDto( std::string visVersion, ItemsMap items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
		SPDLOG_INFO( "Created GmodVersioningDto for VIS version: {}", m_visVersion );
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

	std::optional<GmodVersioningDto> GmodVersioningDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Attempting to parse GMOD versioning data from nlohmann::json" );

		try
		{
			if ( !json.is_object() )
			{
				SPDLOG_ERROR( "JSON value for GmodVersioningDto is not an object" );
				return std::nullopt;
			}

			GmodVersioningDto dto = json.get<GmodVersioningDto>();

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_INFO( "GMOD versioning parsing completed in {} ms ({} items)", duration.count(), dto.items().size() );

			return std::optional<GmodVersioningDto>{ std::move( dto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			std::string visHint = "[unknown version]";
			try
			{
				if ( json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
				{
					visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}
			SPDLOG_ERROR( "nlohmann::json exception during GmodVersioningDto parsing (hint: visRelease='{}'): {}", visHint, ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			std::string visHint = "[unknown version]";
			try
			{
				if ( json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
				{
					visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}
			SPDLOG_ERROR( "Standard exception during GmodVersioningDto parsing (hint: visRelease='{}'): {}", visHint, ex.what() );
			return std::nullopt;
		}
	}

	GmodVersioningDto GmodVersioningDto::fromJson( const nlohmann::json& json )
	{
		try
		{
			return json.get<GmodVersioningDto>();
		}
		catch ( const nlohmann::json::exception& e )
		{
			std::string visHint = "[unknown version]";
			try
			{
				if ( json.is_object() && json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
				{
					visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}
			std::string errorMsg = fmt::format( "Failed to deserialize GmodVersioningDto from JSON (hint: visRelease='{}'): {}", visHint, e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		catch ( const std::exception& e )
		{
			std::string visHint = "[unknown version]";
			try
			{
				if ( json.is_object() && json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
				{
					visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
				}
			}
			catch ( ... )
			{
			}
			std::string errorMsg = fmt::format( "Failed to deserialize GmodVersioningDto from JSON (hint: visRelease='{}'): {}", visHint, e.what() );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
	}

	nlohmann::json GmodVersioningDto::toJson() const
	{
		auto startTime = std::chrono::steady_clock::now();
		SPDLOG_INFO( "Serializing GMOD versioning data to nlohmann::json, {} items", m_items.size() );
		nlohmann::json j = *this;
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Successfully serialized GMOD versioning data for VIS {} in {} ms", m_visVersion, duration.count() );
		return j;
	}

	//----------------------------------------------
	// Private Serialization Methods
	//----------------------------------------------

	void to_json( nlohmann::json& j, const GmodVersioningDto& dto )
	{
		j = nlohmann::json{
			{ VIS_RELEASE_KEY, dto.m_visVersion } };

		if ( !dto.m_items.empty() )
		{
			[[maybe_unused]] auto serializationStartTime = std::chrono::steady_clock::now();

			j[ITEMS_KEY] = dto.m_items;

			auto serializationEndTime = std::chrono::steady_clock::now();
			auto serializationDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
				serializationEndTime - serializationStartTime );

			if ( !dto.m_items.empty() && serializationDuration.count() > 0 )
			{
				[[maybe_unused]] double serializationRatePerSecond = static_cast<double>( dto.m_items.size() ) * 1000.0 / static_cast<double>( serializationDuration.count() );
				SPDLOG_INFO( "Node serialization rate: {:.1f} items/sec", serializationRatePerSecond );
			}
			else if ( !dto.m_items.empty() )
			{
				SPDLOG_INFO( "Node serialization completed very quickly." );
			}

			size_t emptyOperationsCount = 0;
			for ( const auto& [key, value] : dto.m_items )
			{
				if ( value.operations().empty() )
				{
					emptyOperationsCount++;
				}
			}
			if ( emptyOperationsCount > 0 )
			{
				SPDLOG_WARN( "{} nodes have no operations defined during serialization", emptyOperationsCount );
			}
		}
		else
		{
			j[ITEMS_KEY] = nlohmann::json::object();
		}
	}

	void from_json( const nlohmann::json& j, GmodVersioningDto& dto )
	{
		if ( !j.contains( VIS_RELEASE_KEY ) || !j.at( VIS_RELEASE_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, fmt::format( "GmodVersioningDto JSON missing required '{}' field or not a string", VIS_RELEASE_KEY ), nullptr );
		}
		dto.m_visVersion = j.at( VIS_RELEASE_KEY ).get<std::string>();
		SPDLOG_INFO( "GMOD versioning for VIS version: {}", dto.m_visVersion );

		dto.m_items.clear();
		if ( j.contains( ITEMS_KEY ) )
		{
			if ( !j.at( ITEMS_KEY ).is_object() )
			{
				throw nlohmann::json::type_error::create( 302, fmt::format( "GmodVersioningDto JSON field '{}' is not an object", ITEMS_KEY ), nullptr );
			}

			const auto& itemsObj = j.at( ITEMS_KEY );
			size_t itemCount = itemsObj.size();
			SPDLOG_INFO( "Found {} node conversion items to parse", itemCount );

			if ( itemCount > 10000 )
			{
				SPDLOG_INFO( "Large versioning dataset detected ({}), consider performance implications", itemCount );
			}

			dto.m_items.reserve( itemCount );
			size_t successCount = 0;
			size_t emptyOperationsCount = 0;
			auto parseStartTime = std::chrono::steady_clock::now();

			for ( const auto& [key, value] : itemsObj.items() )
			{
				try
				{
					GmodNodeConversionDto nodeDto = value.get<GmodNodeConversionDto>();
					auto& emplaced = dto.m_items.emplace( key, std::move( nodeDto ) ).first->second;
					successCount++;

					if ( emplaced.operations().empty() )
					{
						emptyOperationsCount++;
					}
				}
				catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
				{
					SPDLOG_ERROR( "Error parsing conversion item '{}': {}", key, ex.what() );
				}
				catch ( [[maybe_unused]] const std::exception& ex )
				{
					SPDLOG_ERROR( "Standard exception parsing conversion item '{}': {}", key, ex.what() );
				}
			}

			auto parseEndTime = std::chrono::steady_clock::now();
			auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>( parseEndTime - parseStartTime );

			if ( parseDuration.count() > 0 )
			{
				[[maybe_unused]] double parseRatePerSecond = static_cast<double>( successCount ) * 1000.0 / static_cast<double>( parseDuration.count() );
				SPDLOG_INFO( "Successfully parsed {}/{} node conversion items ({} with empty operations), rate: {:.1f} items/sec",
					successCount, itemCount, emptyOperationsCount, parseRatePerSecond );
			}
			else if ( itemCount > 0 )
			{
				SPDLOG_INFO( "Successfully parsed {}/{} node conversion items ({} with empty operations) very quickly.",
					successCount, itemCount, emptyOperationsCount );
			}

			if ( successCount > 0 && static_cast<double>( successCount ) < static_cast<double>( itemCount ) * 0.9 )
			{
				SPDLOG_INFO( "Optimizing memory usage after parsing {} of {} items", successCount, itemCount );
				dto.m_items.rehash( static_cast<size_t>( static_cast<double>( successCount ) * 1.25 ) );
			}
		}
		else
		{
			SPDLOG_WARN( "No '{}' object found in GMOD versioning data for VIS version {}", ITEMS_KEY, dto.m_visVersion );
		}

		if ( dto.m_items.size() > 1000 )
		{
			[[maybe_unused]] size_t approxMemoryBytes = dto.m_items.size() * ( sizeof( GmodNodeConversionDto ) + sizeof( std::string ) * 4 + 32 );
			SPDLOG_INFO( "Large versioning dataset loaded: {} items, ~{} MB estimated memory", dto.m_items.size(), approxMemoryBytes / ( 1024 * 1024 ) );
		}
	}
}
