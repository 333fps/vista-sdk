/**
 * @file GmodTraversal.cpp
 * @brief GMOD tree traversal performance benchmark testing full tree iteration
 */

#include "pch.h"

#include "dnv/vista/sdk/GmodTraversal.h"
#include "dnv/vista/sdk/VIS.h"

using namespace dnv::vista::sdk;

namespace dnv::vista::sdk::benchmarks
{
	static const Gmod* g_gmod = nullptr;
	static bool g_initialized = false;

	static void initializeData()
	{
		if ( !g_initialized )
		{
			auto& vis = VIS::instance();
			g_gmod = &vis.gmod( VisVersion::v3_4a );
			g_initialized = true;
		}
	}

	static void BM_fullTraversal( benchmark::State& state )
	{
		initializeData();

		struct BenchmarkState
		{
			int nodeCount = 0;
		};

		for ( auto _ : state )
		{
			BenchmarkState benchState;

			TraverseHandlerWithState<BenchmarkState> handler =
				[]( BenchmarkState& state, const std::vector<const GmodNode*>&, const GmodNode& ) -> TraversalHandlerResult {
				++state.nodeCount;
				return TraversalHandlerResult::Continue;
			};

			bool result = dnv::vista::sdk::GmodTraversal::traverse( *g_gmod, benchState, handler );
			benchmark::DoNotOptimize( result );
			benchmark::DoNotOptimize( benchState.nodeCount );
		}
	}

	BENCHMARK( BM_fullTraversal )
		->MinTime( 10.0 )
		->Unit( benchmark::kMillisecond );
}

BENCHMARK_MAIN();
