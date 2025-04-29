/**
 * @file LocationsDto.h
 * @brief Data Transfer Objects for locations in the VIS standard
 */

#pragma once

namespace dnv::vista::sdk
{
	/**
	 * @brief Data Transfer Object (DTO) for a relative location.
	 *
	 * Represents a relative location with a code, name, and optional definition.
	 * This class is immutable - all properties are set during construction.
	 */
	class RelativeLocationsDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor deleted to enforce immutability
		 */
		RelativeLocationsDto() = delete;

		/**
		 * @brief Constructor with parameters
		 *
		 * @param code The character code representing the location
		 * @param name The name of the location
		 * @param definition An optional definition of the location
		 */
		RelativeLocationsDto( char code, std::string name, std::optional<std::string> definition = std::nullopt );

		/**
		 * @brief Copy constructor
		 */
		RelativeLocationsDto( const RelativeLocationsDto& ) = default;

		/**
		 * @brief Move constructor
		 */
		RelativeLocationsDto( RelativeLocationsDto&& ) noexcept = default;

		/**
		 * @brief Destructor
		 */
		~RelativeLocationsDto() = default;

		/**
		 * @brief Copy assignment operator deleted to enforce immutability
		 */
		RelativeLocationsDto& operator=( const RelativeLocationsDto& ) = delete;

		/**
		 * @brief Move assignment operator deleted to enforce immutability
		 */
		RelativeLocationsDto& operator=( RelativeLocationsDto&& ) noexcept = delete;

		//-------------------------------------------------------------------------
		// Public Interface - Accessor Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the location code
		 * @return The character code representing the location
		 */
		char code() const;

		/**
		 * @brief Get the location name
		 * @return The name of the location
		 */
		const std::string& name() const;

		/**
		 * @brief Get the location definition
		 * @return The optional definition of the location
		 */
		const std::optional<std::string>& definition() const;

		//-------------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a RelativeLocationsDto from a RapidJSON object.
		 *
		 * @param json The RapidJSON object to deserialize.
		 * @return An optional containing the deserialized DTO if successful, or std::nullopt if parsing failed
		 */
		static std::optional<RelativeLocationsDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a RelativeLocationsDto from a RapidJSON object.
		 *
		 * @param json The RapidJSON object to deserialize.
		 * @return The deserialized RelativeLocationsDto.
		 * @throws std::invalid_argument If JSON format is invalid or required fields are missing
		 */
		static RelativeLocationsDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this RelativeLocationsDto to a RapidJSON Value
		 *
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		/** @brief The character code representing the location (JSON: "code"). */
		const char m_code;

		/** @brief The name of the location (JSON: "name"). */
		const std::string m_name;

		/** @brief An optional definition of the location (JSON: "definition"). */
		const std::optional<std::string> m_definition;
	};

	/**
	 * @brief Data Transfer Object (DTO) for a collection of locations.
	 *
	 * Represents a collection of relative locations and the VIS version they belong to.
	 * This class is immutable - all properties are set during construction.
	 */
	class LocationsDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor deleted to enforce immutability
		 */
		LocationsDto() = delete;

		/**
		 * @brief Constructor with parameters
		 *
		 * @param visVersion The VIS version string
		 * @param items A collection of relative locations
		 */
		LocationsDto( std::string visVersion, std::vector<RelativeLocationsDto> items );

		/**
		 * @brief Copy constructor
		 */
		LocationsDto( const LocationsDto& ) = default;

		/**
		 * @brief Move constructor
		 */
		LocationsDto( LocationsDto&& ) noexcept = default;

		/**
		 * @brief Destructor
		 */
		~LocationsDto() = default;

		/**
		 * @brief Copy assignment operator deleted to enforce immutability
		 */
		LocationsDto& operator=( const LocationsDto& ) = delete;

		/**
		 * @brief Move assignment operator deleted to enforce immutability
		 */
		LocationsDto& operator=( LocationsDto&& ) noexcept = delete;

		//-------------------------------------------------------------------------
		// Public Interface - Accessor Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the VIS version string
		 * @return The VIS version string
		 */
		const std::string& visVersion() const;

		/**
		 * @brief Get the collection of relative locations
		 * @return A vector of relative locations
		 */
		const std::vector<RelativeLocationsDto>& items() const;

		//-------------------------------------------------------------------------
		// Public Interface - Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Try to deserialize a LocationsDto from a RapidJSON object.
		 *
		 * @param json The RapidJSON object to deserialize.
		 * @return An optional containing the deserialized DTO if successful, or std::nullopt if parsing failed
		 */
		static std::optional<LocationsDto> tryFromJson( const rapidjson::Value& json );

		/**
		 * @brief Deserialize a LocationsDto from a RapidJSON object.
		 *
		 * @param json The RapidJSON object to deserialize.
		 * @return The deserialized LocationsDto.
		 * @throws std::invalid_argument If JSON format is invalid or required fields are missing
		 */
		static LocationsDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Serialize this LocationsDto to a RapidJSON Value
		 *
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		/** @brief The VIS version string (JSON: "visRelease"). */
		const std::string m_visVersion;

		/** @brief A vector of relative locations (JSON: "items"). */
		const std::vector<RelativeLocationsDto> m_items;
	};
}
