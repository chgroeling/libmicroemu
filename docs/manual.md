# Programmer's Manual for `libmicroemu` {#manual}
\tableofcontents

## Introduction 

Welcome to the Programmer's Manual for the `libmicroemu` Library!  
This documentation is currently a work in progress.

### Overview
- The library is structured into two namespaces:
  - **Public namespace**: `::libmicroemu`
  - **Private namespace**: `::libmicroemu::internal`
    - The private namespace is not exposed to users.

### Key Classes

- `libmicroemu::Machine`:  
  The main emulator class.


- `libmicroemu::ExecResult`:  
  This class is returned by `libmicroemu::Exec`. It contains the reason why the emulator stopped execution.

- `libmicroemu::CpuStates`:  
  Represents all states used by the emulator, such as CPU registers and exception states.

## A typical use case explained

A typical use case for `libmicroemu`:

```cpp
#include "libmicroemu/machine.h"
#include <iostream>
#include <vector>

// Path to the ELF file
constexpr static const char *kElfPath = "coremark/prebuilt/bin/coremark.elf";

constexpr static libmicroemu::me_adr_t kFlashSegVadr = 0x0U;
constexpr static libmicroemu::me_adr_t kRam1SegVadr = 0x20000000U;

int main()
{
    libmicroemu::Machine machine;
    std::cout << "Using libmicroemu version: " << machine.GetVersion() << std::endl;

    // Allocate memory for FLASH and RAM1 segments
    auto flash_seg = std::vector<uint8_t>(0x20000u); // 128KB for FLASH
    auto ram1_seg = std::vector<uint8_t>(0x40000u);  // 256KB for RAM1

    // Set up memory segments
    machine.SetFlashSegment(flash_seg.data(), flash_seg.size(), kFlashSegVadr);
    machine.SetRam1Segment(ram1_seg.data(), ram1_seg.size(), kRam1SegVadr);

    // Load the ELF file and set the entry point
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

The `libmicroemu::Machine` class is the core of the emulator. Its usage involves the following steps:

1. **Memory Setup**:  
   Before running the emulator, the memory layout must be configured:
   - Use `libmicroemu::Machine::SetFlashSegment` to define the FLASH memory segment.
   - Use `libmicroemu::Machine::SetRam1Segment` and optionally `libmicroemu::Machine::SetRam2Segment` for RAM memory segments.

2. **Loading ELF Files**:
   - ELF files are loaded into the FLASH segment using `libmicroemu::Machine::Load`.
   - **Important**: Ensure that the memory allocated for the FLASH segment is sufficient to hold the binary contents of the ELF file.
   - The `libmicroemu::Machine::Load` method can optionally set the entry point from the ELF file. If this option is not used, the entry point defaults to the reset interrupt vector.

3. **Emulation**:
   - Start emulation using `libmicroemu::Machine::Exec`.
   - Execution continues indefinitely unless interrupted by:
     - Specifying a maximum number of instructions to execute, which allows using the emulator as a coroutine within a host program.
     - The emulated program itself, through:
       - A specific **SVC call**.
       - **Semihosting**, which can be triggered by calls like `exit()` in the emulated program. Semihosting handles the exit process automatically.

## Logging
- `libmicroemu::StaticLogger` and `libmicroemu::NullLogger`:  
  These classes handle logging for the emulator. The emulator uses them to log relevant information when needed.