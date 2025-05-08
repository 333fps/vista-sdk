/**
 * @file MetadataTag.h
 * @brief Defines the MetadataTag class for representing metadata in the VIS system.
 */

#pragma once

#include "CodebookName.h"

namespace dnv::vista::sdk
{
	/**
	 * @brief Represents a metadata tag in the VIS system.
	 *
	 * A metadata tag consists of a name (from a codebook), a value, and an optional
	 * flag indicating whether it is a custom tag. It is used to store and manage
	 * metadata associated with various entities. This class is immutable;
	 * all properties are set during construction.
	 */
	class MetadataTag final
	{
	public:
		//-------------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------------

		/**
		 * @brief Constructs a MetadataTag object.
		 * @param name The name of the metadata tag (from the CodebookName enumeration).
		 * @param value The value of the metadata tag.
		 * @param isCustom Indicates whether the tag is custom (default is false).
		 */
		MetadataTag( CodebookName name, const std::string& value, bool isCustom = false );

		/** @brief Default constructor. */
		MetadataTag() = delete;

		/** @brief Copy constructor. */
		MetadataTag( const MetadataTag& ) = default;

		/** @brief Move constructor. */
		MetadataTag( MetadataTag&& ) noexcept = default;

		/** @brief Destructor. */
		~MetadataTag() = default;

		//-------------------------------------------------------------------------
		// Assignment Operators
		//-------------------------------------------------------------------------

		/** @brief Copy assignment operator. */
		MetadataTag& operator=( const MetadataTag& ) = default;

		/** @brief Move assignment operator. */
		MetadataTag& operator=( MetadataTag&& ) noexcept = default;

		//-------------------------------------------------------------------------
		// Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Equality operator.
		 * Compares two MetadataTag objects for equality. Tags are considered equal if
		 * their names are the same and their values are equal.
		 * @param other The other MetadataTag to compare with this instance.
		 * @return True if the tags are equal, false otherwise.
		 * @throws std::invalid_argument If the CodebookName of the tags are different,
		 *         as comparison is not meaningful in that context.
		 */
		bool operator==( const MetadataTag& other ) const;

		/**
		 * @brief Inequality operator.
		 * @param other The other MetadataTag to compare with this instance.
		 * @return True if the tags are not equal, false otherwise.
		 * @throws std::invalid_argument If the CodebookName of the tags are different (propagated from operator==).
		 */
		bool operator!=( const MetadataTag& other ) const;

		/**
		 * @brief Implicit conversion to a std::string.
		 * Returns the 'Value' part of the metadata tag.
		 * @return The 'Value' of the metadata tag as a string.
		 */
		[[nodiscard]] operator std::string() const;

		//-------------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Gets the name of the metadata tag.
		 * @return The CodebookName representing the name of the metadata tag.
		 */
		[[nodiscard]] CodebookName name() const noexcept;

		/**
		 * @brief Gets the value of the metadata tag.
		 * @return A constant reference to the string value of the metadata tag.
		 */
		[[nodiscard]] const std::string& value() const noexcept;

		/**
		 * @brief Checks if the metadata tag is custom.
		 * @return True if the tag is custom, false otherwise.
		 */
		[[nodiscard]] bool isCustom() const noexcept;

		/**
		 * @brief Gets the prefix character used for string representation of the metadata tag.
		 * @return '~' if the tag is custom, '-' otherwise.
		 */
		[[nodiscard]] char prefix() const noexcept;

		/**
		 * @brief Gets the hash code of the metadata tag.
		 * The hash code is based solely on the tag's value.
		 * @return The hash code of the tag's value.
		 */
		[[nodiscard]] size_t hashCode() const;

		//-------------------------------------------------------------------------
		// String Conversion Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Converts the metadata tag to its string value.
		 * This method returns only the 'Value' part of the tag.
		 * For a fully formatted string including prefix and name, use toString(std::ostringstream&, char).
		 * @return The 'Value' of the metadata tag as a string.
		 */
		[[nodiscard]] std::string toString() const;

		/**
		 * @brief Appends a fully formatted string representation of the metadata tag to a string stream.
		 * The format is typically: Prefix + CodebookName (as string) + Separator + Value.
		 * Example: "-VESSEL_TYPE/TANKER" or "~MY_CUSTOM_TAG/SOME_VALUE".
		 * @param builder The string stream to append to.
		 * @param separator The separator character to use between the name and value (default is '/').
		 * @throws std::invalid_argument If the CodebookName is not recognized for string conversion.
		 */
		void toString( std::ostringstream& builder, char separator = '/' ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		/** @brief The name of the metadata tag, represented by a CodebookName enum value. */
		CodebookName m_name;

		/** @brief The string value associated with the metadata tag. */
		std::string m_value;

		/** @brief A boolean flag indicating whether this is a custom tag (true) or a standard tag (false). */
		bool m_custom;
	};
}
