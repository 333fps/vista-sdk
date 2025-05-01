/**
 * @file LocalIdItems.h
 * @brief Defines the LocalIdItems class for representing primary and secondary items in LocalIds
 */
#pragma once

#include "GmodPath.h"

namespace dnv::vista::sdk
{
	/**
	 * @brief Immutable structure representing primary and secondary items for a LocalId
	 *
	 * This class stores primary and secondary GmodPath items for use in LocalIds.
	 * It is designed to be immutable, with move-only semantics to prevent unintended
	 * modifications after construction.
	 */
	class LocalIdItems final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors and Assignment
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 *
		 * Creates an empty LocalIdItems instance with no primary or secondary items.
		 */
		LocalIdItems() = default;

		/**
		 * @brief Constructor with primary and secondary items
		 *
		 * @param primaryItem Primary item path
		 * @param secondaryItem Optional secondary item path
		 */
		LocalIdItems(
			const GmodPath& primaryItem,
			const std::optional<GmodPath>& secondaryItem );

		/**
		 * @brief Deleted copy constructor to enforce immutability
		 */
		LocalIdItems( const LocalIdItems& ) = default;

		/**
		 * @brief Deleted copy assignment to enforce immutability
		 */
		LocalIdItems& operator=( const LocalIdItems& ) = default;

		/** @brief Allow move constructor. */
		LocalIdItems( LocalIdItems&& ) noexcept = default;

		/** @brief Allow move assignment. */
		LocalIdItems& operator=( LocalIdItems&& ) noexcept = default;

		/**
		 * @brief Virtual destructor
		 */
		~LocalIdItems() = default;

		//-------------------------------------------------------------------------
		// Core Properties
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the primary item
		 * @return Reference to the primary item path
		 */
		const GmodPath& primaryItem() const noexcept;

		/**
		 * @brief Get the secondary item
		 * @return Optional reference to the secondary item path
		 */
		[[nodiscard]] const std::optional<GmodPath>& secondaryItem() const noexcept;

		/**
		 * @brief Check if this LocalIdItems is empty
		 * @return True if no primary or secondary items are set
		 */
		[[nodiscard]] bool isEmpty() const noexcept;

		//-------------------------------------------------------------------------
		// String Generation
		//-------------------------------------------------------------------------

		/**
		 * @brief Append items to string builder
		 *
		 * Formats and appends the primary and secondary items to the provided string stream
		 * according to LocalId formatting rules.
		 *
		 * @param builder String stream to append to
		 * @param verboseMode Whether to include verbose output with additional details
		 */
		void append( std::stringstream& builder, bool verboseMode ) const;

		/**
		 * @brief Convert to string representation
		 * @param verboseMode Whether to include verbose output
		 * @return String representation of the items
		 */
		[[nodiscard]] std::string toString( bool verboseMode = false ) const;

		//-------------------------------------------------------------------------
		// Comparison Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Equality comparison operator
		 *
		 * Two LocalIdItems are considered equal if they have the same primary item
		 * and either both have the same secondary item or neither has a secondary item.
		 *
		 * @param other The other LocalIdItems to compare with
		 * @return true if equal, false otherwise
		 */
		[[nodiscard]] bool operator==( const LocalIdItems& other ) const noexcept;

		/**
		 * @brief Inequality comparison operator
		 * @param other The other LocalIdItems to compare with
		 * @return true if not equal, false otherwise
		 */
		[[nodiscard]] bool operator!=( const LocalIdItems& other ) const noexcept;

	private:
		//-------------------------------------------------------------------------
		// Member Variables
		//-------------------------------------------------------------------------

		/** @brief The primary item path */
		GmodPath m_primaryItem;

		/** @brief The optional secondary item path */
		std::optional<GmodPath> m_secondaryItem;

		//-------------------------------------------------------------------------
		// Private Helper Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Append common name with location to string builder
		 *
		 * Helper method used during string formatting to handle common name patterns.
		 *
		 * @param builder String stream to append to
		 * @param commonName Common name to append
		 * @param location Optional location string
		 */
		static void appendCommonName(
			std::stringstream& builder,
			std::string_view commonName,
			const std::optional<std::string>& location );
	};
}
