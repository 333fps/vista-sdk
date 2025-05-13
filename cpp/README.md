# Vista SDK - C++ Implementation

This directory contains the C++ implementation of the Vista SDK. It aims to provide a native library for working with the DNV Vessel Information Structure (VIS) standard, ISO 19847/19848, and related functionality.

## Status

> [!NOTE]
> This C++ SDK is currently under active development, aiming for feature parity and API alignment with the established C# SDK.
While core functionalities are being implemented and refined, some features might be missing or under further development.

## Progress Towards C# Parity

*   **Implemented:**
    *   Core `VIS` entry point and caching mechanism for loaded resources.
    *   `Gmod` loading, parsing (from DTOs), and `GmodNode` representation.
    *   `Codebooks` loading, parsing (from DTOs), `Codebook` representation, and `MetadataTag` lookup/creation.
    *   `Locations` loading and parsing (from DTOs).
    *   `LocalId` parsing and immutable builder pattern (`LocalIdBuilder`).
    *   `GmodVersioning` logic for converting GMOD IDs between versions.
    *   Internal Data Transfer Objects (DTOs) for robust resource deserialization.
    *   Embedded resource handling, including decompression of gzipped JSON data.
    *   `ChdDictionary` (Perfect Hashing utility) - Includes input validation for empty/duplicate keys. *Note: The binary format of the dictionary is incompatible with the C# version due to differences in hash inputs and internal caching.*
    *   `ParsingErrors` mechanism for collecting and reporting issues during parsing operations.
*   **Partially Implemented / In Progress:**
    *   `GmodPath`: Core parsing for simple "VE" paths is functional.
*   **Not Yet Implemented:**
    *   `UniversalId`: Parsing, construction, and related functionalities.
    *   MQTT-specific aspects for `LocalId`: Parsing from MQTT topics, specific builder methods, and topic validation.
    *   `LocationBuilder`: A dedicated builder for constructing `Location` objects.
    *   Full validation parity in builders: For example, comprehensive content validation (allowed characters, length constraints) for `LocalIdBuilder` segments beyond basic structural checks.
    *   Benchmarking suite: A suite comparable to the C# SDK's benchmarks for performance analysis.

## Core Features Implemented

*   **VIS Entry Point:** Central access via `dnv::vista::sdk::VIS`.
*   **Gmod:** Representation and parsing of the Generic Product Model (`dnv::vista::sdk::Gmod`, `dnv::vista::sdk::GmodNode`).
*   **Codebooks:** Handling of metadata tags and standard values (`dnv::vista::sdk::Codebooks`, `dnv::vista::sdk::Codebook`, `dnv::vista::sdk::MetadataTag`).
*   **Locations:** Representation and parsing of standard locations (`dnv::vista::sdk::Locations`).
*   **Local ID:** Building, parsing, and representation of Local IDs (`dnv::vista::sdk::LocalId`, `dnv::vista::sdk::LocalIdBuilder`).
*   **Gmod Versioning:** Conversion logic between different Gmod versions (`dnv::vista::sdk::GmodVersioning`).
*   **Data Transfer Objects (DTOs):** Internal structures for loading data from resources (`dnv::vista::sdk::GmodDto`, `dnv::vista::sdk::CodebooksDto`, `dnv::vista::sdk::LocationsDto`, etc.).
*   **Resource Loading:** Loading gzipped JSON resources embedded in the library (`dnv::vista::sdk::EmbeddedResource`).
*   **Perfect Hashing:** Utility for efficient string lookups (`dnv::vista::sdk::ChdDictionary`).
*   **Error Handling:** Mechanism for accumulating parsing errors (`dnv::vista::sdk::ParsingErrors`).


## Comparison with C# SDK

This C++ implementation follows the same core design principles (Immutability, Builder Pattern, VIS entry point) as the C# SDK but utilizes C++ language features and standard libraries.

