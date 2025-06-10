/**
 * @file Locations.h
 * @brief Container for Vessel Information Structure (VIS) location definitions and parsing
 * @details Provides access to standardized location definitions, parsing capabilities,
 *          and validation for location strings according to the VIS standard.
 *          Supports building location objects with component-wise validation.
 */

#pragma once

#include "LocationsDto.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	enum class VisVersion;
	class ParsingErrors;
	class LocationParsingErrorBuilder;

	//=====================================================================
	// Enumerations
	//=====================================================================

	/**
	 * @brief Enumeration of location groups.
	 *
	 * Represents different groups of locations used in the VIS system,
	 * such as side, vertical position, etc.
	 */
	enum class LocationGroup
	{
		Number,
		Side,
		Vertical,
		Transverse,
		Longitudinal
	};

	//=====================================================================
	// Location Class
	//=====================================================================

	/**
	 * @brief Represents a validated location string in the VIS system.
	 *
	 * This class encapsulates a location string (e.g., "P", "CL1", "P1U").
	 * Instances are typically created via parsing methods in the `Locations` class.
	 * This class is immutable; its value is set at construction.
	 */
	class Location final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Constructs a Location object with a specific value.
		 * @param value The location string value.
		 */
		explicit Location( std::string_view value );

		/** @brief Default constructor */
		Location() = default;

		/** @brief Copy constructor */
		Location( const Location& ) = default;

		/** @brief Move constructor */
		Location( Location&& ) noexcept = default;

		/** @brief Destructor */
		~Location() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		Location& operator=( const Location& ) = default;

		/** @brief Move assignment operator */
		Location& operator=( Location&& ) noexcept = default;

		//----------------------------------------------
		// Comparison Operators
		//----------------------------------------------

		/**
		 * @brief Equality operator. Compares this Location with another.
		 * @param other The other Location to compare.
		 * @return True if the location values are equal, false otherwise.
		 */
		inline bool operator==( const Location& other ) const;

		/**
		 * @brief Inequality operator. Compares this Location with another.
		 * @param other The other Location to compare.
		 * @return True if the location values are not equal, false otherwise.
		 */
		inline bool operator!=( const Location& other ) const;

		//----------------------------------------------
		// Conversion Operators
		//----------------------------------------------

		/**
		 * @brief Implicit conversion to std::string.
		 * @return The location value as a string.
		 */
		[[nodiscard]] inline operator std::string() const noexcept;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the string value of the location.
		 * @return A constant reference to the location string value.
		 */
		[[nodiscard]] const std::string& value() const noexcept;

		//----------------------------------------------
		// Conversion
		//----------------------------------------------

		/**
		 * @brief Converts the location to its string representation.
		 * Equivalent to calling `value()` or the implicit string conversion.
		 * @return The string representation of the location.
		 */
		[[nodiscard]] std::string toString() const noexcept;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The string value representing the location. */
		std::string m_value;
	};

	//=====================================================================
	// RelativeLocation
	//=====================================================================

	/**
	 * @brief Represents a relative location within the VIS system, defined by a code, name, and definition.
	 *
	 * This class links a specific character code (e.g., 'P' for Port) to its human-readable name,
	 * its `Location` object representation, and an optional detailed definition.
	 * This class is immutable after construction.
	 */
	class RelativeLocation final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Constructs a RelativeLocation object.
		 * @param code The single character code representing this relative location (e.g., 'P', 'C', 'L', '1').
		 * @param name The human-readable name of the relative location (e.g., "Port", "Center", "Lower", "Zone 1").
		 * @param location The `Location` object representing the code (e.g., a `Location` with value "P").
		 * @param definition An optional detailed description of the relative location.
		 */
		RelativeLocation( char code,
			const std::string& name,
			const Location& location,
			const std::optional<std::string> definition = std::nullopt );

		/** @brief Default constructor */
		RelativeLocation() = default;

		/** @brief Copy constructor */
		RelativeLocation( const RelativeLocation& ) = default;

		/** @brief Move constructor */
		RelativeLocation( RelativeLocation&& ) noexcept = default;

		/** @brief Destructor */
		~RelativeLocation() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		RelativeLocation& operator=( const RelativeLocation& ) = default;

		/** @brief Move assignment operator */
		RelativeLocation& operator=( RelativeLocation&& ) noexcept = default;

		//----------------------------------------------
		// Comparison Operators
		//----------------------------------------------

		/**
		 * @brief Equality operator. Compares based on the `code` member.
		 * @param other The other RelativeLocation to compare.
		 * @return True if the codes are equal, false otherwise.
		 */
		inline bool operator==( const RelativeLocation& other ) const;

		/**
		 * @brief Inequality operator. Compares based on the `code` member.
		 * @param other The other RelativeLocation to compare.
		 * @return True if the codes are not equal, false otherwise.
		 */
		inline bool operator!=( const RelativeLocation& other ) const;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the character code of the relative location.
		 * @return The character code.
		 */
		[[nodiscard]] inline char code() const noexcept;

		/**
		 * @brief Gets the name of the relative location.
		 * @return A constant reference to the name string.
		 */
		[[nodiscard]] inline const std::string& name() const noexcept;

		/**
		 * @brief Gets the optional definition of the relative location.
		 * @return A constant reference to an optional string containing the definition.
		 */
		[[nodiscard]] inline const std::optional<std::string>& definition() const noexcept;

		/**
		 * @brief Gets the `Location` object associated with this relative location.
		 * @return A constant reference to the `Location` object.
		 */
		[[nodiscard]] inline const Location& location() const noexcept;

		/**
		 * @brief Gets the hash code of the relative location, based on its `code`.
		 * @return The hash code.
		 */
		[[nodiscard]] inline size_t hashCode() const noexcept;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The single character code for this relative location (e.g., 'P', '1'). */
		char m_code{};

		/** @brief The human-readable name (e.g., "Port", "Zone 1"). */
		std::string m_name;

		/** @brief The `Location` object corresponding to this code (e.g., Location("P")). */
		Location m_location;

		/** @brief An optional detailed definition. */
		std::optional<std::string> m_definition;
	};

	//=====================================================================
	// LocationCharDict
	//=====================================================================

	/**
	 * @brief A dictionary-like structure for managing location characters within parsing logic.
	 *
	 * This class is an internal helper used by the `Locations` class to ensure
	 * that location strings adhere to rules about character uniqueness within groups.
	 * It is not intended for general public use.
	 * This class is non-copyable but movable.
	 */
	class LocationCharDict final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/** @brief Default constructor. */
		LocationCharDict() = default;

		/** @brief Copy constructor */
		LocationCharDict( const LocationCharDict& ) = delete;

		/** @brief Move constructor */
		LocationCharDict( LocationCharDict&& ) noexcept = default;

		/** @brief Destructor */
		~LocationCharDict() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		LocationCharDict& operator=( const LocationCharDict& ) = delete;

		/** @brief Move assignment operator */
		LocationCharDict& operator=( LocationCharDict&& ) noexcept = default;

		//----------------------------------------------
		// Lookup operators
		//----------------------------------------------

		/**
		 * @brief Access a location character by key
		 * @param key The location group key
		 * @return A reference to the optional character
		 */
		std::optional<char>& operator[]( LocationGroup key );

		//----------------------------------------------
		// Public methods
		//----------------------------------------------

		/**
		 * @brief Try to add a value to the dictionary
		 * @param key The location group key
		 * @param value The character value to add
		 * @param existingValue Output parameter for the existing value, if any
		 * @return True if the value was added, false otherwise
		 */
		bool tryAdd( LocationGroup key, char value, std::optional<char>& existingValue );

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The internal table storing optional characters for each relevant `LocationGroup`.
		 * The array size is 4, corresponding to Side, Vertical, Transverse, and Longitudinal groups.
		 * `LocationGroup::Number` is handled separately in parsing logic.
		 */
		std::array<std::optional<char>, 4> m_table;
	};

	//=====================================================================
	// Locations
	//=====================================================================

	/**
	 * @brief Manages and provides access to all defined locations for a specific VIS version.
	 *
	 * This class serves as the primary entry point for parsing location strings and
	 * retrieving information about predefined relative locations.
	 * An instance of this class is typically initialized with data loaded from a VIS standard definition.
	 * This class is non-copyable (due to potentially large internal data) but movable.
	 */
	class Locations final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Constructs a Locations manager for a specific VIS version using data from a DTO.
		 * @param version The VIS version this Locations instance will represent.
		 * @param dto The data transfer object containing the location definitions.
		 */
		explicit Locations( VisVersion version, const LocationsDto& dto );

		/** @brief Default constructor. */
		Locations() = default;

		/** @brief Copy constructor */
		Locations( const Locations& ) = default;

		/** @brief Move constructor */
		Locations( Locations&& ) noexcept = default;

		/** @brief Destructor */
		~Locations() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		Locations& operator=( const Locations& ) = default;

		/** @brief Move assignment operator */
		Locations& operator=( Locations&& ) noexcept = default;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Gets the VIS version this Locations instance pertains to.
		 * @return The `VisVersion` enum value.
		 */
		[[nodiscard]] VisVersion visVersion() const noexcept;

		/**
		 * @brief Gets a read-only list of all defined relative locations.
		 * @return A constant reference to a vector of `RelativeLocation` objects.
		 */
		[[nodiscard]] const std::vector<RelativeLocation>& relativeLocations() const noexcept;

		/**
		 * @brief Gets a read-only map of location groups to their respective relative locations.
		 * @return A constant reference to an unordered_map where keys are `LocationGroup`
		 *         and values are vectors of `RelativeLocation` objects belonging to that group.
		 */
		[[nodiscard]] const std::unordered_map<LocationGroup, std::vector<RelativeLocation>>& groups() const noexcept;

		/**
		 * @brief Gets a read-only map from character codes to their corresponding location groups.
		 * @details This map is used for validation purposes to determine which LocationGroup
		 *          a specific character belongs to (e.g., 'P' -> LocationGroup::Side,
		 *          'U' -> LocationGroup::Vertical). Essential for validating location components
		 *          during parsing and building operations.
		 * @return A constant reference to an unordered_map where keys are character codes
		 *         (e.g., 'P', 'C', 'S', 'U', 'M', 'L', 'I', 'O', 'F', 'A') and values
		 *         are their corresponding `LocationGroup` classifications.
		 */
		[[nodiscard]] const std::map<char, LocationGroup>& reversedGroups() const noexcept;
		//----------------------------------------------
		// Public methods - Parsing
		//----------------------------------------------

		/**
		 * @brief Parses a location string (represented by a string_view) into a `Location` object.
		 * @param locationStr The location string_view to parse.
		 * @return The parsed `Location` object.
		 * @throws std::invalid_argument If parsing fails.
		 */
		[[nodiscard]] Location parse( std::string_view locationStr ) const;

		/**
		 * @brief Tries to parse a location string.
		 * @param value The location string to parse.
		 * @param location Output parameter: if parsing succeeds, this is set to the parsed `Location`.
		 *                 The state of `location` is undefined if parsing fails.
		 * @return True if parsing succeeded, false otherwise.
		 */
		bool tryParse( const std::string& value, Location& location ) const;

		/**
		 * @brief Tries to parse a location string.
		 * @param value An optional string containing the location to parse. If nullopt or empty, parsing fails.
		 * @param location Output parameter: if parsing succeeds, this is set to the parsed `Location`.
		 *                 The state of `location` is undefined if parsing fails.
		 * @return True if parsing succeeded, false otherwise.
		 */
		bool tryParse( const std::optional<std::string>& value, Location& location ) const;

		/**
		 * @brief Tries to parse a location string, providing detailed error information.
		 * @param value An optional string containing the location to parse.
		 * @param location Output parameter: if parsing succeeds, this is set to the parsed `Location`.
		 * @param errors Output parameter: populated with any parsing errors encountered.
		 * @return True if parsing succeeded (even if `errors` object indicates warnings or non-critical issues,
		 *         as long as a valid `Location` could be formed), false if a fundamental parsing error occurred.
		 */
		bool tryParse( const std::optional<std::string>& value, Location& location, ParsingErrors& errors ) const;

		/**
		 * @brief Tries to parse a location string (represented by a string_view).
		 * @param value The location string_view to parse.
		 * @param location Output parameter: if parsing succeeds, this is set to the parsed `Location`.
		 * @return True if parsing succeeded, false otherwise.
		 */
		bool tryParse( std::string_view value, Location& location ) const;

		/**
		 * @brief Tries to parse a location string (represented by a string_view), providing detailed error information.
		 * @param value The location string_view to parse.
		 * @param location Output parameter: if parsing succeeds, this is set to the parsed `Location`.
		 * @param errors Output parameter: populated with any parsing errors encountered.
		 * @return True if parsing succeeded, false otherwise.
		 */
		bool tryParse( std::string_view value, Location& location, ParsingErrors& errors ) const;

	public:
		//----------------------------------------------
		// Public static helper methods
		//----------------------------------------------

		/**
		 * @brief Tries to parse an integer from a segment of a string_view.
		 * @param span The string_view containing the integer.
		 * @param start The starting index of the integer substring within `span`.
		 * @param length The length of the integer substring.
		 * @param number Output parameter: if parsing succeeds, this is set to the parsed integer.
		 * @return True if parsing succeeded, false otherwise.
		 */
		static bool tryParseInt( std::string_view span, int start, int length, int& number );

	private:
		//----------------------------------------------
		// Private Methods
		//----------------------------------------------

		/**
		 * @brief Internal core method to parse a location string.
		 * @param span The string_view representing the current segment of the location string to parse.
		 * @param originalStr The original, full location string (optional, used for context in errors).
		 * @param location Output parameter: if parsing succeeds, this is set to the parsed `Location`.
		 * @param errorBuilder The `LocationParsingErrorBuilder` to accumulate errors.
		 * @return True if parsing was successful to the point of forming a valid `Location`, false otherwise.
		 */
		bool tryParseInternal( std::string_view span,
			const std::optional<std::string>& originalStr,
			Location& location,
			LocationParsingErrorBuilder& errorBuilder ) const;

		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief A sorted list of all valid single character location codes for quick lookup. */
		std::vector<char> m_locationCodes;

		/** @brief A list of all defined `RelativeLocation` objects for this VIS version. */
		std::vector<RelativeLocation> m_relativeLocations;

		/** @brief A map from character codes to their `LocationGroup` for quick classification. */
		std::map<char, LocationGroup> m_reversedGroups;

		/** @brief The VIS version this `Locations` instance is configured for. */
		VisVersion m_visVersion;

		/** @brief A map grouping `RelativeLocation` objects by their `LocationGroup`. */
		std::unordered_map<LocationGroup, std::vector<RelativeLocation>> m_groups;
	};
}

#include "Locations.inl"
