#include "pch.h"

#include "dnv/vista/sdk/ImoNumber.h"
#include "dnv/vista/sdk/LocalIdBuilder.h"
#include "dnv/vista/sdk/ParsingErrors.h"
#include "dnv/vista/sdk/UniversalId.h"
#include "dnv/vista/sdk/UniversalIdBuilder.h"
#include "dnv/vista/sdk/VisVersion.h"

namespace dnv::vista::sdk
{
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		const std::string UniversalIdBuilder::namingEntity = "data.dnv.com";
	}

	//=====================================================================
	// !!!! CREATE !!!!!
	//=====================================================================

	UniversalIdBuilder UniversalIdBuilder::create( VisVersion version )
	{
		return UniversalIdBuilder().withLocalId( LocalIdBuilder::create( version ) );
	}

	//=====================================================================
	// Core Build Method
	//=====================================================================

	// UniversalId UniversalIdBuilder::build() const
	// {
	// 	SPDLOG_INFO( "Building UniversalId from builder" );
	// 	if ( !isValid() )
	// 	{
	// 		SPDLOG_ERROR( "Invalid UniversalIdBuilder state" );
	// 		throw std::invalid_argument( "Invalid UniversalIdBuilder state" );
	// 	}
	// 	// return UniversalId( std::make_shared<UniversalIdBuilder>( *this ) );
	// 	return UniversalId();
	// }

	//=====================================================================
	// Accessors
	//=====================================================================

	const std::optional<ImoNumber>& UniversalIdBuilder::imoNumber() const
	{
		return m_imoNumber;
	}

	const std::optional<LocalIdBuilder>& UniversalIdBuilder::localId() const
	{
		return m_localIdBuilder;
	}

	//=====================================================================
	// State Inspection Methods
	//=====================================================================

	bool UniversalIdBuilder::isValid() const
	{
		bool valid = m_imoNumber.has_value() && m_localIdBuilder.has_value() && m_localIdBuilder->isValid();
		SPDLOG_INFO( "Validating UniversalIdBuilder state: {}", ( valid ? "valid" : "invalid" ) );
		return valid;
	}

	//=====================================================================
	// Builder Methods (Immutable Fluent Interface)
	//=====================================================================

	//----------------------------------------------
	// Local id
	//----------------------------------------------

	UniversalIdBuilder UniversalIdBuilder::withLocalId( const LocalIdBuilder& localId ) const
	{
		auto b = UniversalIdBuilder::create( localId.visVersion().value() );
		b.withLocalId( localId );

		return b;
	}

	UniversalIdBuilder UniversalIdBuilder::tryWithLocalId( const std::optional<LocalIdBuilder>& localId ) const
	{
		SPDLOG_INFO( "Trying to set local ID on builder" );
		bool succeeded;
		// Delegate to the overload that uses the private constructor
		return tryWithLocalId( localId, succeeded );
	}

	UniversalIdBuilder UniversalIdBuilder::tryWithLocalId( const std::optional<LocalIdBuilder>& localId, bool& succeeded ) const
	{
		SPDLOG_INFO( "Trying to set local ID with success tracking" );
		if ( !localId.has_value() )
		{
			SPDLOG_WARN( "Local ID is null, operation failed" );
			succeeded = false;

			return UniversalIdBuilder();
		}

		succeeded = true;
		SPDLOG_INFO( "Successfully set local ID" );

		auto b = UniversalIdBuilder::create( localId->visVersion().value() );
		b.withLocalId( localId.value() );

		return b;
	}

	UniversalIdBuilder UniversalIdBuilder::withoutLocalId() const
	{
		return UniversalIdBuilder();
	}

	//----------------------------------------------
	// IMO Number
	//----------------------------------------------

	UniversalIdBuilder UniversalIdBuilder::withImoNumber( const ImoNumber& imoNumber ) const
	{
		SPDLOG_INFO( "Setting IMO number on builder" );
		bool succeeded;
		return tryWithImoNumber( std::optional<ImoNumber>( imoNumber ), succeeded );
	}

	UniversalIdBuilder UniversalIdBuilder::tryWithImoNumber( const std::optional<ImoNumber>& imoNumber ) const
	{
		SPDLOG_INFO( "Trying to set IMO number on builder" );
		bool succeeded;
		return tryWithImoNumber( imoNumber, succeeded );
	}

	UniversalIdBuilder UniversalIdBuilder::tryWithImoNumber( const std::optional<ImoNumber>& imoNumber, bool& succeeded ) const
	{
		// SPDLOG_INFO( "Trying to set IMO number with success tracking" );
		// if ( !imoNumber.has_value() )
		// {
		// 	SPDLOG_WARN( "IMO number is null, operation failed" );
		// 	succeeded = false;
		// 	return *this;
		// }

		// UniversalIdBuilder copy( *this );

		// 		copy.m_imoNumber = imoNumber;
		// succeeded = true;
		// SPDLOG_INFO( "Successfully set IMO number" );
		// return copy;

		return UniversalIdBuilder();
	}

	UniversalIdBuilder UniversalIdBuilder::withoutImoNumber() const
	{
		// SPDLOG_INFO( "Removing IMO number from builder" );
		// UniversalIdBuilder copy( *this );
		// copy.m_imoNumber.reset();
		// return copy;

		return UniversalIdBuilder();
	}

	//=====================================================================
	// Conversion and Comparison
	//=====================================================================

	std::string UniversalIdBuilder::toString() const
	{
		SPDLOG_INFO( "Converting UniversalIdBuilder to string" );
		if ( !isValid() )
		{
			SPDLOG_WARN( "toString() called on invalid UniversalIdBuilder state" );
			return "";
		}

		std::ostringstream builder;
		builder << namingEntity << "/";
		builder << m_imoNumber->toString();
		builder << m_localIdBuilder->toString();

		return builder.str();
	}

	bool UniversalIdBuilder::equals( const UniversalIdBuilder& other ) const
	{
		return m_imoNumber == other.m_imoNumber && m_localIdBuilder == other.m_localIdBuilder;
	}

	//=====================================================================
	// Static Parsing Methods
	//=====================================================================

	UniversalIdBuilder UniversalIdBuilder::parse( const std::string& universalIdStr )
	{
		SPDLOG_INFO( "Parsing UniversalId string: {}", universalIdStr );
		ParsingErrors errors;
		std::shared_ptr<UniversalIdBuilder> builder;
		if ( !tryParse( universalIdStr, errors, builder ) )
		{
			SPDLOG_ERROR( "Failed to parse UniversalId: {}", universalIdStr );
			throw std::invalid_argument( "Failed to parse UniversalId: " + universalIdStr );
		}
		// return *builder;

		return UniversalIdBuilder();
	}

	bool UniversalIdBuilder::tryParse( const std::string& universalIdStr, ParsingErrors& errors, std::shared_ptr<UniversalIdBuilder>& builder )
	{
		SPDLOG_INFO( "Trying to parse UniversalId string: {}", universalIdStr );
		builder = nullptr;

		// LocalIdParsingErrorBuilder errorBuilder = LocalIdParsingErrorBuilder::create();

		if ( universalIdStr.empty() )
		{
			SPDLOG_WARN( "Empty UniversalId string" );
			// addError( errorBuilder, LocalIdParsingState::NamingRule, "Failed to find localId start segment" );
			// errors = errorBuilder.build();
			return false;
		}

		size_t localIdStartIndex = universalIdStr.find( "/dnv-v" );
		if ( localIdStartIndex == std::string::npos )
		{
			SPDLOG_WARN( "Unable to find localId start marker in: {}", universalIdStr );
			// addError( errorBuilder, LocalIdParsingState::NamingRule, "Failed to find localId start segment" );
			// errors = errorBuilder.build();
			return false;
		}

		std::string universalIdSegment = universalIdStr.substr( 0, localIdStartIndex );
		std::string localIdSegment = universalIdStr.substr( localIdStartIndex );
		SPDLOG_INFO( "Split UniversalId into segments - Universal: {}, Local: {}", universalIdSegment, localIdSegment );

		std::optional<LocalIdBuilder> localIdBuilder;
		ParsingErrors parseErrors;
		if ( !LocalIdBuilder::tryParse( localIdSegment, parseErrors, localIdBuilder ) )
		{
			SPDLOG_WARN( "Failed to parse LocalId segment: {}", localIdSegment );
			//	errors = errorBuilder.build();
			std::vector<ParsingErrors::ErrorEntry> combinedErrors;
			for ( const auto& err : errors )
				combinedErrors.push_back( err );
			for ( const auto& err : parseErrors )
				combinedErrors.push_back( err );

			errors = ParsingErrors( combinedErrors );
			return false;
		}

		std::optional<ImoNumber> imoNumber;
		LocalIdParsingState state = LocalIdParsingState::NamingEntity;
		size_t i = 0;

		SPDLOG_INFO( "Parsing universal ID segment: {}", universalIdSegment );
		while ( state <= LocalIdParsingState::IMONumber )
		{
			if ( i >= universalIdSegment.length() )
				break;

			size_t nextSlash = universalIdSegment.find( '/', i );
			std::string segment;

			if ( nextSlash == std::string::npos )
			{
				segment = universalIdSegment.substr( i );
			}
			else
			{
				segment = universalIdSegment.substr( i, nextSlash - i );
			}

			SPDLOG_INFO( "Processing segment '{}' in state {}", segment, static_cast<int>( state ) );

			switch ( state )
			{
				case LocalIdParsingState::NamingRule:
				case LocalIdParsingState::VisVersion:
				case LocalIdParsingState::PrimaryItem:
				case LocalIdParsingState::SecondaryItem:
				case LocalIdParsingState::ItemDescription:
				case LocalIdParsingState::MetaQuantity:
				case LocalIdParsingState::MetaContent:
				case LocalIdParsingState::MetaCalculation:
				case LocalIdParsingState::MetaState:
				case LocalIdParsingState::MetaCommand:
				case LocalIdParsingState::MetaType:
				case LocalIdParsingState::MetaPosition:
				case LocalIdParsingState::MetaDetail:
				case LocalIdParsingState::EmptyState:
				case LocalIdParsingState::Formatting:
				case LocalIdParsingState::Completeness:
					break;

				case LocalIdParsingState::NamingEntity:
					if ( segment != namingEntity )
					{
						SPDLOG_WARN( "Invalid naming entity: {}", segment );
						// addError( errorBuilder, state, "Naming entity segment didn't match. Found: " + segment );
					}
					break;

				case LocalIdParsingState::IMONumber:
					std::optional<ImoNumber> imo = ImoNumber::tryParse( segment );
					if ( !imo.has_value() )
					{
						SPDLOG_WARN( "Invalid IMO number: {}", segment );
						// addError( errorBuilder, state, "Invalid IMO number segment" );
					}
					else
					{
						imoNumber = imo;
					}
					break;
			}

			state = static_cast<LocalIdParsingState>( static_cast<int>( state ) + 1 );
			i += segment.length() + 1;
		}

		std::optional<VisVersion> visVersion = localIdBuilder->visVersion();
		if ( !visVersion.has_value() )
		{
			SPDLOG_WARN( "Missing VIS version in LocalId" );
			// addError( errorBuilder, LocalIdParsingState::VisVersion, nullptr );
			// errors = errorBuilder.build();
			return false;
		}

		SPDLOG_INFO( "Creating UniversalIdBuilder with VIS version {}", static_cast<int>( visVersion.value() ) );
		builder = std::make_shared<UniversalIdBuilder>( create( visVersion.value() ) );
		// builder->tryWithLocalId( *localIdBuilder );
		builder->tryWithImoNumber( imoNumber );

		// errors = errorBuilder.build();
		return true;
	}
}
