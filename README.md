# Libmicroemu - ARM Microcontroller Emulator Library
_libmicroemu_ is a lightweight, embeddable library for ARM microcontroller emulation, currently supporting the ARMv7-M architecture with plans to extend to additional ARM architectures. It is designed for:

- **Accelerating development** by enabling local testing of ARM programs
- **Creating digital twins** for commissioning without physical hardware
- **Embedding ARM emulation** seamlessly into larger projects

While shipped with a command-line application (_microemu_), _libmicroemu_ can be used independently, allowing for flexible integration.

[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/chgroeling/libmicroemu?label=release&sort=semver)](https://github.com/chgroeling/libmicroemu/releases)[![tests](https://github.com/chgroeling/libmicroemu/actions/workflows/tests.yml/badge.svg)](https://github.com/chgroeling/libmicroemu/actions/workflows/tests.yml)[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

_libmicroemu_ provides ARM emulation with simplicity and flexibility for embedding in C++ applications.

### Key Features

- **Dependency-Free Core**  
  No external libraries are needed to use libmicroemu.
- **Embeddable Design**  
  Integrate easily into any C++ project or use it as a standalone emulator.
- **Accurate ARM Cortex-M Simulation**  
  Emulates ARMv7-M with full exception support (e.g., UsageFault, HardFault, SysTick). Future plans include ARMv7-A and support for the NVIC.
- **Optional Command-Line Interface**  
  The companion CLI app microemu simplifies testing and debugging. It requires Conan for managing dependencies (fmt, spdlog, cxxopts), while libmicroemu itself remains dependency-free.
## Development Status

_libmicroemu_ is currently under active development and can already execute a significant range of ARMv7-M programs. Floating-point functionality is not natively supported in the emulator at this time, but it can be achieved by compiling programs with `-softfp`, allowing software-based floating-point emulation within the binary itself.

### Roadmap

- **ARM Core Completion** – Prioritize complete ARMv7-M functionality, then expand to other architectures like ARMv7-A.
- **Peripheral Emulation Expansion** – Limited peripheral support initially, focused on ARM core compatibility.
- **NVIC Support** – Planned implementation for ARMv7-M interrupt controller.
- **Hardware Floating-Point Emulation** – Planned support for native floating-point instructions.
- **Remote Debugging Support** – Planned GDB integration for interactive debugging capabilities.
- **Broader Configuration Options** – Further flexibility for custom peripherals and memory configurations.
- **Vendor-Specific Chip Support** - Expand support for vendor-specific peripherals and features (e.g. STM and Renesas chips)

## Getting Started

### Installation

1. **Clone the Repository:**
    Clone the repository to your local machine and navigate into the project directory:

    ```bash
    git clone https://github.com/chgroeling/libmicroemu
    cd libmicroemu
    ```

2. **Install Dependencies:**
    libmicroemu uses Conan 2 for dependency management, with profiles for different machines stored in the ./profiles directory. While Conan is primarily needed for the microemu CLI, it also installs toolchains and GTest, making its use advisable. Run the following command to install dependencies and set options for building standalone components:

    ```bash
    conan install . -pr ./profiles/<your_build_profile> -o build_tests=True -o build_microemu=True --build=missing
    ```

    Replace <your_build_profile> with the appropriate profile names for your setup.

3. **Build the Project with CMake:**
    Create a build directory, configure the project, and compile it:

    ```bash
    cd build && cd Debug
    cmake .
    cmake --build .
    ```

4.	**Run the Tests:**
    To verify that everything is set up correctly, run the tests:

    ```bash
    ctest
    ```

    This version uses the conan -o option for enabling build_tests and build_microemu when installing dependencies.

### Running the Emulator

With the build complete, you can run ARM binaries using microemu:

```bash
./microemu path/to/your-binary.elf
```

The application provides logs showing register states, memory operations, and decoded instructions to assist with debugging. Use the "--help" command line option to learn more.

## Embedding libmicroemu in Your Project

libmicroemu can be embedded directly in C++ projects to emulate ARM code. 

```cpp
#include <iostream>
#include <vector>

#include "libmicroemu/microemu.h"

constexpr static const char *elf_loc = "<elf_file>";

int main()
{
    // CODE Segment
    auto code_seg = std::vector<uint8_t>(0x20000u);
    auto code_seg_size = static_cast<uint32_t>(code_seg.size());
    auto code_seg_vadr = 0x0U;

    // RAM1 Segment
    auto ram1_seg = std::vector<uint8_t>(0x40000u);
    auto ram1_seg_size = static_cast<uint32_t>(ram1_seg.size());
    auto ram1_seg_vadr = 0x20000000U;

    microemu::MicroEmu lib;
    lib.SetCodeSection(code_seg.data(), code_seg_size, code_seg_vadr);
    lib.SetRam1Section(ram1_seg.data(), ram1_seg_size, ram1_seg_vadr);

    auto res = lib.Load(elf_loc, true);
    if (res.IsErr())
    {
        std::cerr << "Failed to load ELF file." << std::endl;
        return EXIT_FAILURE;
    }
    auto res_exec = lib.Exec(-1);
    if (res_exec.IsErr())
    {
        std::cerr << "Failed to load execute ELF file." << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}

````

## Contributing

Contributions are welcome! Please follow the Google C++ Style Guide, and ensure all new code includes documentation and test cases.

1. Fork the repository.
2. Create a feature branch.
3. Commit your changes.
4. Open a pull request.

## License

This project is licensed under the MIT License.