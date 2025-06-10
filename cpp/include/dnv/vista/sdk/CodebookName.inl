/**
 * @file CodebookName.inl
 * @brief Inline implementations for performance-critical CodebookName operations
 */

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr std::string_view POSITION_PREFIX = "pos";
		static constexpr std::string_view QUANTITY_PREFIX = "qty";
		static constexpr std::string_view CALCULATION_PREFIX = "calc";
		static constexpr std::string_view STATE_PREFIX = "state";
		static constexpr std::string_view CONTENT_PREFIX = "cnt";
		static constexpr std::string_view COMMAND_PREFIX = "cmd";
		static constexpr std::string_view TYPE_PREFIX = "type";
		static constexpr std::string_view FUNCTIONAL_SERVICES_PREFIX = "funct.svc";
		static constexpr std::string_view MAINTENANCE_CATEGORY_PREFIX = "maint.cat";
		static constexpr std::string_view ACTIVITY_TYPE_PREFIX = "act.type";
		static constexpr std::string_view DETAIL_PREFIX = "detail";

		//=====================================================================
		// Enum mapping tables
		//=====================================================================

		static const std::unordered_map<std::string_view, CodebookName> s_prefixMap{
			{ POSITION_PREFIX, CodebookName::Position },
			{ QUANTITY_PREFIX, CodebookName::Quantity },
			{ CALCULATION_PREFIX, CodebookName::Calculation },
			{ STATE_PREFIX, CodebookName::State },
			{ CONTENT_PREFIX, CodebookName::Content },
			{ COMMAND_PREFIX, CodebookName::Command },
			{ TYPE_PREFIX, CodebookName::Type },
			{ FUNCTIONAL_SERVICES_PREFIX, CodebookName::FunctionalServices },
			{ MAINTENANCE_CATEGORY_PREFIX, CodebookName::MaintenanceCategory },
			{ ACTIVITY_TYPE_PREFIX, CodebookName::ActivityType },
			{ DETAIL_PREFIX, CodebookName::Detail } };
	}

	//=====================================================================
	// CodebookNames class
	//=====================================================================

	//----------------------------------------------
	// Public static methods
	//----------------------------------------------

	inline CodebookName CodebookNames::fromPrefix( const std::string_view prefix )
	{
		if ( prefix.empty() ) [[unlikely]]
		{
			throw std::invalid_argument( "Prefix cannot be empty." );
		}

		const auto it = s_prefixMap.find( prefix );
		if ( it != s_prefixMap.end() ) [[likely]]
		{
			return it->second;
		}

		throw std::invalid_argument( "Unknown prefix: " + std::string( prefix ) );
	}

	std::string_view CodebookNames::toPrefix( CodebookName name )
	{
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
				throw std::invalid_argument( "Unknown codebook: " + std::to_string( static_cast<int>( name ) ) );
			}
		}
	}

}
