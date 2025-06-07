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
		// Internal helper components
		//=====================================================================

		//----------------------------------------------
		// CPU feature detection
		//----------------------------------------------

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
	}
}
