# Vista SDK - C++ Implementation

This directory contains the C++ implementation of the Vista SDK. It aims to provide a native library for working with the DNV Vessel Information Structure (VIS) standard, ISO 19847/19848, and related functionality.

## Status

> [!NOTE]
> This C++ SDK is currently under development, aiming for feature parity and API alignment with the established C# SDK.
While core functionalities are being implemented, some features might be missing or under refinement.

## Progress Towards C# Parity

*   **Implemented:**
    *   Core `VIS` entry point and caching mechanism.
    *   `Gmod` loading, parsing, and node representation.
    *   `Codebooks` loading, parsing, and metadata tag lookup.
    *   `Locations` loading and parsing.
    *   `LocalId` parsing and immutable builder pattern.
    *   `GmodVersioning` logic.
    *   Internal DTOs for resource loading.
    *   Embedded resource handling (gzipped JSON).
    *   `ChdDictionary` (Perfect Hashing) - *Note: Binary incompatible with C# version.*
    *   `ParsingErrors` mechanism.
*   **Partially Implemented / In Progress:**
    *   `GmodPath`.
*   **Not Yet Implemented:**
    *   `UniversalId` parsing and construction.
    *   MQTT-specific aspects of `LocalId` parsing/building.
    *   `LocationBuilder`.
    *   Full validation parity in builders (e.g., `LocalIdBuilder` segment validation).
    *   Benchmarking suite equivalent to C#.

## Core Features Implemented

*   **VIS Entry Point:** Central access via `dnv::vista::sdk::VIS`.
*   **Gmod:** Representation and parsing of the Generic Product Model (`dnv::vista::sdk::Gmod`, `dnv::vista::sdk::GmodNode`, `dnv::vista::sdk::GmodPath`).
*   **Codebooks:** Handling of metadata tags and standard values (`dnv::vista::sdk::Codebooks`, `dnv::vista::sdk::Codebook`, `dnv::vista::sdk::MetadataTag`).
*   **Locations:** Representation and parsing of standard locations (`dnv::vista::sdk::Locations`).
*   **Local ID:** Building, parsing, and representation of Local IDs (`dnv::vista::sdk::LocalId`, `dnv::vista::sdk::LocalIdBuilder`).
*   **Gmod Versioning:** Conversion logic between different Gmod versions (`dnv::vista::sdk::GmodVersioning`).
*   **Data Transfer Objects (DTOs):** Internal structures for loading data from resources (`dnv::vista::sdk::GmodDto`, `dnv::vista::sdk::CodebooksDto`, etc.).
*   **Resource Loading:** Loading gzipped JSON resources embedded in the library (`dnv::vista::sdk::EmbeddedResource`).

## Comparison with C# SDK

This C++ implementation follows the same core design principles (Immutability, Builder Pattern, VIS entry point) as the C# SDK but utilizes C++ language features and standard libraries.

| Feature             | C++ Implementation                                     | C# Implementation                                       | Notes                                                                 |
| :------------------ | :----------------------------------------------------- | :------------------------------------------------------ | :-------------------------------------------------------------------- |
| **Language**        | C++20                                                  | C# (.NET)                                               | Leverages modern C++ features.                                        |
| **Immutability**    | `const` correctness, returning new builder instances   | `readonly`, records (`struct`/`class`), `with` expressions | Achieves similar goals using different language mechanisms.           |
| **Optional Values** | `std::optional`                                        | Nullable types (`?`)                                    | Standard C++ approach.                                                |
| **Collections**     | `std::vector`, `std::unordered_map`, `std::array`      | `List<T>`, `Dictionary<K,V>`, `T[]`, `FrozenDictionary` | Uses STL containers. C# uses BCL and specialized collections.         |
| **String Handling** | `std::string`, `std::string_view`, `std::stringstream` | `string`, `ReadOnlySpan<char>`, `StringBuilder`         | C++ uses standard strings/views. C# uses spans and pooled builders. |
| **Error Handling**  | `std::exception` hierarchy, `ParsingErrors` class      | `.NET` exceptions, `ParsingErrors` struct               | Similar `TryParse`/`ParsingErrors` pattern. Different exception types. |
| **Hashing (CHD)**   | FNV1a/CRC32 (SSE4.2), thread-local cache               | FNV1a/CRC32 (SSE4.2), no cache                          | **Incompatible binary formats** due to different hash inputs.         |
| **Logging**         | `spdlog`                                               | Minimal built-in logging                                | C++ version includes detailed diagnostic/performance logging.         |
| **Build System**    | CMake (`FetchContent`)                                 | .NET SDK (MSBuild/NuGet)                                | Dependencies fetched during CMake configuration.                      |
| **Dependencies**    | `nlohmann/json`, `spdlog`, `zlib`, `fmt`, `gtest`      | NuGet packages                                          | Managed via `FetchContent`.                                           |


## API Patterns

*   **Immutability:** Core domain objects (`LocalId`, `GmodPath`, `MetadataTag`, etc.) are immutable once created.
*   **Builder Pattern:** Objects like `LocalId` are constructed using an immutable builder (`LocalIdBuilder`) where modification methods return new builder instances.


## TODO List

*   **Testing:**
    * Implement test `Test_Location_Builder` when `LocationBuilder` is available.
    * Implement MQTT-related tests (`Test_LocalId_Mqtt`, `Test_LocalId_Mqtt_Invalid`, `Test_LocalIdBuilder_Mqtt`) when MQTT functionality is added.
*   **Core Implementation:**
    * Implement `UniversalId` parsing and construction logic.
    * Implement loading/caching for `UniversalId` when available.
    * Add validation for MQTT topic format if needed.
*   **Builders:**
    * Consider adding more validation for segment values (e.g., allowed characters).
*   **DTOs & Serialization:**
    * Review DTO immutability: Consider refactoring DTOs for stricter immutability (e.g., `const` members). This would involve ensuring that objects are only populated during construction (e.g., via static `tryFromJson`/`fromJson` methods) and disabling ADL `from_json` hooks that modify existing instances.
*   **Refactoring & Performance:**
    * Consider C++20 heterogeneous lookup for `m_standardValues` and `m_groupMap` for potential performance improvement.
    * Address potential performance issue of creating temporary `std::string` for lookup in `validatePosition`.
    * Consider adding support for other hash functions or making it configurable.

