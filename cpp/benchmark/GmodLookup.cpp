/**
 * @file GmodLookup.cpp
 * @brief GMOD node lookup comparison between CHD Dictionary and std::unordered_map
 */

#include "pch.h"

#include "dnv/vista/sdk/Gmod.h"
#include "dnv/vista/sdk/VIS.h"

using namespace dnv::vista::sdk;

namespace dnv::vista::sdk::benchmarks
{
	static std::unordered_map<std::string, const GmodNode*> g_dict;
	static std::unordered_map<std::string, const GmodNode*> g_frozenDict;
	static const Gmod* g_gmod = nullptr;
	static bool g_initialized = false;

	static void initializeData()
	{
		if ( !g_initialized )
		{
			auto& vis = VIS::instance();
			g_gmod = &vis.gmod( VisVersion::v3_7a );

			g_dict.clear();
			g_frozenDict.clear();

			auto enumerator = g_gmod->enumerator();
			while ( enumerator.next() )
			{
				const auto& node = enumerator.current();
				g_dict[node.code()] = &node;
				g_frozenDict[node.code()] = &node;
			}
			g_initialized = true;
		}
	}

	static void BM_dict( benchmark::State& state )
	{
		initializeData();

#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_start;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_start, sizeof( pmc_start ) );
		size_t initialMemory = pmc_start.WorkingSetSize;
#endif

		for ( auto _ : state )
		{
			bool result =
				( g_dict.find( "VE" ) != g_dict.end() ) &&
				( g_dict.find( "400a" ) != g_dict.end() ) &&
				( g_dict.find( "400" ) != g_dict.end() ) &&
				( g_dict.find( "H346.11112" ) != g_dict.end() );

			benchmark::DoNotOptimize( result );
		}

#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_end;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_end, sizeof( pmc_end ) );
		auto memoryDelta = static_cast<double>( pmc_end.WorkingSetSize - initialMemory );
		state.counters["MemoryDeltaKB"] = benchmark::Counter( memoryDelta / 1024.0 );
#endif
	}

	static void BM_frozenDict( benchmark::State& state )
	{
		initializeData();

#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_start;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_start, sizeof( pmc_start ) );
		size_t initialMemory = pmc_start.WorkingSetSize;
#endif

		for ( auto _ : state )
		{
			bool result =
				( g_frozenDict.find( "VE" ) != g_frozenDict.end() ) &&
				( g_frozenDict.find( "400a" ) != g_frozenDict.end() ) &&
				( g_frozenDict.find( "400" ) != g_frozenDict.end() ) &&
				( g_frozenDict.find( "H346.11112" ) != g_frozenDict.end() );

			benchmark::DoNotOptimize( result );
		}

#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_end;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_end, sizeof( pmc_end ) );
		auto memoryDelta = static_cast<double>( pmc_end.WorkingSetSize - initialMemory );
		state.counters["MemoryDeltaKB"] = benchmark::Counter( memoryDelta / 1024.0 );
#endif
	}

	static void BM_gmod( benchmark::State& state )
	{
		initializeData();

#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_start;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_start, sizeof( pmc_start ) );
		size_t initialMemory = pmc_start.WorkingSetSize;
#endif

		for ( auto _ : state )
		{
			const GmodNode* node;
			bool result =
				g_gmod->tryGetNode( "VE", node ) &&
				g_gmod->tryGetNode( "400a", node ) &&
				g_gmod->tryGetNode( "400", node ) &&
				g_gmod->tryGetNode( "H346.11112", node );

			benchmark::DoNotOptimize( result );
			benchmark::DoNotOptimize( node );
		}

#ifdef _WIN32
		PROCESS_MEMORY_COUNTERS_EX pmc_end;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc_end, sizeof( pmc_end ) );
		auto memoryDelta = static_cast<double>( pmc_end.WorkingSetSize - initialMemory );
		state.counters["MemoryDeltaKB"] = benchmark::Counter( memoryDelta / 1024.0 );
#endif
	}

	BENCHMARK( BM_dict )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_frozenDict )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );

	BENCHMARK( BM_gmod )
		->MinTime( 10.0 )
		->Unit( benchmark::kNanosecond );
}

BENCHMARK_MAIN();
