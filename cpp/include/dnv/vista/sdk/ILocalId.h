/**
 * @file ILocalId.h
 * @brief Interface for Local ID objects in the VIS system.
 * @details Defines the core interface for Local ID objects which represent unique
 *          identifiers within the Vessel Information Structure (VIS) system according
 *          to ISO 19848 standards, using the Curiously Recurring Template Pattern (CRTP).
 */

#pragma once

#include "ParsingErrors.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class GmodPath;
	class MetadataTag;
	enum class VisVersion;

	/**
	 * @interface ILocalId
	 * @brief Base interface for Local IDs using the CRTP pattern.
	 *
	 * @details Provides the core properties and methods required for all Local ID implementations
	 * within the VIS (Vessel Information Structure) system. Derived classes must
	 * provide implementations for the pure virtual functions and static methods.
	 *
	 * This interface uses the Curiously Recurring Template Pattern (CRTP) where `T` is the
	 * concrete derived class type. This allows static methods like `parse` and `tryParse`
	 * to be defined within the interface scope while operating on and returning the
	 * concrete type `T`.
	 *
	 * @tparam T The concrete derived class type (e.g., `LocalId`).
	 */
	template <typename T>
	class ILocalId
	{
		//=====================================================================
		// Construction / Destruction
		//=====================================================================

	protected:
		/** @brief Default constructor. */
		ILocalId() = default;

	public:
		/** @brief Copy constructor */
		ILocalId( const ILocalId& ) = delete;

		/** @brief Move constructor */
		ILocalId( ILocalId&& ) noexcept = default;

		/** @brief Destructor */
		virtual ~ILocalId() = default;

		//=====================================================================
		// Assignment Operators
		//=====================================================================

		/** @brief Copy assignment operator */
		ILocalId& operator=( const ILocalId& ) = delete;

		/** @brief Move assignment operator */
		ILocalId& operator=( ILocalId&& ) noexcept = default;

		//=====================================================================
		// Operators
		//=====================================================================

		/**
		 * @brief Equality comparison operator.
		 * @details Delegates to the virtual `equals` method for state comparison.
		 * @param[in] other The Local ID object (of concrete type T) to compare against.
		 * @return True if the Local IDs represent the same state, false otherwise.
		 * @see equals
		 */
		[[nodiscard]] bool operator==( const T& other ) const;

		/**
		 * @brief Inequality comparison operator.
		 * @details Returns the negation of the `equals` method result.
		 * @param[in] other The Local ID object (of concrete type T) to compare against.
		 * @return True if the Local IDs represent different states, false otherwise.
		 * @see equals
		 */
		[[nodiscard]] bool operator!=( const T& other ) const;

		//=====================================================================
		// Core Properties
		//=====================================================================

		/**
		 * @brief Gets the VIS version associated with this Local ID.
		 * @details The VIS version indicates the specific standard version the Local ID conforms to.
		 * @return The `VisVersion` enum value.
		 */
		[[nodiscard]] virtual VisVersion visVersion() const = 0;

		/**
		 * @brief Checks if the Local ID was parsed or generated in verbose mode.
		 * @details Verbose mode typically affects the `toString()` representation.
		 * @return True if verbose mode is indicated, false otherwise.
		 */
		[[nodiscard]] virtual bool isVerboseMode() const noexcept = 0;

		/**
		 * @brief Gets the primary GMOD path item of the Local ID.
		 * @details The primary item is mandatory for a valid Local ID. For a valid `ILocalId` instance,
		 *          this optional is expected to contain a value.
		 * @return A constant reference to an `std::optional<GmodPath>` which holds the primary item.
		 * @throws std::runtime_error (or derived) if the concrete implementation attempts to dereference
		 *         an empty optional when it should be populated (implementation-defined behavior,
		 *         though this should typically be guaranteed by valid construction).
		 */
		[[nodiscard]] virtual const std::optional<GmodPath>& primaryItem() const = 0;

		/**
		 * @brief Gets the optional secondary GMOD path item.
		 * @details The secondary item provides additional context or relationship information.
		 * @return A constant reference to an `std::optional<GmodPath>` containing the secondary
		 *         item path if present, or `std::nullopt` otherwise.
		 */
		[[nodiscard]] virtual const std::optional<GmodPath>& secondaryItem() const = 0;

		/**
		 * @brief Checks if the Local ID includes any custom (non-standard) metadata tags.
		 * @details Custom tags are typically prefixed with '~' in the string representation.
		 * @return True if at least one custom tag exists, false otherwise.
		 */
		[[nodiscard]] virtual bool hasCustomTag() const noexcept = 0;

		/**
		 * @brief Gets all metadata tags associated with the Local ID.
		 * @details Returns a collection of the `MetadataTag` objects. The order typically
		 *          corresponds to the standard Local ID format (Quantity, Content, etc.).
		 * @return A vector containing copies of the `MetadataTag` objects.
		 */
		[[nodiscard]] virtual std::vector<MetadataTag> metadataTags() const = 0;

		//=====================================================================
		// Conversion and Comparison
		//=====================================================================

		/**
		 * @brief Converts the Local ID to its canonical string representation.
		 * @details The format follows the VIS standard (ISO 19848). The output may be
		 *          affected by the `isVerboseMode()` setting.
		 * @return The `std::string` representation of the Local ID.
		 */
		[[nodiscard]] virtual std::string toString() const = 0;

		/**
		 * @brief Performs a deep equality comparison with another Local ID.
		 * @details Compares all components (VIS version, items, tags, modes) for equality.
		 * @param[in] other The Local ID object (of concrete type T) to compare against.
		 * @return True if all components of the Local IDs are equal, false otherwise.
		 * @throws Can throw exceptions, e.g., `std::invalid_argument` if comparing objects
		 *         with different VIS versions (implementation-defined behavior).
		 */
		[[nodiscard]] virtual bool equals( const T& other ) const = 0;

		//=====================================================================
		// Static Parsing Methods
		//=====================================================================

		/**
		 * @brief Parses a string representation into a concrete Local ID object.
		 * @details Creates an instance of the concrete type `T` from the `localIdStr`.
		 *          Delegates parsing logic to the concrete `T` implementation.
		 * @param[in] localIdStr The Local ID string to parse (e.g., "/dnv-v2/vis-3.0/1000/10/meta/qty-mass").
		 * @return An instance of the concrete Local ID type `T`.
		 * @throws std::invalid_argument or derived exception if parsing fails due to invalid format or content.
		 */
		[[nodiscard]] static T parse( std::string_view localIdStr );

		/**
		 * @brief Attempts to parse a string representation into a concrete Local ID object.
		 * @details Tries to create an instance of `T` from `localIdStr`. If successful, the result
		 *          is placed in the `localId` output parameter. If parsing fails, error information
		 *          is added to the `errors` output parameter. Does not throw on parsing failure.
		 *          Delegates parsing logic to the concrete `T` implementation.
		 * @param[in] localIdStr The Local ID string to parse.
		 * @param[out] errors A `ParsingErrors` object to collect detailed error information if parsing fails.
		 * @param[out] localId An `std::optional<T>` that will contain the resulting Local ID
		 *                     if parsing succeeds, or `std::nullopt` otherwise.
		 * @return True if parsing was successful, false otherwise.
		 */
		[[nodiscard]] static bool tryParse( std::string_view localIdStr, ParsingErrors& errors, std::optional<T>& localId );
	};
}

#include "ILocalId.inl"
