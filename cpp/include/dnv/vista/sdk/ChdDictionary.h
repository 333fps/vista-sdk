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
		// Internal Helper Components
		//=====================================================================

		//----------------------------------------------
		// Constants
		//----------------------------------------------

		/** @brief FNV offset basis constant for hash calculations. */
		constexpr uint32_t FNV_OFFSET_BASIS{ 0x811C9DC5 };

		/** @brief FNV prime constant for hash calculations. */
		constexpr uint32_t FNV_PRIME{ 0x01000193 };

		/** @brief Number of entries in the thread-local hash lookup cache. */
		static inline constexpr size_t HASH_CACHE_SIZE = 128;

		//----------------------------------------------
		// CPU Feature Detection
		//----------------------------------------------

		/**
		 * @brief Gets the cached SSE4.2 support status.
		 * @details Checks CPU capabilities to determine if SSE4.2 instructions are available,
		 *          which can accelerate CRC32 hashing. The result is cached for efficiency.
		 * @return `true` if SSE4.2 is supported, `false` otherwise.
		 */
		bool hasSSE42Support();

		//----------------------------------------------
		// Hashing Class
		//----------------------------------------------

		/**
		 * @class Hashing
		 * @brief Provides hashing function utilities required for the CHD algorithm.
		 */
		class Hashing final
		{
		public:
			//---------------------------
			//  Public Static Methods
			//---------------------------

			/**
			 * @brief Computes one step of the FNV-1a hash function.
			 * @param[in] hash The current hash value.
			 * @param[in] ch The character (byte) to incorporate into the hash.
			 * @return The updated hash value.
			 * @see https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
			 */
			static uint32_t fnv1a( uint32_t hash, uint8_t ch );

			/**
			 * @brief Computes one step of the CRC32 hash function using SSE4.2 instructions if available.
			 * @param[in] hash The current hash value.
			 * @param[in] ch The character (byte) to incorporate into the hash.
			 * @return The updated hash value.
			 * @see https://en.wikipedia.org/wiki/Cyclic_redundancy_check
			 */
			static uint32_t crc32( uint32_t hash, uint8_t ch );

			/**
			 * @brief Computes the final table index using the seed mixing function for the CHD algorithm.
			 * @param[in] seed The seed value associated with the hash bucket.
			 * @param[in] hash The 32-bit hash value of the key.
			 * @param[in] size The total size (capacity) of the dictionary's main table. Must be a power of 2.
			 * @return The final table index for the key (as size_t).
			 * @see https://en.wikipedia.org/wiki/Perfect_hash_function#CHD_algorithm
			 */
			static uint32_t seed( uint32_t seed, uint32_t hash, uint64_t size );
		};
	}
}

namespace dnv::vista::sdk
{
	//=====================================================================
	// ChdDictionary Class
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
	 * @warning **Incompatible with C# Version:**
	 * Due to differences in the underlying string hashing implementations, dictionaries
	 * created by this C++ version are **NOT** binary compatible with dictionaries
	 * created by the C# `Vista.SDK.Internal.ChdDictionary`.
	 *   - **C++ Hashing:** This version hashes **all bytes** of the provided
	 *     `std::string_view` key. For potential cross-language data generation,
	 *     keys should ideally be UTF-8 encoded.
	 *   - **C# Hashing:** The C# version hashes only the **first byte** of each
	 *     2-byte UTF-16 character in the C# `string`.
	 * This fundamental difference results in different hash values and thus
	 * completely different internal table layouts.
	 *
	 * @see https://en.wikipedia.org/wiki/Perfect_hash_function#CHD_algorithm
	 */
	template <typename TValue>
	class ChdDictionary final
	{
	public:
		//----------------------------------------------
		// Iterator Inner Class
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
			// Standard Iterator Traits
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
			 * @param[in] table Pointer to the dictionary's internal storage vector.
			 * @param[in] index The index within the table this iterator should point to.
			 */
			explicit Iterator( const std::vector<std::pair<std::string, TValue>>* table, size_t index );

			//---------------------------
			// Operations
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

			//---------------------------
			// Comparison
			//---------------------------

			/**
			 * @brief Checks if this iterator is equal to another iterator.
			 * @param[in] other The iterator to compare against.
			 * @return `true` if both iterators point to the same element or are both end iterators for the same container, `false` otherwise.
			 */
			bool operator==( const Iterator& other ) const;

			/**
			 * @brief Checks if this iterator is not equal to another iterator.
			 * @param[in] other The iterator to compare against.
			 * @return `true` if the iterators point to different elements, `false` otherwise.
			 */
			bool operator!=( const Iterator& other ) const;

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
			/** @brief Pointer to the dictionary's internal data table. Null for default-constructed iterators. */
			const std::vector<std::pair<std::string, TValue>>* m_table = nullptr;

			/** @brief Current index within the `m_table`. */
			size_t m_index = 0;
		};

		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		/**
		 * @brief Constructs the dictionary from a vector of key-value pairs.
		 * @details Builds the perfect hash function based on the provided `items`.
		 *          The input `items` vector is moved into the dictionary.
		 * @param[in] items A `std::vector` of `std::pair<std::string, TValue>` used to populate the dictionary.
		 *                  The keys within this vector must be unique.
		 */
		explicit ChdDictionary( std::vector<std::pair<std::string, TValue>> items );

