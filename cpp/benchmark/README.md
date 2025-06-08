# Vista SDK - C++ Benchmarking

**Windows:**

```bash
# Build benchmarks
cmake --build build --target BM_CodebooksLookup

# Execute with detailed output
.\build\bin\Release\BM_CodebooksLookup.exe --benchmark_min_time=10
```

**Linux:**

```bash
# Build benchmarks
cmake --build build --target BM_CodebooksLookup

# Execute with detailed output
./build/bin/Release/BM_CodebooksLookup --benchmark_min_time=10
```

## Test Environment

**Hardware Configuration:**

-   **CPU:** 12th Gen Intel Core i7-12800H (20 logical cores, 14 physical cores) @ 2.80 GHz
-   **RAM:** DDR4-3200 (32GB) - _will update with Linux system specs_
-   **CPU Cache:**
    -   L1 Data: 48 KiB (x10 cores)
    -   L1 Instruction: 32 KiB (x10 cores)
    -   L2 Unified: 1280 KiB (x10 cores)
    -   L3 Unified: 24576 KiB (x1 shared)

**Software Configuration:**

| Platform    | C++ Toolchain                              | C# Runtime               | OS Version |
| :---------- | :----------------------------------------- | :----------------------- | :--------- |
| **Windows** | Google Benchmark, MSVC 2022, Release build | .NET 8.0.16, RyuJIT AVX2 | Windows 10 |
| **Linux**   | TBC                                        | TBC                      | TBC        |

---

## Summary

| Operation Category          | C++ vs C# Performance                            | Status | Key Findings                          |
| :-------------------------- | :----------------------------------------------- | :----: | :------------------------------------ |
| **Hash Table Operations**   | **1.06-2.35x faster**                            |   ✅   | Superior low-level performance        |
| **Codebook Access**         | **2.44x faster**                                 |   ✅   | Hash maps dominate dictionaries       |
| **High-Level APIs**         | **5.13x slower**                                 |   ❌   | **Major optimization needed**         |
| **String Hashing**          | **1.02x faster to 1.39x slower**                 |   ✅   | Average parity                        |
| **GMOD Loading**            | **1.29x faster**                                 |   ✅   | Competitive with 99.4% less memory    |
| **GMOD Lookup**             | **1.06-2.35x faster (hash), 3.91x slower (API)** |   ❌   | **CHD implementation critical issue** |
| **GMOD Traversal**          | **1.7x slower**                                  |   ❌   | **Major optimization needed**         |
| **Path Parsing**            | **1.88-32.2x slower**                            |  ❌❌  | **Critical optimization needed**      |
| **Version Path Conversion** | **147x slower**                                  | ❌❌❌ | **CATASTROPHIC**                      |

---

## Codebooks Lookup

### Cross-Platform Performance Comparison

| Operation             | Windows C++ | vs Baseline | Linux C++ | vs Baseline | Notes                         |
| :-------------------- | :---------- | :---------: | :-------- | :---------: | :---------------------------- |
| **Array Lookup**      | 0.928 ns    |  **1.00x**  | _TBD_     |    _TBD_    | ⚡ Fastest possible operation |
| **Hash Table Lookup** | 1.96 ns     |  **2.11x**  | _TBD_     |    _TBD_    | 🔥 2.11x slower than baseline |
| **SDK Codebooks API** | 2.78 ns     |  **2.99x**  | _TBD_     |    _TBD_    | 🐌 2.99x slower than baseline |

### Codebooks Lookup Performance (Windows)

Performance comparison between C++ and C# implementations for codebook access operations:

| C++ Method            | C++ Time | C++ Implementation                    | C# Method     | C# Time | C# Implementation             | Performance Ratio   |
| :-------------------- | :------- | :------------------------------------ | :------------ | :------ | :---------------------------- | :------------------ |
| **hashTableLookup**   | 1.96 ns  | `std::unordered_map::find()`          | **Dict**      | 4.90 ns | `Dictionary.TryGetValue()`    | ✅ **2.50x faster** |
| **sdkApiCodebooks**   | 2.78 ns  | `codebooks_ref[enum_key]` + try/catch | **Codebooks** | 0.69 ns | `array[index]` + bounds check | ❌ **4.03x slower** |
| **sdkApiArrayLookup** | 0.928 ns | `std::array[index] != nullptr`        | _No equiv_    | _N/A_   | _N/A_                         | _N/A_               |

#### Detailed C++ Results (Windows)

