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

# --- Benchmark source files ---
set(BENCHMARK_SOURCES
	CodebooksLookup.cpp
	GmodLoad.cpp
	GmodLookup.cpp
	GmodPathParse.cpp
	GmodTraversal.cpp
	GmodVersioningConvertPath.cpp
	ShortStringHash.cpp
)

# --- Set executables prefix ---
set(BENCHMARK_PREFIX "BM_")

# --- Configure benchmark executables ---
foreach(benchmark_source ${BENCHMARK_SOURCES})
	get_filename_component(benchmark_target_name ${benchmark_source} NAME_WE)

	set(benchmark_target_name ${BENCHMARK_PREFIX}${benchmark_target_name})

	if(NOT TARGET ${benchmark_target_name})
		add_executable(${benchmark_target_name} ${benchmark_source})

		target_precompile_headers(${benchmark_target_name} PRIVATE pch.h)

		target_link_libraries(${benchmark_target_name} PRIVATE
			vista-sdk-cpp
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

		set_target_properties(${benchmark_target_name} PROPERTIES
			CXX_STANDARD 20
			CXX_STANDARD_REQUIRED ON
			RUNTIME_OUTPUT_DIRECTORY "${VISTA_SDK_CPP_BIN_DIR}/$<CONFIG>"
			EXCLUDE_FROM_ALL FALSE
		)

		if(MSVC)
			# ============================================================================
			# Optimization settings by build configuration
			# ============================================================================
			add_compile_options($<$<CONFIG:Release>:/O2>)        # Maximum speed optimization
			add_compile_options($<$<CONFIG:Release>:/Oi>)        # Enable intrinsic functions
			add_compile_options($<$<CONFIG:Release>:/Ot>)        # Favor fast code over small code
			add_compile_options($<$<CONFIG:Release>:/Gy>)        # Enable Function-Level Linking
			add_compile_options($<$<CONFIG:Release>:/DNDEBUG>)   # Disable debug assertions

			# ============================================================================
			# Warning suppressions
			# ============================================================================
			add_compile_options(/wd4505) # unreferenced local function has been removed
			add_compile_options(/wd4514) # unreferenced inline function has been removed
			add_compile_options(/wd4625) # copy constructor was implicitly defined as deleted
			add_compile_options(/wd4626) # assignment operator was implicitly defined as deleted
			add_compile_options(/wd4702) # unreachable code
			add_compile_options(/wd4834) # discarding return value of function with [[nodiscard]] attribute
			add_compile_options(/wd5026) # move constructor was implicitly defined as deleted
			add_compile_options(/wd5027) # move assignment operator was implicitly defined as deleted
		endif()

		if(NOT TARGET benchmarks)
			add_custom_target(benchmarks)
		endif()

		add_dependencies(benchmarks ${benchmark_target_name})
	endif()
endforeach()

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
		set(benchmark_target_name ${BENCHMARK_PREFIX}${benchmark_target_name})

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
