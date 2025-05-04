/**
 * @file LocalId.h
 * @brief Concrete implementation of the Local ID interface.
 * @details This file contains the definition of the `LocalId` class,
 *          which provides a concrete, immutable implementation of the `ILocalId`
 *          interface based on the configuration provided by a `LocalIdBuilder`.
 */

#pragma once

#include "ILocalId.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	enum class VisVersion;
	class GmodPath;
	class LocalIdBuilder;
	class MetadataTag;
	class ParsingErrors;
}

namespace dnv::vista::sdk
{
	/**
	 * @class LocalId
	 * @brief Concrete, immutable representation of a Local ID.
	 *
	 * @details Implements the `ILocalId<LocalId>` interface. An instance of this class
	 * represents a complete and validated Local ID according to the VIS standard (ISO 19848).
	 * Instances are typically created via a `LocalIdBuilder` or the static `parse`/`tryParse` methods.
	 * The internal state is held by a shared pointer to a const `LocalIdBuilder`.
	 */
	class LocalId final : public ILocalId<LocalId>
	{
		//=====================================================================
		// Construction / Destruction
		//=====================================================================

	public:
		/**
		 * @brief Constructs a `LocalId` from a `LocalIdBuilder`.
		 * @details Takes ownership of the builder's state via a shared pointer.
		 *          The provided builder is moved into the `LocalId`.
		 * @param[in] builder A `LocalIdBuilder` instance configured with the desired Local ID state.
		 *                    The builder must be valid (`builder.isValid()` must be true).
		 * @throws std::invalid_argument if the provided `builder` is not valid.
		 */
		explicit LocalId( LocalIdBuilder builder );

		/** @brief Copy constructor */
		LocalId( const LocalId& ) = delete;

		/** @brief Move constructor */
		LocalId( LocalId&& other ) noexcept = default;

		/** @brief Destructor */
		virtual ~LocalId() = default;

		//=====================================================================
		// Assignment Operators
		//=====================================================================

		/** @brief Copy assignment operator */
		LocalId& operator=( const LocalId& ) = delete;

		/** @brief Move assignment operator */
		LocalId& operator=( LocalId&& other ) noexcept = default;

		//=====================================================================
		// Operators
		//=====================================================================

		/**
		 * @brief Equality comparison operator.
		 * @details Delegates to the `equals` method for state comparison.
		 * @param[in] other The `LocalId` instance to compare against.
		 * @return True if the Local IDs represent the same state, false otherwise.
		 * @throws Can throw exceptions if the underlying `equals` method throws (e.g., `std::invalid_argument` on VIS mismatch).
		 * @see equals
		 */
		[[nodiscard]] bool operator==( const LocalId& other ) const;

		/**
		 * @brief Inequality comparison operator.
		 * @details Returns the negation of the `equals` method result.
		 * @param[in] other The `LocalId` instance to compare against.
		 * @return True if the Local IDs represent different states, false otherwise.
		 * @throws Can throw exceptions if the underlying `equals` method throws (e.g., `std::invalid_argument` on VIS mismatch).
		 * @see equals
		 */
		[[nodiscard]] bool operator!=( const LocalId& other ) const;

		//=====================================================================
		// Hashing
		//=====================================================================

		/**
		 * @brief Calculate hash code based on Local ID content.
		 * @details Computes a hash value representing the state of the Local ID,
		 *          suitable for use in hash-based containers like `std::unordered_set`.
		 *          Local IDs that are equal according to `equals()` must produce the same hash code.
		 *          Delegates to the underlying builder's `hashCode`.
		 * @return A `size_t` hash code.
		 * @see equals, LocalIdBuilder::hashCode
		 */
		[[nodiscard]] size_t hashCode() const;

		//=====================================================================
		// Builder Accessor
		//=====================================================================

		/**
		 * @brief Gets a constant reference to the underlying `LocalIdBuilder` holding the state.
		 * @details Provides access to the internal builder state. Primarily for internal use or
		 *          advanced scenarios where direct access to builder properties is needed.
		 * @return A constant reference to the internal `LocalIdBuilder`.
		 */
		[[nodiscard]] const LocalIdBuilder& builder() const;

		//=====================================================================
		// Core Properties
		//=====================================================================

		/**
		 * @brief Gets the VIS version associated with this Local ID.
		 * @details Delegates to the underlying builder. Assumes the VIS version is present
		 *          due to validation during `LocalId` construction.
		 * @return The `VisVersion` enum value.
		 * @note This method relies on the `LocalId` constructor ensuring a valid VIS version exists.
		 */
		[[nodiscard]] virtual VisVersion visVersion() const override;

