/**
 * @file Codebooks.h
 * @brief Container for Vessel Information Structure (VIS) codebooks
 * @details Provides access to standardized codebooks as defined in ISO 19848,
 *          with support for retrieving codebook entries, validating values, and
 *          creating metadata tags.
 * @see ISO 19848 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

#include "Codebook.h"
#include "CodebookName.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class MetadataTag;
	class CodebooksDto;
	enum class VisVersion;

	//=====================================================================
	// Constants
	//=====================================================================

	/**
	 * @brief Number of codebooks based on the enum values.
	 * @details Size accommodates 0-based indexing derived from 1-based enum values.
	 *          Example: CodebookName::Position (value 1) maps to index 0.
	 */
	static constexpr size_t NUM_CODEBOOKS = static_cast<size_t>( CodebookName::Detail );
}

namespace dnv::vista::sdk
{
	/**
	 * @brief Container for all codebooks in a specific VIS version
	 * @details Provides access to standard codebooks defined in ISO 19848,
	 *          supports iteration over codebooks and metadata tag creation.
	 *          This container is immutable after construction.
	 */
	class Codebooks final
	{
	public:
		//-------------------------------------------------------------------
		// Iterator implementation
		//-------------------------------------------------------------------

		/**
		 * @brief Forward iterator for the codebooks collection.
		 * @details Provides standard C++ forward iterator interface.
		 */
		class Iterator final
		{
		public:
			/**
			 * @brief Standard iterator type definitions
			 */
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::tuple<CodebookName, std::reference_wrapper<const Codebook>>;
			using difference_type = std::ptrdiff_t;
			using pointer = value_type*;
			using reference = value_type;

			/**
			 * @brief Construct an iterator
			 * @param codebooks Pointer to the codebooks array
			 * @param index Starting position index (0 to NUM_CODEBOOKS)
			 */
			Iterator( const std::array<Codebook, NUM_CODEBOOKS>* codebooks, size_t index );

			/**
			 * @brief Dereference operator.
			 * @return Tuple containing CodebookName and const reference_wrapper<Codebook>.
			 * @throws std::out_of_range if iterator is out of bounds.
			 */
			reference operator*() const;

			/**
			 * @brief Arrow operator (provides access to tuple members).
			 * @return A temporary tuple object containing CodebookName and const reference_wrapper<Codebook>.
			 * @throws std::out_of_range if iterator is out of bounds.
			 */
			value_type operator->() const;

			/**
			 * @brief Pre-increment operator. Advances to the next valid codebook.
			 * @return Reference to this iterator after increment.
			 */
			Iterator& operator++();

			/**
			 * @brief Post-increment operator. Advances to the next valid codebook.
			 * @return Copy of iterator before increment.
			 */
			Iterator operator++( int );

			/**
			 * @brief Equality comparison operator
			 * @param other Iterator to compare with
			 * @return True if iterators point to the same element in the same container
			 */
			bool operator==( const Iterator& other ) const;

			/**
			 * @brief Inequality comparison operator
			 * @param other Iterator to compare with
			 * @return True if iterators are not equal
			 */
			bool operator!=( const Iterator& other ) const;

		private:
			/** @brief Pointer to the underlying codebooks array */
			const std::array<Codebook, NUM_CODEBOOKS>* m_codebooks;

			/** @brief Current position index (0 to NUM_CODEBOOKS) */
			size_t m_index;
		};

		//-------------------------------------------------------------------
		// Construction / Destruction
		//-------------------------------------------------------------------

		/** @brief Default constructor */
		Codebooks() = default;

		/** @brief Copy constructor */
		Codebooks( const Codebooks& ) = default;

		/** @brief Move constructor */
		Codebooks( Codebooks&& ) noexcept = default;

		/** @brief Copy assignment operator */
		Codebooks& operator=( const Codebooks& ) = default;

		/** @brief Move assignment operator */
		Codebooks& operator=( Codebooks&& ) noexcept = default;

		/** @brief Destructor */
		~Codebooks() = default;

		/**
		 * @brief Construct from DTO
		 * @param version The VIS version
		 * @param dto The codebooks data transfer object
		 */
		Codebooks( VisVersion version, const CodebooksDto& dto );

		//-------------------------------------------------------------------
		// Codebook access methods
		//-------------------------------------------------------------------

		/**
		 * @brief Access a codebook by name using array index operator.
		 * @param name The codebook name (enum value).
		 * @return Const reference to the requested codebook.
		 * @throws std::invalid_argument If the name is invalid or out of range.
		 */
		const Codebook& operator[]( CodebookName name ) const;

		/**
		 * @brief Get a codebook by name (alternative accessor).
		 * @param name The codebook name (enum value).
		 * @return Const reference to the requested codebook.
		 * @throws std::invalid_argument If the name is invalid or out of range.
		 */
		const Codebook& codebook( CodebookName name ) const;

		/**
		 * @brief Get the VIS version associated with these codebooks.
		 * @return The VIS version enum value.
		 */
		VisVersion visVersion() const;

		//-------------------------------------------------------------------
		// Tag creation methods
		//-------------------------------------------------------------------

		/**
		 * @brief Try to create a metadata tag using the appropriate codebook.
		 * @param name The codebook name identifying which codebook to use.
		 * @param value The string value for the tag.
		 * @return std::optional<MetadataTag> containing the tag if successful, otherwise std::nullopt.
		 */
		std::optional<MetadataTag> tryCreateTag( CodebookName name, const std::string_view value ) const;

		/**
		 * @brief Create a metadata tag using the appropriate codebook.
		 * @param name The codebook name identifying which codebook to use.
		 * @param value The string value for the tag.
		 * @return The created MetadataTag.
		 * @throws std::invalid_argument If the value is invalid for the specified codebook,
		 *         or if the codebook name is invalid.
		 */
		MetadataTag createTag( CodebookName name, const std::string& value ) const;

		//-------------------------------------------------------------------
		// Iteration methods (Standard C++)
		//-------------------------------------------------------------------

		/**
		 * @brief Get iterator to the beginning of the codebook collection.
		 * @return Iterator pointing to the first codebook element.
		 */
		Iterator begin() const;

		/**
		 * @brief Get iterator to the end of the codebook collection.
		 * @return Iterator pointing one past the last codebook element.
		 */
		Iterator end() const;

	private:
		//-------------------------------------------------------------------
		// Member variables
		//-------------------------------------------------------------------

		/** @brief The VIS version */
		VisVersion m_visVersion{};

		/** @brief Fixed-size array holding all codebooks */
		std::array<Codebook, NUM_CODEBOOKS> m_codebooks{};
	};
}
