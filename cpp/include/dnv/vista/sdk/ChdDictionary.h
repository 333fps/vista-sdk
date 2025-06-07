/**
 * @file ChdDictionary.h
 * @brief Perfect hashing dictionary using the CHD algorithm.
 *
 * @details This file defines the ChdDictionary class, a core data structure in the VISTA SDK
 * that provides fast, memory-efficient lookups using the Compress, Hash, and Displace
 * (CHD) algorithm for perfect hashing. This implementation optimizes for read-heavy
 * operations with minimal memory overhead.
 *
 * @see https://en.wikipedia.org/wiki/Perfect_hash_function#CHD_algorithm
 */

#pragma once

namespace dnv::vista::sdk
{
	namespace internal
	{
		//=====================================================================
		// Internal helper components
		//=====================================================================

		//----------------------------------------------
		// Constants
		//----------------------------------------------

		/** @brief FNV offset basis constant for hash calculations. */
		static constexpr uint32_t FNV_OFFSET_BASIS{ 0x811C9DC5 };

		/** @brief FNV prime constant for hash calculations. */
		static constexpr uint32_t FNV_PRIME{ 0x01000193 };

		//----------------------------------------------
		// CPU feature detection
		//----------------------------------------------

		/**
		 * @brief Gets the cached SSE4.2 support status.
		 * @details Checks CPU capabilities to determine if SSE4.2 instructions are available,
		 *          which can accelerate CRC32 hashing. The result is cached for efficiency.
		 * @return `true` if SSE4.2 is supported, `false` otherwise.
		 */
		[[nodiscard]] bool hasSSE42Support();

		//----------------------------------------------
		// Hashing class
		//----------------------------------------------

		/**
		 * @class Hashing
		 * @brief Provides hashing function utilities required for the CHD algorithm.
		 */
		class Hashing final
		{
		public:
			//----------------------------
			// Construction / destruction
			//----------------------------

			/** @brief Default constructor. */
			Hashing() = delete;

			/** @brief Copy constructor */
			Hashing( const Hashing& ) = delete;

			/** @brief Move constructor */
			Hashing( Hashing&& ) noexcept = delete;

			/** @brief Destructor */
			~Hashing() = delete;

			//----------------------------
			// Assignment operators
			//----------------------------

			/** @brief Copy assignment operator */
			Hashing& operator=( const Hashing& ) = delete;

			/** @brief Move assignment operator */
			Hashing& operator=( Hashing&& ) noexcept = delete;

			//----------------------------
			// Public static methods
			//----------------------------

			/**
			 * @brief Computes one step of the Larsson hash function.
			 * @details Simple multiplicative hash function provided for:
			 *          - Benchmarking against other hash algorithms
			 *          - Custom hash implementations
			 *          - Research and comparison purposes
			 * @note This function is not used by the main CHD algorithm but is provided
			 *       as a utility for external code that may need alternative hash functions.
			 * @param[in] hash The current hash value.
			 * @param[in] ch The character (byte) to incorporate into the hash.
			 * @return The updated hash value.
			 */
			[[nodiscard]] static constexpr __forceinline uint32_t Larsson( uint32_t hash, uint8_t ch ) noexcept
			{
				return 37 * hash + ch;
			}

			/**
			 * @brief Computes one step of the FNV-1a hash function.
			 * @param[in] hash The current hash value.
			 * @param[in] ch The character (byte) to incorporate into the hash.
			 * @return The updated hash value.
			 * @see https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
			 */
			[[nodiscard]] static constexpr __forceinline uint32_t fnv1a( uint32_t hash, uint8_t ch ) noexcept
			{
				return ( ch ^ hash ) * FNV_PRIME;
			}

			/**
			 * @brief Computes one step of the CRC32 hash function using SSE4.2 instructions.
			 * @param[in] hash The current hash value.
			 * @param[in] ch The character (byte) to incorporate into the hash.
			 * @return The updated hash value.
			 * @note Requires SSE4.2 support. Use hasSSE42Support() to check availability.
			 * @see https://en.wikipedia.org/wiki/Cyclic_redundancy_check
			 */
			[[nodiscard]] static __forceinline uint32_t crc32( uint32_t hash, uint8_t ch ) noexcept
			{
				return _mm_crc32_u8( hash, ch );
			}

			/**
			 * @brief Computes the final table index using the seed mixing function for the CHD algorithm.
			 * @param[in] seed The seed value associated with the hash bucket.
			 * @param[in] hash The 32-bit hash value of the key.
			 * @param[in] size The total size (capacity) of the dictionary's main table. Must be a power of 2.
			 * @return The final table index for the key (as size_t).
			 * @see https://en.wikipedia.org/wiki/Perfect_hash_function#CHD_algorithm
			 */
			[[nodiscard]] static __forceinline uint32_t seed( uint32_t seed, uint32_t hash, uint64_t size )
			{
				/* Mixes the primary hash with the seed to find the final table slot */
				uint32_t x{ seed + hash };
				x ^= x >> 12;
				x ^= x << 25;
				x ^= x >> 27;

				auto result{ static_cast<uint32_t>( ( static_cast<uint64_t>( x ) * 0x2545F4914F6CDD1DUL ) & ( size - 1 ) ) };

				return result;
			}
		};
	}
}

