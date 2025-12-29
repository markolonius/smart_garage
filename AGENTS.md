<!-- OPENSPEC:START -->
# OpenSpec Instructions

These instructions are for AI assistants working in this project.

Always open `@/openspec/AGENTS.md` when the request:
- Mentions planning or proposals (words like proposal, spec, change, plan)
- Introduces new capabilities, breaking changes, architecture shifts, or big performance/security work
- Sounds ambiguous and you need the authoritative spec before coding

Use `@/openspec/AGENTS.md` to learn:
- How to create and apply change proposals
- Spec format and conventions
- Project structure and guidelines

Keep this managed block so 'openspec update' can refresh the instructions.

<!-- OPENSPEC:END -->

## Essential Commands

### Build System
```bash
idf.py build                    # Build project
idf.py -p PORT flash            # Flash to device
idf.py -p PORT monitor          # Monitor serial output
idf.py -p PORT flash monitor    # Build, flash, and monitor
idf.py menuconfig               # Configure project
idf.py fullclean                # Clean build artifacts
```

### Testing
```bash
pytest pytest_hello_world.py                           # Run all tests
pytest pytest_hello_world.py::test_hello_world         # Run single test
pytest -v pytest_hello_world.py                        # Verbose output
```

### Code Quality
```bash
clang-format -i main/*.c              # Format C files
clang-format --dry-run -Werror main/*.c  # Check formatting
clang-tidy main/*.c -- -I$IDF_PATH/components/...  # Static analysis
```

## Code Style Guidelines

### Naming Conventions
- **Functions**: `lower_case_with_underscores`
- **Variables**: `lower_case_with_underscores`
- **Macros**: `UPPER_CASE_WITH_UNDERSCORES`
- **Types/structs/enum**: `lower_case_with_underscores`
- **File names**: `lower_case_with_underscores.c/.h`

### Formatting (Based on .clang-format)
- **Indentation**: 4 spaces, no tabs
- **Line length**: 120 characters maximum
- **Braces**: Linux style (opening brace on same line for functions, new line for others)
- **Pointers**: Right-aligned (`int *ptr` not `int* ptr`)
- **Includes**: Sorted alphabetically

### Imports and Includes
- Keep includes sorted alphabetically
- Use angle brackets for system/ESP-IDF headers: `#include <stdio.h>`
- Use quotes for local headers: `#include "my_header.h"`
- Standard library headers first, then ESP-IDF, then local

### Types
- Use standard C types (`uint32_t`, `int32_t`) from `<inttypes.h>` or `<stdint.h>`
- Use `printf` format macros: `PRIu32` for `uint32_t`, etc.
- ESP-IDF types: `esp_err_t`, `esp_chip_info_t`, etc.

### Error Handling
- Check return values from ESP-IDF functions: `if (func() != ESP_OK) { return; }`
- Use ESP-IDF error codes: `ESP_OK`, `ESP_FAIL`, `ESP_ERR_*`
- Log errors using ESP_LOG* macros when needed
- Return early on errors in `app_main()` (but don't return normally)

### ESP-IDF Specifics
- Entry point: `void app_main(void)` - not `main()`
- Never return from `app_main()` normally - call `esp_restart()` or loop forever
- Use FreeRTOS timing: `vTaskDelay(1000 / portTICK_PERIOD_MS)` for 1 second
- Task creation: `xTaskCreate()` for general tasks
- Flush output before restart: `fflush(stdout)`

### Comments and Documentation
- Use C-style `/* */` for SPDX headers and file comments
- Keep comments concise and meaningful
- No trailing whitespace
- Max 1 empty line between blocks

### FreeRTOS Best Practices
- All timing uses `portTICK_PERIOD_MS`
- Task priorities: `1` (idle) to `24` (max)
- Stack size: multiples of 1024 bytes (e.g., `2048`, `4096`)
- Use mutexes for shared resources
- Queue depth should match expected usage

### Component Registration
Components are registered in `main/CMakeLists.txt` using `idf_component_register()`:
- **SRCS**: Source files to compile
- **PRIV_REQUIRES**: Private component dependencies (e.g., `spi_flash`)
- **INCLUDE_DIRS**: Include directories for the component
- **REQUIRES**: Public component dependencies (exports symbols)

### Project Structure
```
smart_garage/
├── CMakeLists.txt          # Top-level CMake (includes ESP-IDF project.cmake)
├── sdkconfig               # Project configuration (generated/modified by menuconfig)
├── main/                   # Main component (required)
│   ├── CMakeLists.txt      # Component registration
│   └── *.c                 # Source files with app_main() entry point
├── build/                  # Build output directory
└── openspec/               # Spec-driven development files
```

### Minimal Build
Project uses `MINIMAL_BUILD ON` in top-level CMakeLists.txt:
- Only includes explicitly required components
- Reduces build time and binary size
- Requires explicit component dependencies in `PRIV_REQUIRES` or `REQUIRES`

## Landing the Plane

When ending a work session, complete ALL steps:
1. File issues for remaining work
2. Run quality gates (tests, linters, build)
3. Update issue status
4. Push to remote: `git pull --rebase && git push`
5. Clean up: clear stashes, prune branches
6. Verify: all changes committed and pushed
7. Hand off with context for next session

CRITICAL: Work is NOT complete until `git push` succeeds. Never stop before pushing.
