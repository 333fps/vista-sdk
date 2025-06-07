/**
 * @file GmodLoad.cpp
 * @brief GMOD loading and construction performance analysis
 */

#include "pch.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodDto.h"
#include "dnv/vista/sdk/VIS.h"

using namespace dnv::vista::sdk;

namespace dnv::vista::sdk::benchmarks
{
	static void BM_gmodLoad( benchmark::State& state )
	{
#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_start;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_start, sizeof( pmc_start ) );
		size_t initialMemory = pmc_start.WorkingSetSize;
#endif

		for ( auto _ : state )
		{
			auto dto = VIS::loadGmodDto( VisVersion::v3_7a );

			if ( !dto.has_value() )
			{
				state.SkipWithError( "Failed to load GMOD DTO" );
				return;
			}

			auto gmod = std::make_unique<Gmod>( VisVersion::v3_7a, *dto );

			benchmark::DoNotOptimize( dto );
			benchmark::DoNotOptimize( gmod );
		}

#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_end;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_end, sizeof( pmc_end ) );
		auto memoryDelta = static_cast<double>( pmc_end.WorkingSetSize - initialMemory );
		state.counters["MemoryDeltaKB"] = benchmark::Counter( memoryDelta / 1024.0 );
#endif
	}

	BENCHMARK( BM_gmodLoad )
		->MinTime( 10.0 )
		->Unit( benchmark::kMillisecond );
}

BENCHMARK_MAIN();