namespace dnv::vista::sdk
{
	//=====================================================================
	// ChdDictionary class
	//=====================================================================

	/**
	 * @class ChdDictionary
	 * @brief A read-only dictionary using the Compress, Hash, and Displace (CHD)
	 * perfect hashing algorithm for guaranteed O(1) worst-case lookups after construction.
	 *
	 * @details Provides O(1) expected lookup time with minimal memory overhead for essentially read-only
	 * dictionaries. It uses a two-level perfect hashing scheme based on the CHD algorithm
	 * by Botelho, Pagh, and Ziviani, ensuring no collisions for the stored keys.
	 * This implementation is suitable for scenarios where a fixed set of key-value pairs
	 * needs to be queried frequently and efficiently. It includes optimizations like
	 * optional SSE4.2 hashing and thread-local caching.
	 *
	 * @tparam TValue The type of values stored in the dictionary.
	 *
	 * @warning **UTF-16 Compatibility Note:**
	 * This C++ implementation is designed to be **FULLY COMPATIBLE** with the C# version.
	 * Both implementations simulate UTF-16 string processing to ensure identical hash values:
	 *   - **C++ Processing:** Processes each ASCII character as 2 bytes (low byte + high byte 0)
	 *     to match UTF-16 encoding used by C# strings.
	 *   - **C# Processing:** Processes UTF-16 strings by reading low byte of each character
	 *     and skipping the high byte (which is 0 for ASCII characters).
	 * This ensures **binary compatibility** and **identical hash values** between C++ and C# versions.
	 * Dictionaries created by either implementation can be used interchangeably.
	 *
	 * @see https://en.wikipedia.org/wiki/Perfect_hash_function#CHD_algorithm
	 */
	template <typename TValue>
	class ChdDictionary final
	{
	public:
		//----------------------------------------------
		// Forward declarations
		//----------------------------------------------

		class Iterator;

		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		/**
		 * @brief Constructs the dictionary from a vector of key-value pairs.
		 * @param[in] items A vector of key-value pairs. The keys must be unique.
		 * @throws std::invalid_argument if duplicate keys are found.
		 * @throws std::runtime_error if perfect hash construction fails.
		 */
		explicit ChdDictionary( std::vector<std::pair<std::string, TValue>> items );

		/** @brief Default constructor */
		ChdDictionary() = default;

		/** @brief Copy constructor */
		ChdDictionary( const ChdDictionary& other ) = default;

		/** @brief Move constructor */
		ChdDictionary( ChdDictionary&& other ) noexcept = default;

		/** @brief Destructor */
		~ChdDictionary() = default;

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		ChdDictionary& operator=( const ChdDictionary& other ) = default;

		/** @brief Move assignment operator */
		ChdDictionary& operator=( ChdDictionary&& other ) noexcept = default;

		//----------------------------------------------
		// Lookup operators
		//----------------------------------------------

		/**
		 * @brief Accesses the value associated with the specified key (non-const version).
		 * @details Provides read-write access to the value. Performs a lookup using the perfect hash function.
		 *          Allows modification of the retrieved value if TValue is mutable.
		 * @param[in] key The key whose associated value is to be retrieved.
		 * @return A reference to the value associated with `key`.
		 * @throws std::out_of_range if the `key` is not found in the dictionary or if the dictionary is empty.
		 */
		[[nodiscard]] TValue& operator[]( std::string_view key );

		/**
		 * @brief Accesses the value associated with the specified key (const version with bounds checking).
		 * @details Provides read-only access to the value. Performs a lookup using the perfect hash function.
		 * @param[in] key The key whose associated value is to be retrieved.
		 * @return A constant reference to the value associated with `key`.
		 * @throws std::out_of_range if the `key` is not found in the dictionary or if the dictionary is empty.
		 */
		[[nodiscard]] const TValue& at( std::string_view key ) const;

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		/**
		 * @brief Returns the number of elements in the dictionary.
		 * @return The number of key-value pairs stored in the dictionary.
		 */
		[[nodiscard]] size_t size() const;

		//----------------------------------------------
		// State inspection methods
		//----------------------------------------------

		/**
		 * @brief Checks if the dictionary is empty.
		 * @return `true` if the dictionary contains no elements, `false` otherwise.
		 */
		[[nodiscard]] bool isEmpty() const;

		//----------------------------------------------
		// Static query methods
		//----------------------------------------------

		/**
		 * @brief Attempts to retrieve the value associated with the specified key without throwing exceptions.
		 * @details Performs a lookup using the perfect hash function. If the key is found, the output
		 *          parameter `outValue` is updated to point to the associated value within the dictionary's
		 *          internal storage.
		 * @param[in] key The key whose associated value is to be retrieved.
		 * @param[out] outValue A reference to a pointer-to-const TValue. On success, this pointer will be
		 *                      set to the address of the found value. On failure, it will be set to `nullptr`.
		 * @return `true` if the `key` was found and `outValue` was updated, `false` otherwise.
		 */
		[[nodiscard]] bool tryGetValue( std::string_view key, const TValue*& outValue ) const;