| Benchmark             | Time     | CPU      | Iterations | Memory Usage |
| :-------------------- | :------- | :------- | :--------- | :----------- |
| **hashTableLookup**   | 1.96 ns  | 1.96 ns  | 7.3B       | 0 KB         |
| **sdkApiCodebooks**   | 2.78 ns  | 2.78 ns  | 5.0B       | 0 KB         |
| **sdkApiArrayLookup** | 0.928 ns | 0.928 ns | 15.2B      | 0 KB         |

#### Detailed C# Results (Windows)

| Method     |      Mean |     Error |    StdDev |        Ratio | RatioSD | Rank | Allocated | Alloc Ratio |
| ---------- | --------: | --------: | --------: | -----------: | ------: | ---: | --------: | ----------: |
| FrozenDict | 4.9802 ns | 0.0699 ns | 0.0584 ns | 1.02x slower |   0.01x |    3 |         - |          NA |
| Dict       | 4.8993 ns | 0.0239 ns | 0.0212 ns |     baseline |         |    2 |         - |          NA |
| Codebooks  | 0.6937 ns | 0.0219 ns | 0.0195 ns | 7.07x faster |   0.21x |    1 |         - |          NA |

---

## GMOD Load

### Cross-Platform Performance Comparison

| Operation     | Windows C++ | vs Baseline | Linux C++ | vs Baseline | Notes                     |
| :------------ | :---------- | :---------: | :-------- | :---------: | :------------------------ |
| **GMOD Load** | 23.5 ms     |  **1.00x**  | _TBD_     |    _TBD_    | ⚡ Full GMOD construction |

### GMOD Load Performance (Windows)

| C++ Method   | C++ Time | C++ Implementation                  | C# Method | C# Time | C# Implementation               | Performance Ratio   |
| :----------- | :------- | :---------------------------------- | :-------- | :------ | :------------------------------ | :------------------ |
| **gmodLoad** | 23.5 ms  | `VIS::loadGmodDto()` + construction | **Load**  | 30.4 ms | `Gmod.Load()` with full parsing | ✅ **1.29x faster** |

#### Detailed C++ Results (Windows)

| Benchmark    | Time    | CPU     | Iterations | Memory Usage |
| :----------- | :------ | :------ | :--------- | :----------- |
| **gmodLoad** | 23.5 ms | 23.5 ms | 490        | 1.316 KB     |

#### Detailed C# Results (Windows)

| Method   | Mean     | Error    | StdDev   | Gen0   | Gen1   | Gen2  | Allocated |
| :------- | :------- | :------- | :------- | :----- | :----- | :---- | :-------- |
| **Load** | 30.40 ms | 0.607 ms | 1.078 ms | 1562.5 | 1500.0 | 562.5 | 15.41 MB  |

---

## GMOD Lookup

| Operation             | Windows C++ | vs Baseline | Linux C++ | vs Baseline | Notes                    |
| :-------------------- | :---------- | :---------: | :-------- | :---------: | :----------------------- |
| **Frozen Dictionary** | 14.3 ns     |  **1.00x**  | _TBD_     |    _TBD_    | ⚡ Fastest lookup method |
| **Hash Table Lookup** | 16.3 ns     |  **1.14x**  | _TBD_     |    _TBD_    | 🔥 Slightly slower       |
| **GMOD API Lookup**   | 61.0.ns     |  **4.27x**  | _TBD_     |    _TBD_    | 🐌 Needs optimization    |

### GMOD Lookup Performance (Windows)

| C++ Method     | C++ Time | C++ Implementation            | C# Method      | C# Time  | C# Implementation                | Performance Ratio   |
| :------------- | :------- | :---------------------------- | :------------- | :------- | :------------------------------- | :------------------ |
| **frozenDict** | 14.3 ns  | `std::unordered_map` (frozen) | **FrozenDict** | 15.21 ns | `FrozenDictionary.TryGetValue()` | ✅ **1.06x faster** |
| **dict**       | 16.3 ns  | `std::unordered_map::find()`  | **Dict**       | 38.34 ns | `Dictionary.TryGetValue()`       | ✅ **2.35x faster** |
| **gmod**       | 61.0 ns  | CHD implementation lookup     | **Gmod**       | 15.62 ns | Native GMOD API access           | ❌ **3.91x slower** |

#### Detailed C++ Results (Windows)

