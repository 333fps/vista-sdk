[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/build-csharp.yml?branch=main&label=C%23)](https://github.com/dnv-opensource/vista-sdk/actions)
[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/build-js.yml?branch=main&label=JS)](https://github.com/dnv-opensource/vista-sdk/actions)
![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/build-python.yml?branch=main&label=Python)
[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/333fps/vista-sdk/build-cpp.yml?branch=main&label=C%2B%2B)](https://github.com/333fps/vista-sdk/actions)
[![GitHub](https://img.shields.io/github/license/dnv-opensource/vista-sdk?style=flat-square)](https://github.com/dnv-opensource/vista-sdk/blob/main/LICENSE)<br/>
[![SDK NuGet current](https://img.shields.io/nuget/v/DNV.Vista.SDK?label=NuGet%20DNV.Vista.SDK)](https://www.nuget.org/packages/DNV.Vista.SDK)
[![SDK NuGet prerelease](https://img.shields.io/nuget/vpre/DNV.Vista.SDK?label=NuGet%20DNV.Vista.SDK)](https://www.nuget.org/packages/DNV.Vista.SDK)<br/>
[![SDK NPM current](https://img.shields.io/npm/v/dnv-vista-sdk?label=NPM%20dnv-vista-sdk)](https://www.npmjs.com/package/dnv-vista-sdk)
[![SDK NPM current](https://img.shields.io/npm/v/dnv-vista-sdk/preview?label=NPM%20dnv-vista-sdk)](https://www.npmjs.com/package/dnv-vista-sdk)<br/>
![PyPI current](https://img.shields.io/pypi/v/vista-sdk?label=PyPI%20vista-sdk)


## Vista SDK
The Vista team at DNV are working on tooling related to
* DNV Vessel Information Structure (VIS)
* ISO 19847 - Ships and marine technology — Shipboard data servers to share field data at sea
* ISO 19848 - Ships and marine technology — Standard data for shipboard machinery and equipment

In this repository we codify the rules and principles of VIS and related ISO-standards to enable and support
users and implementers of the standards.

Our plan is to develop SDKs for some of the most common platforms. We are starting with .NET, Python and JavaScript.
We will be developing these SDKs as open source projects. Feel free to provide input, request changes or make contributions by creating issues in this repository.

For general documentation relating to VIS and relating standard. See [docs.vista.dnv.com](https://docs.vista.dnv.com).

### Status

> [!NOTE]
> The **v0.1** versions of the SDK are currently in production use at DNV for various services.
> We are currently working on the **v0.2** version of the SDKs where we are adressing several usability and API design issues.
> When **v0.2** is finalized we are hoping that **v1.0** will quickly follow.
> New users should stick to **v0.1** currently while we work on stabilizing APIs and design.
> Functionally (in terms of domain), not much will change

### Content

Each SDK makes use of the contents of the resources and schemas folders to generate code and use the standards.

```
📦vista-sdk
 ┣ 📂cpp
 ┣ 📂csharp
 ┣ 📂js
 ┣ 📂python
 ┣ 📂resources
 ┃ ┣ 📜codebooks-vis-3-4a.json.gz
 ┃ ┣ 📜codebooks-vis-3-5a.json.gz
 ┃ ┣ 📜codebooks-vis-3-6a.json.gz
 ┃ ┣ 📜codebooks-vis-3-7a.json.gz
 ┃ ┣ 📜codebooks-vis-3-8a.json.gz
 ┃ ┣ 📜gmod-vis-3-4a.json.gz
 ┃ ┣ 📜gmod-vis-3-5a.json.gz
 ┃ ┣ 📜gmod-vis-3-6a.json.gz
 ┃ ┣ 📜gmod-vis-3-7a.json.gz
 ┃ ┣ 📜gmod-vis-3-8a.json.gz
 ┃ ┣ 📜gmod-vis-versioning-3-5a.json.gz
 ┃ ┣ 📜gmod-vis-versioning-3-6a.json.gz
 ┃ ┣ 📜gmod-vis-versioning-3-7a.json.gz
 ┃ ┣ 📜gmod-vis-versioning-3-8a.json.gz
 ┃ ┣ 📜iso19848-v2018-data-channel-type-names.json.gz
 ┃ ┣ 📜iso19848-v2018-format-data-types.json.gz
 ┃ ┣ 📜iso19848-v2024-data-channel-type-names.json.gz
 ┃ ┣ 📜iso19848-v2024-format-data-types.json.gz
 ┃ ┣ 📜locations-vis-3-4a.json.gz
 ┃ ┣ 📜locations-vis-3-5a.json.gz
 ┃ ┣ 📜locations-vis-3-6a.json.gz
 ┃ ┣ 📜locations-vis-3-7a.json.gz
 ┃ ┗ 📜locations-vis-3-8a.json.gz
 ┣ 📂schemas
 ┃ ┣ 📂json
 ┃ ┃ ┣ 📜DataChannelList.sample.compact.json
 ┃ ┃ ┣ 📜DataChannelList.sample.json
 ┃ ┃ ┣ 📜DataChannelList.schema.json
 ┃ ┃ ┣ 📜TimeSeriesData.sample.json
 ┃ ┃ ┗ 📜TimeSeriesData.schema.json
 ┃ ┣ 📂xml
 ┃ ┃ ┣ 📜DataChannelList.xsd
 ┃ ┃ ┗ 📜TimeSeriesData.xsd
 ┣ 📂testdata
 ┣ 📜LICENSE
 ┗ 📜README.md
```

### SDK outline

This section will outline the various components and modules in our SDKs.

#### Vessel Information Structure

There are various components of VIS in our SDKs:

* Generic product model (Gmod) - C#, JS
* Product model (Pmod) - JS
* Codebooks (metadata tags) - C#, JS
* Locations - C#, JS

For more information on this concepts, check out [docs.vista.dnv.com](https://docs.vista.dnv.com).

#### ISO-19848 and ISO-19847

Part of these standards are the definition of datastructures used for communicating and sharing sensor data.
Note that while compression isnt explicitly mentioned in these standards, the standard doesnt prohibit use
of compression when implementing these standards, as long as the datastructures remain the same.

Related components:

* Universal ID & Local ID - C#, JS
* DataChannelList & TimeSeriesData - C#, JS

### Benchmarks

We are developing some benchmarks to keep track of the performance characteristics of the libraries we are creating.

#### Transport packages - DataChannelList and TimeSeriesData

The ISO-19848/10947 standards define the schema for the XML encoding of the transport packages.
The JSON encoding is only provded in example form, but we have developed [JSON schemas](https://json-schema.org/) for these packages [here](schemas/json/).

In the benchmark below we try to isolate the difference between Json and Avro encoding, and measure the effect of compression using Bzip2 and Brotli.
See the `Payload size` column below for an insight into size of ISO-19848 packages over the wire.
Avro has been included as an example binary encoding - it is not currently part of the standard.
The latency measurements in this context are less useful, as they are platform-specific.
See [csharp/benchmark](csharp/benchmark) for more details on the method of the benchmarks.

``` ini

BenchmarkDotNet=v0.13.1, OS=Windows 10.0.19043.1645 (21H1/May2021Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK=6.0.202
  [Host]                        : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT
  DataChannelList serialization : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT

Job=DataChannelList serialization  IterationCount=3  LaunchCount=1
WarmupCount=3

```
| Method |   Categories | CompressionLevel |         Mean |        Error |       StdDev | Payload size |
|------- |------------- |----------------- |-------------:|-------------:|-------------:|-------------:|
|   **Json** | **Uncompressed** |                **?** |     **906.4 μs** |     **106.4 μs** |      **5.83 μs** |    **285.92 KB** |
|   Avro | Uncompressed |                ? |     702.2 μs |     337.7 μs |     18.51 μs |    113.22 KB |
|        |              |                  |              |              |              |              |
|   Json |       Brotli |                ? | 363,783.2 μs | 468,803.0 μs | 25,696.67 μs |     18.31 KB |
|   Avro |       Brotli |                ? | 129,235.6 μs |  35,572.9 μs |  1,949.87 μs |     18.56 KB |
|        |              |                  |              |              |              |              |
|   **Json** |        **Bzip2** |                **5** |  **42,353.5 μs** |  **16,058.2 μs** |    **880.20 μs** |     **19.19 KB** |
|   Avro |        Bzip2 |                5 |  12,175.2 μs |   9,095.2 μs |    498.54 μs |      19.5 KB |
|   **Json** |        **Bzip2** |                **9** |  **48,419.8 μs** |  **16,895.3 μs** |    **926.09 μs** |     **19.19 KB** |
|   Avro |        Bzip2 |                9 |  13,762.6 μs |   2,310.1 μs |    126.62 μs |      19.5 KB |


### API patterns

#### Immutability

Domain models exposed in the SDKs are generally immutable,
the builder APIs construct new instances while passing along the old data that is not modified by the builder method invoked.

#### Builder pattern

Typically, when the SDK provides code for building classes, it does so in a Builder Pattern. It provides possibility to chain using With, TryWith and Without methods.

```csharp
builder = Create(someIntro)
    .WithSomeValue(someValue)
    .TryWithSomeOtherValue(someOtherValue)
    .WithoutSomeThirdValue() // usually without/limited arguments
builder = builder.TryWithValue(item, out var success)
```

* `With` should be used when the operation is expected to receive non-nullable values and succeed without further checking. It will throw error if provided with wrong arguments.
* `TryWith` should be used in two cases: When you don't want to be bothered by failures behind the scene, and when you want to know if it went ok, but without exceptions. If you want to check if the opration went as expected, you can use the try do out param - "succeeded" e.g. TryWithSomething(intput, out bool succeeded).
* `Without` provides functionality for removing certain elements from the chain. Typically without arguments/limited arguments

### C++ SDK

The C++ implementation of the Vista SDK provides a native library for working with the VIS standard, ISO 19847/19848, and related functionality.
It follows the same patterns and design principles as the C# implementation while leveraging C++ idioms and performance characteristics.

#### Dependencies

The C++ SDK uses CMake and its [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) module to manage external dependencies directly during the build configuration phase. The required libraries are fetched from their respective sources (primarily GitHub):

- [nlohmann/json](https://github.com/nlohmann/json): For JSON parsing and serialization
- [spdlog](https://github.com/gabime/spdlog): For structured logging
- [zlib](https://www.zlib.net/): For compression and decompression of resources
- [libcpuid](https://github.com/anrieff/libcpuid): For CPU feature detection
- [{fmt}](https://fmt.dev/): Formatting library
- [GoogleTest](https://github.com/google/googletest): For unit testing framework


#### Building with CMake

To build the C++ SDK, ensure you have a C++20 compliant compiler (e.g., MSVC, GCC, Clang) and CMake (version 3.20 or newer) installed.

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/dnv-opensource/vista-sdk.git
    cd vista-sdk
    ```

2.  **Configure CMake:**
    Run CMake from the root of the repository to configure the build. The C++ build artifacts will typically be placed in `vista-sdk/cpp/build`.
    You can specify various options using `-D<OPTION>=<VALUE>`:
    *   `VISTA_SDK_CPP_BUILD_TESTS=ON` (Build unit tests, default is `ON`)
    *   `VISTA_SDK_CPP_BUILD_SAMPLES=ON` (Build sample applications, default is `ON`)
    *   `VISTA_SDK_CPP_BUILD_DOCUMENTATION=ON` (Build Doxygen documentation, default is `OFF`. Requires Doxygen and Graphviz.)
    *   `CMAKE_BUILD_TYPE=Release` (Or `Debug`, `RelWithDebInfo`, etc.)

    Example configuration (e.g., for Ninja, adjust generator as needed):
    ```bash
    cmake -B cpp/build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release -DVISTA_SDK_CPP_BUILD_TESTS=ON
    ```
    For Visual Studio, you might omit `-G Ninja` if MSVC is your default generator:
    ```bash
    cmake -B cpp/build -S . -DCMAKE_BUILD_TYPE=Release -DVISTA_SDK_CPP_BUILD_TESTS=ON
    ```

3.  **Build the SDK:**
    Use CMake's build tool mode to compile the C++ library and any enabled components (like tests or samples).
    ```bash
    cmake --build cpp/build --config Release
    ```

4.  **Run Tests (Optional):**
    If tests were enabled (`VISTA_SDK_CPP_BUILD_TESTS=ON`), navigate to the C++ build directory and run CTest:
    ```bash
    cd cpp/build
    ctest -C Release
    ```
