/**
 * @file CodebookName.cpp
 * @brief Implementation of codebook name conversion functions
 */

#include "pch.h"

#include "dnv/vista/sdk/CodebookName.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	namespace
	{
		static constexpr const char* QUANTITY_PREFIX = "qty";
		static constexpr const char* CONTENT_PREFIX = "cnt";
		static constexpr const char* CALCULATION_PREFIX = "calc";
		static constexpr const char* STATE_PREFIX = "state";
		static constexpr const char* COMMAND_PREFIX = "cmd";
		static constexpr const char* TYPE_PREFIX = "type";
		static constexpr const char* FUNCTIONAL_SERVICES_PREFIX = "funct.svc";
		static constexpr const char* MAINTENANCE_CATEGORY_PREFIX = "maint.cat";
		static constexpr const char* ACTIVITY_TYPE_PREFIX = "act.type";
		static constexpr const char* POSITION_PREFIX = "pos";
		static constexpr const char* DETAIL_PREFIX = "detail";

		static const std::string VALID_PREFIXES = "qty, cnt, calc, state, cmd, type, funct.svc, maint.cat, act.type, pos, detail";

		static const std::unordered_map<std::string_view, CodebookName> s_prefixMap = {
			{ QUANTITY_PREFIX, CodebookName::Quantity },
			{ CONTENT_PREFIX, CodebookName::Content },
			{ CALCULATION_PREFIX, CodebookName::Calculation },
			{ STATE_PREFIX, CodebookName::State },
			{ COMMAND_PREFIX, CodebookName::Command },
			{ TYPE_PREFIX, CodebookName::Type },
			{ FUNCTIONAL_SERVICES_PREFIX, CodebookName::FunctionalServices },
			{ MAINTENANCE_CATEGORY_PREFIX, CodebookName::MaintenanceCategory },
			{ ACTIVITY_TYPE_PREFIX, CodebookName::ActivityType },
			{ POSITION_PREFIX, CodebookName::Position },
			{ DETAIL_PREFIX, CodebookName::Detail } };
	}
}

namespace dnv::vista::sdk
{
	//=====================================================================
	// Public Methods
	//=====================================================================

	CodebookName CodebookNames::fromPrefix( const std::string_view prefix )
	{
		SPDLOG_TRACE( "Attempting to convert prefix string '{}' to CodebookName enum", prefix );

		if ( prefix.empty() )
		{
			SPDLOG_WARN( "Input prefix is empty." );
			throw std::invalid_argument( "prefix cannot be empty" );
		}

		auto it = s_prefixMap.find( prefix );

		if ( it != s_prefixMap.end() )
		{
			SPDLOG_DEBUG( "Successfully mapped prefix '{}' to CodebookName enum value {}",
				prefix, static_cast<int>( it->second ) );

			return it->second;
		}

		SPDLOG_WARN( "Unknown prefix: '{}'. Valid prefixes are: {}",
			prefix, VALID_PREFIXES );

		std::string errorMsg = "unknown prefix: ";
		errorMsg.append( prefix );
		throw std::invalid_argument( errorMsg );
	}

	std::string_view CodebookNames::toPrefix( CodebookName name )
	{
		SPDLOG_TRACE( "Converting CodebookName enum {} to prefix string",
			static_cast<int>( name ) );

		switch ( name )
		{
			case CodebookName::Position:
				return POSITION_PREFIX;
			case CodebookName::Quantity:
				return QUANTITY_PREFIX;
			case CodebookName::Calculation:
				return CALCULATION_PREFIX;
			case CodebookName::State:
				return STATE_PREFIX;
			case CodebookName::Content:
				return CONTENT_PREFIX;
			case CodebookName::Command:
				return COMMAND_PREFIX;
			case CodebookName::Type:
				return TYPE_PREFIX;
			case CodebookName::FunctionalServices:
				return FUNCTIONAL_SERVICES_PREFIX;
			case CodebookName::MaintenanceCategory:
				return MAINTENANCE_CATEGORY_PREFIX;
			case CodebookName::ActivityType:
				return ACTIVITY_TYPE_PREFIX;
			case CodebookName::Detail:
				return DETAIL_PREFIX;
			default:
			{
				SPDLOG_ERROR( "Unknown codebook: {}", static_cast<int>( name ) );
				throw std::invalid_argument( "unknown codebook: " + std::to_string( static_cast<int>( name ) ) );
			}
		}
	}
}
