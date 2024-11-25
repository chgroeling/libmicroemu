# ARM Microcontroller Emulator Library 
_libmicroemu_ is a lightweight, embeddable library for ARM microcontroller emulation, currently supporting the ARMv7-M architecture with plans to extend to additional ARM architectures. It is designed for:

- **Accelerating development** by enabling local testing of ARM programs
- **Creating digital twins** for commissioning without physical hardware
- **Embedding ARM emulation** seamlessly into larger projects

While shipped with a command-line application (called _microemu_), _libmicroemu_ can be used independently, allowing for flexible integration.

[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/chgroeling/libmicroemu?label=release&sort=semver)](https://github.com/chgroeling/libmicroemu/releases)[![tests](https://github.com/chgroeling/libmicroemu/actions/workflows/tests.yml/badge.svg)](https://github.com/chgroeling/libmicroemu/actions/workflows/tests.yml)[![docs](https://github.com/chgroeling/libmicroemu/actions/workflows/docs.yml/badge.svg)](https://chgroeling.github.io/libmicroemu)[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


### Key Features
- **Dependency-Free Core**  
  No external libraries are needed to use _libmicroemu_.
- **Embeddable Design**  
  Integrate easily into any C++ project or use it as a standalone emulator.
- **Accurate ARM Cortex-M Simulation**  
  Emulates ARMv7-M with full exception support (e.g., UsageFault, HardFault, SysTick). Future plans include ARMv7-A and support for the NVIC.
- **Predictable and Deterministic Behavior**  
  The emulator ensures consistent execution by avoiding dynamic memory allocation and non-deterministic algorithms, making it suitable for applications requiring repeatable outcomes.
- **No C++ Exceptions**  
  The emulator is implemented without the use of C++ exceptions, providing a predictable control flow and simplifying integration into projects that avoid exception handling.
- **Optional Command-Line Interface**  
  The companion CLI program _microemu_ simplifies testing and debugging. It requires Conan for managing dependencies (fmt, spdlog, cxxopts), while _libmicroemu_ itself remains dependency-free.
- **Semihosting Support**    
  Provides built-in semihosting, enabling communication between the emulator and the host system for tasks such as file I/O and console output.

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
- **Doxygen Documentation Improvement** – Enhance Doxygen documentation to improve usability and clarity, adding detailed descriptions for modules, functions, and configuration options to assist developers and users.

## Getting Started
### Installation
1. **Clone the Repository:**
    Clone the repository to your local machine and navigate into the project directory:

    ```bash
    git clone https://github.com/chgroeling/libmicroemu
    cd libmicroemu
    ```
2. **Install Dependencies:**
    _libmicroemu_ uses Conan 2 for dependency management, with profiles for different machines stored in the ./profiles directory. While Conan is primarily needed for the _microemu_ CLI, it also installs toolchains and GTest, making its use advisable. Run the following command to install dependencies and set options for building standalone components:

    ```bash
    conan install . -pr ./profiles/<your_build_profile> -o build_tests=True -o build_microemu=True --build=missing
    ```

    Replace <your_build_profile> with the appropriate profile names for your setup.  This version uses the conan -o option for enabling _build_tests_ and _build_microemu_ when installing dependencies.
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

   

### Running the Emulator
With the build complete, you can run ARM binaries using _microemu_:

```bash
./microemu path/to/your-binary.elf
```

The application provides logs showing register states, memory operations, and decoded instructions to assist with debugging. Use the "--help" command line option to learn more.

### Precompiled Libraries and Releases
_libmicroemu_ includes precompiled libraries in its releases, tagged using semantic versioning (e.g., `v1.0.0`). These binaries simplify integration and reduce build time, allowing for quick use without the need for compiling from source. Available on the [GitHub releases page](https://github.com/chgroeling/libmicroemu/releases), they include platform-specific static libraries and the _microemu_ CLI program. For custom needs, building from source remains an option to meet specific configuration requirements.

## Embedding libmicroemu in Your Project
_libmicroemu_ is designed to be easily integrated into larger C++ projects to provide ARM emulation capabilities. Below is a basic example demonstrating how to set up and execute an ELF binary using the library.

### Example: Integrating libmicroemu
```cpp
#include "libmicroemu/machine.h"
#include <iostream>
#include <vector>

// Change this to the path of your ELF file
constexpr static const char *kElfPath = "coremark/prebuilt/bin/coremark.elf";

constexpr static libmicroemu::me_adr_t kFlashSegVadr = 0x0U;
constexpr static libmicroemu::me_adr_t kRam1SegVadr = 0x20000000U;

int main()
{
    libmicroemu::Machine machine;
    std::cout << "Using libmicroemu version: " << machine.GetVersion() << std::endl;

    auto flash_seg = std::vector<uint8_t>(0x20000u); // allocate 128KB for FLASH segment
    auto ram1_seg = std::vector<uint8_t>(0x40000u);  // allocate 256KB for RAM1 segment

    // Set the FLASH and RAM1 segments
    machine.SetFlashSegment(flash_seg.data(), flash_seg.size(), kFlashSegVadr);
    machine.SetRam1Segment(ram1_seg.data(), ram1_seg.size(), kRam1SegVadr);

    // Load the ELF file and set the entry point to the one given in the ELF file
    if (machine.Load(kElfPath, true) != libmicroemu::StatusCode::kSuccess)
    {
        std::cerr << "Failed to load ELF file." << std::endl;
        return EXIT_FAILURE;
    }

    // Execute the ELF file
    auto res_exec = machine.Exec();
    if (res_exec.IsErr())
    {
        std::cerr << "Failed to execute ELF file." << std::endl;
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
## Documentation
[Latest build - Doxygen documentation](https://chgroeling.github.io/libmicroemu)

## Contributing
Contributions are welcome! Please follow the Google C++ Style Guide, and ensure all new code includes documentation and test cases.

1. Fork the repository.
2. Create a feature branch.
3. Commit your changes.
4. Open a pull request.

## License
This project is licensed under the MIT License.