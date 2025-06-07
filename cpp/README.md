# Vista SDK - C++ Implementation

This directory contains the C++ implementation of the Vista SDK, a native library port of the master C# version for working with the DNV Vessel Information Structure (VIS) standard, ISO 19847/19848, and related functionality.

## Dependencies

The C++ SDK uses CMake and its [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) module to manage external dependencies directly during the build configuration phase. The required libraries are fetched from their respective sources (primarily GitHub):

-   [nlohmann/json](https://github.com/nlohmann/json): For JSON parsing and serialization
-   [spdlog](https://github.com/gabime/spdlog): For structured logging
-   [zlib](https://github.com/madler/zlib): For compression and decompression of resources
-   [libcpuid](https://github.com/anrieff/libcpuid): For CPU feature detection
-   [{fmt}](https://github.com/fmtlib/fmt): Formatting library
-   [GoogleTest](https://github.com/google/googletest): For unit testing framework
-   [Google Benchmark](https://github.com/google/benchmark): For performance benchmarking

## Licensing

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

### Third-Party Dependencies

The Vista SDK C++ library depends on several high-quality open source libraries.
See [licenses/README.md](licenses/README.md) for complete licensing information and third-party attributions.

## Status

> [!NOTE]
> This C++ SDK is currently under active development, aiming for feature parity and API alignment with the established C# SDK.
> While core functionalities are being implemented and refined, some features might be missing or under further development.

The C++ SDK currently implements:

-   ✅ **Generic product model (GMOD)** - Complete with optimization focus
-   ✅ **Codebooks (metadata tags)** - Complete with performance improvements needed
-   ✅ **Locations** - Complete implementation
-   ✅ **Universal ID & Local ID** - Core functionality implemented
-   ⏳ **DataChannelList & TimeSeriesData** - Planned
-   ⏳ **Product model (PMOD)** - Planned
-   ✅ **Comprehensive benchmarking suite** - Cross-platform performance validation

## Progress Towards C# Parity

-   **Implemented:**

    -   Core `VIS` entry point and caching mechanism for loaded resources.
    -   `Gmod` loading, parsing (from DTOs), and `GmodNode` representation with full traversal capabilities.
    -   `GmodPath`: Complete parsing implementation including validation, individualization sets, and path construction. Supports both simple "VE" paths and complex paths with location individualization.
    -   `Codebooks` loading, parsing (from DTOs), `Codebook` representation, and `MetadataTag` lookup/creation.
    -   `Locations` loading and parsing (from DTOs) with comprehensive location parsing and validation.
    -   `LocationBuilder` with full immutable builder pattern, fluent interface, and comprehensive validation.
    -   `LocalId` parsing and immutable builder pattern (`LocalIdBuilder`) with full validation.
    -   `LocalIdBuilder` with comprehensive segment validation, metadata handling, and immutable fluent interface.
    -   `GmodVersioning` logic for converting GMOD IDs between versions with complete path transformation support.
    -   `GmodTraversal` utility for tree traversal operations with customizable handlers.
    -   Internal Data Transfer Objects (DTOs) for robust resource deserialization.
    -   Embedded resource handling, including decompression of gzipped JSON data.
    -   `ChdDictionary` (Perfect Hashing utility) - Includes input validation for empty/duplicate keys.
    -   `ParsingErrors` mechanism for collecting and reporting issues during parsing operations.
    -   `ImoNumber` with full validation and parsing capabilities.
    -   Complete `VisVersion` enumeration and extension methods.
    -   `UniversalIdBuilder`: Complete implementation with parsing, building, and validation capabilities.
    -   `UniversalId`: Full implementation with parsing, construction, and string representation.

-   **Partially Implemented / In Progress:**

    -   _None currently_

-   **Not Yet Implemented:**
    -   `GmodPathQuery`
    -   `LocalIdQuery`
    -   `LocalIdQueryBuilder`
    -   `MetadataTagsQuery`
    -   MQTT specifics

## Core Features Implemented

-   **VIS Entry Point:** Central access via `dnv::vista::sdk::VIS` with comprehensive resource caching.
-   **Gmod:** Complete representation and parsing of the Generic Product Model (`dnv::vista::sdk::Gmod`, `dnv::vista::sdk::GmodNode`) with tree traversal capabilities.
-   **GmodPath:** Full path parsing, validation, and construction with support for individualization sets and location-based paths (`dnv::vista::sdk::GmodPath`).
-   **GmodTraversal:** Utility for traversing Gmod trees with customizable handler functions (`dnv::vista::sdk::GmodTraversal`).
-   **Codebooks:** Complete handling of metadata tags and standard values (`dnv::vista::sdk::Codebooks`, `dnv::vista::sdk::Codebook`, `dnv::vista::sdk::MetadataTag`).
-   **Locations:** Full representation, parsing, and building of standard locations with validation (`dnv::vista::sdk::Locations`, `dnv::vista::sdk::LocationBuilder`) with immutable fluent interface.
-   **Location Builder:** Complete building and validation of location objects (`dnv::vista::sdk::LocationBuilder`) with immutable fluent interface.
-   **Local ID:** Complete building, parsing, and representation of Local IDs (`dnv::vista::sdk::LocalId`, `dnv::vista::sdk::LocalIdBuilder`) with comprehensive validation.
-   **Universal ID:** Complete building, parsing, and representation of Universal IDs (`dnv::vista::sdk::UniversalId`, `dnv::vista::sdk::UniversalIdBuilder`) with full validation and string conversion.
-   **IMO Numbers:** Full parsing and validation of IMO numbers (`dnv::vista::sdk::ImoNumber`).
-   **Gmod Versioning:** Complete conversion logic between different Gmod versions (`dnv::vista::sdk::GmodVersioning`) with path transformation support.
-   **Data Transfer Objects (DTOs):** Comprehensive internal structures for loading data from resources (`dnv::vista::sdk::GmodDto`, `dnv::vista::sdk::CodebooksDto`, `dnv::vista::sdk::LocationsDto`, etc.).
-   **Resource Loading:** Complete loading system for gzipped JSON resources embedded in the library (`dnv::vista::sdk::EmbeddedResource`).
-   **Perfect Hashing:** Utility for efficient string lookups (`dnv::vista::sdk::ChdDictionary`).
-   **Error Handling:** Comprehensive mechanism for accumulating parsing errors (`dnv::vista::sdk::ParsingErrors`, `dnv::vista::sdk::LocalIdParsingErrorBuilder`, `dnv::vista::sdk::LocationParsingErrorBuilder`).
-   **Version Management:** Complete VIS version enumeration and utilities (`dnv::vista::sdk::VisVersion`, `dnv::vista::sdk::VisVersionExtensions`).

## Architecture and Design Patterns

### Core Interfaces

-   **Concrete implementations:** `LocalId`, `LocalIdBuilder`, `UniversalId`, `UniversalIdBuilder` with complete functionality.
-   **Immutable fluent pattern:** Builder methods return new instances maintaining immutability.
-   **Direct value storage:** Optimized memory layout for performance-critical operations.

### Template-Based Design

The SDK leverages C++ templates extensively:

-   Template-based visitor patterns for location parsing.
-   Template specializations for performance optimization.

### Memory Management

Memory management patterns:

-   RAII principles with value semantics for optimal performance.

## Comparison with C# SDK

This C++ implementation follows the same core design principles (Immutability, Builder Pattern, VIS entry point) as the C# SDK but utilizes C++ language features and standard libraries.

| Feature             | C++ Implementation                                                                        | C# Implementation                                          | Notes                                                          |
| :------------------ | :---------------------------------------------------------------------------------------- | :--------------------------------------------------------- | :------------------------------------------------------------- |
| **Language**        | C++20                                                                                     | C# (.NET)                                                  | Leverages modern C++ features.                                 |
| **Immutability**    | `const` correctness, returning new builder instances, immutable by design                 | `readonly`, records (`struct`/`class`), `with` expressions |                                                                |
| **Optional Values** | `std::optional`                                                                           | Nullable types (`?`)                                       | Standard C++ approach with comprehensive null-safety patterns. |
| **Collections**     | `std::vector`, `std::unordered_map`, `std::deque`, `std::array`                           | `List<T>`, `Dictionary<K,V>`, `T[]`, `FrozenDictionary`    | Uses STL containers with performance optimizations.            |
| **String Handling** | `std::string`, `std::string_view`, `std::stringstream`, extensive view usage              | `string`, `ReadOnlySpan<char>`, `StringBuilder`            | C++ version uses string_view extensively for performance.      |
| **Error Handling**  | `std::exception` hierarchy, comprehensive `ParsingErrors` system, `std::invalid_argument` | `.NET` exceptions, `ParsingErrors` struct                  |
| **Hashing (CHD)**   | FNV1a/CRC32 (SSE4.2), comprehensive input validation                                      | FNV1a/CRC32 (SSE4.2)                                       |                                                                |
| **Logging**         | `spdlog` with extensive diagnostic logging throughout                                     | Minimal built-in logging                                   |
| **Build System**    | CMake (`FetchContent`) with comprehensive dependency management                           | .NET SDK (MSBuild/NuGet)                                   | CMake configuration with cross-platform support.               |
| **Dependencies**    | `nlohmann/json`, `spdlog`, `zlib`, `fmt`, `gtest`, `libcpuid`                             | NuGet packages                                             | Managed via `FetchContent` with version pinning.               |
| **Path Parsing**    | Validation with root node checking and structure validation                               | Basic validation                                           |

## Features

### Perfect Hashing (ChdDictionary)

The C++ implementation includes a perfect hashing system (as well as c# version):

-   Comprehensive input validation (empty keys, duplicates).
-   SSE4.2 accelerated hashing when available.

### Path Parsing and Validation

`GmodPath` implementation:

-   Root node validation ensuring paths start from valid root nodes.
-   Complex individualization set handling for location-based paths.
-   Comprehensive parent-child relationship validation.
-   Support for both simple and complex path structures.

### Memory-Efficient Design

-   Extensive use of `std::string_view` to avoid unnecessary string copies.
-   Smart pointer usage for automatic memory management.

## API Patterns

-   **Immutability:** Core domain objects (`LocalId`, `GmodPath`, `MetadataTag`, etc.) are immutable once created.
-   **Builder Pattern:** Objects like `LocalId` are constructed using an immutable builder (`LocalIdBuilder`) where modification methods return new builder instances.
-   **Concrete Implementation Design:** Direct concrete implementations (`LocalId`, `LocalIdBuilder`, `UniversalId`, UniversalIdBuilder`) with optimized direct value storage for performance.

## Performance Characteristics

-   **Memory Efficiency:** Direct value storage with RAII principles for optimal cache locality.
-   **CPU Optimization:** SSE4.2 accelerated hashing, optimized hash combining algorithms.
-   **String Optimization:** Extensive `string_view` usage to minimize allocations.

## TODO List

-   **Core Implementation:**

    -   Implement MQTT-specific aspects for `LocalId` (parsing from topics, builder methods, topic validation).

-   **Code Organization / Patterns:**

    -   Clean up public API/interface headers to expose only essential public functionality.
    -   Move private headers to private implementation directories to improve encapsulation.
    -   Consider implementing PIMPL idiom for core classes to hide implementation details, reduce header dependencies, and improve compilation times.

-   **Query Systems:**

    -   Implement `GmodPathQuery` for advanced path querying and filtering capabilities.
    -   Implement `LocalIdQuery` for querying and filtering LocalId collections.
    -   Implement `LocalIdQueryBuilder` with fluent interface for building complex LocalId queries.
    -   Implement `MetadataTagsQuery` for advanced metadata tag searching and filtering.

-   **Testing:**

    -   Implement MQTT-related tests (`Test_LocalId_Mqtt`, `Test_LocalId_Mqtt_Invalid`, `Test_LocalIdBuilder_Mqtt`) when MQTT functionality is added.

-   **Documentation:**

    -   Complete API documentation using Doxygen.
    -   Performance characteristics documentation.

## Testing Infrastructure

Comprehensive test coverage using GoogleTest:

-   **Unit Tests:** Complete coverage of core functionality.
-   **Integration Tests:** End-to-end testing of complex scenarios.
-   **Performance Tests:** Validation of performance characteristics.
-   **Parameterized Tests:** Extensive test matrices for edge cases.

Test suites include:

-   `Codebook`
-   `Codebooks`
-   `Gmod`
-   `GmodPath`
-   `GmodVersioning`
-   `GmodVersioningSmoke`
-   `ImoNumber`
-   `ISOString`
-   `LocalId`
-   `Locations`
-   `UniversalId`

## Benchmarking

Comprehensive performance benchmarking suite implemented comparing C++ and C# implementations across key operations.
See [README.md](benchmark/README.md) for detailed results.

Benchmark suites include:

-   `CodebooksLookup`
-   `GmodLoad`
-   `GmodLookup`
-   `GmodPathParse`
-   `GmodTraversal`
-   `GmodVersioningConvertPath`
-   `ShortStringHash`

## Build Requirements

-   **C++20 compliant compiler** (MSVC 2019+, GCC 10+, Clang 12+)
-   **CMake 3.20+**
-   **Dependencies automatically fetched via FetchContent:**

    -   nlohmann/json
    -   spdlog
    -   {fmt}
    -   zlib
    -   cpuid
    -   GoogleTest
    -   GoogleBenchmark

### Building with CMake

To build the C++ SDK, ensure you have a C++20 compliant compiler (e.g., MSVC, GCC, Clang) and CMake (version 3.20 or newer) installed.

1.  **Clone the repository:**

    ```bash
    git clone https://github.com/dnv-opensource/vista-sdk.git
    cd vista-sdk
    ```

2.  **Configure CMake:**
    Run CMake from the root of the repository to configure the build. The C++ build artifacts will typically be placed in `vista-sdk/build`.
    You can specify various options using `-D<OPTION>=<VALUE>`:

    -   `VISTA_SDK_CPP_BUILD_TESTS=ON` (Build unit tests, default is `ON`)
    -   `VISTA_SDK_CPP_BUILD_SMOKE_TESTS=OFF` (Build smoke tests, default is `OFF`)
    -   `VISTA_SDK_CPP_RUN_TESTS=OFF` (Run tests automatically after build, default is `OFF`)
    -   `VISTA_SDK_CPP_BUILD_BENCHMARKS=ON` (Build performance benchmarks, default is `ON`)
    -   `VISTA_SDK_CPP_RUN_BENCHMARKS=OFF` (Run benchmarks automatically after build, default is `OFF`)
    -   `VISTA_SDK_CPP_BUILD_SAMPLES=OFF` (Build sample applications, default is `OFF`)
    -   `VISTA_SDK_CPP_BUILD_DOCUMENTATION=OFF` (Build Doxygen documentation, default is `OFF`. Requires Doxygen and Graphviz.)
    -   `VISTA_SDK_CPP_COPY_RESOURCES=ON` (Copy resources to build directory, default is `ON`)
    -   `CMAKE_BUILD_TYPE=Release` (Or `Debug`, `RelWithDebInfo`, etc.)

    Example configuration for Visual Studio (Windows):

    ```bash
    cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DVISTA_SDK_CPP_BUILD_TESTS=ON -DVISTA_SDK_CPP_BUILD_BENCHMARKS=ON
    ```

    Example configuration for Ninja (cross-platform):

    ```bash
    cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release -DVISTA_SDK_CPP_BUILD_TESTS=ON -DVISTA_SDK_CPP_BUILD_BENCHMARKS=ON
    ```

3.  **Build the SDK:**
    Use CMake's build tool mode to compile the C++ library and any enabled components.

    ```bash
    cmake --build build --config Release
    ```

4.  **Run Tests (Automatic):**
    If `VISTA_SDK_CPP_RUN_TESTS=ON` (default), tests will run automatically after building.
    Test results are saved to `build/bin/Release/test_results/` with timestamps.

    To run tests manually:

    ```bash
    cmake --build build --config Release --target tests
    ```

5.  **Run Benchmarks (Automatic):**
    If `VISTA_SDK_CPP_RUN_BENCHMARKS=ON` (default), benchmarks will run automatically after building.
    Benchmark results are saved to `build/bin/Release/benchmark_results/` with timestamps.

    To run benchmarks manually:

    ```bash
    # Windows
    .\build\bin\Release\BM_CodebooksLookup.exe
    .\build\bin\Release\BM_GmodLoad.exe
    .\build\bin\Release\BM_GmodLookup.exe
    ...

    # Linux/macOS
    ./build/bin/Release/BM_CodebooksLookup
    ./build/bin/Release/BM_GmodLoad
    ./build/bin/Release/BM_GmodLookup
    ...
    ```

## C++ Coding Conventions

The C++ Vista SDK follows a consistent coding style throughout the codebase.

> [!NOTE]
> This represents my personal coding style preference for C++ development.
> If requested by the project owner, I can adapt the codebase to follow C# naming conventions (PascalCase for methods, etc.) to maintain consistency across language implementations.

### **Naming Conventions**

-   **Classes/Structs:** PascalCase - `LocalIdBuilder`, `GmodPath`, `UniversalId`
-   **Methods/Functions:** camelCase - `toString()`, `hashCode()`, `tryParse()`
-   **Member Variables:** `m_` prefix - `m_imoNumber`, `m_localIdBuilder`, `m_visVersion`
-   **Static Members:** `s_` prefix - `s_leafTypesSet`, `s_hashCache`
-   **Constants:** SCREAMING_SNAKE_CASE - `NUM_CODEBOOKS`, `FNV_OFFSET_BASIS`
-   **Namespaces:** Lowercase with scope resolution - `dnv::vista::sdk`, `dnv::vista::sdk::tests`
-   **Enums:** PascalCase - `CodebookName::Position`, `VisVersion::v3_8a`

### **Code Organization**

-   **Include Order:** PCH first, then main public header, then project headers
-   **Namespace Usage:** Full namespace qualification preferred over `using` statements
-   **Forward Declarations:** Extensive use to minimize compilation dependencies and speed-up recompilation

### **Method Conventions**

-   **Getters:** Simple property names (Qt style) - `imoNumber()`, `localId()`, `visVersion()`
-   **Builders:** Fluent interface with method chaining - `withImoNumber().withLocalId()`, mirroring the c# master implementation
-   **Validation/state inspection:** `isValid()`, `tryParse()`, `hasValue()` patterns
-   **Factory Methods:** Static `create()`, `parse()` methods for object construction

### **Documentation Standards**

-   **Doxygen Comments:** Comprehensive documentation using `@brief`, `@details`, `@param`, `@return`
-   **File Headers:** Standard file documentation with purpose and brief description
-   **Inline Comments (// ):** Only for developpement phase

### **Error Handling**

-   **Exceptions:** `std::invalid_argument`, `std::out_of_range` for validation failures
-   **Optional Types:** `std::optional` for nullable return values
-   **Error Accumulation:** `ParsingErrors` system for collecting multiple validation issues (based on c# master implementation)

### **Performance Considerations**

-   **String Views:** Extensive use of `std::string_view` to avoid unnecessary copies
-   **Move Semantics:** Proper use of move constructors and assignment operators

### **Code Formatting**

-   **Braces:** Opening brace on same line for all constructs - methods, classes, namespaces, and control structures
-   **Indentation:** Tabs for indentation, consistent throughout
-   **Line Length:** Reasonable line lengths with proper wrapping
-   **Spacing:** Consistent spacing around operators and function parameters

---

_Last updated: June 7, 2025_
