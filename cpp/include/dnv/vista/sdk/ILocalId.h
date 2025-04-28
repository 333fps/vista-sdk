/**
 * @file ILocalId.h
 * @brief Interface for Local ID objects in the VIS system
 *
 * Defines the core interface for Local ID objects which represent unique
 * identifiers within the Vessel Information Structure (VIS) system according
 * to ISO 19848 standards.
 */

#pragma once

#include "ParsingErrors.h"
#include "VisVersion.h"
#include "GmodPath.h"
#include "MetadataTag.h"

namespace dnv::vista::sdk
{
	/**
	 * @brief Interface for Local ID
	 *
	 * Provides the core properties and methods for working with Local IDs
	 * in the VIS (Vessel Information Structure) system.
	 *
	 * @tparam T The derived class type (CRTP pattern)
	 */
	template <typename T>
	class ILocalId
	{
	public:
		//-------------------------------------------------------------------------
		// Lifecycle
		//-------------------------------------------------------------------------

		/** @brief Default constructor */
		ILocalId() = default;

		/** @brief Virtual destructor */
		virtual ~ILocalId() = default;

		/** @brief Delete copy constructor - interfaces shouldn't be copied */
		ILocalId( const ILocalId& ) = delete;

		/** @brief Delete copy assignment - interfaces shouldn't be assigned */
		ILocalId& operator=( const ILocalId& ) = delete;

		/** @brief Default move constructor */
		ILocalId( ILocalId&& ) = default;

		/** @brief Default move assignment */
		ILocalId& operator=( ILocalId&& ) = default;

		//-------------------------------------------------------------------------
		// Core Properties
		//-------------------------------------------------------------------------

		/**
		 * @brief Get the VIS version
		 * @return The VIS version
		 */
		virtual VisVersion visVersion() const = 0;

		/**
		 * @brief Check if verbose mode is enabled
		 * @return True if verbose mode is enabled
		 */
		virtual bool isVerboseMode() const = 0;

		/**
		 * @brief Get the primary item
		 * @return Reference to the primary item
		 * @throws std::runtime_error if primary item is not set
		 */
		virtual const GmodPath& primaryItem() const = 0;

		/**
		 * @brief Get the secondary item
		 * @return The secondary item, if present
		 */
		virtual std::optional<GmodPath> secondaryItem() const = 0;

		//-------------------------------------------------------------------------
		// Metadata Access
		//-------------------------------------------------------------------------

		/**
		 * @brief Check if the Local ID has any custom tags
		 * @return True if custom tags exist
		 */
		virtual bool hasCustomTag() const = 0;

		/**
		 * @brief Get all metadata tags
		 * @return A constant reference to the vector of metadata tags
		 */
		virtual std::vector<MetadataTag> metadataTags() const = 0;

		//-------------------------------------------------------------------------
		// Conversion and Comparison
		//-------------------------------------------------------------------------

		/**
		 * @brief Convert the Local ID to a string representation
		 * @return The string representation of the Local ID
		 */
		virtual std::string toString() const = 0;

		/**
		 * @brief Check if this Local ID equals another Local ID
		 * @param other The Local ID to compare with
		 * @return True if equal, false otherwise
		 */
		virtual bool equals( const T& other ) const = 0;

		/**
		 * @brief Equality operator
		 * @param other The Local ID to compare with
		 * @return True if equal, false otherwise
		 */
		bool operator==( const T& other ) const noexcept;

		/**
		 * @brief Inequality operator
		 * @param other The Local ID to compare with
		 * @return True if not equal, false otherwise
		 */
		bool operator!=( const T& other ) const noexcept;

		//-------------------------------------------------------------------------
		// Static Parser Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Parse a string into a Local ID
		 * @param localIdStr The string to parse
		 * @return The parsed Local ID
		 * @throws std::invalid_argument If parsing fails
		 */
		static T parse( const std::string& localIdStr );

		/**
		 * @brief Try to parse a string into a Local ID
		 * @param localIdStr The string to parse
		 * @param errors Output parameter for parsing errors
		 * @param localId Output parameter for the parsed Local ID
		 * @return True if parsing succeeded, false otherwise
		 */
		static bool tryParse( const std::string& localIdStr, ParsingErrors& errors, std::optional<T>& localId );
	};
}

#include "ILocalId.hpp"
