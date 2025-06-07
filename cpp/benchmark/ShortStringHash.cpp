/**
 * @file ShortStringHash.cpp
 * @brief Short string hash function performance comparison between different algorithms
 */

#include "pch.h"

#include "dnv/vista/sdk/VIS.h"
#include "dnv/vista/sdk/ChdDictionary.h"

using namespace dnv::vista::sdk;

namespace dnv::vista::sdk::benchmarks
{
	static bool g_initialized = false;

	static void initializeData()
	{
		if ( !g_initialized )
		{
			g_initialized = true;
		}
	}

	template <typename THasher>
	[[nodiscard]] static inline uint32_t hash( std::string_view inputStr ) noexcept
	{
		uint32_t hash = 0x811C9DC5;

		for ( char ch : inputStr )
		{
			const uint8_t lowByte = static_cast<uint8_t>( ch );
			hash = THasher::hash( hash, lowByte );
			hash = THasher::hash( hash, 0 );
		}

		return hash;
	}

	struct LarssonHasher
	{
		[[nodiscard]] static __forceinline uint32_t hash( uint32_t hash, uint8_t ch ) noexcept
		{
			return internal::Hashing::Larsson( hash, ch );
		}
	};

	struct CRC32IntrinsicHasher
	{
		[[nodiscard]] static __forceinline uint32_t hash( uint32_t hash, uint8_t ch ) noexcept
		{
			return internal::Hashing::crc32( hash, ch );
		}
	};

	struct FnvHasher
	{
		[[nodiscard]] static __forceinline uint32_t hash( uint32_t hash, uint8_t ch ) noexcept
		{
			return internal::Hashing::fnv1a( hash, ch );
		}
	};

	static uint32_t hashCodeOrdinal( std::string_view str )
	{
		const char* data = str.data();
		size_t length = str.length();

		constexpr uint32_t Hash1Start = ( 5381 << 16 ) + 5381;
		constexpr uint32_t Factor = 1566083941;

		uint32_t hash1, hash2;

		switch ( length )
		{
			case 0:
				return Hash1Start + ( Hash1Start * Factor );

			case 1:
				hash2 = ( ( Hash1Start << 5 ) + Hash1Start ) ^ static_cast<uint8_t>( data[0] );
				return Hash1Start + ( hash2 * Factor );

			case 2:
				hash2 = ( ( Hash1Start << 5 ) + Hash1Start ) ^ static_cast<uint8_t>( data[0] );
				hash2 = ( ( hash2 << 5 ) + hash2 ) ^ static_cast<uint8_t>( data[1] );
				return Hash1Start + ( hash2 * Factor );

			case 3:
				hash2 = ( ( Hash1Start << 5 ) + Hash1Start ) ^ static_cast<uint8_t>( data[0] );
				hash2 = ( ( hash2 << 5 ) + hash2 ) ^ static_cast<uint8_t>( data[1] );
				hash2 = ( ( hash2 << 5 ) + hash2 ) ^ static_cast<uint8_t>( data[2] );
				return Hash1Start + ( hash2 * Factor );

			default:
				hash1 = Hash1Start;
				hash2 = hash1;

				size_t i = 0;
				while ( i + 3 < length )
				{
					uint32_t chunk1 = *reinterpret_cast<const uint32_t*>( data + i );
					uint32_t chunk2 = *reinterpret_cast<const uint32_t*>( data + i + 2 );
					hash1 = ( ( hash1 << 5 ) + hash1 ) ^ chunk1;
					hash2 = ( ( hash2 << 5 ) + hash2 ) ^ chunk2;
					i += 4;
				}

				while ( i < length )
				{
					hash2 = ( ( hash2 << 5 ) + hash2 ) ^ static_cast<uint8_t>( data[i] );
					++i;
				}

				return hash1 + ( hash2 * Factor );
		}
	}

	static void BM_bcl_400( benchmark::State& state )
	{
		initializeData();

		const std::string input = "400";
		for ( auto _ : state )
		{
			auto result = std::hash<std::string>{}( input );
			benchmark::DoNotOptimize( result );
		}
	}

	static void BM_bcl_H346_11112( benchmark::State& state )
	{
		initializeData();

		const std::string input = "H346.11112";
		for ( auto _ : state )
		{
			auto result = std::hash<std::string>{}( input );
			benchmark::DoNotOptimize( result );
		}
	}

	static void BM_bclOrd_400( benchmark::State& state )
	{
		initializeData();

		const std::string input = "400";
		for ( auto _ : state )
		{
			auto result = hashCodeOrdinal( input );
			benchmark::DoNotOptimize( result );
		}
	}

	static void BM_bclOrd_H346_11112( benchmark::State& state )
	{
		initializeData();

		const std::string input = "H346.11112";
		for ( auto _ : state )
		{
			auto result = hashCodeOrdinal( input );
			benchmark::DoNotOptimize( result );
		}
	}

	static void BM_larsson_400( benchmark::State& state )
	{
		initializeData();

		const std::string input = "400";
		for ( auto _ : state )
		{
			auto result = hash<LarssonHasher>( input );
			benchmark::DoNotOptimize( result );
		}
	}

	static void BM_larsson_H346_11112( benchmark::State& state )
	{
		initializeData();

		const std::string input = "H346.11112";
		for ( auto _ : state )
		{
			auto result = hash<LarssonHasher>( input );
			benchmark::DoNotOptimize( result );
		}
	}

	static void BM_crc32Intrinsic_400( benchmark::State& state )
	{
		initializeData();

		const std::string input = "400";
		for ( auto _ : state )
		{
			auto result = hash<CRC32IntrinsicHasher>( input );
			benchmark::DoNotOptimize( result );
		}
	}

	static void BM_crc32Intrinsic_H346_11112( benchmark::State& state )
	{
		initializeData();

		const std::string input = "H346.11112";
		for ( auto _ : state )
		{
			auto result = hash<CRC32IntrinsicHasher>( input );
			benchmark::DoNotOptimize( result );
		}
	}

	static void BM_fnv_400( benchmark::State& state )
	{
		initializeData();

		const std::string input = "400";
		for ( auto _ : state )
		{
			auto result = hash<FnvHasher>( input );
			benchmark::DoNotOptimize( result );
		}
	}

	static void BM_fnv_H346_11112( benchmark::State& state )
	{
		initializeData();

		const std::string input = "H346.11112";
		for ( auto _ : state )
		{
			auto result = hash<FnvHasher>( input );
			benchmark::DoNotOptimize( result );
		}
	}

	BENCHMARK( BM_bcl_400 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_bcl_H346_11112 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_bclOrd_400 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_bclOrd_H346_11112 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_larsson_400 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_larsson_H346_11112 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_crc32Intrinsic_400 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_crc32Intrinsic_H346_11112 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_fnv_400 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_fnv_H346_11112 )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );
}

BENCHMARK_MAIN();