| Benchmark      | Time    | CPU     | Iterations | Memory Usage |
| :------------- | :------ | :------ | :--------- | :----------- |
| **Dict**       | 16.3 ns | 16.3 ns | 887M       | 0 KB         |
| **FrozenDict** | 14.3 ns | 14.3 ns | 974M       | 18.0 KB      |
| **Gmod**       | 61.0 ns | 60.3 ns | 235M       | 0 KB         |

#### Detailed C# Results (Windows)

| Method         | Mean     | Error    | StdDev   | Ratio        | RatioSD | Rank | Allocated | Alloc Ratio |
| :------------- | :------- | :------- | :------- | :----------- | :------ | :--- | :-------- | :---------- |
| **Dict**       | 38.34 ns | 0.227 ns | 0.201 ns | baseline     |         | 3    | -         | NA          |
| **FrozenDict** | 15.21 ns | 0.039 ns | 0.035 ns | 2.52x faster | 0.02x   | 1    | -         | NA          |
| **Gmod**       | 15.62 ns | 0.255 ns | 0.226 ns | 2.46x faster | 0.04x   | 2    | -         | NA          |

---

## GMOD Path Parsing

### Cross-Platform Performance Comparison

| Operation                          | Windows C++ | Linux C++ | Status | Notes |
| :--------------------------------- | :---------- | :-------- | :----: | :---- |
| **TryParse**                       | 6.32 μs     | _TBD_     |        |       |
| **TryParseFullPath**               | 18.4 μs     | _TBD_     |        |       |
| **TryParseIndividualized**         | 4.03 μs     | _TBD_     |        |       |
| **TryParseFullPathIndividualized** | 18.8 μs     | _TBD_     |        |       |

### GMOD Path Parsing Performance (Windows)

Performance comparison between C++ and C# implementations for GMOD path parsing operations:

| Operation                          | C++ (Google Benchmark) | C# (BenchmarkDotNet) | Performance Ratio | Status | Notes                         |
| :--------------------------------- | :--------------------- | :------------------- | :---------------- | :----: | :---------------------------- |
| **TryParse**                       | 6.32 μs                | 3.36 μs              | **1.88x slower**  |   ❌   | C++ path parsing overhead     |
| **TryParseFullPath**               | 18.4 μs                | 571 ns               | **32.2x slower**  | ❌❌❌ | Critical performance issue    |
| **TryParseIndividualized**         | 4.03 μs                | 1.49 μs              | **2.70x slower**  |   ❌   | C++ location parsing overhead |
| **TryParseFullPathIndividualized** | 18.8 μs                | 694 ns               | **27.1x slower**  | ❌❌❌ | Critical performance issue    |

#### Detailed C++ Results (Windows)

| Benchmark                          | Time    | CPU     | Iterations | Memory Usage |
| :--------------------------------- | :------ | :------ | :--------- | :----------- |
| **tryParse**                       | 6.32 μs | 6.31 μs | 2.3M       | 0 KB         |
| **tryParseFullPath**               | 18.4 μs | 18.4 μs | 759K       | 18.0 KB      |
| **tryParseIndividualized**         | 4.03 μs | 4.03 μs | 3.5M       | 0 KB         |
| **tryParseFullPathIndividualized** | 18.8 μs | 18.8 μs | 740K       | 0 KB         |

#### Detailed C# Results (Windows)

| Method                             | Mean       | Error    | StdDev   | Categories    | Gen0   | Allocated |
| :--------------------------------- | :--------- | :------- | :------- | :------------ | :----- | :-------- |
| **TryParse**                       | 3,360.8 ns | 41.66 ns | 38.97 ns | No location   | 0.2213 | 2,792 B   |
| **TryParseFullPath**               | 571.2 ns   | 7.45 ns  | 5.82 ns  | No location   | 0.0601 | 760 B     |
| **TryParseIndividualized**         | 1,492.2 ns | 28.77 ns | 51.14 ns | With location | 0.2251 | 2,832 B   |
| **TryParseFullPathIndividualized** | 694.1 ns   | 4.33 ns  | 3.84 ns  | With location | 0.0935 | 1,176 B   |

---

## GMOD Traversal

### Cross-Platform Performance Comparison

| Operation          | Windows C++ | Linux C++ | Status | Notes |
| :----------------- | :---------- | :-------- | :----: | :---- |
| **Full Traversal** | 277 ms      | _TBD_     |        |       |

### GMOD Traversal Performance (Windows)

Performance comparison between C++ and C# implementations for GMOD node lookup operations:

