/**
 * @file LocalIdParsingErrorBuilder.cpp
 * @brief Implementation of the LocalIdParsingErrorBuilder class.
 */

#include "pch.h"

#include "dnv/vista/sdk/LocalIdParsingErrorBuilder.h"

#include "dnv/vista/sdk/ParsingErrors.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Static Factory Method
	//=====================================================================

	LocalIdParsingErrorBuilder LocalIdParsingErrorBuilder::create()
	{
		return LocalIdParsingErrorBuilder();
	}

	//=====================================================================
	// Public Methods
	//=====================================================================

	LocalIdParsingErrorBuilder& LocalIdParsingErrorBuilder::addError( LocalIdParsingState state )
	{
		const char* message = nullptr;
		switch ( state )
		{
			case LocalIdParsingState::NamingRule:
				message = "Invalid naming rule prefix";
				break;
			case LocalIdParsingState::VisVersion:
				message = "Invalid VIS version";
				break;
			case LocalIdParsingState::PrimaryItem:
				message = "Invalid primary item path";
				break;
			case LocalIdParsingState::SecondaryItem:
				message = "Invalid secondary item path";
				break;
			case LocalIdParsingState::ItemDescription:
				message = "Invalid item description";
				break;
			case LocalIdParsingState::MetaQuantity:
				message = "Invalid quantity metadata tag";
				break;
			case LocalIdParsingState::MetaContent:
				message = "Invalid content metadata tag";
				break;
			case LocalIdParsingState::MetaCalculation:
				message = "Invalid calculation metadata tag";
				break;
			case LocalIdParsingState::MetaState:
				message = "Invalid state metadata tag";
				break;
			case LocalIdParsingState::MetaCommand:
				message = "Invalid command metadata tag";
				break;
			case LocalIdParsingState::MetaType:
				message = "Invalid type metadata tag";
				break;
			case LocalIdParsingState::MetaPosition:
				message = "Invalid position metadata tag";
				break;
			case LocalIdParsingState::MetaDetail:
				message = "Invalid detail metadata tag";
				break;
			case LocalIdParsingState::EmptyState:
				message = "Empty state value provided";
				break;
			case LocalIdParsingState::Formatting:
				message = "Formatting error in Local ID string";
				break;
			case LocalIdParsingState::Completeness:
				message = "Incomplete Local ID structure";
				break;
			case LocalIdParsingState::NamingEntity:
				message = "Invalid naming entity";
				break;
			case LocalIdParsingState::IMONumber:
				message = "Invalid IMO number";
				break;
			default:
				SPDLOG_WARN( "Attempted to add predefined error for unknown state: {}", static_cast<int>( state ) );
				break;
		}

		if ( message != nullptr )
		{
			SPDLOG_ERROR( "Adding predefined parsing error for state {}: {}", static_cast<int>( state ), message );
			m_errors.emplace_back( state, message );
		}
		return *this;
	}

	LocalIdParsingErrorBuilder& LocalIdParsingErrorBuilder::addError( LocalIdParsingState state, const std::string& message )
	{
		SPDLOG_ERROR( "Adding custom parsing error for state {}: {}", static_cast<int>( state ), message );
		m_errors.emplace_back( state, message );

		return *this;
	}

	bool LocalIdParsingErrorBuilder::hasError() const
	{
		return !m_errors.empty();
	}

	//=====================================================================
	// Build Method
	//=====================================================================

	ParsingErrors LocalIdParsingErrorBuilder::build() const
	{
		if ( m_errors.empty() )
		{
			return ParsingErrors();
		}

		ParsingErrors errors;

		for ( const auto& [state, message] : m_errors )
		{
			std::string stateStr;
			switch ( state )
			{
				case LocalIdParsingState::NamingRule:
					stateStr = "NamingRule";
					break;
				case LocalIdParsingState::VisVersion:
					stateStr = "VisVersion";
					break;
				case LocalIdParsingState::PrimaryItem:
					stateStr = "PrimaryItem";
					break;
				case LocalIdParsingState::SecondaryItem:
					stateStr = "SecondaryItem";
					break;
				case LocalIdParsingState::ItemDescription:
					stateStr = "ItemDescription";
					break;
				case LocalIdParsingState::MetaQuantity:
					stateStr = "MetaQuantity";
					break;
				case LocalIdParsingState::MetaContent:
					stateStr = "MetaContent";
					break;
				case LocalIdParsingState::MetaCalculation:
					stateStr = "MetaCalculation";
					break;
				case LocalIdParsingState::MetaState:
					stateStr = "MetaState";
					break;
				case LocalIdParsingState::MetaCommand:
					stateStr = "MetaCommand";
					break;
				case LocalIdParsingState::MetaType:
					stateStr = "MetaType";
					break;
				case LocalIdParsingState::MetaPosition:
					stateStr = "MetaPosition";
					break;
				case LocalIdParsingState::MetaDetail:
					stateStr = "MetaDetail";
					break;
				case LocalIdParsingState::EmptyState:
					stateStr = "EmptyState";
					break;
				case LocalIdParsingState::Formatting:
					stateStr = "Formatting";
					break;
				case LocalIdParsingState::Completeness:
					stateStr = "Completeness";
					break;
				case LocalIdParsingState::NamingEntity:
					stateStr = "NamingEntity";
					break;
				case LocalIdParsingState::IMONumber:
					stateStr = "IMONumber";
					break;
				default:
					stateStr = "State" + std::to_string( static_cast<int>( state ) );
					SPDLOG_WARN( "Building ParsingErrors with unknown state enum value: {}", static_cast<int>( state ) );
					break;
			}
			errors.addError( stateStr, message );
		}
		return errors;
	}
}
