/**
 * @file ImoNumber.h
 * @brief Defines the ImoNumber class for representing and validating International Maritime Organization (IMO) numbers.
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// ImoNumber class
	//=====================================================================

	/**
	 * @brief Represents an International Maritime Organization (IMO) number.
	 *
	 * An IMO number is a unique seven-digit identifier assigned to maritime vessels,
	 * featuring a check digit for validation according to the IMO standard.
	 * This class provides functionality to create, validate, parse, and represent IMO numbers.
	 * Instances of this class are immutable once constructed.
	 */
	class ImoNumber final
	{
	public:
		//----------------------------------------------
		// Construction
		//----------------------------------------------

		/**
		 * @brief Constructs an ImoNumber from an integer value.
		 * @param value The IMO number as an integer (e.g., 9074729).
		 *              The value must be a 7-digit number and pass checksum validation.
		 * @throws std::invalid_argument If the provided integer is not a valid IMO number
		 *         (e.g., incorrect number of digits or failed checksum).
		 */
		explicit ImoNumber( int value );

		/**
		 * @brief Constructs an ImoNumber from a string representation.
		 * @param value The IMO number as a string (e.g., "IMO9074729" or "9074729").
		 *              The string can optionally be prefixed with "IMO".
		 * @throws std::invalid_argument If the provided string is not a valid IMO number
		 *         (e.g., incorrect format, non-numeric characters, or failed checksum).
		 */
		explicit ImoNumber( const std::string& value );

	private:
		/**
		 * @brief Private constructor for internal use, typically by tryParse.
		 * Assumes the integer value has already been validated.
		 * @param validatedValue The validated IMO number as an integer.
		 * @param bUnused An unused parameter to differentiate this constructor
		 *                from the public integer constructor.
		 */
		explicit ImoNumber( int value, bool bUnused ) noexcept;

	public:
		/** @brief Default constructor. */
		ImoNumber() = delete;

		/** @brief Copy constructor */
		ImoNumber( const ImoNumber& );

		/** @brief Move constructor */
		ImoNumber( ImoNumber&& ) noexcept;

		//----------------------------------------------
		// Destruction
		//----------------------------------------------

		/** @brief Destructor */
		~ImoNumber() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		ImoNumber& operator=( const ImoNumber& ) = default;

		/** @brief Move assignment operator */
		ImoNumber& operator=( ImoNumber&& ) noexcept = default;

		//----------------------------------------------
		// Operators
		//----------------------------------------------

		/**
		 * @brief Equality comparison operator.
		 * @param other The ImoNumber object to compare with this object.
		 * @return True if both ImoNumber objects represent the same IMO number, false otherwise.
		 */
		inline bool operator==( const ImoNumber& other ) const noexcept;

		/**
		 * @brief Inequality comparison operator.
		 * @param other The ImoNumber object to compare with this object.
		 * @return True if the ImoNumber objects represent different IMO numbers, false otherwise.
		 */
		inline bool operator!=( const ImoNumber& other ) const noexcept;

		/**
		 * @brief Explicit conversion to an integer.
		 * @return The underlying 7-digit integer value of the IMO number.
		 */
		[[nodiscard]] inline explicit operator int() const noexcept;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Hash function for ImoNumber
		 */
		[[nodiscard]] inline size_t hashCode() const noexcept;

		//----------------------------------------------
		// String conversion
		//----------------------------------------------

		/**
		 * @brief Get the string representation of this IMO number
		 * @return String in format "IMO<number>"
		 */
		[[nodiscard]] std::string toString() const;

		//----------------------------------------------
		// State inspection
		//----------------------------------------------

		/**
		 * @brief Checks if an integer value represents a valid IMO number.
		 *
		 * This method validates the 7-digit structure and the checksum.
		 * @param imoNumber The integer value to check (e.g., 9074729).
		 * @return True if the integer is a valid IMO number, false otherwise.
		 */
		[[nodiscard]] static bool isValid( int imoNumber );

		//----------------------------------------------
		// Parsing
		//----------------------------------------------

		/**
		 * @brief Parses a string representation into an ImoNumber object.
		 * @param value The string to parse (e.g., "IMO9074729" or "9074729").
		 *              Can optionally be prefixed with "IMO".
		 * @return The parsed ImoNumber object.
		 * @throws std::invalid_argument If the string is not a valid IMO number.
		 */
		[[nodiscard]] static ImoNumber parse( const std::string& value );

		/**
		 * @brief Parses a C-style string representation into an ImoNumber object.
		 * @param value The C-style string to parse (e.g., "IMO9074729" or "9074729").
		 *              Must be null-terminated. Can optionally be prefixed with "IMO".
		 * @return The parsed ImoNumber object.
		 * @throws std::invalid_argument If the string is not a valid IMO number or is null.
		 */
		[[nodiscard]] static ImoNumber parse( const char* value );

		/**
		 * @brief Tries to parse a string representation into an ImoNumber object.
		 * @param value The string_view to parse (e.g., "IMO9074729" or "9074729").
		 *              Can optionally be prefixed with "IMO".
		 * @return An std::optional containing the ImoNumber if parsing was successful,
		 *         or std::nullopt if the string is not a valid IMO number.
		 */
		[[nodiscard]] static std::optional<ImoNumber> tryParse( std::string_view value );

	private:
		//----------------------------------------------
		// Private Members
		//----------------------------------------------

		int m_value;
	};
}

#include "ImoNumber.inl"
