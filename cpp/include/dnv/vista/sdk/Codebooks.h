/**
 * @file Codebooks.h
 * @brief Container for Vessel Information Structure (VIS) codebooks
 * @details Provides access to standardized codebooks as defined in ISO 19848:2018,
 *          with support for retrieving codebook entries, validating values, and
 *          creating metadata tags.
 * @see ISO 19848:2018 - Ships and marine technology - Standard data for shipboard machinery and equipment
 */

#pragma once

#include "Codebook.h"
#include "CodebookName.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Forward declarations
	//-------------------------------------------------------------------

	class MetadataTag;
	class CodebooksDto;
	enum class VisVersion;

	//-------------------------------------------------------------------
	// Constants
	//-------------------------------------------------------------------

	/**
	 * @brief Number of codebooks based on the enum values
	 * @details Size includes an offset of 1 since enum values start at 1
	 */
	static constexpr size_t NUM_CODEBOOKS = static_cast<size_t>( CodebookName::Detail ) + 1;

	/**
	 * @brief Container for all codebooks in a specific VIS version
	 * @details Provides access to standard codebooks defined in ISO 19848:2018,
	 *          supports iteration over codebooks and metadata tag creation.
	 */
	class Codebooks final
	{
	public:
		//-------------------------------------------------------------------
		// Iterator implementation
		//-------------------------------------------------------------------

		/**
		 * @brief Forward iterator for codebooks collection
		 * @details Provides standard iterator interface plus additional enumeration functionality
		 *          through next()/current()/reset() methods
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
			using pointer = const value_type*;
			using reference = const value_type&;

			/**
			 * @brief Construct an iterator
			 * @param codebooks Pointer to the codebooks array
			 * @param index Starting position index
			 */
			Iterator( const std::array<Codebook, static_cast<size_t>( NUM_CODEBOOKS )>* codebooks, size_t index );

			/**
			 * @brief Dereference operator
			 * @return Reference to current codebook tuple
			 */
			reference operator*() const;

			/**
			 * @brief Arrow operator
			 * @return Pointer to current codebook tuple
			 */
			pointer operator->() const;

			/**
			 * @brief Pre-increment operator
			 * @return Reference to this iterator after increment
			 */
			Iterator& operator++();

			/**
			 * @brief Post-increment operator
			 * @return Copy of iterator before increment
			 */
			Iterator operator++( int );

			/**
			 * @brief Equality comparison operator
			 * @param other Iterator to compare with
			 * @return True if iterators are equal
			 */
			bool operator==( const Iterator& other ) const;

			/**
			 * @brief Inequality comparison operator
			 * @param other Iterator to compare with
			 * @return True if iterators are not equal
			 */
			bool operator!=( const Iterator& other ) const;

			/**
			 * @brief Move to next codebook in the sequence
			 * @return True if moved to a valid codebook, false if at end
			 */
			bool next();

			/**
			 * @brief Update current value cache
			 * @return True if current position is valid
			 */
			bool current();

			/**
			 * @brief Reset iterator position to before first element
			 */
			void reset();

		private:
			/** @brief Pointer to codebooks array */
			const std::array<Codebook, static_cast<size_t>( NUM_CODEBOOKS )>* m_codebooks;

			/** @brief Current position index */
			size_t m_index;

			/** @brief Cached current value */
			mutable std::optional<value_type> m_current;
		};

		/** @brief Alias for alternative enumeration pattern */
		using Enumerator = Iterator;

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
		 * @brief Access a codebook by name
		 * @param name The codebook name
		 * @return Reference to the codebook
		 * @throws std::invalid_argument If the name is invalid
		 */
		const Codebook& operator[]( CodebookName name ) const;

		/**
		 * @brief Get a codebook by name
		 * @param name The codebook name
		 * @return Reference to the codebook
		 * @throws std::invalid_argument If the name is invalid
		 */
		const Codebook& codebook( CodebookName name ) const;

		/**
		 * @brief Get the VIS version
		 * @return The VIS version
		 */
		VisVersion visVersion() const;

		//-------------------------------------------------------------------
		// Tag creation methods
		//-------------------------------------------------------------------

		/**
		 * @brief Try to create a metadata tag
		 * @param name The codebook name
		 * @param value The tag value
		 * @return The metadata tag, or nullopt if invalid
		 */
		std::optional<MetadataTag> tryCreateTag( CodebookName name, const std::string_view value ) const;

		/**
		 * @brief Create a metadata tag
		 * @param name The codebook name
		 * @param value The tag value
		 * @return The metadata tag
		 * @throws std::invalid_argument If the value is invalid
		 */
		MetadataTag createTag( CodebookName name, const std::string& value ) const;

		//-------------------------------------------------------------------
		// Iteration methods
		//-------------------------------------------------------------------

		/**
		 * @brief Get iterator to the beginning
		 * @return Iterator to the first codebook
		 */
		Iterator begin() const;

		/**
		 * @brief Get iterator to the end
		 * @return Iterator past the last codebook
		 */
		Iterator end() const;

		/**
		 * @brief Get an alternative enumerator
		 * @details Returns an iterator configured for enumeration using next()/current() methods,
		 *          initially positioned before the first element. This enables a different
		 *          iteration pattern than standard C++ iterators.
		 * @return Iterator for alternative enumeration pattern
		 */
		Enumerator enumerator() const;

	private:
		//-------------------------------------------------------------------
		// Member variables
		//-------------------------------------------------------------------

		/** @brief The VIS version */
		VisVersion m_visVersion;

		/** @brief Array of codebooks */
		std::array<Codebook, static_cast<size_t>( NUM_CODEBOOKS )> m_codebooks;
	};
}
