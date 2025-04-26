/**
 * @file CodebooksDto.cpp
 * @brief Implementation of data transfer objects for ISO 19848 codebook serialization
 */

#include "pch.h"

#include "dnv/vista/sdk/CodebooksDto.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------

	static constexpr const char* NAME_KEY = "name";
	static constexpr const char* VALUES_KEY = "values";
	static constexpr const char* ITEMS_KEY = "items";
	static constexpr const char* VIS_RELEASE_KEY = "visRelease";

	//-------------------------------------------------------------------
	// CodebookDto Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	CodebookDto::CodebookDto( std::string name, std::unordered_map<std::string, std::vector<std::string>> values )
		: m_name( std::move( name ) ), m_values( std::move( values ) )
	{
	}

	//-------------------------------------------------------------------
	// Accessor Methods
	//-------------------------------------------------------------------

	const std::string& CodebookDto::name() const
	{
		return m_name;
	}

	const std::unordered_map<std::string, std::vector<std::string>>& CodebookDto::values() const
	{
		return m_values;
	}

	//-------------------------------------------------------------------
	// Serialization Methods
	//-------------------------------------------------------------------

	CodebookDto CodebookDto::fromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();

		CodebookDto dto;

		if ( !json.HasMember( NAME_KEY ) || !json[NAME_KEY].IsString() )
		{
			std::string errorMsg = fmt::format( "Codebook JSON missing required '{}' field or field is not a string", NAME_KEY );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}

		dto.m_name = json[NAME_KEY].GetString();
		SPDLOG_DEBUG( "Parsing CodebookDto with name: {}", dto.m_name );

		if ( json.HasMember( VALUES_KEY ) && json[VALUES_KEY].IsObject() )
		{
			size_t totalValues = 0;
			for ( auto it = json[VALUES_KEY].MemberBegin(); it != json[VALUES_KEY].MemberEnd(); ++it )
			{
				const char* groupName = it->name.GetString();

				if ( !it->value.IsArray() )
				{
					SPDLOG_WARN( "Group '{}' values are not in array format, skipping", groupName );
					continue;
				}

				std::vector<std::string> groupValues;
				groupValues.reserve( it->value.Size() );

				for ( const auto& value : it->value.GetArray() )
				{
					if ( value.IsString() )
					{
						groupValues.emplace_back( value.GetString() );
					}
					else
					{
						SPDLOG_WARN( "Non-string value found in group '{}', skipping", groupName );
					}
				}

				if ( !groupValues.empty() )
				{
					totalValues += groupValues.size();
					dto.m_values[groupName] = std::move( groupValues );
				}
			}

			SPDLOG_DEBUG( "Parsed {} groups with {} total values for codebook '{}'", dto.m_values.size(), totalValues, dto.m_name );
		}
		else
		{
			SPDLOG_WARN( "No values found for codebook '{}'", dto.m_name );
		}

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );

		SPDLOG_DEBUG( "Parsed CodebookDto '{}' in {} ms", dto.m_name, duration.count() );

		return dto;
	}

	bool CodebookDto::tryFromJson( const rapidjson::Value& json, CodebookDto& dto )
	{
		try
		{
			dto = fromJson( json );
			SPDLOG_DEBUG( "Successfully parsed CodebookDto '{}' with {} value groups", dto.m_name, dto.m_values.size() );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing CodebookDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value CodebookDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		auto startTime = std::chrono::steady_clock::now();

		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( rapidjson::StringRef( NAME_KEY ), rapidjson::Value( rapidjson::StringRef( m_name.c_str() ), allocator ), allocator );

		rapidjson::Value valuesObj( rapidjson::kObjectType );

		for ( const auto& group : m_values )
		{
			rapidjson::Value groupValues( rapidjson::kArrayType );

			{
				const auto size = group.second.size();
				if ( size > std::numeric_limits<rapidjson::SizeType>::max() )
				{
					SPDLOG_WARN( "Vector size {} exceeds maximum RapidJSON capacity, truncating", size );
				}
				groupValues.Reserve( static_cast<rapidjson::SizeType>( std::min( size, static_cast<size_t>( std::numeric_limits<rapidjson::SizeType>::max() ) ) ), allocator );
			}

			for ( const auto& value : group.second )
			{
				groupValues.PushBack( rapidjson::Value( rapidjson::StringRef( value.c_str() ), allocator ), allocator );
			}

			valuesObj.AddMember( rapidjson::Value( rapidjson::StringRef( group.first.c_str() ), allocator ), groupValues, allocator );
		}

		obj.AddMember( rapidjson::StringRef( VALUES_KEY ), valuesObj, allocator );
		SPDLOG_DEBUG( "Serialized CodebookDto '{}' with {} groups", m_name, m_values.size() );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Serialized CodebookDto '{}' with {} groups in {} ms", m_name, m_values.size(), duration.count() );

		return obj;
	}

	//-------------------------------------------------------------------
	// CodebooksDto Implementation
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	CodebooksDto::CodebooksDto( std::string visVersion, std::vector<CodebookDto> items )
		: m_visVersion( std::move( visVersion ) ), m_items( std::move( items ) )
	{
	}

	//-------------------------------------------------------------------
	// Accessor Methods
	//-------------------------------------------------------------------

	const std::string& CodebooksDto::visVersion() const
	{
		return m_visVersion;
	}

	const std::vector<CodebookDto>& CodebooksDto::items() const
	{
		return m_items;
	}

	//-------------------------------------------------------------------
	// Serialization Methods
	//-------------------------------------------------------------------

	CodebooksDto CodebooksDto::fromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();

		CodebooksDto dto;

		if ( !json.HasMember( VIS_RELEASE_KEY ) || !json[VIS_RELEASE_KEY].IsString() )
		{
			std::string errorMsg = fmt::format( "Codebooks JSON missing required '{}' field or field is not a string", VIS_RELEASE_KEY );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}

		dto.m_visVersion = json[VIS_RELEASE_KEY].GetString();
		SPDLOG_DEBUG( "Parsing CodebooksDto for VIS version: {}", dto.m_visVersion );

		if ( json.HasMember( ITEMS_KEY ) && json[ITEMS_KEY].IsArray() )
		{
			size_t totalItems = json[ITEMS_KEY].Size();
			dto.m_items.reserve( totalItems );

			SPDLOG_DEBUG( "Found {} codebook items to parse", totalItems );

			size_t successCount = 0;
			for ( const auto& item : json[ITEMS_KEY].GetArray() )
			{
				try
				{
					dto.m_items.emplace_back( CodebookDto::fromJson( item ) );
					successCount++;
				}
				catch ( const std::exception& e )
				{
					SPDLOG_WARN( "Skipping invalid codebook: {}", e.what() );
				}
			}

			SPDLOG_DEBUG( "Successfully parsed {}/{} codebooks for VIS version {}", successCount, totalItems, dto.m_visVersion );

			dto.m_items.shrink_to_fit();
		}
		else
		{
			SPDLOG_WARN( "No 'items' array found in CodebooksDto or not an array" );
		}

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );

		SPDLOG_DEBUG( "Parsed CodebooksDto with {} items in {} ms", dto.m_items.size(), duration.count() );

		return dto;
	}

	bool CodebooksDto::tryFromJson( const rapidjson::Value& json, CodebooksDto& dto )
	{
		try
		{
			dto = fromJson( json );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing CodebooksDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value CodebooksDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		auto startTime = std::chrono::steady_clock::now();

		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( rapidjson::StringRef( VIS_RELEASE_KEY ), rapidjson::Value( rapidjson::StringRef( m_visVersion.c_str() ), allocator ), allocator );

		rapidjson::Value itemsArray( rapidjson::kArrayType );
		{
			const auto size = m_items.size();
			if ( size > std::numeric_limits<rapidjson::SizeType>::max() )
			{
				SPDLOG_WARN( "Array size {} exceeds maximum RapidJSON capacity, truncating", size );
			}
			itemsArray.Reserve( static_cast<rapidjson::SizeType>( std::min( size, static_cast<size_t>( std::numeric_limits<rapidjson::SizeType>::max() ) ) ), allocator );
		}

		for ( const auto& item : m_items )
		{
			itemsArray.PushBack( item.toJson( allocator ), allocator );
		}

		obj.AddMember( rapidjson::StringRef( ITEMS_KEY ), itemsArray, allocator );

		SPDLOG_DEBUG( "Serialized CodebooksDto with {} items for VIS version {}", m_items.size(), m_visVersion );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Serialized CodebooksDto with {} items for VIS version {} in {} ms", m_items.size(), m_visVersion, duration.count() );

		return obj;
	}
}
