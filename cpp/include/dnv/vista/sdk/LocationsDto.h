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
	 */
	class RelativeLocationsDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		RelativeLocationsDto() = default;

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
		 * @brief Copy assignment operator
		 */
		RelativeLocationsDto& operator=( const RelativeLocationsDto& ) = default;

		/**
		 * @brief Move assignment operator
		 */
		RelativeLocationsDto& operator=( RelativeLocationsDto&& ) noexcept = default;

		//-------------------------------------------------------------------------
		// Accessor Methods
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
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize a RelativeLocationsDto from a RapidJSON object.
		 * @param json The RapidJSON object to deserialize.
		 * @return The deserialized RelativeLocationsDto.
		 * @throws std::runtime_error If JSON format is invalid
		 */
		static RelativeLocationsDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize a RelativeLocationsDto from a RapidJSON object.
		 * @param json The RapidJSON object to deserialize.
		 * @param dto Output parameter to receive the deserialized object.
		 * @return True if deserialization was successful, false otherwise.
		 */
		static bool tryFromJson( const rapidjson::Value& json, RelativeLocationsDto& dto );

		/**
		 * @brief Serialize this RelativeLocationsDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		/** @brief The character code representing the location (JSON: "code"). */
		char m_code{};

		/** @brief The name of the location (JSON: "name"). */
		std::string m_name;

		/** @brief An optional definition of the location (JSON: "definition"). */
		std::optional<std::string> m_definition;
	};

	/**
	 * @brief Data Transfer Object (DTO) for a collection of locations.
	 *
	 * Represents a collection of relative locations and the VIS version they belong to.
	 */
	class LocationsDto final
	{
	public:
		//-------------------------------------------------------------------------
		// Constructors / Destructor
		//-------------------------------------------------------------------------

		/**
		 * @brief Default constructor
		 */
		LocationsDto() = default;

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
		 * @brief Copy assignment operator
		 */
		LocationsDto& operator=( const LocationsDto& ) = default;

		/**
		 * @brief Move assignment operator
		 */
		LocationsDto& operator=( LocationsDto&& ) noexcept = default;

		//-------------------------------------------------------------------------
		// Accessor Methods
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
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize a LocationsDto from a RapidJSON object.
		 * @param json The RapidJSON object to deserialize.
		 * @return The deserialized LocationsDto.
		 * @throws std::runtime_error If JSON format is invalid
		 */
		static LocationsDto fromJson( const rapidjson::Value& json );

		/**
		 * @brief Try to deserialize a LocationsDto from a RapidJSON object.
		 * @param json The RapidJSON object to deserialize.
		 * @param dto Output parameter to receive the deserialized object.
		 * @return True if deserialization was successful, false otherwise.
		 */
		static bool tryFromJson( const rapidjson::Value& json, LocationsDto& dto );

		/**
		 * @brief Serialize this LocationsDto to a RapidJSON Value
		 * @param allocator The JSON value allocator to use
		 * @return The serialized JSON value
		 */
		rapidjson::Value toJson( rapidjson::Document::AllocatorType& allocator ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------------

		/** @brief The VIS version string (JSON: "visRelease"). */
		std::string m_visVersion;

		/** @brief A vector of relative locations (JSON: "items"). */
		std::vector<RelativeLocationsDto> m_items;
	};
}