		/**
		 * @brief Checks if the Local ID was parsed or generated in verbose mode.
		 * @details Delegates to the underlying builder.
		 * @return True if verbose mode is indicated, false otherwise.
		 */
		[[nodiscard]] virtual bool isVerboseMode() const override;

		/**
		 * @brief Gets the primary GMOD path item of the Local ID.
		 * @details Delegates to the underlying builder. Assumes the primary item is present
		 *          due to validation during `LocalId` construction.
		 * @return A constant reference to the primary `GmodPath`.
		 * @note This method relies on the `LocalId` constructor ensuring a valid primary item exists.
		 */
		[[nodiscard]] virtual const GmodPath& primaryItem() const override;

		/**
		 * @brief Gets the optional secondary GMOD path item.
		 * @details Delegates to the underlying builder.
		 * @return A constant reference to an `std::optional<GmodPath>` containing the secondary
		 *         item path if present, or `std::nullopt` otherwise.
		 */
		[[nodiscard]] virtual const std::optional<GmodPath>& secondaryItem() const override;

		/**
		 * @brief Checks if the Local ID includes any custom (non-standard) metadata tags.
		 * @details Delegates to the underlying builder.
		 * @return True if at least one custom tag exists, false otherwise.
		 */
		[[nodiscard]] virtual bool hasCustomTag() const override;

		/**
		 * @brief Gets all metadata tags associated with the Local ID.
		 * @details Delegates to the underlying builder. Returns a collection of the `MetadataTag` objects.
		 * @return A vector containing copies of the `MetadataTag` objects.
		 */
		[[nodiscard]] virtual std::vector<MetadataTag> metadataTags() const override;

		//=====================================================================
		// Metadata Accessors
		//=====================================================================

		/**
		 * @brief Gets the quantity metadata tag, if present.
		 * @details Convenience method, delegates to the underlying builder.
		 * @return A const reference to an `std::optional<MetadataTag>` for quantity.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& quantity() const;

		/**
		 * @brief Gets the content metadata tag, if present.
		 * @details Convenience method, delegates to the underlying builder.
		 * @return A const reference to an `std::optional<MetadataTag>` for content.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& content() const;

		/**
		 * @brief Gets the calculation metadata tag, if present.
		 * @details Convenience method, delegates to the underlying builder.
		 * @return A const reference to an `std::optional<MetadataTag>` for calculation.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& calculation() const;

		/**
		 * @brief Gets the state metadata tag, if present.
		 * @details Convenience method, delegates to the underlying builder.
		 * @return A const reference to an `std::optional<MetadataTag>` for state.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& state() const;

		/**
		 * @brief Gets the command metadata tag, if present.
		 * @details Convenience method, delegates to the underlying builder.
		 * @return A const reference to an `std::optional<MetadataTag>` for command.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& command() const;

		/**
		 * @brief Gets the type metadata tag, if present.
		 * @details Convenience method, delegates to the underlying builder.
		 * @return A const reference to an `std::optional<MetadataTag>` for type.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& type() const;

		/**
		 * @brief Gets the position metadata tag, if present.
		 * @details Convenience method, delegates to the underlying builder.
		 * @return A const reference to an `std::optional<MetadataTag>` for position.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& position() const;

		/**
		 * @brief Gets the detail metadata tag, if present.
		 * @details Convenience method, delegates to the underlying builder.
		 * @return A const reference to an `std::optional<MetadataTag>` for detail.
		 */
		[[nodiscard]] const std::optional<MetadataTag>& detail() const;

		//=====================================================================
		// Conversion and Comparison
		//=====================================================================

		/**
		 * @brief Converts the Local ID to its canonical string representation.
		 * @details Delegates to the underlying builder's `toString` method.
		 * @return The `std::string` representation of the Local ID.
		 */
		[[nodiscard]] virtual std::string toString() const override;

		/**
		 * @brief Performs a deep equality comparison with another Local ID.
		 * @details Compares the underlying builders for equality.
		 * @param[in] other The `LocalId` object to compare against.
		 * @return True if the underlying builders are equal, false otherwise.
		 * @throws Can throw exceptions if the underlying `LocalIdBuilder::equals` method throws
		 *         (e.g., `std::invalid_argument` on VIS mismatch).
		 */
		[[nodiscard]] virtual bool equals( const LocalId& other ) const override;

	private:
		//=====================================================================
		// Private Member Variables
		//=====================================================================

		/**
		 * @brief Shared pointer to the immutable builder holding the Local ID's state.
		 * @details Using `shared_ptr<const LocalIdBuilder>` allows efficient sharing of the
		 *          immutable state generated by the builder.
		 */
		std::shared_ptr<const LocalIdBuilder> m_builder;
	};
}
