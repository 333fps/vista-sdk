/**
 * @file UniversalIdBuilder.h
 * @brief High-performance fluent builder for UniversalId objects.
 * @details Concrete implementation using immutable fluent pattern with direct value storage.
 *          Optimized for performance without interface overhead.
 */

#pragma once

#include "ImoNumber.h"
#include "LocalIdBuilder.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class ParsingErrors;
	class UniversalId;
	class LocalIdParsingErrorBuilder;
	enum class LocalIdParsingState;
	enum class VISVersion;

	//=====================================================================
	// UniversalIdBuilder class
	//=====================================================================

	/**
	 * @class UniversalIdBuilder
	 * @brief High-performance fluent builder for UniversalId objects.
	 *
	 * @details Concrete implementation using immutable fluent pattern with direct value storage.
	 * Uses move semantics and direct method calls for optimal performance.
	 */
	class UniversalIdBuilder final
	{
	public:
		//----------------------------------------------
		// Constants
		//----------------------------------------------

		/**
		 * @brief Standard naming entity for Universal IDs.
		 * @details Always "data.dnv.com" for DNV Universal ID format.
		 */
		static const std::string namingEntity;

		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

	protected:
		/**
		 * @brief Protected default constructor.
		 * @details Use static factory methods for construction.
		 */
		UniversalIdBuilder() = default;

	public:
		/** @brief Copy constructor */
		UniversalIdBuilder( const UniversalIdBuilder& ) = default;

		/** @brief Move constructor */
		UniversalIdBuilder( UniversalIdBuilder&& ) noexcept = default;

		/** @brief Destructor */
		~UniversalIdBuilder() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		UniversalIdBuilder& operator=( const UniversalIdBuilder& ) = delete;

		/** @brief Move assignment operator */
		UniversalIdBuilder& operator=( UniversalIdBuilder&& other ) noexcept;

		//----------------------------------------------
		// Operators
		//----------------------------------------------

		/**
		 * @brief Equality operator.
		 * @param[in] other The other builder to compare.
		 * @return True if equal, false otherwise.
		 */
		inline bool operator==( const UniversalIdBuilder& other ) const;

		/**
		 * @brief Inequality operator.
		 * @param[in] other The other builder to compare.
		 * @return True if not equal, false otherwise.
		 */
		inline bool operator!=( const UniversalIdBuilder& other ) const;

		/**
		 * @brief Deep equality comparison with another builder.
		 * @param[in] other The other builder to compare.
		 * @return True if all components are equal.
		 */
		inline bool equals( const UniversalIdBuilder& other ) const;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the IMO number currently set in builder.
		 * @return Constant reference to optional IMO number.
		 * @note Zero-copy access via const reference.
		 */
		[[nodiscard]] inline const std::optional<ImoNumber>& imoNumber() const noexcept;

		/**
		 * @brief Gets the Local ID builder currently set.
		 * @return Constant reference to optional LocalIdBuilder.
		 * @note Zero-copy access via const reference.
		 */
		[[nodiscard]] inline const std::optional<LocalIdBuilder>& localId() const noexcept;

		/**
		 * @brief Computes hash code for container usage.
		 * @return Hash value combining IMO number and Local ID builder.
		 * @note This method is guaranteed not to throw (noexcept).
		 */
		[[nodiscard]] inline size_t hashCode() const noexcept;

		//----------------------------------------------
		// State inspection methods
		//----------------------------------------------

		/**
		 * @brief Checks if builder state is valid for building UniversalId.
		 * @details Requires both IMO number and valid LocalIdBuilder.
		 * @return True if build() will succeed.
		 * @note This method is guaranteed not to throw (noexcept).
		 */
		[[nodiscard]] inline bool isValid() const noexcept;

		//----------------------------------------------
		// String conversion
		//----------------------------------------------

		/**
		 * @brief Generates string representation of current builder state.
		 * @return String in Universal ID format.
		 * @throws std::invalid_argument If builder state is invalid.
		 */
		[[nodiscard]] std::string toString() const;

		//----------------------------------------------
		// Static factory methods
		//----------------------------------------------

		/**
		 * @brief Creates new UniversalIdBuilder for specified VIS version.
		 * @param[in] version VIS version for the Local ID component.
		 * @return New UniversalIdBuilder instance.
		 */
		static UniversalIdBuilder create( VisVersion version );

		//----------------------------------------------
		// Build methods (Immutable fluent interface)
		//----------------------------------------------

		/**
		 * @brief Builds UniversalId from current builder state.
		 * @return Constructed UniversalId.
		 * @throws std::invalid_argument If builder state is invalid.
		 */
		[[nodiscard]] UniversalId build() const;

		//----------------------------
		// Local Id
		//----------------------------

		/**
		 * @brief Returns new builder with Local ID builder set.
		 * @param[in] localId LocalIdBuilder to set.
		 * @return New UniversalIdBuilder instance.
		 * @throws std::invalid_argument If localId is invalid.
		 */
		[[nodiscard]] UniversalIdBuilder withLocalId( const LocalIdBuilder& localId ) const;

		/**
		 * @brief Returns new builder with optional Local ID builder.
		 * @param[in] localId Optional LocalIdBuilder to set.
		 * @return New UniversalIdBuilder instance.
		 */
		[[nodiscard]] UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId ) const;

		/**
		 * @brief Returns new builder with optional Local ID builder, reporting success.
		 * @param[in] localId Optional LocalIdBuilder to set.
		 * @param[out] succeeded True if LocalIdBuilder was set successfully.
		 * @return New UniversalIdBuilder instance.
		 */
		[[nodiscard]] UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId, bool& succeeded ) const;

		/**
		 * @brief Returns new builder without Local ID builder.
		 * @return New UniversalIdBuilder instance.
		 */
		[[nodiscard]] UniversalIdBuilder withoutLocalId() const;

		//----------------------------
		// IMO number
		//----------------------------

		/**
		 * @brief Returns new builder with IMO number set.
		 * @param[in] imoNumber IMO number to set.
		 * @return New UniversalIdBuilder instance.
		 * @throws std::invalid_argument If imoNumber is invalid.
		 */
		[[nodiscard]] UniversalIdBuilder withImoNumber( const ImoNumber& imoNumber ) const;

		/**
		 * @brief Returns new builder with optional IMO number.
		 * @param[in] imoNumber Optional IMO number to set.
		 * @return New UniversalIdBuilder instance.
		 */
		[[nodiscard]] UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber ) const;

		/**
		 * @brief Returns new builder with optional IMO number, reporting success.
		 * @param[in] imoNumber Optional IMO number to set.
		 * @param[out] succeeded True if IMO number was set successfully.
		 * @return New UniversalIdBuilder instance.
		 */
		[[nodiscard]] UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber, bool& succeeded ) const;

		/**
		 * @brief Returns new builder without IMO number.
		 * @return New UniversalIdBuilder instance.
		 */
		[[nodiscard]] UniversalIdBuilder withoutImoNumber() const;

		//----------------------------------------------
		// Static parsing methods
		//----------------------------------------------

		/**
		 * @brief Parses UniversalIdBuilder from string representation.
		 * @param[in] universalIdStr String to parse.
		 * @return Parsed UniversalIdBuilder.
		 * @throws std::invalid_argument If parsing fails.
		 */
		static UniversalIdBuilder parse( std::string_view universalIdStr );

		/**
		 * @brief Attempts to parse UniversalIdBuilder from string.
		 * @param[in] universalIdStr String to parse.
		 * @param[out] universalIdBuilder Parsed result if successful.
		 * @return True if parsing succeeded.
		 */
		static bool tryParse( std::string_view universalIdStr, std::optional<UniversalIdBuilder>& universalIdBuilder );

		/**
		 * @brief Attempts to parse UniversalIdBuilder from string with error reporting.
		 * @param[in] universalIdStr String to parse.
		 * @param[out] errors Parsing errors if unsuccessful.
		 * @param[out] universalIdBuilder Parsed result if successful.
		 * @return True if parsing succeeded.
		 */
		static bool tryParse( std::string_view universalIdStr, ParsingErrors& errors, std::optional<UniversalIdBuilder>& universalIdBuilder );

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		std::optional<LocalIdBuilder> m_localIdBuilder;
		std::optional<ImoNumber> m_imoNumber;
	};
}

#include "UniversalIdBuilder.inl"
