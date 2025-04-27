#pragma once

#include "GmodPath.h"

namespace dnv::vista::sdk
{
	/**
	 * @brief Structure representing primary and secondary items for a LocalId
	 * @note Internal implementation detail for LocalId functionality
	 */
	class LocalIdItems final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors and Assignment
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		LocalIdItems() = default;

		/**
		 * @brief Copy constructor
		 * @param other The object to copy from
		 */
		LocalIdItems( const LocalIdItems& other );

		/**
		 * @brief Assignment operator
		 * @param other The object to copy from
		 * @return Reference to this object
		 */
		LocalIdItems& operator=( const LocalIdItems& other );

		/**
		 * @brief Constructor with primary and secondary items
		 * @param primaryItem Optional primary item
		 * @param secondaryItem Optional secondary item
		 */
		LocalIdItems(
			const GmodPath& primaryItem,
			std::optional<GmodPath> secondaryItem );

		//-------------------------------------------------------------------------
		// Core Properties
		//-------------------------------------------------------------------------

		/**
		 * @brief Get primary item
		 * @return Optional primary item
		 */
		const GmodPath& primaryItem() const;

		/**
		 * @brief Get secondary item
		 * @return Optional secondary item
		 */
		std::optional<GmodPath> secondaryItem() const;

		//-------------------------------------------------------------------------
		// String Generation
		//-------------------------------------------------------------------------

		/**
		 * @brief Append items to string builder
		 * @param builder String stream to append to
		 * @param verboseMode Whether to include verbose output
		 */
		void append( std::stringstream& builder, bool verboseMode ) const;

		//-------------------------------------------------------------------------
		// Comparison Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Equality operator
		 * @param other The other LocalIdItems to compare
		 * @return true if equal, false otherwise
		 */
		bool operator==( const LocalIdItems& other ) const;

		/**
		 * @brief Inequality operator
		 * @param other The other LocalIdItems to compare
		 * @return true if not equal, false otherwise
		 */
		bool operator!=( const LocalIdItems& other ) const;

	private:
		//-------------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------------
		GmodPath m_primaryItem;
		std::optional<GmodPath> m_secondaryItem;

		//-------------------------------------------------------------------------
		// Private Helper Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Append common name with location
		 * @param builder String stream to append to
		 * @param commonName Common name to append
		 * @param location Optional location string
		 */
		static void appendCommonName(
			std::stringstream& builder,
			const std::string& commonName,
			std::optional<std::string> location );
	};
}
