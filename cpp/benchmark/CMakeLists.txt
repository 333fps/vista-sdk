# ==============================================================================
# VISTA-SDK-CPP - Benchmark suite CMake configuration
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

project(vista-sdk-cpp-benchmarks
	VERSION 0.1.0
	DESCRIPTION "DNV VISTA SDK C++ Benchmarks"
	LANGUAGES CXX)

# --- Benchmark condition check ---
if(NOT VISTA_SDK_CPP_BUILD_BENCHMARKS)
	return()
endif()

# --- Benchmarks source files ---
set(BENCHMARK_SOURCES
	BM_ChdDictionary.cpp
	BM_CodebooksLookup.cpp
	BM_GmodLoad.cpp
	BM_GmodLookup.cpp
	BM_GmodPathParse.cpp
	BM_GmodTraversal.cpp
	BM_GmodVersioningConvertPath.cpp
	BM_ShortStringHash.cpp
)

# --- Configure benchmark executables ---
foreach(benchmark_source ${BENCHMARK_SOURCES})
	get_filename_component(benchmark_target_name ${benchmark_source} NAME_WE)

	if(NOT TARGET ${benchmark_target_name})
		add_executable(${benchmark_target_name} ${benchmark_source})

		target_precompile_headers(${benchmark_target_name} PRIVATE pch.h)

		target_link_libraries(${benchmark_target_name} PRIVATE
			vista-sdk-cpp
			spdlog::spdlog
			benchmark::benchmark
			fmt::fmt
			nlohmann_json::nlohmann_json
		)

		target_compile_definitions(${benchmark_target_name} PRIVATE
			$<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_OFF>
			$<$<AND:$<CONFIG:Debug>,$<BOOL:${CI_FORCE_DEBUG_SPDLOG_WARN}>>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_WARN>
		)

		target_include_directories(${benchmark_target_name} PRIVATE
			${VISTA_SDK_CPP_INCLUDE_DIR}
		)

		# --- Properties with LTO disabled ---
		set_target_properties(${benchmark_target_name} PROPERTIES
			CXX_STANDARD 20
			CXX_STANDARD_REQUIRED ON
			RUNTIME_OUTPUT_DIRECTORY "${VISTA_SDK_CPP_BIN_DIR}/$<CONFIG>"
			EXCLUDE_FROM_ALL FALSE
			INTERPROCEDURAL_OPTIMIZATION FALSE
		)

		# --- Target-specific MSVC optimisation ---
		if(MSVC)
			target_compile_options(${benchmark_target_name} PRIVATE
				/std:c++20                             # C++20 standard
				/MP                                    # Multi-processor compilation
				/W4                                    # High warning level
				/Wall                                  # All warnings
				/WX-                                   # Warnings not as errors
				/permissive-                           # Strict conformance mode
				/fp:fast                               # Fast floating point
				/Zc:__cplusplus                        # Correct __cplusplus macro
				/Zc:inline                             # Remove unreferenced COMDAT
				/Zc:preprocessor                       # Conforming preprocessor
				/external:anglebrackets                # Treat angle bracket includes as external
				/external:W0                           # No warnings for external headers
				/arch:AVX2                             # Enable AVX2 (covers AVX, SSE4.2, etc.)

				# Optimization settings
				$<$<CONFIG:Release>:/O2>               # Maximum speed optimization
				$<$<CONFIG:Release>:/Oi>               # Enable intrinsic functions
				$<$<CONFIG:Release>:/Ot>               # Favor fast code over small code
				$<$<CONFIG:Release>:/Ob3>              # Aggressive inlining
				$<$<CONFIG:Release>:/Gy>               # Function-Level Linking
				$<$<CONFIG:Release>:/Qpar>             # Auto-parallelization
				$<$<CONFIG:Release>:/DNDEBUG>          # Disable debug assertions
				$<$<CONFIG:Release>:/GS->              # Disable buffer security checks
				$<$<CONFIG:Release>:/Gw>               # Optimize global data
				$<$<CONFIG:Release>:/GT>               # Support fiber-safe TLS
				$<$<CONFIG:Release>:/fp:except->       # Disable floating point exceptions

				# Warning suppressions
				/wd4244                                # conversion from 'type1' to 'type2', possible loss of data
				/wd4505                                # unreferenced local function has been removed
				/wd4514                                # unreferenced inline function has been removed
				/wd4625                                # copy constructor was implicitly defined as deleted
				/wd4626                                # assignment operator was implicitly defined as deleted
				/wd4834                                # discarding return value of function with 'nodiscard' attribute
				/wd5026                                # move constructor was implicitly defined as deleted
				/wd5027                                # move assignment operator was implicitly defined as deleted
			)

			target_link_options(${benchmark_target_name} PRIVATE
				$<$<CONFIG:Release>:/OPT:REF>          # Remove unreferenced functions
				$<$<CONFIG:Release>:/OPT:ICF>          # Identical COMDAT folding
				$<$<CONFIG:Release>:/LTCG>             # Link Time Code Generation
				$<$<CONFIG:Release>:/INCREMENTAL:NO>   # Disable incremental linking
			)
		endif()

		# --- Add to benchmark target ---
		if(NOT TARGET benchmarks)
			add_custom_target(benchmarks)
		endif()
		add_dependencies(benchmarks ${benchmark_target_name})
	endif()
