/**
 * @file IUniversalIdBuilder.h
 * @brief Defines the abstract interface contract for Universal ID builders.
 * @details This file specifies the `IUniversalIdBuilder` class, which acts as an
 *          interface defining the methods required for building Universal IDs.
 */

#pragma once

#include "ImoNumber.h"
#include "LocalIdBuilder.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class UniversalIdBuilder;

	//=====================================================================
	// IUniversalIdBuilder Class
	//=====================================================================

	/**
	 * @class IUniversalIdBuilder
	 * @brief Abstract base class defining the contract for Universal ID builders.
	 *
	 * @details This class serves as an interface, outlining the properties and methods
	 *          required for an immutable fluent builder pattern for Universal IDs.
	 *          It is designed to be implemented by concrete builder classes like
	 *          `UniversalIdBuilder`.
	 */
	class IUniversalIdBuilder
	{
	public:
		//=====================================================================
		// Construction / Destruction
		//=====================================================================

	protected:
		/** @brief Default constructor. */
		IUniversalIdBuilder() = default;

	public:
		/** @brief Copy constructor */
		IUniversalIdBuilder( const IUniversalIdBuilder& ) = delete;

		/** @brief Move constructor */
		IUniversalIdBuilder( IUniversalIdBuilder&& ) noexcept = default;

		/** @brief Destructor */
		virtual ~IUniversalIdBuilder() = default;

		//=====================================================================
		// Assignment Operators
		//=====================================================================

		/** @brief Copy assignment operator */
		IUniversalIdBuilder& operator=( const IUniversalIdBuilder& ) = delete;

		/** @brief Move assignment operator */
		IUniversalIdBuilder& operator=( IUniversalIdBuilder&& ) noexcept = default;

		//=====================================================================
		// Accessors
		//=====================================================================

		/**
		 * @brief Gets the IMO number currently set in the builder, if any.
		 * @return A constant reference to an `std::optional<ImoNumber>` containing the IMO number if set,
		 *         or `std::nullopt` if no IMO number is set.
		 */
		[[nodiscard]] virtual const std::optional<ImoNumber>& imoNumber() const = 0;

		/**
		 * @brief Gets the Local ID builder currently associated with this builder, if any.
		 * @return A constant reference to an `std::optional<LocalIdBuilder>` containing the Local ID builder if set,
		 *         or `std::nullopt` otherwise.
		 */
		[[nodiscard]] virtual const std::optional<LocalIdBuilder>& localId() const = 0;

		//=====================================================================
		// Builder Methods (Immutable Fluent Interface)
		//=====================================================================

		//-------------------------------------------------------------------------
		// Local Id
		//-------------------------------------------------------------------------

		/**
		 * @brief Returns a new builder with the Local ID builder set.
		 * @details Replaces the existing Local ID builder configuration with the provided one.
		 * @param[in] localId The `LocalIdBuilder` instance to set. Passed by const reference.
		 * @return A new `UniversalIdBuilder` instance with the updated Local ID builder.
		 */
		[[nodiscard]] virtual UniversalIdBuilder withLocalId( const LocalIdBuilder& localId ) const = 0;

		/**
		 * @brief Returns a new builder, potentially with the Local ID builder set from an optional.
		 * @details If `localId` contains a value, the returned builder has that Local ID builder set.
		 *          Otherwise, the behavior might be implementation-defined (e.g., no change or clear).
		 * @param[in] localId An `std::optional<LocalIdBuilder>` containing the builder to set, if present. Passed by const reference.
		 * @return A new `UniversalIdBuilder` instance, potentially updated.
		 */
		[[nodiscard]] virtual UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId ) const = 0;

		/**
		 * @brief Returns a new builder, potentially with the Local ID builder set from an optional. Reports success.
		 * @details If `localId` contains a value, attempts to set it. Reports success via `succeeded`.
		 * @param[in] localId An `std::optional<LocalIdBuilder>` containing the builder to set, if present. Passed by const reference.
		 * @param[out] succeeded Set to true if the Local ID builder was present and successfully set, false otherwise. Passed by reference.
		 * @return A new `UniversalIdBuilder` instance, updated if successful, otherwise potentially identical to the current one.
		 */
		[[nodiscard]] virtual UniversalIdBuilder tryWithLocalId( const std::optional<LocalIdBuilder>& localId, bool& succeeded ) const = 0;

		/**
		 * @brief Returns a new builder with the Local ID builder configuration removed or reset.
		 * @return A new `UniversalIdBuilder` instance without a specific Local ID builder set (or with a default/empty one).
		 */
		[[nodiscard]] virtual UniversalIdBuilder withoutLocalId() const = 0;

		//-------------------------------------------------------------------------
		// IMO Number
		//-------------------------------------------------------------------------

		/**
		 * @brief Returns a new builder with the IMO number set.
		 * @param[in] imoNumber The `ImoNumber` to set. Passed by const reference.
		 * @return A new `UniversalIdBuilder` instance with the updated IMO number.
		 */
		[[nodiscard]] virtual UniversalIdBuilder withImoNumber( const ImoNumber& imoNumber ) const = 0;

		/**
		 * @brief Returns a new builder, potentially with the IMO number set from an optional.
		 * @details If `imoNumber` contains a value, the returned builder has that IMO number set.
		 *          Otherwise, the behavior might be implementation-defined.
		 * @param[in] imoNumber An `std::optional<ImoNumber>` containing the IMO number to set, if present. Passed by const reference.
		 * @return A new `UniversalIdBuilder` instance, potentially updated.
		 */
		[[nodiscard]] virtual UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber ) const = 0;

		/**
		 * @brief Returns a new builder, potentially with the IMO number set from an optional. Reports success.
		 * @details If `imoNumber` contains a value, attempts to set it. Reports success via `succeeded`.
		 * @param[in] imoNumber An `std::optional<ImoNumber>` containing the IMO number to set, if present. Passed by const reference.
		 * @param[out] succeeded Set to true if the IMO number was present and successfully set, false otherwise. Passed by reference.
		 * @return A new `UniversalIdBuilder` instance, updated if successful, otherwise potentially identical to the current one.
		 */
		[[nodiscard]] virtual UniversalIdBuilder tryWithImoNumber( const std::optional<ImoNumber>& imoNumber, bool& succeeded ) const = 0;

		/**
		 * @brief Returns a new builder with the IMO number removed.
		 * @return A new `UniversalIdBuilder` instance without an IMO number set.
		 */
		[[nodiscard]] virtual UniversalIdBuilder withoutImoNumber() const = 0;

		//=====================================================================
		// State Inspection Methods
		//=====================================================================

		/**
		 * @brief Checks if the builder state is valid to build a Local ID.
		 * @details Validity typically requires at least a VIS version, a primary item,
		 *          and one or more metadata tags, depending on the specific `TResult` rules.
		 * @return True if the current state allows for a successful `build()`, false otherwise.
		 */
		[[nodiscard]] virtual bool isValid() const = 0;

		//=====================================================================
		// Conversion and Comparison
		//=====================================================================

		/**
		 * @brief Generates the string representation of the Universal ID based on the current builder state.
		 * @details The format typically combines the IMO number and the Local ID string
		 *          (e.g., "IMO1234567/dnv-v2/vis-3-4a/..."). Returns an empty or specific
		 *          "invalid" string if the state is not valid (`isValid()` is false).
		 * @return The `std::string` representation of the configured Universal ID.
		 */
		[[nodiscard]] virtual std::string toString() const = 0;
	};
}
