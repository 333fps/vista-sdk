/**
 * @file ChdDictionary.cpp
 * @brief Implementation of CHD Dictionary components
 */

#include "pch.h"

#include "dnv/vista/sdk/ChdDictionary.h"

namespace dnv::vista::sdk
{
	namespace internal
	{
		//=====================================================================
		// CPU Feature Detection
		//=====================================================================

		bool hasSSE42Support()
		{
			static const bool s_hasSSE42{ []() {
				bool hasSupport{ false };

#if defined( _MSC_VER )
				std::array<int, 4> cpuInfo{};
				::__cpuid( cpuInfo.data(), 1 );
				hasSupport = ( cpuInfo[2] & ( 1 << 20 ) ) != 0;
#elif defined( __GNUC__ )
				unsigned int eax{}, ebx{}, ecx{}, edx{};
				if ( ::__get_cpuid( 1, &eax, &ebx, &ecx, &edx ) )
				{
					hasSupport = ( ecx & ( 1 << 20 ) ) != 0;
				}
#else
				hasSupport = false;
#endif
				SPDLOG_INFO( "SSE4.2 support: {}", hasSupport ? "available" : "not available" );

				return hasSupport;
			}() };

			return s_hasSSE42;
		}

		//=====================================================================
		// Exception Handling
		//=====================================================================

		void ThrowHelper::throwKeyNotFoundException( std::string_view key )
		{
			throw std::out_of_range( "Key not found in dictionary: " + std::string( key ) );
		}

		void ThrowHelper::throwInvalidOperationException()
		{
			throw std::logic_error( "Invalid operation" );
		}

		//=====================================================================
		// Hashing
		//=====================================================================

		uint32_t Hashing::fnv1a( uint32_t hash, uint8_t ch )
		{
			auto result{ ( ch ^ hash ) * FNV_PRIME };

			return result;
		}

		uint32_t Hashing::crc32( uint32_t hash, uint8_t ch )
		{
			auto result{ _mm_crc32_u8( hash, ch ) };

			return result;
		}

		uint32_t Hashing::seed( uint32_t seed, uint32_t hash, uint64_t size )
		{
			/* Mixes the primary hash with the seed to find the final table slot */
			uint32_t x{ seed + hash };
			x ^= x >> 12;
			x ^= x << 25;
			x ^= x >> 27;

			auto result{ static_cast<uint32_t>( ( static_cast<uint64_t>( x ) * 0x2545F4914F6CDD1DUL ) & ( size - 1 ) ) };

			return result;
		}
	}
}
