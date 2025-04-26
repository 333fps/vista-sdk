/**
 * @file ChdDictionary.h
 * @brief Perfect hashing dictionary using the CHD algorithm
 *
 * This file defines the ChdDictionary class, a core data structure in the VISTA SDK
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
		//-------------------------------------------------------------------
		// Constants
		//-------------------------------------------------------------------

		/** @brief FNV offset basis constant for hash calculations */
		constexpr uint32_t FNV_OFFSET_BASIS{ 0x811C9DC5 };

		/** @brief FNV prime constant for hash calculations */
		constexpr uint32_t FNV_PRIME{ 0x01000193 };

		/** @brief Number of entries in the thread-local hash lookup cache */
		static inline constexpr size_t HASH_CACHE_SIZE = 128;

		//-------------------------------------------------------------------
		// CPU Feature Detection
		//-------------------------------------------------------------------

		/**
		 * @brief Get the cached SSE4.2 support status
		 * @return true if SSE4.2 is supported, false otherwise
		 */
		bool hasSSE42Support();

		//-------------------------------------------------------------------
		// Exception Handling
		//-------------------------------------------------------------------

		/**
		 * @brief Helper class for throwing standardized exceptions
		 */
		class ThrowHelper final
		{
		public:
			/**
			 * @brief Throw an exception for key not found
			 * @param key The key that was not found
			 * @throws std::out_of_range Always
			 */
			[[noreturn]] static void throwKeyNotFoundException( std::string_view key );

			/**
			 * @brief Throw an exception for invalid operation
			 * @throws std::invalid_argument Always
			 */
			[[noreturn]] static void throwInvalidOperationException();
		};

		//-------------------------------------------------------------------
		// Hashing
		//-------------------------------------------------------------------

		/**
		 * @brief Hash function utilities for the CHD algorithm
		 */
		class Hashing final
		{
		public:
			/**
			 * @brief FNV-1a hash function
			 * @param hash The current hash value
			 * @param ch The character to hash
			 * @return The updated hash value
			 * @see https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
			 */
			static uint32_t fnv1a( uint32_t hash, uint8_t ch );

			/**
			 * @brief CRC32 hash function using SSE4.2 instructions
			 * @param hash The current hash value
			 * @param ch The character to hash
			 * @return The updated hash value
			 * @see https://en.wikipedia.org/wiki/Cyclic_redundancy_check
			 */
			static uint32_t crc32( uint32_t hash, uint8_t ch );

			/**
			 * @brief Seed mixing function for CHD algorithm
			 * @param seed The seed value
			 * @param hash The hash value
			 * @param size The table size
			 * @return The final table index
			 * @see https://en.wikipedia.org/wiki/Perfect_hash_function#CHD_algorithm
			 */
			static uint32_t seed( uint32_t seed, uint32_t hash, uint64_t size );
		};
	}

	/**
	 * @brief Perfect hashing dictionary using CHD algorithm
	 *
	 * Provides O(1) lookup time with minimal memory overhead for read-only
	 * dictionaries. Uses a two-level perfect hashing scheme with no collisions.
	 * Implementation is based on the Compress, Hash, and Displace (CHD) algorithm
	 * by Botelho, Pagh, and Ziviani.
	 *
	 * @tparam TValue The type of values stored in the dictionary
	 * @see https://en.wikipedia.org/wiki/Perfect_hash_function#CHD_algorithm
	 */
	template <typename TValue>
	class ChdDictionary
	{
	public:
		//-------------------------------------------------------------------
		// Public Types Definitions
		//-------------------------------------------------------------------

		/**
		 * @brief Iterator for ChdDictionary key-value pairs
		 */
		class Iterator final
		{
		public:
			/** @brief STL iterator category */
			using iterator_category = std::forward_iterator_tag;

			/** @brief Type of values returned by the iterator */
			using value_type = std::pair<std::string, TValue>;

			/** @brief Type representing differences between iterators */
			using difference_type = std::ptrdiff_t;

			/** @brief Type for pointers to values */
			using pointer = const value_type*;

			/** @brief Type for references to values */
			using reference = const value_type&;

			/** @brief Default constructor */
			Iterator() = default;

			/**
			 * @brief Constructor from table and position
			 * @param table The dictionary table to iterate
			 * @param index The starting index position
			 */
			explicit Iterator( const std::vector<std::pair<std::string, TValue>>* table, size_t index );

			/** @brief Dereference operator */
			reference operator*() const;

			/** @brief Arrow operator */
			pointer operator->() const;

			/** @brief Pre-increment operator */
			Iterator& operator++();

			/** @brief Post-increment operator */
			Iterator operator++( int );

			/** @brief Equality operator */
			bool operator==( const Iterator& other ) const;

			/** @brief Inequality operator */
			bool operator!=( const Iterator& other ) const;

			/**
			 * @brief Reset the iterator
			 */
			void reset();

		private:
			/** @brief Reference to dictionary table */
			const std::vector<std::pair<std::string, TValue>>* m_table;

			/** @brief Current position in table */
			size_t m_index;

			/** @brief Current key-value pair */
			mutable std::pair<std::string, TValue> m_current;
		};

		/** @brief Alias for Iterator to support legacy code */
		using Enumerator = Iterator;

		//-------------------------------------------------------------------
		// Constructors & Assignment Operators
		//-------------------------------------------------------------------

		/**
		 * @brief Default constructor creates an empty dictionary
		 */
		ChdDictionary();

		/**
		 * @brief Construct from vector of key-value pairs
		 * @param items Key-value pairs to populate the dictionary
		 */
		explicit ChdDictionary( const std::vector<std::pair<std::string, TValue>>& items );

		/**
		 * @brief Copy constructor
		 * @param other Source dictionary to copy
		 */
		ChdDictionary( const ChdDictionary& other );

		/**
		 * @brief Move constructor for efficient transfer of dictionary contents
		 * @param other Dictionary to move from (will be left in valid but empty state)
		 */
		ChdDictionary( ChdDictionary&& other ) noexcept;

		/**
		 * @brief Copy assignment operator
		 * @param other Source dictionary to copy
		 * @return Reference to this dictionary
		 */
		ChdDictionary& operator=( const ChdDictionary& other );

		/**
		 * @brief Move assignment operator
		 * @param other Source dictionary to move from
		 * @return Reference to this dictionary
		 */
		ChdDictionary& operator=( ChdDictionary&& other ) noexcept;

		//-------------------------------------------------------------------
		// Public Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Access value by key with validation
		 * @param key The key to look up
		 * @return Reference to the associated value
		 * @throws std::out_of_range if key not found or dictionary is empty
		 */
		TValue& operator[]( std::string_view key );

		/**
		 * @brief Access value by key with validation (const version)
		 * @param key The key to look up
		 * @return Const reference to the associated value
		 * @throws std::out_of_range if key not found or dictionary is empty
		 */
		const TValue& operator[]( std::string_view key ) const;

		/**
		 * @brief Try to get a value by key without exceptions
		 * @param key The key to look up
		 * @param[out] value Pointer to store the value if found
		 * @return true if key was found, false otherwise
		 */
		bool tryGetValue( std::string_view key, TValue* value ) const;

		/**
		 * @brief Check if dictionary is empty
		 * @return true if dictionary contains no elements, false otherwise
		 */
		bool isEmpty() const;

		/**
		 * @brief Get iterator to the first element
		 * @return Iterator positioned at the first element
		 */
		Iterator begin() const;

		/**
		 * @brief Get iterator to the end position
		 * @return Iterator positioned after the last element
		 */
		Iterator end() const;

		/**
		 * @brief Get enumerator for the dictionary
		 * @return Enumerator for iterating dictionary contents
		 */
		Enumerator enumerator() const;

	private:
		//-------------------------------------------------------------------
		// Private Methods
		//-------------------------------------------------------------------

		/**
		 * @brief Process a single byte through the hash function
		 * @param hash Current hash value
		 * @param byte Byte to process
		 * @return Updated hash value
		 */
		static uint32_t processHashByte( uint32_t hash, uint8_t byte );

		/**
		 * @brief Calculate hash value for a string key
		 * @param key The string key to hash
		 * @return The calculated hash value
		 */
		static uint32_t hash( std::string_view key );

		/**
		 * @brief Compare two strings for equality
		 * @param a First string (as string_view)
		 * @param b Second string
		 * @return true if strings are equal, false otherwise
		 */
		[[nodiscard]] static bool stringsEqual( std::string_view a, const std::string& b ) noexcept;

		/**
		 * @brief Compare two character spans for equality
		 * @param a First character span
		 * @param b Second character span
		 * @return true if spans contain identical characters, false otherwise
		 * @note This overload is useful for comparing raw character arrays
		 *       or memory buffers without constructing string objects
		 */
		[[nodiscard]] static bool stringsEqual( std::span<const char> a, std::span<const char> b ) noexcept;

		//-------------------------------------------------------------------
		// Private Member Variables
		//-------------------------------------------------------------------

		/** @brief The primary storage table for key-value pairs */
		std::vector<std::pair<std::string, TValue>> m_table;

		/** @brief Seeds for the perfect hash function */
		std::vector<int> m_seeds;

		//-------------------------------------------------------------------
		// Caching and Performance Monitoring
		//-------------------------------------------------------------------

		/**  @brief Cache structure for faster hash lookups */
		struct HashCacheEntry
		{
			std::string_view key;
			uint32_t hash;
		};

		/** @brief Thread-local cache for faster hash lookups */
		alignas( 64 ) static thread_local std::array<HashCacheEntry, internal::HASH_CACHE_SIZE> s_hashCache;

		/** @brief Storage for string keys to ensure string_views remain valid */
		alignas( 64 ) static thread_local std::array<std::string, internal::HASH_CACHE_SIZE> s_hashCacheStorage;

		/** @brief Counter for cache hits (for performance monitoring) */
		static thread_local size_t s_cacheHits;

		/** @brief Counter for cache misses (for performance monitoring) */
		static thread_local size_t s_cacheMisses;

		/** @brief Counter for total lookups (for performance monitoring) */
		static thread_local size_t s_lookupCount;

		/** @brief Counter for successful lookups (for performance monitoring) */
		static thread_local size_t s_lookupHits;
	};
}

#include "ChdDictionary.hpp"
