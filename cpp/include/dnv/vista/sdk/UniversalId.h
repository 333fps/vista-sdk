#pragma once

#include "IUniversalId.h"

namespace dnv::vista::sdk
{
	class IUniversalIdBuilder;
	class LocalId;
	class ParsingErrors;

	/**
	 * Represents a Universal Identifier.
	 */
	class UniversalId final : public IUniversalId
	{
	public:
		//-------------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------------

		/**
		 * Constructs a UniversalId from a builder.
		 * @param builder The builder used to construct the UniversalId.
		 * @throws std::invalid_argument If the builder is in an invalid state.
		 */
		explicit UniversalId( const std::shared_ptr<IUniversalIdBuilder>& builder );

		/**
		 * Copy constructor (deleted)
		 * @details UniversalId objects shouldn't be copied since they contain non-copyable members
		 */
		UniversalId( const UniversalId& ) = delete;

		/**
		 * Copy assignment operator (deleted)
		 * @details UniversalId objects shouldn't be assigned since they contain non-copyable members
		 */
		UniversalId& operator=( const UniversalId& ) = delete;

		/**
		 * Default virtual destructor.
		 */
		virtual ~UniversalId() = default;

		//-------------------------------------------------------------------------
		// Interface Implementation
		//-------------------------------------------------------------------------

		/**
		 * Gets the IMO number.
		 * @return The IMO number.
		 * @throws std::runtime_error If the IMO number is invalid.
		 */
		virtual const ImoNumber& imoNumber() const override;

		/**
		 * Gets the local ID.
		 * @return The local ID.
		 */
		virtual const LocalId& localId() const override;

		//-------------------------------------------------------------------------
		// Static Methods - Parsing
		//-------------------------------------------------------------------------

		/**
		 * Parses a UniversalId from a string.
		 * @param universalIdStr The string representation of the UniversalId.
		 * @return The parsed UniversalId.
		 * @throws std::invalid_argument If parsing fails.
		 */
		static UniversalId parse( const std::string& universalIdStr );

		/**
		 * Tries to parse a UniversalId from a string.
		 * @param universalIdStr The string representation of the UniversalId.
		 * @param errors Container for parsing errors.
		 * @param universalId The parsed UniversalId if successful.
		 * @return True if parsing succeeded, false otherwise.
		 */
		static bool tryParse( const std::string& universalIdStr, ParsingErrors& errors, std::unique_ptr<UniversalId>& universalId );

		//-------------------------------------------------------------------------
		// Instance Methods - Comparison
		//-------------------------------------------------------------------------

		/**
		 * Checks equality with another UniversalId.
		 * @param other The other UniversalId to compare.
		 * @return True if equal, false otherwise.
		 */
		bool equals( const UniversalId& other ) const;

		/**
		 * Equality operator.
		 * @param other The other UniversalId to compare.
		 * @return True if equal, false otherwise.
		 */
		bool operator==( const UniversalId& other ) const;

		/**
		 * Inequality operator.
		 * @param other The other UniversalId to compare.
		 * @return True if not equal, false otherwise.
		 */
		bool operator!=( const UniversalId& other ) const;

		//-------------------------------------------------------------------------
		// Instance Methods - Conversion
		//-------------------------------------------------------------------------

		/**
		 * Converts the UniversalId to a string representation.
		 * @return The string representation.
		 */
		std::string toString() const;

		/**
		 * Gets the hash code of the UniversalId.
		 * @return The hash code.
		 */
		size_t hashCode() const;

	private:
		//-------------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------------

		std::shared_ptr<IUniversalIdBuilder> m_builder;
		LocalId m_localId;
		ImoNumber m_imoNumber;
	};
}
