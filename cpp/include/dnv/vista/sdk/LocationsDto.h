/**
 * @file LocationsDto.h
 * @brief Data Transfer Objects for locations in the VIS standard
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// Relative Location Data Transfer Objects
	//=====================================================================

	/**
	 * @brief Data Transfer Object (DTO) for a relative location.
	 *
	 * Represents a relative location with a code, name, and optional definition.
	 * This class is immutable - all properties are set during construction.
	 */
	class RelativeLocationsDto final
	{
	public:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructor with parameters
		 *
		 * @param code The character code representing the location
		 * @param name The name of the location
		 * @param definition An optional definition of the location
		 */
		explicit RelativeLocationsDto( char code, std::string name, std::optional<std::string> definition = std::nullopt );

		/** @brief Default constructor. */
		RelativeLocationsDto() = default;

		/** @brief Copy constructor */
		RelativeLocationsDto( const RelativeLocationsDto& ) = default;

		/** @brief Move constructor */
		RelativeLocationsDto( RelativeLocationsDto&& ) noexcept = default;

		/** @brief Destructor */
		~RelativeLocationsDto() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		RelativeLocationsDto& operator=( const RelativeLocationsDto& ) = delete;

		/** @brief Move assignment operator */
		RelativeLocationsDto& operator=( RelativeLocationsDto&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the location code
		 * @return The character code representing the location
		 */
		[[nodiscard]] char code() const;

		/**
		 * @brief Get the location name
		 * @return The name of the location
		 */
		[[nodiscard]] const std::string& name() const;

		/**
		 * @brief Get the location definition
		 * @return The optional definition of the location
		 */
		[[nodiscard]] const std::optional<std::string>& definition() const;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a RelativeLocationsDto from an nlohmann::json object.
		 *
		 * @param json The nlohmann::json object to deserialize.
		 * @return An optional containing the deserialized DTO if successful, or std::nullopt if parsing failed
		 */
		static std::optional<RelativeLocationsDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a RelativeLocationsDto from an nlohmann::json object.
		 *
		 * @param json The nlohmann::json object to deserialize.
		 * @return The deserialized RelativeLocationsDto.
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static RelativeLocationsDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this RelativeLocationsDto to an nlohmann::json object
		 *
		 * @return The serialized nlohmann::json object
		 */
		nlohmann::json toJson() const;

	private:
		//----------------------------------------------
		// Private Serialization Methods
		//---------------------------------------------

		friend void from_json( const nlohmann::json& j, RelativeLocationsDto& dto );
		friend void to_json( nlohmann::json& j, const RelativeLocationsDto& dto );

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief The character code representing the location (JSON: "code"). */
		char m_code;

		/** @brief The name of the location (JSON: "name"). */
		std::string m_name;

		/** @brief An optional definition of the location (JSON: "definition"). */
		std::optional<std::string> m_definition;
	};

	//=====================================================================
	// Location Data Transfer Objects
	//=====================================================================

	/**
	 * @brief Data Transfer Object (DTO) for a collection of locations.
	 *
	 * Represents a collection of relative locations and the VIS version they belong to.
	 * This class is immutable - all properties are set during construction.
	 */
	class LocationsDto final
	{
	public:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructor with parameters
		 *
		 * @param visVersion The VIS version string
		 * @param items A collection of relative locations
		 */
		explicit LocationsDto( std::string visVersion, std::vector<RelativeLocationsDto> items );

		/** @brief Default constructor. */
		LocationsDto() = default;

		/** @brief Copy constructor */
		LocationsDto( const LocationsDto& ) = default;

		/** @brief Move constructor */
		LocationsDto( LocationsDto&& ) noexcept = default;

		/** @brief Destructor */
		~LocationsDto() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		LocationsDto& operator=( const LocationsDto& ) = delete;

		/** @brief Move assignment operator */
		LocationsDto& operator=( LocationsDto&& ) noexcept = delete;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the VIS version string
		 * @return The VIS version string
		 */
		[[nodiscard]] const std::string& visVersion() const;

		/**
		 * @brief Get the collection of relative locations
		 * @return A vector of relative locations
		 */
		[[nodiscard]] const std::vector<RelativeLocationsDto>& items() const;

		//----------------------------------------------
		// Serialization
		//----------------------------------------------

		/**
		 * @brief Try to deserialize a LocationsDto from an nlohmann::json object.
		 *
		 * @param json The nlohmann::json object to deserialize.
		 * @return An optional containing the deserialized DTO if successful, or std::nullopt if parsing failed
		 */
		static std::optional<LocationsDto> tryFromJson( const nlohmann::json& json );

		/**
		 * @brief Deserialize a LocationsDto from an nlohmann::json object.
		 *
		 * @param json The nlohmann::json object to deserialize.
		 * @return The deserialized LocationsDto.
		 * @throws std::invalid_argument If deserialization fails (e.g., missing fields, type errors)
		 * @throws nlohmann::json::exception If JSON parsing/access errors occur
		 */
		static LocationsDto fromJson( const nlohmann::json& json );

		/**
		 * @brief Serialize this LocationsDto to an nlohmann::json object
		 * @return The serialized nlohmann::json object
		 */
		nlohmann::json toJson() const;

	private:
		//----------------------------------------------
		// Private Serialization Methods
		//----------------------------------------------

		friend void from_json( const nlohmann::json& j, LocationsDto& dto );
		friend void to_json( nlohmann::json& j, const LocationsDto& dto );

	private:
		//----------------------------------------------
		// Private Member Variables
		//--------------------------------------------

		/** @brief The VIS version string (JSON: "visRelease"). */
		std::string m_visVersion;

		/** @brief A vector of relative locations (JSON: "items"). */
		std::vector<RelativeLocationsDto> m_items;
	};
}
