/**
 * @file LocalIdParsingErrorBuilder.cpp
 * @brief Implementation of the LocalIdParsingErrorBuilder class.
 */

#include "pch.h"

#include "dnv/vista/sdk/LocalIdParsingErrorBuilder.h"

#include "dnv/vista/sdk/ParsingErrors.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Internal helper functions and constants
		//=====================================================================

		//----------------------------------------------
		// String view constants for Enum Conversion
		//----------------------------------------------

		static constexpr std::string_view NAMING_RULE_STATE = "NamingRule";
		static constexpr std::string_view VIS_VERSION_STATE = "VisVersion";
		static constexpr std::string_view PRIMARY_ITEM_STATE = "PrimaryItem";
		static constexpr std::string_view SECONDARY_ITEM_STATE = "SecondaryItem";
		static constexpr std::string_view ITEM_DESCRIPTION_STATE = "ItemDescription";
		static constexpr std::string_view META_QUANTITY_STATE = "MetaQuantity";
		static constexpr std::string_view META_CONTENT_STATE = "MetaContent";
		static constexpr std::string_view META_CALCULATION_STATE = "MetaCalculation";
		static constexpr std::string_view META_STATE_STATE = "MetaState";
		static constexpr std::string_view META_COMMAND_STATE = "MetaCommand";
		static constexpr std::string_view META_TYPE_STATE = "MetaType";
		static constexpr std::string_view META_POSITION_STATE = "MetaPosition";
		static constexpr std::string_view META_DETAIL_STATE = "MetaDetail";
		static constexpr std::string_view EMPTY_STATE_STATE = "EmptyState";
		static constexpr std::string_view FORMATTING_STATE = "Formatting";
		static constexpr std::string_view COMPLETENESS_STATE = "Completeness";
		static constexpr std::string_view NAMING_ENTITY_STATE = "NamingEntity";
		static constexpr std::string_view IMO_NUMBER_STATE = "IMONumber";
		static constexpr std::string_view UNKNOWN = "Unknown";

		static constexpr std::string_view NAMING_RULE_MESSAGE = "Missing or invalid naming rule";
		static constexpr std::string_view VIS_VERSION_MESSAGE = "Missing or invalid vis version";
		static constexpr std::string_view PRIMARY_ITEM_MESSAGE = "Invalid or missing Primary item. Local IDs require atleast primary item and 1 metadata tag.";
		static constexpr std::string_view SECONDARY_ITEM_MESSAGE = "Invalid secondary item";
		static constexpr std::string_view ITEM_DESCRIPTION_MESSAGE = "Missing or invalid /meta prefix";
		static constexpr std::string_view META_QUANTITY_MESSAGE = "Invalid metadata tag: Quantity";
		static constexpr std::string_view META_CONTENT_MESSAGE = "Invalid metadata tag: Content";
		static constexpr std::string_view META_CALCULATION_MESSAGE = "Invalid metadata tag: Calculation";
		static constexpr std::string_view META_STATE_MESSAGE = "Invalid metadata tag: State";
		static constexpr std::string_view META_COMMAND_MESSAGE = "Invalid metadata tag: Command";
		static constexpr std::string_view META_TYPE_MESSAGE = "Invalid metadata tag: Type";
		static constexpr std::string_view META_POSITION_MESSAGE = "Invalid metadata tag: Position";
		static constexpr std::string_view META_DETAIL_MESSAGE = "Invalid metadata tag: Detail";
		static constexpr std::string_view EMPTY_STATE_MESSAGE = "Missing primary path or metadata";
		static constexpr std::string_view FORMATTING_MESSAGE = "Formatting error in Local ID string";
		static constexpr std::string_view COMPLETENESS_MESSAGE = "Incomplete Local ID structure";
		static constexpr std::string_view NAMING_ENTITY_MESSAGE = "Invalid naming entity";
		static constexpr std::string_view IMO_NUMBER_MESSAGE = "Invalid IMO number";
		static constexpr std::string_view UNKNOWN_PARSING_ERROR = "Unknown parsing error";

		//----------------------------------------------
		// Enum to string view conversion function
		//----------------------------------------------

		constexpr std::string_view localIdParsingStateToStringView( LocalIdParsingState state ) noexcept
		{
			switch ( state )
			{
				case LocalIdParsingState::NamingRule:
					return NAMING_RULE_STATE;
				case LocalIdParsingState::VisVersion:
					return VIS_VERSION_STATE;
				case LocalIdParsingState::PrimaryItem:
					return PRIMARY_ITEM_STATE;
				case LocalIdParsingState::SecondaryItem:
					return SECONDARY_ITEM_STATE;
				case LocalIdParsingState::ItemDescription:
					return ITEM_DESCRIPTION_STATE;
				case LocalIdParsingState::MetaQuantity:
					return META_QUANTITY_STATE;
				case LocalIdParsingState::MetaContent:
					return META_CONTENT_STATE;
				case LocalIdParsingState::MetaCalculation:
					return META_CALCULATION_STATE;
				case LocalIdParsingState::MetaState:
					return META_STATE_STATE;
				case LocalIdParsingState::MetaCommand:
					return META_COMMAND_STATE;
				case LocalIdParsingState::MetaType:
					return META_TYPE_STATE;
				case LocalIdParsingState::MetaPosition:
					return META_POSITION_STATE;
				case LocalIdParsingState::MetaDetail:
					return META_DETAIL_STATE;
				case LocalIdParsingState::EmptyState:
					return EMPTY_STATE_STATE;
				case LocalIdParsingState::Formatting:
					return FORMATTING_STATE;
				case LocalIdParsingState::Completeness:
					return COMPLETENESS_STATE;
				case LocalIdParsingState::NamingEntity:
					return NAMING_ENTITY_STATE;
				case LocalIdParsingState::IMONumber:
					return IMO_NUMBER_STATE;
				default:
					return UNKNOWN;
			}
		}

		constexpr std::string_view predefinedErrorMessage( LocalIdParsingState state ) noexcept
		{
			switch ( state )
			{
				case LocalIdParsingState::NamingRule:
					return NAMING_RULE_MESSAGE;
				case LocalIdParsingState::VisVersion:
					return VIS_VERSION_MESSAGE;
				case LocalIdParsingState::PrimaryItem:
					return PRIMARY_ITEM_MESSAGE;
				case LocalIdParsingState::SecondaryItem:
					return SECONDARY_ITEM_MESSAGE;
				case LocalIdParsingState::ItemDescription:
					return ITEM_DESCRIPTION_MESSAGE;
				case LocalIdParsingState::MetaQuantity:
					return META_QUANTITY_MESSAGE;
				case LocalIdParsingState::MetaContent:
					return META_CONTENT_MESSAGE;
				case LocalIdParsingState::MetaCalculation:
					return META_CALCULATION_MESSAGE;
				case LocalIdParsingState::MetaState:
					return META_STATE_MESSAGE;
				case LocalIdParsingState::MetaCommand:
					return META_COMMAND_MESSAGE;
				case LocalIdParsingState::MetaType:
					return META_TYPE_MESSAGE;
				case LocalIdParsingState::MetaPosition:
					return META_POSITION_MESSAGE;
				case LocalIdParsingState::MetaDetail:
					return META_DETAIL_MESSAGE;
				case LocalIdParsingState::EmptyState:
					return EMPTY_STATE_MESSAGE;
				case LocalIdParsingState::Formatting:
					return FORMATTING_MESSAGE;
				case LocalIdParsingState::Completeness:
					return COMPLETENESS_MESSAGE;
				case LocalIdParsingState::NamingEntity:
					return NAMING_ENTITY_MESSAGE;
				case LocalIdParsingState::IMONumber:
					return IMO_NUMBER_MESSAGE;
				default:
					return UNKNOWN_PARSING_ERROR;
			}
		}
	}

	//=====================================================================
	// LocalIdParsingErrorBuilder class
	//=====================================================================

	//----------------------------------------------
	// Construction / destruction
	//----------------------------------------------

	LocalIdParsingErrorBuilder::LocalIdParsingErrorBuilder()
	{
		/* Memory: 8 × sizeof(std::pair<LocalIdParsingState, std::string>) = 8 × (4 + 32) = ~288 bytes */
		m_errors.reserve( 8 );
	}

	//----------------------------------------------
	// State inspection methods
	//----------------------------------------------

	bool LocalIdParsingErrorBuilder::hasError() const
	{
		return !m_errors.empty();
	}

	//----------------------------------------------
	// Static factory method
	//----------------------------------------------

	LocalIdParsingErrorBuilder LocalIdParsingErrorBuilder::create()
	{
		return LocalIdParsingErrorBuilder{};
	}

	//----------------------------------------------
	// ParsingErrors construction
	//----------------------------------------------

	ParsingErrors LocalIdParsingErrorBuilder::build() const
	{
		if ( m_errors.empty() )
		{
			return ParsingErrors::Empty;
		}

		std::vector<ParsingErrors::ErrorEntry> errorEntries;
		errorEntries.reserve( m_errors.size() );

		for ( const auto& [state, message] : m_errors )
		{
			errorEntries.emplace_back( localIdParsingStateToStringView( state ), message );
		}

		return ParsingErrors{ std::move( errorEntries ) };
	}

	//----------------------------------------------
	// Error addition
	//----------------------------------------------

	LocalIdParsingErrorBuilder& LocalIdParsingErrorBuilder::addError( LocalIdParsingState state )
	{
		if ( m_errors.size() == m_errors.capacity() ) [[unlikely]]
		{
			m_errors.reserve( std::max( static_cast<size_t>( 8 ), m_errors.capacity() * 2 ) );
		}

		std::string_view message = predefinedErrorMessage( state );

		m_errors.emplace_back( state, std::string{ message } );

		return *this;
	}

	LocalIdParsingErrorBuilder& LocalIdParsingErrorBuilder::addError(
		LocalIdParsingState state,
		const std::optional<std::string>& message )
	{
		if ( m_errors.size() == m_errors.capacity() ) [[unlikely]]
		{
			m_errors.reserve( std::max( static_cast<size_t>( 8 ), m_errors.capacity() * 2 ) );
		}

		if ( message.has_value() ) [[likely]]
		{
			if ( !message->empty() ) [[likely]]
			{
				m_errors.emplace_back( state, *message );
			}
			else
			{
				m_errors.emplace_back( state, std::string{} );
			}
		}
		else
		{
			std::string_view defaultMessage = predefinedErrorMessage( state );
			m_errors.emplace_back( state, std::string{ defaultMessage } );
		}

		return *this;
	}
}
