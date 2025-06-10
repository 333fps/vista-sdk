/**
 * @file UniversalId.h
 * @brief High-performance Universal ID implementation.
 * @details Combines IMO number with Local ID for globally unique vessel identification.
 *          Uses direct value storage for optimal performance.
 */

#pragma once

#include "ImoNumber.h"
#include "LocalId.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class UniversalIdBuilder;
	class ParsingErrors;

	//=====================================================================
	// UniversalId class
	//=====================================================================

	/**
	 * @class UniversalId
	 * @brief High-performance Universal ID with direct value storage.
	 *
	 * @details Immutable Universal ID combining vessel IMO number with Local ID.
	 * Uses direct member storage instead of interface inheritance for optimal performance.
	 */
	class UniversalId final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Constructs UniversalId from validated UniversalIdBuilder.
		 * @param[in] builder Valid UniversalIdBuilder instance.
		 * @throws std::invalid_argument If builder is invalid.
		 */
		explicit UniversalId( const UniversalIdBuilder& builder );

		/** @brief Default constructor */
		UniversalId() = delete;

		/** @brief Copy constructor */
		UniversalId( const UniversalId& ) = default;

		/** @brief Move constructor */
		UniversalId( UniversalId&& ) noexcept = default;

		/** @brief Destructor */
		~UniversalId() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		UniversalId& operator=( const UniversalId& ) = default;

		/** @brief Move assignment operator */
		UniversalId& operator=( UniversalId&& ) noexcept = default;

		//----------------------------------------------
		// Operators
		//----------------------------------------------

		/**
		 * @brief Equality operator.
		 * @param[in] other The other UniversalId to compare.
		 * @return True if equal, false otherwise.
		 */
		inline bool operator==( const UniversalId& other ) const noexcept;

		/**
		 * @brief Inequality operator.
		 * @param[in] other The other UniversalId to compare.
		 * @return True if not equal, false otherwise.
		 */
		inline bool operator!=( const UniversalId& other ) const noexcept;

		/**
		 * @brief Deep equality comparison.
		 * @param[in] other The other UniversalId to compare.
		 * @return True if all components are equal.
		 */
		inline bool equals( const UniversalId& other ) const noexcept;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the IMO number component.
		 * @return Constant reference to the IMO number.
		 * @note Guaranteed to be valid for successfully constructed UniversalId.
		 */
		[[nodiscard]] inline const ImoNumber& imoNumber() const noexcept;

		/**
		 * @brief Gets the Local ID component.
		 * @return Constant reference to the Local ID.
		 * @note Guaranteed to be valid for successfully constructed UniversalId.
		 */
		[[nodiscard]] inline const LocalId& localId() const noexcept;

		/**
		 * @brief Computes hash code for container usage.
		 * @return Hash value suitable for std::unordered_set/map.
		 * @note This method is guaranteed not to throw (noexcept).
		 */
		[[nodiscard]] inline size_t hashCode() const noexcept;

		//----------------------------------------------
		// String conversion
		//----------------------------------------------

		/**
		 * @brief Converts to canonical string representation.
		 * @return String in format "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/...".
		 */
		[[nodiscard]] std::string toString() const;

		//----------------------------------------------
		// Static parsing methods
		//----------------------------------------------

		/**
		 * @brief Parses UniversalId from string representation.
		 * @param[in] universalIdStr String to parse.
		 * @return Parsed UniversalId.
		 * @throws std::invalid_argument If parsing fails.
		 */
		static UniversalId parse( std::string_view universalIdStr );

		/**
		 * @brief Attempts to parse UniversalId from string.
		 * @param[in] universalIdStr String to parse.
		 * @param[out] errors Parsing errors if unsuccessful.
		 * @param[out] universalId Parsed result if successful.
		 * @return True if parsing succeeded.
		 */
		static bool tryParse( std::string_view universalIdStr, ParsingErrors& errors, std::optional<UniversalId>& universalId );

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		ImoNumber m_imoNumber;
		LocalId m_localId;
	};
}

#include "UniversalId.inl"