| Operation          | C++ (Google Benchmark) | C# (BenchmarkDotNet) | Performance Ratio | Status | Notes                                |
| :----------------- | :--------------------- | :------------------- | :---------------- | :----: | :----------------------------------- |
| **Full Traversal** | 277 ms                 | 162.9 ms             | **1.7x slower**   |   ❌   | Significant optimization opportunity |

#### Detailed C++ Results (Windows)

| Benchmark         | Time   | CPU    | Iterations | Memory Usage |
| :---------------- | :----- | :----- | :--------- | :----------- |
| **FullTraversal** | 277 ms | 277 ms | 50-51      | 0 KB         |

#### Detailed C# Results (Windows)

| Method            | Mean     | Error   | StdDev  | Allocated |
| :---------------- | :------- | :------ | :------ | :-------- |
| **FullTraversal** | 162.9 ms | 0.97 ms | 0.91 ms | 5.3 KB    |

---

## GMOD Versioning Path Conversion

### Cross-Platform Performance Comparison

| Operation        | Windows C++ | Linux C++ | Status | Notes |
| :--------------- | :---------- | :-------- | :----: | :---- |
| **Convert Path** | 219 μs      | _TBD_     |        |       |

### GMOD Versioning Path Conversion Performance (Windows)

Performance comparison between C++ and C# implementations for GMOD version path conversion operations:

| Operation        | C++ (Google Benchmark) | C# (BenchmarkDotNet) | Performance Ratio | Status | Notes            |
| :--------------- | :--------------------- | :------------------- | :---------------- | :----: | :--------------- |
| **Convert Path** | 219 μs                 | 1.489 μs             | **147x slower**   | ❌❌❌ | **CATASTROPHIC** |

#### Detailed C++ Results (Windows)

| Benchmark       | Time   | CPU    | Iterations | Memory Usage |
| :-------------- | :----- | :----- | :--------- | :----------- |
| **convertPath** | 219 μs | 219 μs | 64,000     | 0 KB         |

#### Detailed C# Results (Windows)

| Method          | Mean     | Error     | StdDev    | Gen0   | Allocated |
| :-------------- | :------- | :-------- | :-------- | :----- | :-------- |
| **ConvertPath** | 1.489 μs | 0.0108 μs | 0.0090 μs | 0.2575 | 3.17 KB   |

---

## Short String Hashing

### Cross-Platform Performance Comparison

| Operation          | Windows C++ | Linux C++ | Status | Notes |
| :----------------- | :---------- | :-------- | :----: | :---- |
| **bcl (400)**      | 1.55 ns     | _TBD_     |        |       |
| **bcl (H346)**     | 3.76 ns     | _TBD_     |        |       |
| **bclOrd (400)**   | 1.79 ns     | _TBD_     |        |       |
| **bclOrd (H346)**  | 3.05 ns     | _TBD_     |        |       |
| **larsson (400)**  | 1.70 ns     | _TBD_     |        |       |
| **larsson (H346)** | 4.61 ns     | _TBD_     |        |       |
| **crc32 (400)**    | 1.51 ns     | _TBD_     |        |       |
| **crc32 (H346)**   | 4.34 ns     | _TBD_     |        |       |
| **fnv (400)**      | 1.65 ns     | _TBD_     |        |       |
| **fnv (H346)**     | 3.26 ns     | _TBD_     |        |       |

### Short String Hashing Performance (Windows)

Performance comparison between C++ and C# implementations for hash function operations:

#### String "400" (3 characters)

| Algorithm          | C++ (Google Benchmark) | C# (BenchmarkDotNet) | Performance Ratio | Status | Notes |
| :----------------- | :--------------------- | :------------------- | :---------------- | :----: | :---- |
| **Bcl**            | 1.55 ns                | 1.135 ns             | **1.37x slower**  |        |       |
| **BclOrd**         | 1.79 ns                | 1.514 ns             | **1.18x slower**  |        |       |
| **Larsson**        | 1.70 ns                | 1.219 ns             | **1.39x slower**  |        |       |
| **Crc32Intrinsic** | 1.51 ns                | 1.215 ns             | **1.24x slower**  |        |       |
| **Fnv**            | 1.65 ns                | 1.205 ns             | **1.37x slower**  |        |       |

#### String "H346.11112" (10 characters)

