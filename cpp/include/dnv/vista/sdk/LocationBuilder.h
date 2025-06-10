/**
 * @file LocationBuilder.h
 * @brief Defines the LocationBuilder class for constructing Location objects with validation.
 *
 * @details This file provides the LocationBuilder class, which implements a fluent builder
 * pattern for constructing Location objects with component-wise validation against
 * the VIS standard. The builder supports setting numeric, side, vertical, transverse,
 * and longitudinal components with immutable operations.
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class Location;
	class Locations;
	enum class LocationGroup;
	enum class VisVersion;

	//=====================================================================
	// LocationBuilder class
	//=====================================================================

	/**
	 * @brief A fluent builder for constructing Location objects with validation.
	 * @details Provides a type-safe way to build locations by setting individual components
	 *          (number, side, vertical, transverse, longitudinal) with validation against
	 *          the VIS standard for a specific version.
	 */
	class LocationBuilder final
	{
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Private constructor for internal use.
		 * @param visVersion The VIS version.
		 * @param reversedGroups Map from characters to their location groups.
		 */
		explicit LocationBuilder( VisVersion visVersion, const std::map<char, LocationGroup>& reversedGroups );

	protected:
		/** @brief Default constructor. */
		LocationBuilder() = default;

	public:
		/** @brief Copy constructor */
		LocationBuilder( const LocationBuilder& ) = default;

		/** @brief Move constructor */
		LocationBuilder( LocationBuilder&& ) noexcept = default;

		/** @brief Destructor */
		~LocationBuilder() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		LocationBuilder& operator=( const LocationBuilder& ) = default;

		/** @brief Move assignment operator */
		LocationBuilder& operator=( LocationBuilder&& ) noexcept = default;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the VIS version this builder is configured for.
		 * @return The VisVersion enum value.
		 */
		[[nodiscard]] inline VisVersion visVersion() const noexcept;

		/**
		 * @brief Gets the numeric component of the location.
		 * @return Optional integer representing the number component.
		 */
		[[nodiscard]] inline const std::optional<int>& number() const noexcept;

		/**
		 * @brief Gets the side component of the location.
		 * @return Optional character representing the side (e.g., 'P', 'C', 'S').
		 */
		[[nodiscard]] inline const std::optional<char>& side() const noexcept;

		/**
		 * @brief Gets the vertical component of the location.
		 * @return Optional character representing vertical position (e.g., 'U', 'M', 'L').
		 */
		[[nodiscard]] inline const std::optional<char>& vertical() const noexcept;

		/**
		 * @brief Gets the transverse component of the location.
		 * @return Optional character representing transverse position (e.g., 'I', 'O').
		 */
		[[nodiscard]] inline const std::optional<char>& transverse() const noexcept;

		/**
		 * @brief Gets the longitudinal component of the location.
		 * @return Optional character representing longitudinal position (e.g., 'F', 'A').
		 */
		[[nodiscard]] inline const std::optional<char>& longitudinal() const noexcept;

		//----------------------------------------------
		// Conversion and comparison
		//----------------------------------------------

		/**
		 * @brief Generates the string representation of the location.
		 * @details Components are alphabetically sorted as per VIS standard.
		 * @return A string representing the current builder state.
		 */
		[[nodiscard]] std::string toString() const;

		//----------------------------------------------
		// Static factory methods
		//----------------------------------------------

		/**
		 * @brief Creates a new LocationBuilder for the specified Locations context.
		 * @param locations The Locations object providing validation context.
		 * @return A new LocationBuilder instance.
		 */
		[[nodiscard]] static LocationBuilder create( const Locations& locations );

		//----------------------------------------------
		// Build methods (Immutable fluent interface)
		//----------------------------------------------

		//----------------------------
		// Build
		//----------------------------

		/**
		 * @brief Builds a Location from the current builder state.
		 * @return The constructed Location object.
		 * @details The resulting location string is alphabetically sorted according to VIS standards.
		 */
		[[nodiscard]] Location build() const;

		//----------------------------
		// Location
		//----------------------------

		/**
		 * @brief Sets all components by parsing an existing Location.
		 * @param location The Location to parse and apply to this builder.
		 * @return A new LocationBuilder instance with the parsed components.
		 * @throws std::invalid_argument If the location contains invalid components.
		 */
		[[nodiscard]] LocationBuilder withLocation( const Location& location ) const;

		//----------------------------
		// Number
		//----------------------------

		/**
		 * @brief Sets the numeric component.
		 * @param number The number component (must be >= 1).
		 * @return A new LocationBuilder instance with the updated number.
		 * @throws std::invalid_argument If number is less than 1.
		 */
		[[nodiscard]] LocationBuilder withNumber( int number ) const;

		/**
		 * @brief Removes the numeric component.
		 * @return A new LocationBuilder instance without the number component.
		 */
		[[nodiscard]] LocationBuilder withoutNumber() const;

		//----------------------------
		// Side
		//----------------------------

		/**
		 * @brief Sets the side component.
		 * @param side The side character (e.g., 'P', 'C', 'S').
		 * @return A new LocationBuilder instance with the updated side.
		 * @throws std::invalid_argument If the character is not a valid side value.
		 */
		[[nodiscard]] LocationBuilder withSide( char side ) const;

		/**
		 * @brief Removes the side component.
		 * @return A new LocationBuilder instance without the side component.
		 */
		[[nodiscard]] LocationBuilder withoutSide() const;

		//----------------------------
		// Vertical
		//----------------------------

		/**
		 * @brief Sets the vertical component.
		 * @param vertical The vertical character (e.g., 'U', 'M', 'L').
		 * @return A new LocationBuilder instance with the updated vertical.
		 * @throws std::invalid_argument If the character is not a valid vertical value.
		 */
		[[nodiscard]] LocationBuilder withVertical( char vertical ) const;

		/**
		 * @brief Removes the vertical component.
		 * @return A new LocationBuilder instance without the vertical component.
		 */
		[[nodiscard]] LocationBuilder withoutVertical() const;

		//----------------------------
		// Transverse
		//----------------------------

		/**
		 * @brief Sets the transverse component.
		 * @param transverse The transverse character (e.g., 'I', 'O').
		 * @return A new LocationBuilder instance with the updated transverse.
		 * @throws std::invalid_argument If the character is not a valid transverse value.
		 */
		[[nodiscard]] LocationBuilder withTransverse( char transverse ) const;

		/**
		 * @brief Removes the transverse component.
		 * @return A new LocationBuilder instance without the transverse component.
		 */
		[[nodiscard]] LocationBuilder withoutTransverse() const;

		//----------------------------
		// Longitudinal
		//----------------------------

		/**
		 * @brief Sets the longitudinal component.
		 * @param longitudinal The longitudinal character (e.g., 'F', 'A').
		 * @return A new LocationBuilder instance with the updated longitudinal.
		 * @throws std::invalid_argument If the character is not a valid longitudinal value.
		 */
		[[nodiscard]] LocationBuilder withLongitudinal( char longitudinal ) const;

		/**
		 * @brief Removes the longitudinal component.
		 * @return A new LocationBuilder instance without the longitudinal component.
		 */
		[[nodiscard]] LocationBuilder withoutLongitudinal() const;

		//----------------------------
		// Value
		//----------------------------

		/**
		 * @brief Sets a numeric value (equivalent to withNumber).
		 * @param value The numeric value to set.
		 * @return A new LocationBuilder instance with the updated number.
		 * @throws std::invalid_argument If value is less than 1.
		 */
		[[nodiscard]] LocationBuilder withValue( int value ) const;

		/**
		 * @brief Sets a character value to the appropriate component based on validation.
		 * @param value The character value to set.
		 * @return A new LocationBuilder instance with the updated component.
		 * @throws std::invalid_argument If the character is not valid for any component.
		 */
		[[nodiscard]] LocationBuilder withValue( char value ) const;

		/**
		 * @brief Removes a component by group type.
		 * @param group The LocationGroup to remove.
		 * @return A new LocationBuilder instance with the specified component removed.
		 */
		[[nodiscard]] LocationBuilder withoutValue( LocationGroup group ) const;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The numeric component (1-based). */
		std::optional<int> m_number;

		/** @brief The side component ('P', 'C', 'S'). */
		std::optional<char> m_side;

		/** @brief The vertical component ('U', 'M', 'L'). */
		std::optional<char> m_vertical;

		/** @brief The transverse component ('I', 'O'). */
		std::optional<char> m_transverse;

		/** @brief The longitudinal component ('F', 'A'). */
		std::optional<char> m_longitudinal;

		/** @brief The VIS version this builder is configured for. */
		VisVersion m_visVersion;

		/** @brief Map from character codes to their LocationGroup for validation. */
		std::map<char, LocationGroup> m_reversedGroups;
	};
}

#include "LocationBuilder.inl"
