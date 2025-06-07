/**
 * @file ChdDictionary.inl
 * @brief Template implementation of CHD Dictionary class
 */

#include "ChdDictionary.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Constants
	//=====================================================================

	static constexpr uint32_t MAX_SEED_SEARCH_MULTIPLIER = 100;

	//=====================================================================
	// ChdDictionary class
	//=====================================================================

	//----------------------------------------------
	// ChdDictionary::Iterator
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
			std::string errorMsg = fmt::format(
				"Iterator: Dereference out of bounds (index: {}, table size: {})", m_index, m_table->size() );
			throw std::runtime_error( errorMsg );
		}
		return ( *m_table )[m_index];
	}

	template <typename TValue>
	typename ChdDictionary<TValue>::Iterator::pointer ChdDictionary<TValue>::Iterator::operator->() const
	{
		if ( m_index >= m_table->size() )
		{
			std::string errorMsg = fmt::format(
				"Iterator: Arrow operator out of bounds (index: {}, table size: {})", m_index, m_table->size() );
			throw std::runtime_error( errorMsg );
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
	// Construction / destruction
	//----------------------------------------------

	template <typename TValue>
	ChdDictionary<TValue>::ChdDictionary( std::vector<std::pair<std::string, TValue>> items )
		: m_table{},
		  m_seeds{}
	{
		if ( items.empty() )
		{
			return;
		}

		std::unordered_set<std::string_view> unique_keys;
		unique_keys.reserve( items.size() );

		for ( size_t i = 0; i < items.size(); ++i )
		{
			const auto& [key, value] = items[i];

			if ( key.empty() )
			{
				std::string errorMsg = fmt::format( "Input item at index {} has an empty key, which is not allowed.", i );
				throw std::invalid_argument( errorMsg );
			}

			auto [_, inserted] = unique_keys.insert( key );
			if ( !inserted )
			{
				std::string errorMsg = fmt::format( "Duplicate key found in input items: '{}' at index {}. "
													"CHD dictionaries require unique keys.",
					key, i );
				throw std::invalid_argument( errorMsg );
			}
		}

		uint64_t size{ 1 };
		/* Ensure table size is a power of 2 and at least 2x item count for efficient modulo operations (using '&') */
		while ( size < items.size() )
		{
			size *= 2;
		}
		size *= 2;

		auto hashBuckets{ std::vector<std::vector<std::pair<unsigned, uint32_t>>>( size ) };
		for ( auto& bucket : hashBuckets )
		{
			bucket.reserve( 4 );
		}

		for ( size_t i{ 0 }; i < items.size(); ++i )
		{
			const auto& key{ items[i].first };
			uint32_t hashValue{ hash( key ) };
			auto bucketForItemIdx{ hashValue & ( size - 1 ) };
			hashBuckets[bucketForItemIdx].emplace_back( static_cast<unsigned int>( i + 1 ), hashValue );
		}

		std::sort( hashBuckets.begin(), hashBuckets.end(), []( const auto& a, const auto& b ) {
			return a.size() > b.size();
		} );

		/* Process buckets with the most items (highest collision potential) first */
		auto indices{ std::vector<unsigned int>( size, 0 ) };
		auto seeds{ std::vector<int>( size, 0 ) };

		size_t currentBucketIdx{ 0 };
		for ( ; currentBucketIdx < hashBuckets.size() && hashBuckets[currentBucketIdx].size() > 1; ++currentBucketIdx )
		{
			const auto& subKeys{ hashBuckets[currentBucketIdx] };
			auto entries{ std::unordered_map<size_t, unsigned>() };
			entries.reserve( subKeys.size() );
			uint32_t currentSeedValue{ 0 };

			while ( true )
			{
				++currentSeedValue;
				entries.clear();
				bool seedValid{ true };

				for ( const auto& k : subKeys )
				{
					/* Calculate final position using secondary hash with current seed */
					auto finalHash{ internal::Hashing::seed( currentSeedValue, k.second, size ) };
					bool slotOccupied = indices[finalHash] != 0;
					bool entryClaimedThisTry = entries.count( finalHash ) != 0;

					if ( !slotOccupied && !entryClaimedThisTry )
					{
						entries[finalHash] = k.first;
					}
					else
					{
						seedValid = false;
						break;
					}
				}

				if ( seedValid )
				{
					break;
				}

				if ( currentSeedValue > size * MAX_SEED_SEARCH_MULTIPLIER )
				{
					std::string errorMsg = fmt::format( "Bucket {}: Seed search exceeded threshold ({}), aborting construction!", currentBucketIdx, currentSeedValue );
					throw std::runtime_error( errorMsg );
				}
			}

			if ( !entries.empty() )
			{
				for ( const auto& [finalHash, itemIdx] : entries )
				{
					indices[finalHash] = itemIdx;
				}
				seeds[subKeys[0].second & ( size - 1 )] = static_cast<int>( currentSeedValue );
			}
			else
			{
				std::string errorMsg = fmt::format(
					"Bucket {}: Failed to populate entries despite finding seed {}. "
					"This indicates a serious bug in the CHD algorithm implementation. "
					"SubKeys size: {}, entries size: {}",
					currentBucketIdx, currentSeedValue, subKeys.size(), entries.size() );

				throw std::runtime_error( errorMsg );
			}
		}

		/* Resizes m_table to 'size' elements, initializing new slots with an empty string key and a value copied from the first input item. */
		m_table.resize( size, { std::string(), items[0].second } );

		std::vector<size_t> freeSlots;
		freeSlots.reserve( size );

		for ( size_t i{ 0 }; i < indices.size(); ++i )
		{
			if ( indices[i] != 0 )
			{
				auto itemIndex = indices[i] - 1;
				if ( itemIndex < items.size() )
				{
					m_table[i] = std::move( items[itemIndex] );
				}
				else
				{
					std::string errorMsg = fmt::format(
						"ChdDictionary constructor: Invalid item index {} (adjusted: {}) from 'indices' "
						"for items.size() {}. This indicates a serious bug in CHD construction.",
						indices[i], itemIndex, items.size() );

					throw std::runtime_error( errorMsg );
				}
			}
			else
			{
				freeSlots.push_back( i );
			}
		}

		size_t freeSlotsIndex{ 0 };
		for ( ; currentBucketIdx < hashBuckets.size() && !hashBuckets[currentBucketIdx].empty(); ++currentBucketIdx )
		{
			const auto& k{ hashBuckets[currentBucketIdx][0] };
			if ( freeSlotsIndex < freeSlots.size() )
			{
				auto slotIndexInMTable{ freeSlots[freeSlotsIndex++] };
				auto itemIndex = k.first - 1;
				if ( itemIndex < items.size() )
				{
					m_table[slotIndexInMTable] = std::move( items[itemIndex] );
				}
				else
				{
					std::string errorMsg = fmt::format(
						"ChdDictionary constructor: Invalid item index {} (adjusted: {}) from hashBucket "
						"for items.size() {}. This indicates a serious bug in CHD construction.",
						k.first, itemIndex, items.size() );

					throw std::runtime_error( errorMsg );
				}

				/* Use negative seed to directly encode the final table index for single-item buckets */
				seeds[k.second & ( size - 1 )] = -static_cast<int>( slotIndexInMTable + 1 );
			}
			else
			{
				std::string errorMsg = fmt::format(
					"CHD construction failed: Ran out of free slots for single-item buckets! "
					"Required: {}, Available: {}. This indicates a serious algorithm bug.",
					currentBucketIdx - ( hashBuckets.size() - std::count_if( hashBuckets.begin(), hashBuckets.end(),
																  []( const auto& bucket ) { return bucket.size() <= 1; } ) ),
					freeSlots.size() );

				throw std::runtime_error( errorMsg );
			}
		}

		m_seeds = std::move( seeds );
	}

	//----------------------------------------------
	// Lookup operators
	//----------------------------------------------

	template <typename TValue>
	TValue& ChdDictionary<TValue>::operator[]( std::string_view key )
	{
		if ( isEmpty() )
		{
			throw std::out_of_range( "Key not found in dictionary: ChdDictionary is empty." );
		}

		const TValue* outValue = nullptr;
		if ( tryGetValue( key, outValue ) && outValue != nullptr )
		{
			return const_cast<TValue&>( *outValue );
		}

		throw std::out_of_range( "Key not found in dictionary: " + std::string( key ) );
	}

	//----------------------------------------------
	// Lookup methods
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
	__forceinline bool ChdDictionary<TValue>::tryGetValue( std::string_view key, const TValue*& outValue ) const
	{
		outValue = nullptr;

		if ( key.empty() ) [[unlikely]]
		{
			return false;
		}

		if ( m_table.empty() ) [[unlikely]]
		{
			return false;
		}

		const uint32_t hashValue = hash( key );
		const uint32_t tableSize = static_cast<uint32_t>( m_table.size() );
		const uint32_t index = hashValue & ( tableSize - 1 );
		const int seed = m_seeds[index];

		size_t finalIndex;
		if ( seed < 0 ) [[likely]]
		{
			finalIndex = static_cast<size_t>( -seed - 1 );
		}
		else [[unlikely]]
		{
			finalIndex = static_cast<size_t>( internal::Hashing::seed(
				static_cast<uint32_t>( seed ), hashValue,
				static_cast<uint64_t>( tableSize ) ) );
		}

		const auto& kvp = m_table[finalIndex];

		if ( key != kvp.first ) [[unlikely]]
		{
			return false;
		}

		outValue = &kvp.second;

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
		for ( size_t i{ 0 }; i < m_table.size(); ++i )
		{
			if ( !m_table[i].first.empty() )
			{
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
	// Private helper methods
	//----------------------------------------------

	//---------------------------
	// Hashing
	//---------------------------

	template <typename TValue>
	uint32_t ChdDictionary<TValue>::hash( std::string_view key ) noexcept
	{
		if ( key.empty() )
		{
			return internal::FNV_OFFSET_BASIS;
		}

		uint32_t hashValue = internal::FNV_OFFSET_BASIS;
		static const bool hasSSE42 = internal::hasSSE42Support();

		if ( hasSSE42 )
		{
			for ( const char ch : key )
			{
				hashValue = internal::Hashing::crc32( hashValue, static_cast<uint8_t>( ch ) );
			}
		}
		else
		{
			for ( const char ch : key )
			{
				hashValue = internal::Hashing::fnv1a( hashValue, static_cast<uint8_t>( ch ) );
			}
		}

		return hashValue;
	}
}
