/**
 * @file ChdDictionary.inl
 * @brief Template implementation of CHD Dictionary class
 */

#pragma once

#include "ChdDictionary.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// ChdDictionary Class
	//=====================================================================

	//----------------------------------------------
	// Iterator Inner Class
	//----------------------------------------------

	//---------------------------
	// Construction
	//---------------------------

	template <typename TValue>
	ChdDictionary<TValue>::Iterator::Iterator( const std::vector<std::pair<std::string, TValue>>* table, size_t index )
		: m_table{ table },
		  m_index{ index }
	{
	}

	//---------------------------
	// Operations
	//---------------------------

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator::reference ChdDictionary<TValue>::Iterator::operator*() const
	{
		if ( m_index >= m_table->size() )
		{
			SPDLOG_ERROR( "Iterator: Dereference out of bounds (index: {}, table size: {}).", m_index, m_table->size() );
			throw std::logic_error( "Invalid operation" );
		}

		return ( *m_table )[m_index];
	}

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator::pointer ChdDictionary<TValue>::Iterator::operator->() const
	{
		if ( m_index >= m_table->size() )
		{
			SPDLOG_ERROR( "Iterator: Arrow operator out of bounds (index: {}, table size: {}).", m_index, m_table->size() );
			throw std::logic_error( "Invalid operation" );
		}
		return &( ( *m_table )[m_index] );
	}

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator& ChdDictionary<TValue>::Iterator::operator++()
	{
		if ( m_table == nullptr )
		{
			return *this;
		}

		while ( ++m_index < m_table->size() )
		{
			const auto& entry{ ( *m_table )[m_index] };
			if ( !entry.first.empty() )
			{
				return *this;
			}
		}

		m_index = m_table->size();

		return *this;
	}

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator ChdDictionary<TValue>::Iterator::operator++( int )
	{
		auto tmp{ Iterator{ *this } };
		++( *this );
		return tmp;
	}

	//---------------------------
	// Comparison
	//---------------------------

	template <typename TValue>
	bool ChdDictionary<TValue>::Iterator::operator==( const Iterator& other ) const
	{
		return m_table == other.m_table && m_index == other.m_index;
	}

	template <typename TValue>
	bool ChdDictionary<TValue>::Iterator::operator!=( const Iterator& other ) const
	{
		return !( *this == other );
	}

	//---------------------------
	// Utility
	//---------------------------

	template <typename TValue>
	void ChdDictionary<TValue>::Iterator::reset()
	{
		m_index = std::numeric_limits<size_t>::max();
	}

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	template <typename TValue>
	ChdDictionary<TValue>::ChdDictionary( std::vector<std::pair<std::string, TValue>> items )
		: m_table{},
		  m_seeds{}
	{
		auto start = std::chrono::high_resolution_clock::now();

		if ( items.empty() )
		{
			SPDLOG_DEBUG( "Created empty CHD Dictionary from empty input" );
			return;
		}

		uint64_t size{ 1 };
		/* Ensure table size is a power of 2 and at least 2x item count for efficient modulo operations (using '&') */
		while ( size < items.size() )
		{
			size *= 2;
		}
		size *= 2;

		SPDLOG_INFO( "Building CHD dictionary with {} items, table size {}", items.size(), size );

		auto hashBuckets{ std::vector<std::vector<std::pair<unsigned, uint32_t>>>( size ) };
		for ( auto& bucket : hashBuckets )
		{
			bucket.reserve( 4 );
		}

		for ( size_t i{ 0 }; i < items.size(); ++i )
		{
			const auto& key{ items[i].first };
			uint32_t hashValue{ hash( key ) };
			auto index{ hashValue & ( size - 1 ) };
			hashBuckets[index].emplace_back( static_cast<unsigned int>( i + 1 ), hashValue );
		}

		std::sort( hashBuckets.begin(), hashBuckets.end(), []( const auto& a, const auto& b ) {
			return a.size() > b.size();
		} );

		/* Process buckets with the most items (highest collision potential) first */
		auto indices{ std::vector<unsigned int>( size, 0 ) };
		auto seeds{ std::vector<int>( size, 0 ) };

		size_t index{ 0 };
		for ( ; index < hashBuckets.size() && hashBuckets[index].size() > 1; ++index )
		{
			const auto& subKeys{ hashBuckets[index] };
			auto entries{ std::unordered_map<size_t, unsigned>() };
			entries.reserve( subKeys.size() );
			uint32_t seed{ 0 };

			SPDLOG_TRACE( "Bucket {}: Starting seed search for {} items", index, subKeys.size() );

			while ( true )
			{
				++seed;
				SPDLOG_TRACE( "Bucket {}: Trying seed {}", index, seed );
				entries.clear();
				bool seedValid{ true };

				for ( const auto& k : subKeys )
				{
					/* Calculate final position using secondary hash with current seed */
					auto finalHash{ internal::Hashing::seed( seed, k.second, size ) };
					bool slotOccupied = indices[finalHash] != 0;
					bool entryClaimed = entries.find( finalHash ) != entries.end();

					if ( !slotOccupied && !entryClaimed )
					{
						entries[finalHash] = k.first;
					}
					else
					{
						SPDLOG_TRACE( "Bucket {}: Seed {} collision for item {} (hash {}) at finalHash {}. Slot occupied: {}, Claimed this trial: {}",
							index, seed, k.first, k.second, finalHash, slotOccupied, entryClaimed );
						seedValid = false;
						break;
					}
				}

				if ( seedValid )
				{
					SPDLOG_TRACE( "Bucket {}: Found valid seed {}", index, seed );
					break;
				}

				if ( seed > size * 100 )
				{
					std::string errorMsg = fmt::format( "Bucket {}: Seed search exceeded threshold ({}), aborting construction!", index, seed );
					SPDLOG_CRITICAL( errorMsg );
					throw std::runtime_error( errorMsg );
					break;
				}
			}

			if ( !entries.empty() )
			{
				for ( const auto& [finalHash, itemIdx] : entries )
				{
					indices[finalHash] = itemIdx;
				}
				seeds[subKeys[0].second & ( size - 1 )] = static_cast<int>( seed );
			}
			else
			{
				std::string errorMsg = fmt::format( "Bucket {}: Failed to populate entries despite finding seed {}.", index, seed );
				SPDLOG_CRITICAL( errorMsg );
				throw std::runtime_error( errorMsg );
			}
		}

		m_table.resize( size );
		std::vector<size_t> freeSlots;
		freeSlots.reserve( size );

		for ( size_t i{ 0 }; i < indices.size(); ++i )
		{
			if ( indices[i] == 0 )
			{
				freeSlots.push_back( i );
			}
			else
			{
				m_table[i] = std::move( items[indices[i] - 1] );
			}
		}

		size_t freeIndex{ 0 };
		for ( ; index < hashBuckets.size() && !hashBuckets[index].empty(); ++index )
		{
			const auto& k{ hashBuckets[index][0] };
			if ( freeIndex < freeSlots.size() )
			{
				auto slotIndex{ freeSlots[freeIndex++] };
				m_table[slotIndex] = std::move( items[k.first - 1] );
				/* Use negative seed to directly encode the final table index for single-item buckets */
				seeds[k.second & ( size - 1 )] = -static_cast<int>( slotIndex + 1 );
			}
			else
			{
				SPDLOG_ERROR( "Ran out of free slots!" );
				break;
			}
		}

		m_seeds = std::move( seeds );

		auto end = std::chrono::high_resolution_clock::now();
		[[maybe_unused]] auto duration = std::chrono::duration<double, std::milli>( end - start ).count();
		SPDLOG_INFO( "CHD Dictionary construction complete: {} entries, {} seeds in {:.2f}ms", m_table.size(), m_seeds.size(), duration );

		[[maybe_unused]] auto memoryUsage = sizeof( typename decltype( m_table )::value_type ) * m_table.capacity() + sizeof( int ) * m_seeds.capacity();
		SPDLOG_INFO( "CHD Dictionary memory usage: {:.2f}KB ({:.2f}MB) ({} table entries, {} seeds)", static_cast<float>( memoryUsage ) / 1024.0f, static_cast<float>( memoryUsage ) / ( 1024.0f * 1024.0f ), m_table.size(), m_seeds.size() );
	}

	//----------------------------------------------
	// Lookup Operators
	//----------------------------------------------

	template <typename TValue>
	TValue& ChdDictionary<TValue>::operator[]( std::string_view key )
	{
		const TValue& const_val_ref = static_cast<const ChdDictionary<TValue>*>( this )->at( key );
		return const_cast<TValue&>( const_val_ref );
	}

	//----------------------------------------------
	// Lookup Methods
	//----------------------------------------------

	template <typename TValue>
	const TValue& ChdDictionary<TValue>::at( std::string_view key ) const
	{
		if ( isEmpty() )
		{
			throw std::out_of_range( "Key not found in dictionary: ChdDictionary is empty." );
		}

		const TValue* outValue = nullptr;
		if ( tryGetValue( key, outValue ) && outValue != nullptr )
		{
			return *outValue;
		}

		throw std::out_of_range( "Key not found in dictionary: " + std::string( key ) );
	}

	template <typename TValue>
	bool ChdDictionary<TValue>::tryGetValue( std::string_view key, const TValue*& outValue ) const
	{
		auto start = std::chrono::high_resolution_clock::now();
		outValue = nullptr;

		if ( key.empty() || m_table.empty() || m_seeds.empty() )
		{
			SPDLOG_TRACE( "Skipped lookup: empty key or dictionary" );
			return false;
		}

		++s_lookupCount;
		if ( s_lookupCount % 10000 == 0 )
		{
			SPDLOG_DEBUG( "Dictionary performance: {} lookups, hit rate: {:.1f}%", s_lookupCount, 100.0f * static_cast<float>( s_lookupHits ) / static_cast<float>( s_lookupCount ) );
		}

		uint32_t hashValue{ hash( key ) };
		auto size{ m_table.size() };
		auto index{ hashValue & ( size - 1 ) }; /* Use bitwise AND as fast modulo since size is power of 2 */
		int seed{ m_seeds[index] };

		SPDLOG_TRACE( "Key: '{}', Hash: {}, Index: {}, Seed: {}", key, hashValue, index, seed );

		const std::pair<std::string, TValue>* kvp{ nullptr };
		size_t finalIndex = 0;
		if ( seed < 0 )
		{
			/* Negative seed directly encodes the final index (minus 1) */
			finalIndex = static_cast<size_t>( -seed - 1 );
			if ( finalIndex >= m_table.size() )
			{
				SPDLOG_WARN( "Invalid negative seed index {} for key '{}'", finalIndex, key );

				return false;
			}
			kvp = &m_table[finalIndex];
		}
		else
		{
			/* Positive seed requires secondary hash calculation */
			finalIndex = internal::Hashing::seed( static_cast<uint32_t>( seed ), hashValue, static_cast<uint64_t>( size ) );
			if ( finalIndex >= m_table.size() )
			{
				SPDLOG_WARN( "Invalid positive seed index {} for key '{}'", finalIndex, key );

				return false;
			}
			kvp = &m_table[finalIndex];
		}

		if ( !stringsEqual( key, kvp->first ) )
		{
			SPDLOG_WARN( "Key mismatch at index {}: expected '{}', found '{}'", finalIndex, key, kvp->first );

			auto end = std::chrono::high_resolution_clock::now();
			s_totalLookupDuration += ( end - start );

			return false;
		}

		outValue = &kvp->second;

		++s_lookupHits;

		auto end = std::chrono::high_resolution_clock::now();
		s_totalLookupDuration += ( end - start );

		if ( s_lookupCount > 0 && s_lookupCount % 100000 == 0 )
		{
			[[maybe_unused]] auto avgDurationNs = s_totalLookupDuration / s_lookupCount;
			[[maybe_unused]] auto avgDurationMs = std::chrono::duration<double, std::milli>( avgDurationNs ).count();

			SPDLOG_DEBUG( "Dictionary lookup stats: avg time {:.3f}ms, hit rate {:.1f}%", avgDurationMs, 100.0f * static_cast<float>( s_lookupHits ) / static_cast<float>( s_lookupCount ) );
		}

		return true;
	}

	//----------------------------------------------
	// Capacity
	//----------------------------------------------

	template <typename TValue>
	bool ChdDictionary<TValue>::isEmpty() const
	{
		return m_table.empty();
	}

	template <typename TValue>
	size_t ChdDictionary<TValue>::size() const
	{
		return m_table.size();
	}

	//----------------------------------------------
	// Iteration
	//----------------------------------------------

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator ChdDictionary<TValue>::begin() const
	{
		SPDLOG_TRACE( "Creating iterator - table has {} entries", m_table.size() );

		for ( size_t i{ 0 }; i < m_table.size(); i++ )
		{
			if ( !m_table[i].first.empty() )
			{
				SPDLOG_TRACE( "Found first valid entry at {}: key={}", i, m_table[i].first );

				return Iterator{ &m_table, i };
			}
		}

		return end();
	}

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator ChdDictionary<TValue>::end() const
	{
		return Iterator{ &m_table, m_table.size() };
	}

	//----------------------------------------------
	// Private Helper Methods
	//----------------------------------------------

	//---------------------------
	// Hashing
	//---------------------------

	template <typename TValue>
	uint32_t ChdDictionary<TValue>::processHashByte( uint32_t hash, uint8_t byte )
	{
		static const auto hasSSE42Support{ internal::hasSSE42Support() };

		return hasSSE42Support ? internal::Hashing::crc32( hash, byte ) : internal::Hashing::fnv1a( hash, byte );
	}

	template <typename TValue>
	uint32_t ChdDictionary<TValue>::hash( std::string_view key )
	{
		if ( key.empty() )
		{
			return internal::FNV_OFFSET_BASIS;
		}

		/* Cache lookup - Simple cache index using key length and first/last chars */
		auto cacheIndex = ( ( static_cast<size_t>( key[0] ) * 23 ) ^ ( key.length() * 37 ) ^ ( key.back() * 41 ) ) % s_hashCache.size();
		if ( s_hashCache[cacheIndex].key == key )
		{
			++s_cacheHits;
			if ( s_cacheHits % 1000 == 0 )
			{
				SPDLOG_TRACE( "Hash cache hit for '{}' (hits: {}, rate: {:.1f}%)", key, s_cacheHits, 100.0f * static_cast<float>( s_cacheHits ) / static_cast<float>( s_cacheHits + s_cacheMisses ) );
			}

			return s_hashCache[cacheIndex].hash;
		}
		++s_cacheMisses;

		if ( s_cacheMisses % 1000 == 0 )
		{
			SPDLOG_DEBUG( "Hash cache performance: {} hits, {} misses, {:.1f}% hit rate", s_cacheHits, s_cacheMisses, 100.0f * static_cast<float>( s_cacheHits ) / static_cast<float>( s_cacheHits + s_cacheMisses ) );
		}

		/* Hashing logic */
		auto length{ key.length() };
		auto hashValue{ internal::FNV_OFFSET_BASIS };
		auto data{ key.data() };

		for ( size_t i = 0; i < length; ++i )
		{
			hashValue = processHashByte( hashValue, static_cast<uint8_t>( data[i] ) );
		}

		/* Cache update */
		s_hashCacheStorage[cacheIndex].assign( key.data(), key.size() );
		s_hashCache[cacheIndex] = HashCacheEntry{ .key = s_hashCacheStorage[cacheIndex], .hash = hashValue };

		SPDLOG_TRACE( "Calculated hash for key '{}': {}", key, hashValue );
		if ( s_cacheMisses % 1000 == 0 )
		{
			SPDLOG_TRACE( "Hash cache updated at index {}: key='{}', hash={}", cacheIndex, s_hashCache[cacheIndex].key, s_hashCache[cacheIndex].hash );
		}

		return hashValue;
	}

	//---------------------------
	// Utility
	//---------------------------

	template <typename TValue>
	bool ChdDictionary<TValue>::stringsEqual( std::string_view a, const std::string& b ) noexcept
	{
		const auto aLen{ a.size() };

		if ( aLen != b.size() )
		{
			return false;
		}

		if ( a.empty() )
		{
			return true;
		}

		if ( aLen < 16 )
		{
			for ( size_t i{ 0 }; i < aLen; i++ )
			{
				if ( a[i] != b[i] )
				{
					return false;
				}
			}
			return true;
		}
		else
		{
			return std::memcmp( a.data(), b.data(), aLen ) == 0;
		}
	}

	template <typename TValue>
	bool ChdDictionary<TValue>::stringsEqual( std::span<const char> a, std::span<const char> b ) noexcept
	{
		const auto aLen{ a.size() };

		if ( aLen != b.size() )
		{
			return false;
		}

		if ( aLen == 0 )
		{
			return true;
		}

		if ( aLen < 16 )
		{
			for ( size_t i{ 0 }; i < aLen; ++i )
			{
				if ( a[i] != b[i] )
				{
					return false;
				}
			}
			return true;
		}

		return std::memcmp( a.data(), b.data(), aLen ) == 0;
	}

	//----------------------------------------------
	// Caching and Performance Monitoring
	//----------------------------------------------

	template <typename TValue>
	thread_local std::array<std::string, internal::HASH_CACHE_SIZE> ChdDictionary<TValue>::s_hashCacheStorage{};

	template <typename TValue>
	thread_local std::array<typename ChdDictionary<TValue>::HashCacheEntry, internal::HASH_CACHE_SIZE> ChdDictionary<TValue>::s_hashCache{};

	template <typename TValue>
	thread_local size_t ChdDictionary<TValue>::s_cacheHits = 0;

	template <typename TValue>
	thread_local size_t ChdDictionary<TValue>::s_cacheMisses = 0;

	template <typename TValue>
	thread_local size_t ChdDictionary<TValue>::s_lookupCount = 0;

	template <typename TValue>
	thread_local size_t ChdDictionary<TValue>::s_lookupHits = 0;

	template <typename TValue>
	thread_local std::chrono::nanoseconds ChdDictionary<TValue>::s_totalLookupDuration{ 0 };
}
