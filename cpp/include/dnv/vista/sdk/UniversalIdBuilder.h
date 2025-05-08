#pragma once

#include "IUniversalIdBuilder.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class ImoNumber;
	class LocalIdBuilder;
	class ParsingErrors;
	class UniversalId;
	class LocalIdParsingErrorBuilder;
	enum class LocalIdParsingState;

	//=====================================================================
	// UniversalIdBuilder Class
	//=====================================================================

	class UniversalIdBuilder final : public IUniversalIdBuilder
	{
	public:
		//=====================================================================
		// Constants
		//=====================================================================

		/** @brief Naming entity constant for universal IDs.*/
		static const std::string namingEntity;

		//=====================================================================
		// Construction / Destruction
		//=====================================================================

	protected:
		/** @brief Default constructor. */
		UniversalIdBuilder() = default;

	public:
		/** @brief Copy constructor */
		UniversalIdBuilder( const UniversalIdBuilder& ) = delete;

		/** @brief Move constructor */
		UniversalIdBuilder( UniversalIdBuilder&& ) noexcept = default;

		/** @brief Destructor */
		virtual ~UniversalIdBuilder() = default;

		//=====================================================================
		// Assignment Operators
		//=====================================================================

		/** @brief Copy assignment operator */
		UniversalIdBuilder& operator=( const UniversalIdBuilder& ) = delete;

		/** @brief Move assignment operator */
		UniversalIdBuilder& operator=( UniversalIdBuilder&& ) noexcept = default;

		//=====================================================================
		// Operators
		//=====================================================================

		//=====================================================================
		// Hashing
		//=====================================================================

		/**
		 * Gets the hash code for this builder.
		 * @return The hash code.
		 */
		size_t hashCode() const;

		//=====================================================================
		// !!!! CREATE !!!!!
		//=====================================================================

		/**
		 * Creates a new UniversalIdBuilder for the specified VIS version.
		 * @param version The VIS version.
		 * @return A new UniversalIdBuilder instance.
		 */
		static UniversalIdBuilder create( VisVersion version );

		//=====================================================================
		// Core Build Method
		//=====================================================================

		/**
		 * Builds a UniversalId from this builder.
		 * @return The constructed UniversalId.
		 * @throws std::invalid_argument If the builder is in an invalid state.
		 */
		UniversalId build() const;

		//=====================================================================
		// Accessors
		//=====================================================================

		/**
		 * Gets the IMO number.
		 * @return The optional IMO number.
		 */
		virtual const std::optional<ImoNumber>& imoNumber() const override;

		/**
		 * Gets the local ID builder.
		 * @return The optional local ID builder.
		 */
		virtual const std::optional<LocalIdBuilder>& localId() const override;

		//=====================================================================
		// State Inspection Methods
		//=====================================================================

		/**
		 * Checks if the builder is in a valid state.
		 * @return True if valid, false otherwise.
		 */
		virtual bool isValid() const override;

		//=====================================================================
		// Builder Methods (Immutable Fluent Interface)
		//=====================================================================

		//----------------------------------------------
		// Local id
		//----------------------------------------------

		/**
		 * Sets the local ID.
		 * @param localId The local ID builder.
		 * @return A new UniversalIdBuilder instance with the updated local ID.
		 */
		virtual UniversalIdBuilder withLocalId( const LocalIdBuilder& localId ) const override;

		/**
		 * Tries to set the local ID.
		 * @param localId The optional local ID builder.
		 * @return A new UniversalIdBuilder instance with the updated local ID.
		 */
		virtual UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId ) const override;

		/**
		 * Tries to set the local ID.
		 * @param localId The optional local ID builder.
		 * @param succeeded Output parameter indicating success.
		 * @return A new UniversalIdBuilder instance with the updated local ID.
		 */
		virtual UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId, bool& succeeded ) const override;

		/**
		 * Removes the local ID.
		 * @return A new UniversalIdBuilder instance without the local ID.
		 */
		virtual UniversalIdBuilder withoutLocalId() const override;

		//----------------------------------------------
		// IMO Number
		//----------------------------------------------

		/**
		 * Sets the IMO number.
		 * @param imoNumber The IMO number.
		 * @return A new UniversalIdBuilder instance with the updated IMO number.
		 */
		virtual UniversalIdBuilder withImoNumber( const ImoNumber& imoNumber ) const override;

		/**
		 * Removes the IMO number.
		 * @return A new UniversalIdBuilder instance without the IMO number.
		 */
		virtual UniversalIdBuilder withoutImoNumber() const override;

		/**
		 * Tries to set the IMO number.
		 * @param imoNumber The optional IMO number.
		 * @return A new UniversalIdBuilder instance with the updated IMO number.
		 */
		virtual UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber ) const override;

		/**
		 * Tries to set the IMO number.
		 * @param imoNumber The optional IMO number.
		 * @param succeeded Output parameter indicating success.
		 * @return A new UniversalIdBuilder instance with the updated IMO number.
		 */
		virtual UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber, bool& succeeded ) const override;

		//=====================================================================
		// Conversion and Comparison
		//=====================================================================

		/**
		 * Converts the builder to a string representation.
		 * @return The string representation.
		 */
		virtual std::string toString() const override;

		/**
		 * Checks equality with another UniversalIdBuilder.
		 * @param other The other UniversalIdBuilder to compare.
		 * @return True if equal, false otherwise.
		 */
		bool equals( const UniversalIdBuilder& other ) const;

		//=====================================================================
		// Static Parsing Methods
		//=====================================================================

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
		//=====================================================================
		// Private Member Variables
		//=====================================================================

		std::optional<LocalIdBuilder> m_localIdBuilder;
		std::optional<ImoNumber> m_imoNumber;
	};
}
