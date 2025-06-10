/**
 * @file Codebooks.h
 * @brief Container for Vessel Information Structure (VIS) codebooks
 * @details Provides access to standardized codebooks as defined in ISO 19848,
 *          with support for retrieving codebook entries, validating values, and
 *          creating metadata tags. This container is immutable after construction.
 * @see ISO 19848 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

#include "Codebook.h"
#include "CodebookName.h"
#include "MetadataTag.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class CodebooksDto;
	enum class VisVersion;

	//=====================================================================
	// Constants
	//=====================================================================

	/**
	 * @brief Number of codebooks based on the enum values.
	 * @details Array size matches the highest enum value for direct indexing.
	 *          All enum values are guaranteed to be valid array indices.
	 */
	static constexpr size_t NUM_CODEBOOKS = static_cast<size_t>( CodebookName::Detail );

	//=====================================================================
	// Codebooks class
	//=====================================================================

	/**
	 * @class Codebooks
	 * @brief Container for all codebooks in a specific VIS version.
	 * @details Provides access to standard codebooks defined in ISO 19848,
	 *          supports iteration over codebooks and metadata tag creation.
	 *          This container is immutable after construction.
	 */
	class Codebooks final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Construct codebooks from a Data Transfer Object (DTO).
		 * @details Initializes the container with codebooks based on the provided DTO
		 *          for the specified VIS version. Ensures the 'detail' codebook exists.
		 * @param version The `VisVersion` these codebooks belong to.
		 * @param dto The `CodebooksDto` containing the raw codebook data.
		 */
		explicit Codebooks( VisVersion version, const CodebooksDto& dto );

		/** @brief Default constructor */
		Codebooks() = default;

		/** @brief Copy constructor */
		Codebooks( const Codebooks& ) = default;

		/** @brief Move constructor */
		Codebooks( Codebooks&& ) noexcept = default;

		/** @brief Destructor */
		~Codebooks() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		Codebooks& operator=( const Codebooks& ) = default;

		/** @brief Move assignment operator */
		Codebooks& operator=( Codebooks&& ) noexcept = default;

		//----------------------------------------------
		// Lookup operators
		//----------------------------------------------

		/**
		 * @brief Fast access to codebook by name (no bounds checking)
		 * @param name The codebook name (must be valid)
		 * @return Reference to the requested codebook
		 * @note This method does not validate the input for maximum performance.
		 *       Use codebook() if you need bounds checking.
		 * @warning Undefined behavior if name is invalid
		 */
		[[nodiscard]] inline const Codebook& operator[]( CodebookName name ) const noexcept;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Get the VIS version associated with these codebooks.
		 * @return The `VisVersion` enum value.
		 */
		[[nodiscard]] inline VisVersion visVersion() const noexcept;

		/**
		 * @brief Safe access to codebook by name (with bounds checking)
		 * @param name The codebook name
		 * @return Reference to the requested codebook
		 * @throws std::invalid_argument If name is invalid
		 * @note This method always validates the input. Use operator[] for maximum performance
		 *       when you know the input is valid.
		 */
		[[nodiscard]] inline const Codebook& codebook( CodebookName name ) const;

		//----------------------------------------------
		// Range-based iterator
		//----------------------------------------------

		/** @brief Const iterator type for iterating over codebooks. */
		using Iterator = const Codebook*;

		/**
		 * @brief Returns iterator to the first codebook.
		 * @return Iterator to the beginning of the codebook container
		 * @note Zero-overhead pointer-based iteration
		 */
		[[nodiscard]] inline Iterator begin() const noexcept;

		/**
		 * @brief Returns iterator to one past the last codebook.
		 * @return Iterator to the end of the codebook container
		 * @note Zero-overhead pointer-based iteration
		 */
		[[nodiscard]] inline Iterator end() const noexcept;

		//----------------------------------------------
		// Tag creation
		//----------------------------------------------

		/**
		 * @brief Try to create a metadata tag using the appropriate codebook for validation.
		 * @details Attempts to validate the `value` against the codebook specified by `name`.
		 *          If validation succeeds, a `MetadataTag` is created and returned.
		 * @param name The `CodebookName` identifying which codebook to use for validation.
		 * @param value The string value for the tag. Can be empty.
		 * @return An `std::optional<MetadataTag>` containing the created tag if the value is valid
		 *         (or if the codebook allows any value, like 'detail'), otherwise `std::nullopt`.
		 * @throws std::invalid_argument If the `name` enum value is invalid (propagated from `operator[]`).
		 */
		[[nodiscard]] inline std::optional<MetadataTag> tryCreateTag( CodebookName name, const std::string_view value ) const;

		/**
		 * @brief Create a metadata tag using the appropriate codebook, throwing on failure.
		 * @details Validates the `value` against the codebook specified by `name`.
		 *          If validation succeeds, a `MetadataTag` is created and returned.
		 *          If validation fails, an exception is thrown.
		 * @param name The `CodebookName` identifying which codebook to use for validation.
		 * @param value The string value for the tag. Cannot be empty if validation is required.
		 * @return The created `MetadataTag`.
		 * @throws std::invalid_argument If the `value` is invalid for the specified codebook,
		 *         or if the `name` enum value is invalid.
		 */
		[[nodiscard]] inline MetadataTag createTag( CodebookName name, const std::string& value ) const;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The VIS version these codebooks belong to. */
		VisVersion m_visVersion{};

		/** @brief Fixed-size array holding all codebooks */
		std::array<Codebook, NUM_CODEBOOKS> m_codebooks{};
	};
}

#include "Codebooks.inl"
