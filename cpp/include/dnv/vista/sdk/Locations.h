#pragma once

#include "LocationsDto.h"

namespace dnv::vista::sdk
{
	enum class VisVersion;
	class ParsingErrors;
	struct LocationParsingErrorBuilder;

	/**
	 * @brief Enumeration of location groups
	 *
	 * Represents different groups of locations used in the VIS system.
	 */
	enum class LocationGroup
	{
		Number,		 ///< Represents a numerical location
		Side,		 ///< Represents a side location
		Vertical,	 ///< Represents a vertical location
		Transverse,	 ///< Represents a transverse location
		Longitudinal ///< Represents a longitudinal location
	};

	/**
	 * @brief Enumeration of location validation results
	 */
	enum class LocationValidationResult
	{
		Invalid,		  ///< The location is invalid
		InvalidCode,	  ///< The location code is invalid
		InvalidOrder,	  ///< The location order is invalid
		NullOrWhiteSpace, ///< The location is null or whitespace
		Valid			  ///< The location is valid
	};

	/**
	 * @brief Represents a location in the VIS system
	 */
	class Location final
	{
	public:
		/**
		 * @brief Default constructor
		 */
		Location() = default;

		/**
		 * @brief Constructor with a location value
		 * @param value The location value as a string
		 */
		explicit Location( const std::string& value );

		//-------------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the location value
		 * @return The location value as a string
		 */
		const std::string& value() const;

		//-------------------------------------------------------------------------
		// Conversion Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Convert the location to a string
		 * @return The string representation of the location
		 */
		std::string toString() const;

		/**
		 * @brief Implicit conversion to string
		 * @return The location value as a string
		 */
		operator std::string() const;

		//-------------------------------------------------------------------------
		// Comparison Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Equality operator
		 * @param other The other location to compare
		 * @return True if equal, false otherwise
		 */
		bool operator==( const Location& other ) const;

		/**
		 * @brief Inequality operator
		 * @param other The other location to compare
		 * @return True if not equal, false otherwise
		 */
		bool operator!=( const Location& other ) const;

	private:
		std::string m_value; ///< The location value
	};

	/**
	 * @brief Represents a relative location in the VIS system
	 */
	class RelativeLocation final
	{
	public:
		/**
		 * @brief Constructor with location details
		 * @param code The location code
		 * @param name The location name
		 * @param location The location object
		 * @param definition Optional definition of the location
		 */
		RelativeLocation( char code,
			const std::string& name,
			const Location& location,
			const std::optional<std::string> definition = std::nullopt );

		//-------------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the location code
		 * @return The location code
		 */
		char code() const;

		/**
		 * @brief Get the location name
		 * @return The location name
		 */
		const std::string& name() const;

		/**
		 * @brief Get the location definition
		 * @return The optional location definition
		 */
		std::optional<std::string> definition() const;

		/**
		 * @brief Get the location object
		 * @return The location object
		 */
		const Location& location() const;

		/**
		 * @brief Get the hash code of the location
		 * @return The hash code
		 */
		size_t hashCode() const;

		//-------------------------------------------------------------------------
		// Comparison Operators
		//-------------------------------------------------------------------------

		/**
		 * @brief Equality operator
		 * @param other The other relative location to compare
		 * @return True if equal, false otherwise
		 */
		bool operator==( const RelativeLocation& other ) const;

		/**
		 * @brief Inequality operator
		 * @param other The other relative location to compare
		 * @return True if not equal, false otherwise
		 */
		bool operator!=( const RelativeLocation& other ) const;

	private:
		char m_code;							 ///< The location code
		std::string m_name;						 ///< The location name
		Location m_location;					 ///< The location object
		std::optional<std::string> m_definition; ///< The optional location definition
	};

	/**
	 * @brief Dictionary for managing location characters
	 */
	class LocationCharDict final
	{
	public:
		/**
		 * @brief Constructor with a specified size
		 * @param size The size of the dictionary
		 */
		explicit LocationCharDict();

		//-------------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Access a location character by key
		 * @param key The location group key
		 * @return A reference to the optional character
		 */
		std::optional<char>& operator[]( LocationGroup key );

