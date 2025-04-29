/**
 * @file CodebooksDto.cpp
 * @brief Implementation of ISO 19848 codebook data transfer objects
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
	// Constructors / Destructor
	//-------------------------------------------------------------------

	CodebookDto::CodebookDto( std::string name, ValuesMap values )
		: m_name{ std::move( name ) },
		  m_values{ std::move( values ) }
	{
		SPDLOG_INFO( "CodebookDto constructed with name '{}' containing {} value groups", m_name, m_values.size() );
	}

	//-------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------

	const std::string& CodebookDto::name() const
	{
		return m_name;
	}

	const CodebookDto::ValuesMap& CodebookDto::values() const
	{
		return m_values;
	}

	//-------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------

	std::optional<CodebookDto> CodebookDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();

		if ( !json.HasMember( NAME_KEY ) || !json[NAME_KEY].IsString() )
		{
			SPDLOG_ERROR( "Codebook JSON missing required '{}' field or field is not a string", NAME_KEY );
			return std::nullopt;
		}
		std::string tempName = json[NAME_KEY].GetString();
		SPDLOG_DEBUG( "Attempting to parse CodebookDto with name: {}", tempName );

		ValuesMap tempValues;
		size_t totalValuesParsed = 0;

		if ( json.HasMember( VALUES_KEY ) )
		{
			if ( !json[VALUES_KEY].IsObject() )
			{
				SPDLOG_WARN( "No '{}' object found or not an object for codebook '{}'", VALUES_KEY, tempName );
			}
			else
			{
				const auto& valuesObject = json[VALUES_KEY];
				tempValues.reserve( valuesObject.MemberCount() );

				for ( auto it = valuesObject.MemberBegin(); it != valuesObject.MemberEnd(); ++it )
				{
					if ( !it->name.IsString() )
					{
						SPDLOG_WARN( "Skipping group with non-string name in codebook '{}'", tempName );
						continue;
					}
					const char* groupName = it->name.GetString();

					if ( !it->value.IsArray() )
					{
						SPDLOG_WARN( "Group '{}' values are not in array format for codebook '{}', skipping", groupName, tempName );
						continue;
					}

					ValueGroup groupValues;
					const auto& jsonArray = it->value.GetArray();
					groupValues.reserve( jsonArray.Size() );

					for ( const auto& value : jsonArray )
					{
						if ( value.IsString() )
						{
							groupValues.emplace_back( value.GetString() );
						}
						else
						{
							SPDLOG_WARN( "Non-string value found in group '{}' for codebook '{}', skipping", groupName, tempName );
						}
					}

					if ( !groupValues.empty() )
					{
						totalValuesParsed += groupValues.size();
						tempValues.emplace( groupName, std::move( groupValues ) );
					}
				}
				SPDLOG_DEBUG( "Parsed {} groups with {} total values for codebook '{}'", tempValues.size(), totalValuesParsed, tempName );
			}
		}
		else
		{
			SPDLOG_WARN( "No '{}' object found for codebook '{}'", VALUES_KEY, tempName );
		}

		CodebookDto resultDto( std::move( tempName ), std::move( tempValues ) );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Successfully parsed CodebookDto '{}' in {} ms", resultDto.name(), duration.count() );

		return std::optional<CodebookDto>{ std::move( resultDto ) };
	}

	CodebookDto CodebookDto::fromJson( const rapidjson::Value& json )
	{
		auto dtoOpt = CodebookDto::tryFromJson( json );
		if ( !dtoOpt.has_value() )
		{
			std::string nameHint = ( json.HasMember( NAME_KEY ) && json[NAME_KEY].IsString() ) ? json[NAME_KEY].GetString() : "[unknown name]";
			std::string errorMsg = fmt::format( "Failed to deserialize CodebookDto from JSON (hint: name='{}')", nameHint );
			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		return std::move( dtoOpt.value() );
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
	// Constructors / Destructor
	//-------------------------------------------------------------------

	CodebooksDto::CodebooksDto( std::string visVersion, Items items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
		SPDLOG_INFO( "CodebooksDto constructed with VIS version '{}' containing {} codebook items", m_visVersion, m_items.size() );
	}

	//-------------------------------------------------------------------
	// Public Interface - Accessor Methods
	//-------------------------------------------------------------------

	const std::string& CodebooksDto::visVersion() const
	{
		return m_visVersion;
	}

	const CodebooksDto::Items& CodebooksDto::items() const
	{
		return m_items;
	}

	//-------------------------------------------------------------------
	// Public Interface - Serialization Methods
	//-------------------------------------------------------------------

	std::optional<CodebooksDto> CodebooksDto::tryFromJson( const rapidjson::Value& json )
	{
		auto startTime = std::chrono::steady_clock::now();

		if ( !json.HasMember( VIS_RELEASE_KEY ) || !json[VIS_RELEASE_KEY].IsString() )
		{
			SPDLOG_ERROR( "Codebooks JSON missing required '{}' field or field is not a string", VIS_RELEASE_KEY );
			return std::nullopt;
		}
		std::string tempVisVersion = json[VIS_RELEASE_KEY].GetString();
		SPDLOG_DEBUG( "Attempting to parse CodebooksDto for VIS version: {}", tempVisVersion );

		Items tempItems;
		size_t totalItems = 0;
		size_t successCount = 0;

		if ( json.HasMember( ITEMS_KEY ) )
		{
			if ( !json[ITEMS_KEY].IsArray() )
			{
				SPDLOG_WARN( "'{}' field is not an array for VIS version {}", ITEMS_KEY, tempVisVersion );
			}
			else
			{
				const auto& itemsArray = json[ITEMS_KEY].GetArray();
				totalItems = itemsArray.Size();
				tempItems.reserve( totalItems );
				SPDLOG_DEBUG( "Found {} codebook items to parse", totalItems );

				for ( const auto& item : itemsArray )
				{
					auto codebookOpt = CodebookDto::tryFromJson( item );
					if ( codebookOpt.has_value() )
					{
						tempItems.emplace_back( std::move( codebookOpt.value() ) );
						successCount++;
					}
					else
					{
						SPDLOG_WARN( "Skipping invalid codebook item during CodebooksDto parsing for VIS version {}.", tempVisVersion );
					}
				}

				SPDLOG_DEBUG( "Successfully parsed {}/{} codebooks for VIS version {}", successCount, totalItems, tempVisVersion );

				if ( totalItems > 0 && static_cast<double>( successCount ) < static_cast<double>( totalItems ) * 0.9 )
				{
					SPDLOG_INFO( "Shrinking items vector due to high parsing failure rate ({}/{}) for VIS version {}", successCount, totalItems, tempVisVersion );
					tempItems.shrink_to_fit();
				}
			}
		}
		else
		{
			SPDLOG_WARN( "No '{}' array found in CodebooksDto for VIS version {}", ITEMS_KEY, tempVisVersion );
		}

		CodebooksDto resultDto( std::move( tempVisVersion ), std::move( tempItems ) );

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Successfully parsed CodebooksDto with {} items for VIS version {} in {} ms", resultDto.items().size(), resultDto.visVersion(), duration.count() );

		return std::optional<CodebooksDto>{ std::move( resultDto ) };
	}

	CodebooksDto CodebooksDto::fromJson( const rapidjson::Value& json )
	{
		auto dtoOpt = CodebooksDto::tryFromJson( json );
		if ( !dtoOpt.has_value() )
		{
			std::string visHint = ( json.HasMember( VIS_RELEASE_KEY ) && json[VIS_RELEASE_KEY].IsString() ) ? json[VIS_RELEASE_KEY].GetString() : "[unknown version]";
			std::string errorMsg = fmt::format( "Failed to deserialize CodebooksDto from JSON (hint: visRelease='{}')", visHint );

			SPDLOG_ERROR( errorMsg );
			throw std::invalid_argument( errorMsg );
		}
		return std::move( dtoOpt.value() );
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