| Feature             | C++ Implementation                                                                 | C# Implementation                                       | Notes                                                                              |
| :------------------ | :--------------------------------------------------------------------------------- | :------------------------------------------------------ | :--------------------------------------------------------------------------------- |
| **Language**        | C++20                                                                              | C# (.NET)                                               | Leverages modern C++ features.                                                     |
| **Immutability**    | `const` correctness, returning new builder instances                               | `readonly`, records (`struct`/`class`), `with` expressions | Achieves similar goals using different language mechanisms.                        |
| **Optional Values** | `std::optional`                                                                    | Nullable types (`?`)                                    | Standard C++ approach.                                                             |
| **Collections**     | `std::vector`, `std::unordered_map`, `std::array`                                  | `List<T>`, `Dictionary<K,V>`, `T[]`, `FrozenDictionary` | Uses STL containers. C# uses BCL and specialized collections.                      |
| **String Handling** | `std::string`, `std::string_view`, `std::stringstream`                             | `string`, `ReadOnlySpan<char>`, `StringBuilder`         | C++ uses standard strings/views. C# uses spans and pooled builders.              |
| **Error Handling**  | `std::exception` hierarchy, `ParsingErrors` class, `std::invalid_argument` for input validation | `.NET` exceptions, `ParsingErrors` struct               | Similar `TryParse`/`ParsingErrors` pattern. Different exception types. C++ has more explicit input validation in `ChdDictionary`. |
| **Hashing (CHD)**   | FNV1a/CRC32 (SSE4.2), thread-local cache, input validation for keys                | FNV1a/CRC32 (SSE4.2), no cache                          | **Incompatible binary formats** due to different hash inputs and C++ cache.        |
| **Logging**         | `spdlog`                                                                           | Minimal built-in logging                                | C++ version includes detailed diagnostic/performance logging.                      |
| **Build System**    | CMake (`FetchContent`)                                                             | .NET SDK (MSBuild/NuGet)                                | Dependencies fetched during CMake configuration.                                   |
| **Dependencies**    | `nlohmann/json`, `spdlog`, `zlib`, `fmt`, `gtest`                                  | NuGet packages                                          | Managed via `FetchContent`.                                                        |

## API Patterns

*   **Immutability:** Core domain objects (`LocalId`, `GmodPath`, `MetadataTag`, etc.) are immutable once created.
*   **Builder Pattern:** Objects like `LocalId` are constructed using an immutable builder (`LocalIdBuilder`) where modification methods return new builder instances.

## TODO List

*   **Testing:**
    *   Implement test `Test_Location_Builder` when `LocationBuilder` is available.
    *   Implement MQTT-related tests (`Test_LocalId_Mqtt`, `Test_LocalId_Mqtt_Invalid`, `Test_LocalIdBuilder_Mqtt`) when MQTT functionality for `LocalId` is added.
    *   Expand `GmodPath` tests to cover all advanced parsing scenarios once implemented.
*   **Core Implementation:**
    *   Implement `UniversalId` parsing, construction, loading, and caching.
    *   Implement MQTT-specific aspects for `LocalId` (parsing from topics, builder methods, topic validation).
    *   Complete `GmodPath::parse` implementation for all path types.
*   **Builders:**
    *   Implement `LocationBuilder`.
    *   Enhance `LocalIdBuilder` with comprehensive segment content validation beyond basic structural checks.
    *   Add similar comprehensive validation to `GmodPath` parsing and construction.
*   **DTOs & Serialization:**
    *   Review DTO immutability strategy: The current DTOs use non-`const` members to allow population by ADL `from_json` hooks, while deleting assignment operators and providing `const` accessors. For stricter immutability (i.e., `const` members ensuring objects are only ever populated at construction via parameterized constructors or static factory methods), a refactor would be needed. This would likely involve removing or significantly altering the ADL `from_json` hooks that modify existing instances.
*   **Refactoring & Performance:**
    *   Investigate C++20 heterogeneous lookup for `std::unordered_map` instances for potential performance improvements.
    *   Address potential performance issue of creating temporary `std::string` for lookups in methods like `Codebook::validatePosition` by consistently using `std::string_view` where feasible.
    *   Consider adding support for other hash functions in `ChdDictionary` or making the hash function selection configurable.
*   **Benchmarking:**
    *   Develop a benchmarking suite comparable to the C# SDK to track and improve performance.
