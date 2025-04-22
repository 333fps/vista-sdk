#pragma once

namespace dnv::vista::sdk
{
	/**
	 * @brief Data Transfer Object (DTO) for a relative location.
	 *
	 * Represents a relative location with a code, name, and optional definition.
	 */
	struct RelativeLocationsDto final
	{
		//-------------------------------------------------------------------------
		// Constructors
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

		//-------------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize a RelativeLocationsDto from a RapidJSON object.
		 * @param json The RapidJSON object to deserialize.
		 * @return The deserialized RelativeLocationsDto.
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

		//-------------------------------------------------------------------------
		// Data Members
		//-------------------------------------------------------------------------

		/** @brief The character code representing the location (JSON: "code"). */
		char code;

		/** @brief The name of the location (JSON: "name"). */
		std::string name;

		/** @brief An optional definition of the location (JSON: "definition"). */
		std::optional<std::string> definition;
	};

	/**
	 * @brief Data Transfer Object (DTO) for a collection of locations.
	 *
	 * Represents a collection of relative locations and the VIS version they belong to.
	 */
	struct LocationsDto final
	{
		//-------------------------------------------------------------------------
		// Constructors
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

		//-------------------------------------------------------------------------
		// Serialization Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Deserialize a LocationsDto from a RapidJSON object.
		 * @param json The RapidJSON object to deserialize.
		 * @return The deserialized LocationsDto.
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

		//-------------------------------------------------------------------------
		// Data Members
		//-------------------------------------------------------------------------

		/** @brief The VIS version string (JSON: "visRelease"). */
		std::string visVersion;

		/** @brief A vector of relative locations (JSON: "items"). */
		std::vector<RelativeLocationsDto> items;
	};
}