		//----------------------------------------------
		// Iteration
		//----------------------------------------------

		/**
		 * @brief Gets an iterator pointing to the first element of the dictionary.
		 * @return An `Iterator` positioned at the beginning of the dictionary's data.
		 *         If the dictionary is empty, this will be equal to `end()`.
		 */
		[[nodiscard]] Iterator begin() const;

		/**
		 * @brief Gets an iterator pointing past the last element of the dictionary.
		 * @return An `Iterator` representing the position after the last element.
		 *         This iterator should not be dereferenced.
		 */
		[[nodiscard]] Iterator end() const;

	private:
		//----------------------------------------------
		// Private helper methods
		//----------------------------------------------

		//---------------------------
		// Hashing
		//---------------------------

		/**
		 * @brief Calculates the hash value for a given string key using UTF-16 simulation.
		 * @details Simulates C# UTF-16 string processing by treating each ASCII character as
		 *          2 bytes (character value + 0), ensuring compatibility with C# implementation.
		 *          Uses SSE4.2 CRC32 instructions when available, falls back to FNV-1a.
		 * @param[in] key The string key to hash (ASCII characters assumed).
		 * @return The calculated 32-bit hash value, compatible with C# implementation.
		 */
		static uint32_t hash( std::string_view key ) noexcept;

		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		/** @brief The primary storage table containing the key-value pairs. Order determined during construction. */
		std::vector<std::pair<std::string, TValue>> m_table;

		/** @brief The seed values used by the CHD perfect hash function to resolve hash collisions. Size matches `m_table`. */
		std::vector<int> m_seeds;

	public:
		//----------------------------------------------
		// ChdDictionary::Iterator
		//----------------------------------------------

		/**
		 * @class Iterator
		 * @brief Provides forward iteration capabilities over the key-value pairs in the ChdDictionary.
		 *
		 * @details Conforms to the requirements of a C++ forward iterator, allowing range-based for loops
		 *          and standard algorithms to be used with the dictionary.
		 */
		class Iterator final
		{
		public:
			//---------------------------
			// Standard iterator traits
			//---------------------------

			using iterator_category = std::forward_iterator_tag;
			using value_type = std::pair<std::string, TValue>;
			using difference_type = std::ptrdiff_t;
			using pointer = const value_type*;
			using reference = const value_type&;

			//---------------------------
			// Construction
			//---------------------------

			/** @brief Default constructor */
			Iterator() = default;

			/**
			 * @brief Constructs an iterator pointing to a specific element in the dictionary's table.
			 * @param[in] table Pointer to the dictionary's internal storage vector. Must not be null.
			 * @param[in] index The index within the table this iterator should point to.
			 * @note If index >= table->size(), the iterator represents an end iterator.
			 */
			explicit Iterator( const std::vector<std::pair<std::string, TValue>>* table, size_t index );

			//---------------------------
			// Operators
			//---------------------------

			/**
			 * @brief Dereferences the iterator to access the current key-value pair.
			 * @return A constant reference to the `std::pair<std::string, TValue>` at the current position.
			 */
			reference operator*() const;

			/**
			 * @brief Provides member access to the current key-value pair.
			 * @return A constant pointer to the `std::pair<std::string, TValue>` at the current position.
			 */
			pointer operator->() const;

			/**
			 * @brief Advances the iterator to the next element (pre-increment).
			 * @return A reference to this iterator after advancing.
			 */
			Iterator& operator++();

			/**
			 * @brief Advances the iterator to the next element (post-increment).
			 * @return A copy of the iterator *before* it was advanced.
			 */
			Iterator operator++( int );

			/**
			 * @brief Checks if this iterator is equal to another iterator.
			 * @param[in] other The iterator to compare against.
			 * @return `true` if both iterators point to the same element or are both end iterators for the same container, `false` otherwise.
			 */
			[[nodiscard]] bool operator==( const Iterator& other ) const;

			/**
			 * @brief Checks if this iterator is not equal to another iterator.
			 * @param[in] other The iterator to compare against.
			 * @return `true` if the iterators point to different elements, `false` otherwise.
			 */
			[[nodiscard]] bool operator!=( const Iterator& other ) const;

			//---------------------------
			// Utility
			//---------------------------

			/**
			 * @brief Resets the iterator to an invalid state (index set beyond bounds).
			 * @details After reset, the iterator is typically not equal to begin() or end() unless
			 *          the dictionary is empty, and it should not be dereferenced.
			 */
			void reset();

		private:
			//---------------------------
			// Private member variables
			//---------------------------

			/** @brief Pointer to the dictionary's internal data table. Null for default-constructed iterators. */
			const std::vector<std::pair<std::string, TValue>>* m_table = nullptr;

			/** @brief Current index within the `m_table`. */
			size_t m_index = 0;
		};
	};
}

#include "ChdDictionary.inl"