		/** @brief Default constructor */
		ChdDictionary() = default;

		/** @brief Copy constructor */
		ChdDictionary( const ChdDictionary& other ) = delete;

		/** @brief Move constructor */
		ChdDictionary( ChdDictionary&& other ) noexcept = default;

		/** @brief Destructor */
		~ChdDictionary() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		/** @brief Copy assignment operator */
		ChdDictionary& operator=( const ChdDictionary& other ) = delete;

		/** @brief Move assignment operator */
		ChdDictionary& operator=( ChdDictionary&& other ) noexcept = default;

		//----------------------------------------------
		// Lookup Operators
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

		//----------------------------------------------
		// Lookup Methods
		//----------------------------------------------

		/**
		 * @brief Accesses the value associated with the specified key (const version with bounds checking).
		 * @details Provides read-only access to the value. Performs a lookup using the perfect hash function.
		 * @param[in] key The key whose associated value is to be retrieved.
		 * @return A constant reference to the value associated with `key`.
		 * @throws std::out_of_range if the `key` is not found in the dictionary or if the dictionary is empty.
		 */
		[[nodiscard]] const TValue& at( std::string_view key ) const;

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
		// Capacity
		//----------------------------------------------

		/**
		 * @brief Checks if the dictionary is empty.
		 * @return `true` if the dictionary contains no elements, `false` otherwise.
		 */
		[[nodiscard]] bool isEmpty() const;

		/**
		 * @brief Returns the number of elements in the dictionary.
		 * @return The number of key-value pairs stored in the dictionary.
		 */
		[[nodiscard]] size_t size() const;

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
		// Private Helper Methods
		//----------------------------------------------

		//---------------------------
		// Hashing
		//---------------------------

		/**
		 * @brief Processes a single byte using the selected hash function (FNV1a or CRC32).
		 * @param[in] hash The current hash value.
		 * @param[in] byte The byte to incorporate into the hash.
		 * @return The updated hash value.
		 */
		static uint32_t processHashByte( uint32_t hash, uint8_t byte );

		/**
		 * @brief Calculates the hash value for a given string key using the selected hash function.
		 * @details May utilize a thread-local cache for frequently accessed keys.
		 * @param[in] key The string key to hash.
		 * @return The calculated 32-bit hash value.
		 */
		static uint32_t hash( std::string_view key );

		//---------------------------
		// Utility
		//---------------------------

		/**
		 * @brief Performs a no-exception comparison between a `std::string_view` and a `std::string`.
		 * @param[in] a The first string view to compare.
		 * @param[in] b The second string to compare.
		 * @return `true` if the contents of `a` and `b` are identical, `false` otherwise.
		 */
		[[nodiscard]] static bool stringsEqual( std::string_view a, const std::string& b ) noexcept;

		/**
		 * @brief Performs a no-exception comparison between two character spans.
		 * @details Useful for comparing raw character arrays or memory buffers efficiently.
		 * @param[in] a The first character span to compare.
		 * @param[in] b The second character span to compare.
		 * @return `true` if the spans have the same size and identical character sequences, `false` otherwise.
		 */
		[[nodiscard]] static bool stringsEqual( std::span<const char> a, std::span<const char> b ) noexcept;

		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		/** @brief The primary storage table containing the key-value pairs. Order determined during construction. */
		std::vector<std::pair<std::string, TValue>> m_table;

		/** @brief The seed values used by the CHD perfect hash function to resolve hash collisions. Size matches `m_table`. */
		std::vector<int> m_seeds;

		//----------------------------------------------
		// Caching and Performance Monitoring
		//----------------------------------------------

		/** @brief Structure defining an entry in the thread-local hash cache. */
		struct HashCacheEntry
		{
			/** @brief View of the key (points into s_hashCacheStorage). */
			std::string_view key;

			/** @brief Cached hash result. */
			uint32_t hash;
		};

		/**
		 * @brief Thread-local cache storing recent hash results to speed up repeated lookups of the same keys.
		 * @details Aligned to 64 bytes to potentially avoid false sharing on multi-core systems.
		 */
		alignas( 64 ) static thread_local std::array<HashCacheEntry, internal::HASH_CACHE_SIZE> s_hashCache;

		/**
		 * @brief Thread-local storage for the actual string keys corresponding to the `string_view`s in `s_hashCache`.
		 * @details Ensures the `string_view`s in the cache remain valid.
		 * @details Aligned to 64 bytes.
		 */
		alignas( 64 ) static thread_local std::array<std::string, internal::HASH_CACHE_SIZE> s_hashCacheStorage;

		/** @brief Thread-local counter for hash cache hits (for performance analysis). */
		static thread_local size_t s_cacheHits;

		/** @brief Thread-local counter for hash cache misses (for performance analysis). */
		static thread_local size_t s_cacheMisses;

		/** @brief Thread-local counter for total dictionary lookups performed (for performance analysis). */
		static thread_local size_t s_lookupCount;

		/** @brief Thread-local counter for successful dictionary lookups (key found) (for performance analysis). */
		static thread_local size_t s_lookupHits;

		/** @brief Thread-local accumulator for total time spent in lookups (for average time calculation). */
		static thread_local std::chrono::nanoseconds s_totalLookupDuration;
	};
}

#include "ChdDictionary.inl"
