#pragma once

#include "ImoNumber.h"
#include "LocalIdBuilder.h"

namespace dnv::vista::sdk
{
	class UniversalIdBuilder;

	class IUniversalIdBuilder
	{
	public:
		/**
		 * Default constructor
		 */
		IUniversalIdBuilder() = default;

		/**
		 * Virtual destructor for proper cleanup of derived classes.
		 */
		virtual ~IUniversalIdBuilder() = default;

		/**
		 * Delete copy constructor - interfaces shouldn't be copied
		 */
		IUniversalIdBuilder( const IUniversalIdBuilder& ) = delete;

		/**
		 * Delete copy assignment operator - interfaces shouldn't be assigned
		 */
		IUniversalIdBuilder& operator=( const IUniversalIdBuilder& ) = delete;

		//-------------------------------------------------------------------------
		// State Validation
		//-------------------------------------------------------------------------

		/**
		 * Checks if the builder is in a valid state.
		 * @return True if valid, false otherwise.
		 */
		virtual bool isValid() const = 0;

		//-------------------------------------------------------------------------
		// Getter Methods
		//-------------------------------------------------------------------------

		/**
		 * Gets the IMO number.
		 * @return The optional IMO number.
		 */
		virtual std::optional<ImoNumber> imoNumber() const = 0;

		/**
		 * Gets the local ID builder.
		 * @return The optional local ID builder.
		 */
		virtual std::optional<LocalIdBuilder> localId() const = 0;

		//-------------------------------------------------------------------------
		// LocalId Modifier Methods
		//-------------------------------------------------------------------------

		/**
		 * Sets the local ID.
		 * @param localId The local ID builder.
		 * @return A new UniversalIdBuilder instance with the updated local ID.
		 */
		virtual UniversalIdBuilder withLocalId( const LocalIdBuilder& localId ) = 0;

		/**
		 * Tries to set the local ID.
		 * @param localId The optional local ID builder.
		 * @return A new UniversalIdBuilder instance with the updated local ID.
		 */
		virtual UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId ) = 0;

		/**
		 * Tries to set the local ID.
		 * @param localId The optional local ID builder.
		 * @param succeeded Output parameter indicating success.
		 * @return A new UniversalIdBuilder instance with the updated local ID.
		 */
		virtual UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId, bool& succeeded ) = 0;

		/**
		 * Removes the local ID.
		 * @return A new UniversalIdBuilder instance without the local ID.
		 */
		virtual UniversalIdBuilder withoutLocalId() = 0;

		//-------------------------------------------------------------------------
		// IMO Number Modifier Methods
		//-------------------------------------------------------------------------

		/**
		 * Sets the IMO number.
		 * @param imoNumber The IMO number.
		 * @return A new UniversalIdBuilder instance with the updated IMO number.
		 */
		virtual UniversalIdBuilder withImoNumber( const ImoNumber& imoNumber ) = 0;

		/**
		 * Tries to set the IMO number.
		 * @param imoNumber The optional IMO number.
		 * @return A new UniversalIdBuilder instance with the updated IMO number.
		 */
		virtual UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber ) = 0;

		/**
		 * Tries to set the IMO number.
		 * @param imoNumber The optional IMO number.
		 * @param succeeded Output parameter indicating success.
		 * @return A new UniversalIdBuilder instance with the updated IMO number.
		 */
		virtual UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber, bool& succeeded ) = 0;

		/**
		 * Removes the IMO number.
		 * @return A new UniversalIdBuilder instance without the IMO number.
		 */
		virtual UniversalIdBuilder withoutImoNumber() = 0;

		//-------------------------------------------------------------------------
		// Conversion Methods
		//-------------------------------------------------------------------------

		/**
		 * Converts the builder to a string representation.
		 * @return The string representation.
		 */
		virtual std::string toString() const = 0;
	};
}
