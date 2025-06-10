/**
 * @file LocalIdItems.h
 * @brief Defines the LocalIdItems class for representing primary and secondary items in LocalIds.
 */
#pragma once

#include "GmodPath.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// LocalIdItems class
	//=====================================================================

	/**
	 * @brief Immutable structure representing primary and optional secondary GMOD items for a LocalId.
	 *
	 * This class stores primary and secondary GmodPath items. It is designed to be
	 * immutable after construction. Due to GmodPath being non-copyable, this class
	 * also enforces move-only semantics for construction involving GmodPath objects.
	 */
	class LocalIdItems final
	{
	public:
		//----------------------------------------------
		// Construction
		//----------------------------------------------

		/**
		 * @brief Base move constructor.
		 *
		 * Constructs LocalIdItems by taking ownership of the provided GmodPath objects.
		 * This is the primary way to create a populated instance from scratch.
		 *
		 * @param primaryItem The primary GmodPath (moved into `m_primaryItem`).
		 * @param secondaryItem The optional secondary GmodPath (moved into `m_secondaryItem`).
		 */
		LocalIdItems( GmodPath&& primaryItem, std::optional<GmodPath>&& secondaryItem );

		/**
		 * @brief Constructor to create a new instance by replacing the primary item.
		 *
		 * Creates a new instance by moving the secondary item from `other` and
		 * moving the provided `newPrimaryItem`.
		 * @param other The existing LocalIdItems instance (moved from).
		 * @param newPrimaryItem The new primary GmodPath (moved).
		 */
		LocalIdItems( LocalIdItems&& other, GmodPath&& newPrimaryItem );

		/**
		 * @brief Constructor to create a new instance by replacing the secondary item.
		 *
		 * Creates a new instance by moving the primary item from `other` and
		 * moving the provided `newSecondaryItem`.
		 * @param other The existing LocalIdItems instance (moved from).
		 * @param newSecondaryItem The new optional secondary GmodPath (moved).
		 */
		LocalIdItems( LocalIdItems&& other, std::optional<GmodPath>&& newSecondaryItem );

		/** @brief Default constructor */
		LocalIdItems() = default;

		/** @brief Copy constructor */
		LocalIdItems( const LocalIdItems& ) = default;

		/** @brief Move constructor */
		LocalIdItems( LocalIdItems&& other ) noexcept = default;

		//----------------------------------------------
		// Destruction
		//----------------------------------------------
		/** @brief Destructor */
		~LocalIdItems() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		LocalIdItems& operator=( const LocalIdItems& ) = delete;

		/** @brief Move assignment operator */
		LocalIdItems& operator=( LocalIdItems&& other ) noexcept = default;

		//----------------------------------------------
		// Comparison operators
		//----------------------------------------------

		/**
		 * @brief Equality comparison operator.
		 *
		 * Compares the `m_primaryItem` and `m_secondaryItem` members for equality.
		 * `std::optional` handles comparisons involving `std::nullopt` correctly.
		 * Requires `GmodPath` to have `operator==` defined.
		 *
		 * @param other The other LocalIdItems instance to compare with.
		 * @return true if both primary and secondary items are equal, false otherwise.
		 */
		[[nodiscard]] inline bool operator==( const LocalIdItems& other ) const noexcept;

		/**
		 * @brief Inequality comparison operator.
		 * @param other The other LocalIdItems instance to compare with.
		 * @return true if the instances are not equal, false otherwise.
		 */
		[[nodiscard]] inline bool operator!=( const LocalIdItems& other ) const noexcept;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the primary item.
		 * @return A const reference to the optional primary GmodPath.
		 */
		[[nodiscard]] inline const std::optional<GmodPath>& primaryItem() const noexcept;

		/**
		 * @brief Gets the optional secondary item.
		 * @return A const reference to the optional secondary GmodPath.
		 */
		[[nodiscard]] inline const std::optional<GmodPath>& secondaryItem() const noexcept;

		/**
		 * @brief Checks if both primary and secondary items are uninitialized (nullopt).
		 * @return True if both `m_primaryItem` and `m_secondaryItem` are `std::nullopt`, false otherwise.
		 */
		[[nodiscard]] inline bool isEmpty() const noexcept;

		//----------------------------------------------
		// Public methods
		//----------------------------------------------

		/**
		 * @brief Appends the string representation of the items to a stringstream.
		 *
		 * Formats and appends the primary and secondary items according to LocalId
		 * formatting rules. Includes verbose common name details if requested.
		 *
		 * @param builder The stringstream to append to.
		 * @param verboseMode If true, appends verbose common name information.
		 */
		void append( std::stringstream& builder, bool verboseMode ) const;

	private:
		//----------------------------------------------
		// Private helper methods
		//----------------------------------------------

		/**
		 * @brief Appends a formatted common name with optional location to a stringstream.
		 *
		 * Helper method used by `append` during verbose string formatting.
		 *
		 * @param builder The stringstream to append to.
		 * @param commonName The common name string_view to format and append.
		 * @param location An optional location string to append if present.
		 */
		static void appendCommonName(
			std::stringstream& builder,
			std::string_view commonName,
			const std::optional<std::string>& location );

		//----------------------------------------------
		// Member variables
		//----------------------------------------------

		/** @brief The optional primary item path. */
		std::optional<GmodPath> m_primaryItem;

		/** @brief The optional secondary item path. */
		std::optional<GmodPath> m_secondaryItem;
	};
}

#include "LocalIdItems.inl"
