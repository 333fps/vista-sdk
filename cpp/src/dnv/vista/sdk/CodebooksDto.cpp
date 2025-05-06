/**
 * @file CodebooksDto.cpp
 * @brief Implementation of ISO 19848 codebook data transfer objects
 */

#include "pch.h"

#include "dnv/vista/sdk/CodebooksDto.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	static constexpr const char* NAME_KEY = "name";
	static constexpr const char* VALUES_KEY = "values";
	static constexpr const char* ITEMS_KEY = "items";
	static constexpr const char* VIS_RELEASE_KEY = "visRelease";
}

namespace dnv::vista::sdk
{
	//=====================================================================
	// Codebook Data Transfer Object
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	CodebookDto::CodebookDto( std::string name, ValuesMap values )
		: m_name{ std::move( name ) },
		  m_values{ std::move( values ) }
	{
		SPDLOG_INFO( "CodebookDto constructed with name '{}' containing {} value groups", m_name, m_values.size() );
	}

	//----------------------------------------------
	// Accessor
	//----------------------------------------------

	const std::string& CodebookDto::name() const
	{
		return m_name;
	}

	const CodebookDto::ValuesMap& CodebookDto::values() const
	{
		return m_values;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<CodebookDto> CodebookDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			if ( !json.contains( NAME_KEY ) || !json.at( NAME_KEY ).is_string() )
			{
				SPDLOG_ERROR( "Codebook JSON missing required '{}' field or field is not a string", NAME_KEY );

				return std::nullopt;
			}

			std::string tempName = json.at( NAME_KEY ).get<std::string>();
			SPDLOG_DEBUG( "Attempting to parse CodebookDto with name: {}", tempName );

			ValuesMap tempValues;
			size_t totalValuesParsed = 0;

			if ( json.contains( VALUES_KEY ) )
			{
				if ( !json.at( VALUES_KEY ).is_object() )
				{
					SPDLOG_WARN( "No '{}' object found or not an object for codebook '{}'", VALUES_KEY, tempName );
				}
				else
				{
					/* Each key is a group name, and its value should be an array of strings. */
					const auto& valuesObject = json.at( VALUES_KEY );
					tempValues.reserve( valuesObject.size() );

					for ( const auto& [groupName, groupValueJson] : valuesObject.items() )
					{
						if ( !groupValueJson.is_array() )
						{
							SPDLOG_WARN( "Group '{}' values are not in array format for codebook '{}', skipping", groupName, tempName );

							continue;
						}

						ValueGroup groupValues;
						try
						{
							/* Attempt to parse the array of strings for the current group */
							groupValues = groupValueJson.get<ValueGroup>();
							totalValuesParsed += groupValues.size();
							tempValues.emplace( groupName, std::move( groupValues ) );
						}
						catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
						{
							SPDLOG_WARN( "Error parsing values for group '{}' in codebook '{}': {}. Skipping group.", groupName, tempName, ex.what() );
						}
					}

					SPDLOG_DEBUG( "Parsed {} groups with {} total values for codebook '{}'", tempValues.size(), totalValuesParsed, tempName );
				}
			}
			else
			{
				SPDLOG_WARN( "No '{}' object found for codebook '{}'", VALUES_KEY, tempName );
			}

			/* Construct the final DTO using successfully parsed data */
			CodebookDto resultDto( std::move( tempName ), std::move( tempValues ) );

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Successfully parsed CodebookDto '{}' in {} ms", resultDto.name(), duration.count() );

			return std::optional<CodebookDto>{ std::move( resultDto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			std::string nameHint = "[unknown name]";
			if ( json.contains( NAME_KEY ) && json.at( NAME_KEY ).is_string() )
			{
				nameHint = json.at( NAME_KEY ).get<std::string>();
			}

			SPDLOG_ERROR( "JSON exception during CodebookDto parsing (hint: name='{}'): {}", nameHint, ex.what() );

			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			std::string nameHint = "[unknown name]";
			if ( json.contains( NAME_KEY ) && json.at( NAME_KEY ).is_string() )
			{
				nameHint = json.at( NAME_KEY ).get<std::string>();
			}

			SPDLOG_ERROR( "Standard exception during CodebookDto parsing (hint: name='{}'): {}", nameHint, ex.what() );

			return std::nullopt;
		}
	}

	CodebookDto CodebookDto::fromJson( const nlohmann::json& json )
	{
		auto dtoOpt = CodebookDto::tryFromJson( json );
		if ( !dtoOpt.has_value() )
		{
			std::string nameHint = "[unknown name]";
			if ( json.is_object() && json.contains( NAME_KEY ) && json.at( NAME_KEY ).is_string() )
			{
				nameHint = json.at( NAME_KEY ).get<std::string>();
			}

			std::string errorMsg = fmt::format( "Failed to deserialize CodebookDto from JSON (hint: name='{}')", nameHint );
			throw std::invalid_argument( errorMsg );
		}

		return std::move( dtoOpt.value() );
	}

	nlohmann::json CodebookDto::toJson() const
	{
		auto startTime = std::chrono::steady_clock::now();

		/* Directly construct JSON object from members */
		nlohmann::json obj = { { NAME_KEY, m_name }, { VALUES_KEY, m_values } };

		SPDLOG_DEBUG( "Serialized CodebookDto '{}' with {} groups", m_name, m_values.size() );
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Serialized CodebookDto '{}' with {} groups in {} ms", m_name, m_values.size(), duration.count() );

		return obj;
	}

	//-------------------------------------------------------------------
	// Private Serialization Methods
	//-------------------------------------------------------------------

	void to_json( nlohmann::json& j, const CodebookDto& dto )
	{
		/* ADL hook for nlohmann::json serialization */
		j = nlohmann::json{ { NAME_KEY, dto.name() }, { VALUES_KEY, dto.values() } };
	}

	void from_json( const nlohmann::json& j, CodebookDto& dto )
	{
		/* ADL hook for nlohmann::json deserialization. Direct member assignment and throws on error, unlike tryFromJson which returns std::optional. */
		if ( !j.contains( NAME_KEY ) || !j.at( NAME_KEY ).is_string() )
		{
			throw nlohmann::json::parse_error::create( 101, 0u, "Codebook JSON missing required 'name' field or field is not a string", nullptr );
		}

		std::string tempName = j.at( NAME_KEY ).get<std::string>();
		CodebookDto::ValuesMap tempValues;
		if ( j.contains( VALUES_KEY ) )
		{
			if ( !j.at( VALUES_KEY ).is_object() )
			{
				SPDLOG_WARN( "Codebook JSON 'values' field is not an object for name '{}'", tempName );
			}
			else
			{
				tempValues = j.at( VALUES_KEY ).get<CodebookDto::ValuesMap>();
			}
		}
		else
		{
			SPDLOG_WARN( "Codebook JSON missing 'values' field for name '{}'", tempName );
		}

		dto.m_name = std::move( tempName );
		dto.m_values = std::move( tempValues );
	}

	//=====================================================================
	// Codebooks Data Transfer Object
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	CodebooksDto::CodebooksDto( std::string visVersion, Items items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
		SPDLOG_INFO( "CodebooksDto constructed with VIS version '{}' containing {} codebook items", m_visVersion, m_items.size() );
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& CodebooksDto::visVersion() const
	{
		return m_visVersion;
	}

	const CodebooksDto::Items& CodebooksDto::items() const
	{
		return m_items;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<CodebooksDto> CodebooksDto::tryFromJson( const nlohmann::json& json )
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			if ( !json.contains( VIS_RELEASE_KEY ) || !json.at( VIS_RELEASE_KEY ).is_string() )
			{
				SPDLOG_ERROR( "Codebooks JSON missing required '{}' field or field is not a string", VIS_RELEASE_KEY );

				return std::nullopt;
			}

			std::string tempVisVersion = json.at( VIS_RELEASE_KEY ).get<std::string>();
			SPDLOG_DEBUG( "Attempting to parse CodebooksDto for VIS version: {}", tempVisVersion );

			Items tempItems;
			size_t totalItems = 0;
			size_t successCount = 0;

			if ( json.contains( ITEMS_KEY ) )
			{
				if ( !json.at( ITEMS_KEY ).is_array() )
				{
					SPDLOG_WARN( "'{}' field is not an array for VIS version {}", ITEMS_KEY, tempVisVersion );
				}
				else
				{
					/* The "items" key should contain a JSON array of codebook objects. */
					const auto& itemsArray = json.at( ITEMS_KEY );
					totalItems = itemsArray.size();
					tempItems.reserve( totalItems );
					SPDLOG_DEBUG( "Found {} codebook items to parse", totalItems );

					for ( const auto& itemJson : itemsArray )
					{
						/* Recursively parse each codebook item using its own tryFromJson. */
						auto codebookOpt = CodebookDto::tryFromJson( itemJson );
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

					/* If parsing failed for more than 10% of items, shrink the vector to potentially save memory. */
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

			/* Construct the final DTO using successfully parsed data */
			CodebooksDto resultDto( std::move( tempVisVersion ), std::move( tempItems ) );

			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
			SPDLOG_DEBUG( "Successfully parsed CodebooksDto with {} items for VIS version {} in {} ms", resultDto.items().size(), resultDto.visVersion(), duration.count() );

			return std::optional<CodebooksDto>{ std::move( resultDto ) };
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			std::string visHint = "[unknown version]";
			if ( json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
			{
				visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
			}

			SPDLOG_ERROR( "JSON exception during CodebooksDto parsing (hint: visRelease='{}'): {}", visHint, ex.what() );

			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			std::string visHint = "[unknown version]";
			if ( json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
			{
				visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
			}

			SPDLOG_ERROR( "Standard exception during CodebooksDto parsing (hint: visRelease='{}'): {}", visHint, ex.what() );

			return std::nullopt;
		}
	}

	CodebooksDto CodebooksDto::fromJson( const nlohmann::json& json )
	{
		auto dtoOpt = CodebooksDto::tryFromJson( json );
		if ( !dtoOpt.has_value() )
		{
			std::string visHint = "[unknown version]";
			if ( json.is_object() && json.contains( VIS_RELEASE_KEY ) && json.at( VIS_RELEASE_KEY ).is_string() )
			{
				visHint = json.at( VIS_RELEASE_KEY ).get<std::string>();
			}

			std::string errorMsg = fmt::format( "Failed to deserialize CodebooksDto from JSON (hint: visRelease='{}')", visHint );
			throw std::invalid_argument( errorMsg );
		}

		return std::move( dtoOpt.value() );
	}

	nlohmann::json CodebooksDto::toJson() const
	{
		auto startTime = std::chrono::steady_clock::now();

		/* Directly construct JSON object from members */
		nlohmann::json obj = { { VIS_RELEASE_KEY, m_visVersion }, { ITEMS_KEY, m_items } };

		SPDLOG_DEBUG( "Serialized CodebooksDto with {} items for VIS version {}", m_items.size(), m_visVersion );
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_INFO( "Serialized CodebooksDto with {} items for VIS version {} in {} ms", m_items.size(), m_visVersion, duration.count() );

		return obj;
	}
}
