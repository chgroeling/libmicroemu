# ARM Microcontroller Emulator Library
_libmicroemu_ is a lightweight, embeddable library for ARM microcontroller emulation, currently supporting the ARMv7-M architecture with plans to extend to additional ARM architectures. It is designed for:

- **Accelerating development** by enabling local testing of ARM programs
- **Creating digital twins** for commissioning without physical hardware
- **Embedding ARM emulation** seamlessly into larger projects

While shipped with a command-line application (_microemu_), _libmicroemu_ can be used independently, allowing for flexible integration.

_libmicroemu_ provides ARM emulation with simplicity and flexibility for embedding in C++ applications.

[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/chgroeling/libmicroemu?label=release&sort=semver)](https://github.com/chgroeling/libmicroemu/releases)[![tests](https://github.com/chgroeling/libmicroemu/actions/workflows/tests.yml/badge.svg)](https://github.com/chgroeling/libmicroemu/actions/workflows/tests.yml)[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


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
_libmicroemu_ is currently under active development and can already execute a significant range of ARMv7-M programs. Floating-point functionality is not natively supported in the emulator at this time, but it can be achieved by compiling programs with `-mfloat-abi=soft`, allowing software-based floating-point emulation within the binary itself.

### Roadmap
- **ARM Core Completion** – Prioritize complete ARMv7-M functionality, then expand to other architectures like ARMv7-A.
- **Performance Optimization** – Implement performance optimizations after achieving full ARMv7-M support.
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
_libmicroemu_ is designed to be easily integrated into larger C++ projects to provide ARM emulation capabilities. Below is a basic example demonstrating how to set up and execute an ELF binary using the library.

### Example: Integrating libmicroemu
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
```

### Embedding Considerations
- **CMake Integration**: You can include _libmicroemu_ directly in your project by adding it as a CMake subdirectory:
  ```cmake
  add_subdirectory(path/to/libmicroemu)
  target_link_libraries(your_project PRIVATE libmicroemu)
  ```
- **Precompiled Libraries**: For convenience, precompiled versions of _libmicroemu_ are available. These can be linked to your project to save build time and simplify integration.
- **Memory Configuration**: Ensure you allocate and set up code and RAM segments properly to emulate the ARM memory layout accurately.
- **Integration Points**: _libmicroemu_ can be seamlessly embedded into any C++ project, supporting both development testing and deployment phases.
- **Custom Enhancements**: Depending on your application, you can extend memory configurations, include peripheral simulation, or adapt the emulator for specific use cases.

## Performance Results
The CoreMark benchmark for a 2K performance run yielded **162.73 CoreMarks** on the emulator (detailed report follows), roughly equivalent to a Cortex-M4 at 50 MHz. This baseline result was measured on a MacBook Air with an M1 processor. Currently, the focus is on completing ARMv7-M support, with performance optimizations planned for future updates.

**Background**: CoreMark is a widely recognized benchmark used to assess the performance of microcontrollers and processors. For comparison, a typical ARM Cortex-M4 running at 100 MHz might score between 250 to 350 CoreMarks, assuming linear scaling with clock speed.

```
2K performance run parameters for coremark.
CoreMark Size    : 666
Total ticks      : 2458
Total time (secs): 24.580000
Iterations/Sec   : 162.733930
Iterations       : 4000
Compiler version : GCC10.3.1 20210824 (release)
Compiler flags   : -g
Memory location  : STACK
seedcrc          : 0xe9f5
[0]crclist       : 0xe714
[0]crcmatrix     : 0x1fd7
[0]crcstate      : 0x8e3a
[0]crcfinal      : 0x65c5
Correct operation validated. See README.md for run and reporting rules.
CoreMark 1.0 : 162.733930 / GCC10.3.1 20210824 (release) -g / STACK
```

## Contributing
Contributions are welcome! Please follow the Google C++ Style Guide, and ensure all new code includes documentation and test cases.

1. Fork the repository.
2. Create a feature branch.
3. Commit your changes.
4. Open a pull request.

## License
This project is licensed under the MIT License.