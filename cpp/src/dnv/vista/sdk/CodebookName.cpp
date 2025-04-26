/**
 * @file CodebookName.cpp
 * @brief Implementation of codebook name conversion functions
 */

#include "pch.h"

#include "dnv/vista/sdk/CodebookName.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------

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

		static const std::unordered_map<std::string_view, CodebookName> g_prefixMap = {
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

	//-------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------

	CodebookName CodebookNames::fromPrefix( const std::string_view prefix )
	{
		if ( prefix.empty() )
		{
			SPDLOG_ERROR( "Rejecting empty codebook prefix" );
			throw std::invalid_argument( "Rejecting empty codebook prefix" );
		}

		SPDLOG_TRACE( "Converting prefix '{}' to CodebookName enum", prefix );

		auto it = g_prefixMap.find( prefix );
		if ( it != g_prefixMap.end() )
		{
			SPDLOG_DEBUG( "Successfully mapped prefix '{}' to CodebookName enum value {}", prefix, static_cast<int>( it->second ) );
			return it->second;
		}

		SPDLOG_WARN( "Unknown prefix: '{}'. Valid prefixes are: {}", prefix, VALID_PREFIXES );

		throw std::invalid_argument( std::string( "unknown prefix: " ).append( prefix ) );
	}

	std::string CodebookNames::toPrefix( CodebookName name )
	{
		SPDLOG_WARN( "Converting CodebookName enum {} to prefix string", static_cast<int>( name ) );

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
				SPDLOG_WARN( "Unknown codebook: {}", static_cast<int>( name ) );
				throw std::invalid_argument( "unknown codebook: " + std::to_string( static_cast<int>( name ) ) );
			}
		}
	}
}
