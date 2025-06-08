/**
 * @file CodebooksDto.cpp
 * @brief Implementation of ISO 19848 codebook data transfer objects
 */

#include "pch.h"

#include "dnv/vista/sdk/CodebooksDto.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr std::string_view NAME_KEY = "name";
		static constexpr std::string_view VALUES_KEY = "values";
		static constexpr std::string_view ITEMS_KEY = "items";
		static constexpr std::string_view VIS_RELEASE_KEY = "visRelease";

		//=====================================================================
		// Helper functions
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
	// CodebookDto Implementation
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	CodebookDto::CodebookDto( std::string name, ValuesMap values )
		: m_name{ std::move( name ) },
		  m_values{ std::move( values ) }
	{
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	std::string_view CodebookDto::name() const noexcept
	{
		return m_name;
	}

	const CodebookDto::ValuesMap& CodebookDto::values() const noexcept
	{
		return m_values;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<CodebookDto> CodebookDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "CodebookDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto nameElement = obj[NAME_KEY];
			if ( nameElement.error() )
			{
				SPDLOG_ERROR( "Codebook JSON missing required '{}' field", NAME_KEY );
				return std::nullopt;
			}

			auto nameOpt = safeGetString( nameElement.value() );
			if ( !nameOpt )
			{
				SPDLOG_ERROR( "Codebook JSON '{}' field is not a string", NAME_KEY );
				return std::nullopt;
			}

			std::string tempName = std::move( *nameOpt );

			ValuesMap tempValues;
			size_t totalValuesParsed = 0;

			auto valuesElement = obj[VALUES_KEY];
			if ( !valuesElement.error() )
			{
				auto valuesObjOpt = safeGetObject( valuesElement.value() );
				if ( valuesObjOpt )
				{
					auto valuesObj = *valuesObjOpt;
					tempValues.reserve( valuesObj.size() );

					for ( auto [key, value] : valuesObj )
					{
						std::string groupName( key );

						auto arrayOpt = safeGetArray( value );
						if ( !arrayOpt )
						{
							SPDLOG_WARN( "Group '{}' values are not in array format for codebook '{}', skipping", groupName, tempName );
							continue;
						}

						ValueGroup groupValues;
						auto arr = *arrayOpt;
						groupValues.reserve( arr.size() );

						for ( auto valueElement : arr )
						{
							auto valueStrOpt = safeGetString( valueElement );
							if ( valueStrOpt )
							{
								groupValues.emplace_back( std::move( *valueStrOpt ) );
								totalValuesParsed++;
							}
							else
							{
								SPDLOG_WARN( "Non-string value in group '{}' for codebook '{}', skipping", groupName, tempName );
							}
						}

						tempValues.emplace( std::move( groupName ), std::move( groupValues ) );
					}
				}
				else
				{
					SPDLOG_WARN( "No '{}' object found or not an object for codebook '{}'", VALUES_KEY, tempName );
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
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during CodebookDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<CodebookDto> CodebookDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	CodebookDto CodebookDto::fromJson( simdjson::dom::element element )
	{
		auto dtoOpt = tryFromJson( element );
		if ( !dtoOpt.has_value() )
		{
			throw std::invalid_argument( "Failed to deserialize CodebookDto from simdjson element" );
		}
		return std::move( dtoOpt.value() );
	}

	CodebookDto CodebookDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto dtoOpt = tryFromJsonString( jsonString, parser );
		if ( !dtoOpt.has_value() )
		{
			throw std::invalid_argument( "Failed to deserialize CodebookDto from JSON string" );
		}
		return std::move( dtoOpt.value() );
	}

	std::string CodebookDto::toJsonString() const
	{
		auto startTime = std::chrono::steady_clock::now();

		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"" << NAME_KEY << "\": \"" << escapeJsonString( m_name ) << "\"";

		if ( !m_values.empty() )
		{
			oss << ",\n  \"" << VALUES_KEY << "\": {\n";
			bool first = true;
			for ( const auto& [groupName, groupValues] : m_values )
			{
				if ( !first )
					oss << ",\n";
				first = false;

				oss << "    \"" << escapeJsonString( groupName ) << "\": [";
				bool firstValue = true;
				for ( const auto& value : groupValues )
				{
					if ( !firstValue )
						oss << ", ";
					firstValue = false;
					oss << "\"" << escapeJsonString( value ) << "\"";
				}
				oss << "]";
			}
			oss << "\n  }";
		}

		oss << "\n}";

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized CodebookDto '{}' with {} groups in {} ms", m_name, m_values.size(), duration.count() );

		return oss.str();
	}

	//=====================================================================
	// CodebooksDto Implementation
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	CodebooksDto::CodebooksDto( std::string visVersion, Items items )
		: m_visVersion{ std::move( visVersion ) },
		  m_items{ std::move( items ) }
	{
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	const std::string& CodebooksDto::visVersion() const noexcept
	{
		return m_visVersion;
	}

	const CodebooksDto::Items& CodebooksDto::items() const noexcept
	{
		return m_items;
	}

	//----------------------------------------------
	// Serialization
	//----------------------------------------------

	std::optional<CodebooksDto> CodebooksDto::tryFromJson( simdjson::dom::element element ) noexcept
	{
		auto startTime = std::chrono::steady_clock::now();
		try
		{
			auto objOpt = safeGetObject( element );
			if ( !objOpt )
			{
				SPDLOG_ERROR( "CodebooksDto: Root element is not an object" );
				return std::nullopt;
			}

			auto obj = *objOpt;

			auto visElement = obj[VIS_RELEASE_KEY];
			if ( visElement.error() )
			{
				SPDLOG_ERROR( "Codebooks JSON missing required '{}' field", VIS_RELEASE_KEY );
				return std::nullopt;
			}

			auto visVersionOpt = safeGetString( visElement.value() );
			if ( !visVersionOpt )
			{
				SPDLOG_ERROR( "Codebooks JSON '{}' field is not a string", VIS_RELEASE_KEY );
				return std::nullopt;
			}

			std::string tempVisVersion = std::move( *visVersionOpt );

			Items tempItems;
			size_t totalItems = 0;
			size_t successCount = 0;

			auto itemsElement = obj[ITEMS_KEY];
			if ( !itemsElement.error() )
			{
				auto itemsArrayOpt = safeGetArray( itemsElement.value() );
				if ( itemsArrayOpt )
				{
					auto itemsArray = *itemsArrayOpt;
					totalItems = itemsArray.size();
					tempItems.reserve( totalItems );

					for ( auto itemElement : itemsArray )
					{
						auto codebookOpt = CodebookDto::tryFromJson( itemElement );
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

					if ( totalItems > 0 && static_cast<double>( successCount ) < static_cast<double>( totalItems ) * 0.9 )
					{
						SPDLOG_WARN( "Shrinking items vector due to high parsing failure rate ({}/{}) for VIS version {}", successCount, totalItems, tempVisVersion );
						tempItems.shrink_to_fit();
					}
				}
				else
				{
					SPDLOG_WARN( "'{}' field is not an array for VIS version {}", ITEMS_KEY, tempVisVersion );
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
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during CodebooksDto parsing: {}", ex.what() );
			return std::nullopt;
		}
	}

	std::optional<CodebooksDto> CodebooksDto::tryFromJsonString( std::string_view jsonString, simdjson::dom::parser& parser ) noexcept
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

	CodebooksDto CodebooksDto::fromJson( simdjson::dom::element element )
	{
		auto dtoOpt = tryFromJson( element );
		if ( !dtoOpt.has_value() )
		{
			throw std::invalid_argument( "Failed to deserialize CodebooksDto from simdjson element" );
		}
		return std::move( dtoOpt.value() );
	}

	CodebooksDto CodebooksDto::fromJsonString( std::string_view jsonString, simdjson::dom::parser& parser )
	{
		auto dtoOpt = tryFromJsonString( jsonString, parser );
		if ( !dtoOpt.has_value() )
		{
			throw std::invalid_argument( "Failed to deserialize CodebooksDto from JSON string" );
		}
		return std::move( dtoOpt.value() );
	}

	std::string CodebooksDto::toJsonString() const
	{
		auto startTime = std::chrono::steady_clock::now();

		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"" << VIS_RELEASE_KEY << "\": \"" << escapeJsonString( m_visVersion ) << "\"";

		if ( !m_items.empty() )
		{
			oss << ",\n  \"" << ITEMS_KEY << "\": [\n";
			bool first = true;
			for ( const auto& item : m_items )
			{
				if ( !first )
					oss << ",\n";
				first = false;

				std::string itemJson = item.toJsonString();
				std::istringstream iss( itemJson );
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
			oss << "\n  ]";
		}

		oss << "\n}";

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - startTime );
		SPDLOG_DEBUG( "Serialized CodebooksDto with {} items for VIS version {} in {} ms", m_items.size(), m_visVersion, duration.count() );

		return oss.str();
	}
}
