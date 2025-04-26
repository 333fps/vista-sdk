/**
 * @file ChdDictionary.hpp
 * @brief Template implementation of CHD Dictionary class
 */

#pragma once

#include "ChdDictionary.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Hashing Implementation
	//-------------------------------------------------------------------

	template <typename TValue>
	uint32_t ChdDictionary<TValue>::hash( std::string_view key )
	{
		if ( key.empty() )
		{
			return internal::FNV_OFFSET_BASIS;
		}

		auto cacheIndex = ( ( static_cast<size_t>( key[0] ) * 23 ) ^ ( key.length() * 37 ) ^ ( key.back() * 41 ) ) % s_hashCache.size();
		if ( s_hashCache[cacheIndex].key == key )
		{
			++s_cacheHits;
			SPDLOG_DEBUG( "Hash cache hit for '{}' (hits: {}, rate: {:.1f}%)", key, s_cacheHits, 100.0f * static_cast<float>( s_cacheHits ) / static_cast<float>( s_cacheHits + s_cacheMisses ) );

			return s_hashCache[cacheIndex].hash;
		}
		++s_cacheMisses;

		if ( s_cacheMisses % 1000 == 0 )
		{
			SPDLOG_DEBUG( "Hash cache performance: {} hits, {} misses, {:.1f}% hit rate", s_cacheHits, s_cacheMisses, 100.0f * static_cast<float>( s_cacheHits ) / static_cast<float>( s_cacheHits + s_cacheMisses ) );
		}

		auto length{ key.length() * sizeof( char ) };
		auto hashValue{ internal::FNV_OFFSET_BASIS };
		auto data{ key.data() };

		for ( size_t i = 0; i < length; i += 2 )
		{
			hashValue = processHashByte( hashValue, static_cast<uint8_t>( data[i] ) );
			if ( i + 1 < length )
			{
				hashValue = processHashByte( hashValue, static_cast<uint8_t>( data[i + 1] ) );
			}
		}

		s_hashCacheStorage[cacheIndex].assign( key.data(), key.size() );
		s_hashCache[cacheIndex] = HashCacheEntry{ .key = s_hashCacheStorage[cacheIndex], .hash = hashValue };

		SPDLOG_TRACE( "Calculated hash for key '{}': {}", key, hashValue );
		SPDLOG_TRACE( "Hash cache updated at index {}: key='{}', hash={}", cacheIndex, s_hashCache[cacheIndex].first, s_hashCache[cacheIndex].second );

		return hashValue;
	}

	template <typename TValue>
	uint32_t ChdDictionary<TValue>::processHashByte( uint32_t hash, uint8_t byte )
	{
		static const auto hasSSE42Support{ internal::hasSSE42Support() };

		return hasSSE42Support ? internal::Hashing::crc32( hash, byte ) : internal::Hashing::fnv1a( hash, byte );
	}

	//-------------------------------------------------------------------
	// Construction Implementation
	//-------------------------------------------------------------------

	template <typename TValue>
	ChdDictionary<TValue>::ChdDictionary()
	{
	}

	template <typename TValue>
	ChdDictionary<TValue>::ChdDictionary( const std::vector<std::pair<std::string, TValue>>& items )
	{
		auto start = std::chrono::high_resolution_clock::now();

		if ( items.empty() )
		{
			SPDLOG_DEBUG( "Created empty CHD Dictionary" );
			return;
		}

		uint64_t size{ 1 };
		while ( size < items.size() )
		{
			size *= 2;
		}
		size *= 2;

		SPDLOG_INFO( "Building CHD dictionary with {} items, table size {}", items.size(), size );

		m_table.reserve( size );
		m_seeds.reserve( size );

		auto hashBuckets{ std::vector<std::vector<std::pair<unsigned, uint32_t>>>( size ) };
		for ( auto& bucket : hashBuckets )
		{
			bucket.reserve( 8 );
		}

		for ( size_t i{ 0 }; i < items.size(); ++i )
		{
			const auto& key{ items[i].first };
			auto hashValue{ hash( key ) };
			auto index{ hashValue & ( size - 1 ) };
			hashBuckets[index].push_back( { static_cast<int>( i + 1 ), hashValue } );
		}

		std::sort( hashBuckets.begin(), hashBuckets.end(), []( const auto& a, const auto& b ) {
			return a.size() > b.size();
		} );

		{
			size_t collisionCount{ 0 };
			size_t maxCollisions{ 0 };
			size_t bucketsWithCollisions{ 0 };

			for ( const auto& bucket : hashBuckets )
			{
				if ( bucket.size() > 1 )
				{
					++bucketsWithCollisions;
					collisionCount += bucket.size() - 1;
					maxCollisions = std::max( maxCollisions, bucket.size() );
				}
			}

			SPDLOG_INFO( "CHD dictionary collision stats: {} items, {} total collisions, {} buckets with collisions, max collision chain: {}", items.size(), collisionCount, bucketsWithCollisions, maxCollisions );
		}

		auto indices{ std::vector<unsigned int>( size, 0 ) };
		auto seeds{ std::vector<int>( size, 0 ) };

		size_t index{ 0 };
		for ( ; index < hashBuckets.size() && hashBuckets[index].size() > 1; ++index )
		{
			const auto& subKeys{ hashBuckets[index] };
			auto entries{ std::unordered_map<uint32_t, unsigned>() };
			entries.reserve( subKeys.size() );
			uint32_t seed{ 0 };

			while ( true )
			{
				++seed;
				entries.clear();
				bool seedValid{ true };

				SPDLOG_TRACE( "Trying seed {} for bucket {}", seed, index );

				for ( const auto& k : subKeys )
				{
					auto hash{ uint32_t{ internal::Hashing::seed( seed, k.second, size ) } };

					if ( entries.find( hash ) == entries.end() && indices[hash] == 0 )
					{
						entries[hash] = k.first;
					}
					else
					{
						seedValid = false;
						break;
					}
				}

				if ( seedValid )
				{
					SPDLOG_TRACE( "Found valid seed {} for bucket {}", seed, index );
					break;
				}
			}

			for ( const auto& [hash, idx] : entries )
			{
				indices[hash] = idx;
			}

			seeds[subKeys[0].second & ( size - 1 )] = static_cast<int>( seed );
		}

		m_table.resize( size );
		std::vector<size_t> free{};

		for ( size_t i{ 0 }; i < indices.size(); ++i )
		{
			if ( indices[i] == 0 )
			{
				free.push_back( i );
			}
			else
			{
				--indices[i];
				m_table[i] = items[indices[i]];
			}
		}

		size_t freeIndex{ 0 };
		for ( size_t i{ 0 }; index < hashBuckets.size() && hashBuckets[index].size() > 0; ++i, ++index )
		{
			const auto& k{ hashBuckets[index][0] };
			if ( freeIndex < free.size() )
			{
				auto slotIndex{ free[freeIndex++] };
				m_table[slotIndex] = items[k.first - 1];
				seeds[k.second & ( size - 1 )] = -static_cast<int>( slotIndex + 1 );
			}
		}

		m_seeds = std::move( seeds );

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration<double, std::milli>( end - start ).count();

		SPDLOG_INFO( "CHD Dictionary construction complete: {} entries, {} seeds in {:.2f}ms", m_table.size(), m_seeds.size(), duration );

		auto memoryUsage = sizeof( decltype( m_table )::value_type ) * m_table.capacity() + sizeof( int ) * m_seeds.capacity();
		SPDLOG_INFO( "CHD Dictionary memory usage: {:.2f}KB ({:.2f}MB) ({} table entries, {} seeds)", static_cast<float>( memoryUsage ) / 1024.0f, static_cast<float>( memoryUsage ) / ( 1024.0f * 1024.0f ), m_table.size(), m_seeds.size() );
	}

	//-------------------------------------------------------------------
	// Copy and Move Operations
	//-------------------------------------------------------------------

	template <typename TValue>
	ChdDictionary<TValue>::ChdDictionary( const ChdDictionary<TValue>& other )
		: m_table{ other.m_table },
		  m_seeds{ other.m_seeds }
	{
		SPDLOG_DEBUG( "ChdDictionary copied with {} entries ({} seeds)", m_table.size(), m_seeds.size() );
	}

	template <typename TValue>
	ChdDictionary<TValue>::ChdDictionary( ChdDictionary<TValue>&& other ) noexcept
		: m_table{ std::move( other.m_table ) },
		  m_seeds{ std::move( other.m_seeds ) }
	{
		SPDLOG_DEBUG( "ChdDictionary moved with {} entries ({} seeds)", m_table.size(), m_seeds.size() );
	}

	template <typename TValue>
	ChdDictionary<TValue>& ChdDictionary<TValue>::operator=( const ChdDictionary<TValue>& other )
	{
		if ( this != &other )
		{
			m_table = other.m_table;
			m_seeds = other.m_seeds;
		}

		SPDLOG_DEBUG( "ChdDictionary copy-assigned with {} entries ({} seeds)", m_table.size(), m_seeds.size() );

		return *this;
	}

	template <typename TValue>
	ChdDictionary<TValue>& ChdDictionary<TValue>::operator=( ChdDictionary<TValue>&& other ) noexcept
	{
		if ( this != &other )
		{
			m_table = std::move( other.m_table );
			m_seeds = std::move( other.m_seeds );
		}

		SPDLOG_DEBUG( "ChdDictionary move-assigned with {} entries ({} seeds)", m_table.size(), m_seeds.size() );

		return *this;
	}

	//-------------------------------------------------------------------
	// Lookup Implementation
	//-------------------------------------------------------------------

	template <typename TValue>
	TValue& ChdDictionary<TValue>::operator[]( std::string_view key )
	{
		if ( isEmpty() )
		{
			SPDLOG_ERROR( "Attempted to access empty dictionary with key '{}'", key );
			internal::ThrowHelper::throwInvalidOperationException();
		}

		TValue* value{ nullptr };
		if ( !tryGetValue( key, value ) )
		{
			SPDLOG_ERROR( "Key '{}' not found in dictionary", key );
			internal::ThrowHelper::throwKeyNotFoundException( key );
		}

		return *value;
	}

	template <typename TValue>
	const TValue& ChdDictionary<TValue>::operator[]( std::string_view key ) const
	{
		if ( isEmpty() )
		{
			SPDLOG_ERROR( "Attempted to access empty dictionary with key '{}'", key );
			internal::ThrowHelper::throwInvalidOperationException();
		}

		TValue* value{ nullptr };
		if ( !tryGetValue( key, value ) )
		{
			SPDLOG_ERROR( "Key '{}' not found in dictionary", key );
			internal::ThrowHelper::throwKeyNotFoundException( key );
		}

		return *value;
	}

	template <typename TValue>
	bool ChdDictionary<TValue>::tryGetValue( std::string_view key, TValue* value ) const
	{
		auto start = std::chrono::high_resolution_clock::now();

		if ( key.empty() || m_table.empty() || m_seeds.empty() )
		{
			SPDLOG_TRACE( "Skipped lookup: empty key or dictionary" );
			return false;
		}

		++s_lookupCount;
		if ( s_lookupCount % 10000 == 0 )
		{
			SPDLOG_INFO( "Dictionary performance: {} lookups, hit rate: {:.1f}%", s_lookupCount, 100.0f * static_cast<float>( s_lookupHits ) / static_cast<float>( s_lookupCount ) );
		}

		auto hashValue{ hash( key ) };
		auto size{ m_table.size() };
		auto index{ hashValue & ( size - 1 ) };
		int seed{ m_seeds[index] };

		SPDLOG_TRACE( "Key: '{}', Hash: {}, Index: {}, Seed: {}", key, hashValue, index, seed );

		const std::pair<std::string, TValue>* kvp{ nullptr };
		if ( seed < 0 )
		{
			kvp = &m_table[static_cast<size_t>( -seed - 1 )];
		}
		else
		{
			index = internal::Hashing::seed( static_cast<uint32_t>( seed ), hashValue, size );
			kvp = &m_table[index];
		}

		if ( !stringsEqual( key, kvp->first ) )
		{
			return false;
		}

		if ( value )
		{
			*value = kvp->second;
		}

		++s_lookupHits;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration<double, std::milli>( end - start ).count();

		if ( s_lookupCount % 100000 == 0 )
		{
			auto avgTime = static_cast<float>( duration ) / static_cast<float>( s_lookupCount );
			SPDLOG_INFO( "Dictionary lookup stats: avg time {:.3f}μs, hit rate {:.1f}%", avgTime, 100.0f * static_cast<float>( s_lookupHits ) / static_cast<float>( duration ) );
		}

		return true;
	}

	template <typename TValue>
	bool ChdDictionary<TValue>::isEmpty() const
	{
		return m_table.empty() || m_seeds.empty();
	}

	//-------------------------------------------------------------------
	// Iterator Implementation
	//-------------------------------------------------------------------

	template <typename TValue>
	ChdDictionary<TValue>::Iterator::Iterator( const std::vector<std::pair<std::string, TValue>>* table, size_t index )
		: m_table{ table }, m_index{ index }
	{
		if ( m_table != nullptr && index == 0 )
		{
			while ( m_index < m_table->size() && ( *m_table )[m_index].first.empty() )
			{
				++m_index;
			}
		}
	}

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator::reference
	ChdDictionary<TValue>::Iterator::operator*() const
	{
		if ( static_cast<size_t>( m_index ) >= m_table->size() )
		{
			internal::ThrowHelper::throwInvalidOperationException();
		}

		return ( *m_table )[m_index];
	}

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator::pointer
	ChdDictionary<TValue>::Iterator::operator->() const
	{
		if ( static_cast<size_t>( m_index ) >= m_table->size() )
		{
			internal::ThrowHelper::throwInvalidOperationException();
		}

		return &( ( *m_table )[m_index] );
	}

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator&
	ChdDictionary<TValue>::Iterator::operator++()
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
	typename ChdDictionary<TValue>::Iterator
	ChdDictionary<TValue>::Iterator::operator++( int )
	{
		auto tmp{ Iterator{ *this } };
		++( *this );
		return tmp;
	}

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

	template <typename TValue>
	void ChdDictionary<TValue>::Iterator::reset()
	{
		m_index = std::numeric_limits<size_t>::max();
	}

	//-------------------------------------------------------------------
	// Iteration Support
	//-------------------------------------------------------------------

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

	template <typename TValue>
	typename ChdDictionary<TValue>::Enumerator ChdDictionary<TValue>::enumerator() const
	{
		return begin();
	}

	//-------------------------------------------------------------------
	// Utility Functions
	//-------------------------------------------------------------------

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

	//-------------------------------------------------------------------
	// Caching and Performance Monitoring
	//-------------------------------------------------------------------

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
}
