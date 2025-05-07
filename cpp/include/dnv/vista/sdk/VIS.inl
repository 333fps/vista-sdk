/**
 * @file VIS.inl
 * @brief Template implementations for VIS class.
 */

#pragma once

#include "VIS.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Caching
	//=====================================================================

	//----------------------------------------------
	// Construction / Destruction
	//----------------------------------------------

	template <typename K, typename V>
	inline VIS::Cache<K, V>::Cache()
		: m_lastCleanup{ std::chrono::steady_clock::now() }
	{
	}

	//------------------------------------------
	// Public Methods
	//------------------------------------------

	template <typename K, typename V>
	inline V& VIS::Cache<K, V>::getOrCreate( const K& key, std::function<V()> factory ) const
	{
		const auto now = std::chrono::steady_clock::now();

		std::scoped_lock lock( m_mutex );

		if ( now - m_lastCleanup > std::chrono::hours( 1 ) )
		{
			cleanupCache();
			m_lastCleanup = now;
		}

		auto it = m_cache.find( key );
		if ( it == m_cache.end() )
		{
			if ( m_cache.size() >= 10 )
			{
				removeOldestEntry();
			}

			auto result = factory();
			auto [inserted_it, success] = m_cache.emplace( key, CacheItem{ std::move( result ), now } );
			SPDLOG_TRACE( "Cache miss for key. Created and inserted." );

			return inserted_it->second.value;
		}

		it->second.lastAccess = now;

		SPDLOG_TRACE( "Cache hit for key." );

		return it->second.value;
	}

	//------------------------------------------
	// Private Methods
	//------------------------------------------

	template <typename K, typename V>
	inline void VIS::Cache<K, V>::cleanupCache() const
	{
		const auto now = std::chrono::steady_clock::now();
		for ( auto it = m_cache.begin(); it != m_cache.end(); )
		{
			if ( now - it->second.lastAccess > std::chrono::hours( 1 ) )
				it = m_cache.erase( it );
			else
				++it;
		}

		SPDLOG_TRACE( "Cache cleanup performed." );
	}

	template <typename K, typename V>
	inline void VIS::Cache<K, V>::removeOldestEntry() const
	{
		if ( m_cache.empty() )
			return;

		auto oldest = m_cache.begin();
		for ( auto it = std::next( m_cache.begin() ); it != m_cache.end(); ++it )
		{
			if ( it->second.lastAccess < oldest->second.lastAccess )
			{
				oldest = it;
			}
		}
		m_cache.erase( oldest );

		SPDLOG_TRACE( "Cache eviction performed (removed oldest)." );
	}
}