		/**
		 * @brief Try to add a value to the dictionary
		 * @param key The location group key
		 * @param value The character value to add
		 * @param existingValue Output parameter for the existing value, if any
		 * @return True if the value was added, false otherwise
		 */
		bool tryAdd( LocationGroup key, char value, std::optional<char>& existingValue );

	private:
		std::array<std::optional<char>, 4> m_table; ///< The dictionary table for 4 groups
	};

	/**
	 * @brief Represents a collection of locations in the VIS system
	 */
	class Locations final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		Locations() = default;

		/**
		 * @brief Constructor with VIS version and DTO
		 * @param version The VIS version
		 * @param dto The locations data transfer object
		 */
		explicit Locations( VisVersion version, const LocationsDto& dto );

		//-------------------------------------------------------------------------
		// Accessor Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the VIS version
		 * @return The VIS version
		 */
		VisVersion visVersion() const;

		/**
		 * @brief Get the relative locations
		 * @return A vector of relative locations
		 */
		const std::vector<RelativeLocation>& relativeLocations() const;

		/**
		 * @brief Get the location groups
		 * @return A map of location groups to relative locations
		 */
		const std::unordered_map<LocationGroup, std::vector<RelativeLocation>>& groups() const;

		//-------------------------------------------------------------------------
		// Parsing Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Parse a location string
		 * @param locationStr The location string to parse
		 * @return The parsed location
		 * @throws std::invalid_argument If parsing fails
		 */
		Location parse( const std::string& locationStr );

		/**
		 * @brief Parse a location string view
		 * @param locationStr The location string view to parse
		 * @return The parsed location
		 * @throws std::invalid_argument If parsing fails
		 */
		Location parse( std::string_view locationStr );

		/**
		 * @brief Try to parse a location string
		 * @param value The location string to parse
		 * @param location Output parameter for the parsed location
		 * @return True if parsing succeeded, false otherwise
		 */
		bool tryParse( const std::optional<std::string>& value, Location& location ) const;

		/**
		 * @brief Try to parse a location string with errors
		 * @param value The location string to parse
		 * @param location Output parameter for the parsed location
		 * @param errors Output parameter for parsing errors
		 * @return True if parsing succeeded, false otherwise
		 */
		bool tryParse( const std::optional<std::string>& value, Location& location, ParsingErrors& errors );

		/**
		 * @brief Try to parse a location string view
		 * @param value The location string view to parse
		 * @param location Output parameter for the parsed location
		 * @return True if parsing succeeded, false otherwise
		 */
		bool tryParse( std::string_view value, Location& location );

		/**
		 * @brief Try to parse a location string view with errors
		 * @param value The location string view to parse
		 * @param location Output parameter for the parsed location
		 * @param errors Output parameter for parsing errors
		 * @return True if parsing succeeded, false otherwise
		 */
		bool tryParse( std::string_view value, Location& location, ParsingErrors& errors );

	private:
		//-------------------------------------------------------------------------
		// Private Helper Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Add an error to the error builder
		 * @param errorBuilder The error builder
		 * @param name The validation result
		 * @param message The error message
		 */
		static void addError( LocationParsingErrorBuilder& errorBuilder,
			LocationValidationResult name,
			const std::string& message );

		/**
		 * @brief Internal method to parse a location string
		 * @param span The location string view
		 * @param originalStr The original string
		 * @param location Output parameter for the parsed location
		 * @param errorBuilder The error builder
		 * @return True if parsing succeeded, false otherwise
		 */
		bool tryParseInternal( std::string_view span,
			const std::optional<std::string>& originalStr,
			Location& location,
			LocationParsingErrorBuilder& errorBuilder ) const;

		/**
		 * @brief Try to parse an integer from a string view
		 * @param span The string view
		 * @param start The start position
		 * @param length The length of the substring
		 * @param number Output parameter for the parsed integer
		 * @return True if parsing succeeded, false otherwise
		 */
		static bool tryParseInt( std::string_view span, int start, int length, int& number );

		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		std::vector<char> m_locationCodes;										   ///< The location codes
		std::vector<RelativeLocation> m_relativeLocations;						   ///< The relative locations
		std::unordered_map<char, LocationGroup> m_reversedGroups;				   ///< The reversed groups
		VisVersion m_visVersion;												   ///< The VIS version
		std::unordered_map<LocationGroup, std::vector<RelativeLocation>> m_groups; ///< The location groups
	};
}
