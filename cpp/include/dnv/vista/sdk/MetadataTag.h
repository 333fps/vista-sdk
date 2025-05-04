#pragma once

#include "CodebookName.h"

namespace dnv::vista::sdk
{
	/**
	 * @brief Represents a metadata tag in the VIS system.
	 *
	 * A metadata tag consists of a name, value, and an optional flag indicating
	 * whether it is a custom tag. It is used to store and manage metadata
	 * associated with various entities in the system.
	 */
	class MetadataTag final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors
		//-------------------------------------------------------------------------

		/**
		 * @brief Constructs a MetadataTag object.
		 * @param name The name of the metadata tag (from the CodebookName enumeration).
		 * @param value The value of the metadata tag.
		 * @param isCustom Indicates whether the tag is custom (default is false).
		 */
		MetadataTag( CodebookName name, const std::string& value, bool isCustom = false );

		//-------------------------------------------------------------------------
		// Accessors
		//-------------------------------------------------------------------------

		/**
		 * @brief Gets the name of the metadata tag.
		 * @return The name of the metadata tag.
		 */
		CodebookName name() const;

		/**
		 * @brief Gets the value of the metadata tag.
		 * @return A reference to the string value of the metadata tag.
		 */
		const std::string& value() const;

		/**
		 * @brief Checks if the metadata tag is custom.
		 * @return True if the tag is custom, false otherwise.
		 */
		bool isCustom() const;

		/**
		 * @brief Gets the prefix character for the metadata tag.
		 * @return '~' if the tag is custom, '-' otherwise.
		 */
		char prefix() const;

		/**
		 * @brief Gets the hash code of the metadata tag.
		 * @return The hash code of the tag's value.
		 */
		size_t hashCode() const;

		//-------------------------------------------------------------------------
		// String Conversion Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Converts the metadata tag to a string representation.
		 * @return A string representation of the metadata tag.
		 */
		std::string toString() const;

		/**
		 * @brief Appends the metadata tag to a string stream.
		 * @param builder The string stream to append to.
		 * @param separator The separator character to use (default is '/').
		 */
		void toString( std::ostringstream& builder, char separator = '/' ) const;

		//-------------------------------------------------------------------------
		// Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Implicit conversion to a string.
		 * @return The value of the metadata tag as a string.
		 */
		operator std::string() const;

		/**
		 * @brief Equality operator.
		 * @param other The other MetadataTag to compare.
		 * @return True if the tags are equal, false otherwise.
		 * @throws std::invalid_argument If the names of the tags are different.
		 */
		bool operator==( const MetadataTag& other ) const;

		/**
		 * @brief Inequality operator.
		 * @param other The other MetadataTag to compare.
		 * @return True if the tags are not equal, false otherwise.
		 */
		bool operator!=( const MetadataTag& other ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Members
		//-------------------------------------------------------------------------

		CodebookName m_name; ///< The name of the metadata tag.
		std::string m_value; ///< The value of the metadata tag.
		bool m_custom;		 ///< Indicates whether the tag is custom.
	};
}
