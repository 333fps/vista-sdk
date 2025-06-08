# Third-Party Licenses

This directory contains the licenses for all external libraries used in the Vista SDK C++ implementation.

## Overview

The Vista SDK C++ library depends on several high-quality open source libraries. This document provides a complete inventory of all third-party dependencies and their respective licenses to ensure full legal compliance.

## Libraries Used

| Library         | Version | License Type     | Usage                         | License File                                                                                                      |
| --------------- | ------- | ---------------- | ----------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| spdlog          | 1.15.3  | MIT              | Logging framework             | [LICENSE-spdlog.txt](LICENSE-spdlog.txt)                                                                          |
| simdjson        | 3.13.0  | MIT / Apache 2.0 | High-performance JSON parsing | [LICENSE-simdjson-MIT.txt](LICENSE-simdjson-MIT.txt) / [LICENSE-simdjson-APACHE.txt](LICENSE-simdjson-APACHE.txt) |
| fmt             | 11.2.0  | MIT              | String formatting             | [LICENSE-fmt.txt](LICENSE-fmt.txt)                                                                                |
| zlib            | 1.3.1   | zlib License     | Compression utilities         | [LICENSE-zlib.txt](LICENSE-zlib.txt)                                                                              |
| libcpuid        | 0.8.0   | Custom           | CPU identification            | [LICENSE-libcpuid.txt](LICENSE-libcpuid.txt)                                                                      |
| GoogleTest      | 1.17.0  | BSD 3-Clause     | Unit testing framework        | [LICENSE-googletest.txt](LICENSE-googletest.txt)                                                                  |
| GoogleBenchmark | 1.9.4   | Apache 2.0       | Performance benchmarking      | [LICENSE-googlebenchmark.txt](LICENSE-googlebenchmark.txt)                                                        |

## License Compliance

### Runtime Dependencies

The following libraries are required for Vista SDK runtime operation:

-   **spdlog** + **fmt**: Logging and string formatting
-   **simdjson**: High-performance JSON parsing
-   **zlib**: Data compression utilities
-   **libcpuid**: CPU feature detection

### Development Dependencies

The following libraries are used only during development and testing:

-   **GoogleTest**: Unit testing framework
-   **Google Benchmark**: Performance measurement

### Distribution Requirements

When distributing software that includes the Vista SDK, you must:

1. **Include all license files** from this directory
2. **Preserve copyright notices** as specified in each license
3. **Provide attribution** for BSD and Apache licensed components
4. **Follow specific terms** outlined in each individual license

## Updating Licenses

When updating library versions, ensure corresponding license files are updated:

1. Check if the library license has changed
2. Update version numbers in this README
3. Replace license files if text has changed
4. Verify compliance with any new license terms

## Additional Information

-   **Vista SDK License**: The Vista SDK itself is licensed under the MIT License (see [LICENSE](../../LICENSE))

---

_Last updated: June 5, 2025_
