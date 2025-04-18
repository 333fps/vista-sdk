#pragma once

namespace dnv::vista::sdk
{
	/**
	 * @brief Represents an International Maritime Organization (IMO) number
	 *
	 * An IMO number is a unique seven-digit identifier assigned to maritime vessels
	 * with a check digit for validation according to the IMO standard.
	 */
	class ImoNumber
	{
	public:
		/**
		 * @brief Default constructor is deleted
		 */
		ImoNumber() = delete;

		/**
		 * @brief Construct from integer value
		 * @param value The IMO number as integer (without IMO prefix)
		 * @throws std::invalid_argument If the value is not a valid IMO number
		 */
		explicit ImoNumber( int value );

		/**
		 * @brief Construct from string value
		 * @param value The IMO number as string (with or without IMO prefix)
		 * @throws std::invalid_argument If the value is not a valid IMO number
		 */
		explicit ImoNumber( const std::string& value );

		static ImoNumber Parse( const char* value );

		/**
		 * @brief Parse a string into an IMO number
		 * @param value The string to parse
		 * @return The parsed IMO number
		 * @throws std::invalid_argument If the value is not a valid IMO number
		 */
		static ImoNumber Parse( const std::string& value );

		/**
		 * @brief Try to parse a string into an IMO number
		 * @param value The string to parse
		 * @return The IMO number if parsing was successful, or nullopt if not
		 */
		static std::optional<ImoNumber> TryParse( const std::string& value );

		/**
		 * @brief Check if an integer value is a valid IMO number
		 * @param imoNumber The value to check
		 * @return True if the value is a valid IMO number, false otherwise
		 */
		static bool IsValid( int imoNumber );

		/**
		 * @brief Get the string representation of this IMO number
		 * @return String in format "IMO<number>"
		 */
		std::string ToString() const;

		/**
		 * @brief Explicit conversion to integer
		 * @return The IMO number as integer
		 */
		explicit operator int() const;

		/**
		 * @brief Hash function for ImoNumber
		 */
		struct Hash
		{
			std::size_t operator()( const ImoNumber& imoNumber ) const noexcept;
		};

		/**
		 * @brief Equality operator
		 * @param other The other IMO number to compare with
		 * @return True if the IMO numbers are equal
		 */
		bool operator==( const ImoNumber& other ) const;

	private:
		/**
		 * @brief Private constructor used by TryParse
		 * @param value The validated IMO number as integer
		 * @param _ Unused parameter to disambiguate from public constructor
		 */
		explicit ImoNumber( int value, bool ) noexcept;

		/**
		 * @brief Extract digits from an integer into an array
		 * @param number The number to extract digits from
		 * @param digits The array to store the digits in (must be at least 7 elements)
		 */
		static void GetDigits( int number, unsigned char* digits );

		int m_value; ///< The IMO number as integer
	};
}