| Algorithm          | C++ (Google Benchmark) | C# (BenchmarkDotNet) | Performance Ratio | Status | Notes |
| :----------------- | :--------------------- | :------------------- | :---------------- | :----: | :---- |
| **Bcl**            | 3.76 ns                | 4.551 ns             | **1.21x faster**  |        |       |
| **BclOrd**         | 3.05 ns                | 3.127 ns             | **1.03x faster**  |        |       |
| **Larsson**        | 4.61 ns                | 3.349 ns             | **1.38x slower**  |        |       |
| **Crc32Intrinsic** | 4.34 ns                | 3.259 ns             | **1.33x slower**  |        |       |
| **Fnv**            | 3.26 ns                | 3.337 ns             | **1.02x faster**  |        |       |

#### Detailed C++ Results (Windows)

| Benchmark                       | Input      | Time    | CPU     | Iterations | Memory Usage |
| :------------------------------ | :--------- | :------ | :------ | :--------- | :----------- |
| **short strings (400)**         |            |         |         |            |              |
| bcl_400                         | 400        | 1.55 ns | 1.55 ns | 8.6B       | 0 KB         |
| bclOrd_400                      | 400        | 1.79 ns | 1.79 ns | 7.8B       | 0 KB         |
| larsson_400                     | 400        | 1.70 ns | 1.70 ns | 8.4B       | 0 KB         |
| crc32Intrinsic_400              | 400        | 1.51 ns | 1.51 ns | 9.4B       | 0 KB         |
| fnv_400                         | 400        | 1.65 ns | 1.66 ns | 8.4B       | 0 KB         |
| **longer strings (H346.11112)** |            |         |         |            |              |
| bcl_H346_11112                  | H346.11112 | 3.76 ns | 3.75 ns | 4.5B       | 0 KB         |
| bclOrd_H346_11112               | H346.11112 | 3.05 ns | 3.05 ns | 4.6B       | 0 KB         |
| larsson_H346_11112              | H346.11112 | 4.61 ns | 4.61 ns | 3.0B       | 0 KB         |
| crc32Intrinsic_H346_11112       | H346.11112 | 4.34 ns | 4.33 ns | 3.2B       | 0 KB         |
| fnv_H346_11112                  | H346.11112 | 3.26 ns | 3.25 ns | 4.3B       | 0 KB         |

#### Detailed C# Results (Windows)

| Method                          | Input      | Mean     | Error     | StdDev    | Ratio        | RatioSD | Rank | Allocated | Alloc Ratio |
| :------------------------------ | :--------- | :------- | :-------- | :-------- | :----------- | :-----: | :--: | :-------- | :---------- |
| **Short Strings (400)**         |            |          |           |           |              |         |      |           |             |
| Bcl                             | 400        | 1.135 ns | 0.0232 ns | 0.0217 ns | baseline     |         |  1   | -         | NA          |
| BclOrd                          | 400        | 1.514 ns | 0.0372 ns | 0.0348 ns | 1.33x slower |  0.04x  |  3   | -         | NA          |
| Larsson                         | 400        | 1.219 ns | 0.0031 ns | 0.0029 ns | 1.07x slower |  0.02x  |  2   | -         | NA          |
| Crc32Intrinsic                  | 400        | 1.215 ns | 0.0222 ns | 0.0208 ns | 1.07x slower |  0.03x  |  2   | -         | NA          |
| Fnv                             | 400        | 1.205 ns | 0.0237 ns | 0.0198 ns | 1.06x slower |  0.03x  |  2   | -         | NA          |
| **Longer Strings (H346.11112)** |            |          |           |           |              |         |      |           |             |
| BclOrd                          | H346.11112 | 3.127 ns | 0.0910 ns | 0.0807 ns | 1.46x faster |  0.03x  |  1   | -         | NA          |
| Crc32Intrinsic                  | H346.11112 | 3.259 ns | 0.0057 ns | 0.0053 ns | 1.40x faster |  0.01x  |  2   | -         | NA          |
| Fnv                             | H346.11112 | 3.337 ns | 0.0423 ns | 0.0353 ns | 1.36x faster |  0.02x  |  3   | -         | NA          |
| Larsson                         | H346.11112 | 3.349 ns | 0.0147 ns | 0.0130 ns | 1.36x faster |  0.01x  |  3   | -         | NA          |
| Bcl                             | H346.11112 | 4.551 ns | 0.0336 ns | 0.0298 ns | baseline     |         |  4   | -         | NA          |

---

_Last updated: June 8, 2025_
