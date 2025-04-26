#pragma once

#include "IUniversalIdBuilder.h"

namespace dnv::vista::sdk
{
	class ImoNumber;
	class LocalIdBuilder;
	class ParsingErrors;
	class UniversalId;
	class LocalIdParsingErrorBuilder;
	enum class LocalIdParsingState;

	/**
	 * Builder class for creating UniversalId instances.
	 * Implements the IUniversalIdBuilder interface.
	 */
	class UniversalIdBuilder final : public IUniversalIdBuilder
	{
	public:
		//-------------------------------------------------------------------------
		// Constants
		//-------------------------------------------------------------------------

		/** @brief Naming entity constant for universal IDs.*/
		static const std::string namingEntity;

		//-------------------------------------------------------------------------
		// Construction / Factory Methods
		//-------------------------------------------------------------------------

		/** Default constructor. */
		UniversalIdBuilder() = default;

		UniversalIdBuilder( const UniversalIdBuilder& other );

		/**
		 * Copy assignment operator.
		 * @param other The other UniversalIdBuilder to copy from.
		 * @return Reference to this object after assignment.
		 */
		UniversalIdBuilder& operator=( const UniversalIdBuilder& other );

		/**
		 * Creates a new UniversalIdBuilder for the specified VIS version.
		 * @param version The VIS version.
		 * @return A new UniversalIdBuilder instance.
		 */
		static UniversalIdBuilder create( VisVersion version );

		/**
		 * Builds a UniversalId from this builder.
		 * @return The constructed UniversalId.
		 * @throws std::invalid_argument If the builder is in an invalid state.
		 */
		UniversalId build() const;

		//-------------------------------------------------------------------------
		// Interface Implementation - State/Getters
		//-------------------------------------------------------------------------

		/**
		 * Checks if the builder is in a valid state.
		 * @return True if valid, false otherwise.
		 */
		virtual bool isValid() const override;

		/**
		 * Gets the IMO number.
		 * @return The optional IMO number.
		 */
		virtual std::optional<ImoNumber> imoNumber() const override;

		/**
		 * Gets the local ID builder.
		 * @return The optional local ID builder.
		 */
		virtual std::optional<LocalIdBuilder> localId() const override;

		//-------------------------------------------------------------------------
		// Interface Implementation - LocalId Modifiers
		//-------------------------------------------------------------------------

		/**
		 * Sets the local ID.
		 * @param localId The local ID builder.
		 * @return A new UniversalIdBuilder instance with the updated local ID.
		 */
		virtual UniversalIdBuilder withLocalId( const LocalIdBuilder& localId ) override;

		/**
		 * Removes the local ID.
		 * @return A new UniversalIdBuilder instance without the local ID.
		 */
		virtual UniversalIdBuilder withoutLocalId() override;

		/**
		 * Tries to set the local ID.
		 * @param localId The optional local ID builder.
		 * @return A new UniversalIdBuilder instance with the updated local ID.
		 */
		virtual UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId ) override;

		/**
		 * Tries to set the local ID.
		 * @param localId The optional local ID builder.
		 * @param succeeded Output parameter indicating success.
		 * @return A new UniversalIdBuilder instance with the updated local ID.
		 */
		virtual UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId, bool& succeeded ) override;

		//-------------------------------------------------------------------------
		// Interface Implementation - IMO Number Modifiers
		//-------------------------------------------------------------------------

		/**
		 * Sets the IMO number.
		 * @param imoNumber The IMO number.
		 * @return A new UniversalIdBuilder instance with the updated IMO number.
		 */
		virtual UniversalIdBuilder withImoNumber( const ImoNumber& imoNumber ) override;

		/**
		 * Removes the IMO number.
		 * @return A new UniversalIdBuilder instance without the IMO number.
		 */
		virtual UniversalIdBuilder withoutImoNumber() override;

		/**
		 * Tries to set the IMO number.
		 * @param imoNumber The optional IMO number.
		 * @return A new UniversalIdBuilder instance with the updated IMO number.
		 */
		virtual UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber ) override;

		/**
		 * Tries to set the IMO number.
		 * @param imoNumber The optional IMO number.
		 * @param succeeded Output parameter indicating success.
		 * @return A new UniversalIdBuilder instance with the updated IMO number.
		 */
		virtual UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber, bool& succeeded ) override;

		//-------------------------------------------------------------------------
		// Interface Implementation - Conversion
		//-------------------------------------------------------------------------

		/**
		 * Converts the builder to a string representation.
		 * @return The string representation.
		 */
		virtual std::string toString() const override;

		//-------------------------------------------------------------------------
		// Comparison Methods
		//-------------------------------------------------------------------------

		/**
		 * Checks equality with another UniversalIdBuilder.
		 * @param other The other UniversalIdBuilder to compare.
		 * @return True if equal, false otherwise.
		 */
		bool equals( const UniversalIdBuilder& other ) const;

		/**
		 * Gets the hash code for this builder.
		 * @return The hash code.
		 */
		size_t hashCode() const;

		//-------------------------------------------------------------------------
		// Static Parsing Methods
		//-------------------------------------------------------------------------

		/**
		 * Parses a UniversalIdBuilder from a string.
		 * @param universalIdStr The string representation of the UniversalId.
		 * @return The parsed UniversalIdBuilder.
		 * @throws std::invalid_argument If parsing fails.
		 */
		static UniversalIdBuilder parse( const std::string& universalIdStr );

		/**
		 * Tries to parse a UniversalIdBuilder from a string.
		 * @param universalIdStr The string representation of the UniversalId.
		 * @param errors Container for parsing errors.
		 * @param builder The parsed UniversalIdBuilder if successful.
		 * @return True if parsing succeeded, false otherwise.
		 */
		static bool tryParse( const std::string& universalIdStr, ParsingErrors& errors, std::shared_ptr<UniversalIdBuilder>& builder );

	private:
		//-------------------------------------------------------------------------
		// Private Helper Methods
		//-------------------------------------------------------------------------

		/**
		 * Adds an error to the error builder.
		 * @param errorBuilder The error builder.
		 * @param state The parsing state.
		 * @param message The error message.
		 */
		static void addError( LocalIdParsingErrorBuilder& errorBuilder, LocalIdParsingState state, const std::string& message );

		//-------------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------------

		std::optional<LocalIdBuilder> m_localId;
		std::optional<ImoNumber> m_imoNumber;
	};
}