endforeach()

# --- Benchmark execution ---
if(TARGET benchmarks AND VISTA_SDK_CPP_RUN_BENCHMARKS)
	set(BENCHMARK_OUTPUT_DIR "${VISTA_SDK_CPP_BIN_DIR}/$<CONFIG>/benchmark_results")
	string(TIMESTAMP BENCHMARK_TIMESTAMP "%Y%m%d_%H%M%S")

	add_custom_target(run_benchmarks ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory ${BENCHMARK_OUTPUT_DIR}
		COMMENT "Running Vista SDK C++ Benchmark Suite..."
		SOURCES ${BENCHMARK_SOURCES}
	)

	set_target_properties(run_benchmarks PROPERTIES
		FOLDER "Benchmarks"
		EXCLUDE_FROM_DEFAULT_BUILD FALSE
	)

	foreach(benchmark_source ${BENCHMARK_SOURCES})
		get_filename_component(benchmark_target_name ${benchmark_source} NAME_WE)
		set(OUTPUT_FILE "${BENCHMARK_OUTPUT_DIR}/${benchmark_target_name}_${BENCHMARK_TIMESTAMP}.txt")

		add_custom_command(TARGET run_benchmarks POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E echo ""
			COMMAND ${CMAKE_COMMAND} -E echo "================================================================================"
			COMMAND ${CMAKE_COMMAND} -E echo "Running: ${benchmark_target_name} -> ${OUTPUT_FILE}"
			COMMAND ${CMAKE_COMMAND} -E echo "================================================================================"
			COMMAND powershell -Command "$ProgressPreference='SilentlyContinue'; $output = & '${VISTA_SDK_CPP_BIN_DIR}/$<CONFIG>/${benchmark_target_name}.exe' --benchmark_format=console 2>&1; $output | ForEach-Object { Write-Host $_; Add-Content -Path '${OUTPUT_FILE}' -Value $_ -Force }"
			COMMAND ${CMAKE_COMMAND} -E echo ""
			COMMAND ${CMAKE_COMMAND} -E echo "Results saved to: ${OUTPUT_FILE}"
			WORKING_DIRECTORY "$<TARGET_FILE_DIR:${benchmark_target_name}>"
			VERBATIM
		)
	endforeach()

	add_dependencies(run_benchmarks benchmarks)
	message(STATUS "Benchmark execution enabled - results will be saved to ${BENCHMARK_OUTPUT_DIR}")
else()
	if(TARGET benchmarks)
		message(STATUS "Benchmark execution disabled - use -DVISTA_SDK_CPP_RUN_BENCHMARKS=ON to enable")
	endif()
endif()
