/**
 * @file IUniversalId.h
 * @brief Defines the interface for Universal ID objects.
 * @details A Universal ID combines an IMO number with a Local ID to create a globally
 *          unique identifier within the VIS (Vessel Information Structure) ecosystem.
 *          This file specifies the abstract base class `IUniversalId`.
 */

#pragma once

#include "ImoNumber.h"
#include "LocalId.h"

namespace dnv::vista::sdk
{
	/**
	 * @interface IUniversalId
	 * @brief Abstract base class representing a Universal Identifier.
	 *
	 * @details A Universal ID uniquely identifies a data point or component across different
	 * vessels by combining a vessel's unique IMO number with a vessel-specific Local ID.
	 * This interface defines the common contract for all Universal ID implementations.
	 */
	class IUniversalId
	{
	public:
		//=====================================================================
		// Construction / Destruction
		//=====================================================================

	protected:
		/** @brief Default constructor. */
		IUniversalId() = default;

	public:
		/** @brief Copy constructor */
		IUniversalId( const IUniversalId& ) = delete;

		/** @brief Move constructor */
		IUniversalId( IUniversalId&& ) noexcept = default;

		/** @brief Destructor */
		virtual ~IUniversalId() = default;

		//=====================================================================
		// Assignment Operators
		//=====================================================================

		/** @brief Copy assignment operator */
		IUniversalId& operator=( const IUniversalId& ) = delete;

		/** @brief Move assignment operator */
		IUniversalId& operator=( IUniversalId&& ) noexcept = default;

		//=====================================================================
		// Core Properties
		//=====================================================================

		/**
		 * @brief Gets the IMO number component of the Universal ID.
		 * @details The IMO number uniquely identifies the vessel.
		 * @return A constant reference to the `ImoNumber` object.
		 */
		[[nodiscard]] virtual const ImoNumber& imoNumber() const = 0;

		/**
		 * @brief Gets the Local ID component of the Universal ID.
		 * @details The Local ID identifies the specific data point or component within the vessel's context.
		 * @return A constant reference to the `LocalId` object.
		 */
		[[nodiscard]] virtual const LocalId& localId() const = 0;

		//=====================================================================
		// Conversion and Comparison
		//=====================================================================

		/**
		 * @brief Converts the Universal ID to its canonical string representation.
		 * @details The format typically combines the IMO number and the Local ID string
		 *          (e.g., "IMO1234567/dnv-v2/vis-3-4a/..."). The exact format is
		 *          implementation-defined but should be consistent and parsable.
		 * @return The `std::string` representation of the Universal ID.
		 */
		[[nodiscard]] virtual std::string toString() const = 0;
	};
}
