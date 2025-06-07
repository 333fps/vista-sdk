/**
 * @file GmodVersioningConvertPath.cpp
 * @brief GMOD version conversion performance benchmark testing path conversion between VIS versions
 */

#include "pch.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/GmodPath.h"
#include "dnv/vista/sdk/VIS.h"

using namespace dnv::vista::sdk;

namespace dnv::vista::sdk::benchmarks
{
	static const Gmod* g_gmod = nullptr;
	static GmodPath g_gmodPath;
	static bool g_initialized = false;

	static void initializeData()
	{
		if ( !g_initialized )
		{
			auto& vis = VIS::instance();
			g_gmod = &vis.gmod( VisVersion::v3_4a );

			std::optional<GmodPath> parsedPath;
			if ( !g_gmod->tryParsePath( "411.1/C101.72/I101", parsedPath ) || !parsedPath.has_value() )
			{
				throw std::runtime_error( "Failed to parse test path" );
			}

			g_gmodPath = std::move( parsedPath.value() );
			g_initialized = true;
		}
	}

	static void BM_convertPath( benchmark::State& state )
	{
		initializeData();

#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_start;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_start, sizeof( pmc_start ) );
		size_t initialMemory = pmc_start.WorkingSetSize;
#endif

		auto& vis = VIS::instance();

		for ( auto _ : state )
		{
			auto result = vis.convertPath( VisVersion::v3_4a, g_gmodPath, VisVersion::v3_5a );
			benchmark::DoNotOptimize( result );
		}

#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_end;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_end, sizeof( pmc_end ) );
		auto memoryDelta = static_cast<double>( pmc_end.WorkingSetSize - initialMemory );
		state.counters["MemoryDeltaKB"] = benchmark::Counter( memoryDelta / 1024.0 );
#endif
	}

	BENCHMARK( BM_convertPath )
		->MinTime( 10.0 )
		->Unit( benchmark::kMicrosecond );
}

BENCHMARK_MAIN();
