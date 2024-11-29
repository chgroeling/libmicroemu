# Programmer's Manual for `libmicroemu` {#manual}

\tableofcontents


## Introduction 

Welcome to the Programmer's Manual for the `libmicroemu` Library!  
This documentation provides an overview of the library and its key functionalities. It is a work in progress and will be updated regularly.


## Overview of the Library

### Namespaces
- **Public Namespace**:  
  All user-accessible features are within `::libmicroemu`.
  
- **Private Namespace**:  
  Internals of the library reside in `::libmicroemu::internal`. These are not accessible to library users.

### Key Classes
- **libmicroemu::Machine**:  
  The primary class for controlling the emulator. It allows configuration, ELF loading, and execution.

- **libmicroemu::ExecResult**:  
  Represents the result of an emulation run, detailing why execution stopped.

- **libmicroemu::CpuStates**:  
  Encapsulates the emulator's state, including CPU registers and exception states.


## Typical Usage

Here’s an example demonstrating a typical use case for the emulator:

### Example Code

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

### Step 1: Memory Setup
- Use `libmicroemu::Machine::SetFlashSegment` to define the FLASH memory.
- Use `libmicroemu::Machine::SetRam1Segment` (and optionally `SetRam2Segment`) to set up RAM memory.

### Step 2: Load ELF Files
- Load an ELF file using `libmicroemu::Machine::Load`.
  - Ensure that the allocated FLASH memory can hold the ELF file contents.
  - The entry point can be set automatically from the ELF file or manually.

### Step 3: Start Emulation
- Start emulation using `libmicroemu::Machine::Exec`.
  - Execution will stop under specific conditions:
    - Reaching a maximum instruction count (useful for coroutine-like workflows).
    - Program exit via:
      - A **SVC call**.
      - **Semihosting** (e.g., `exit()` in the emulated program). Semihosting automatically handles program termination.


## Logging

The library provides customizable logging to capture emulator activity.

### Available Loggers
- **libmicroemu::StaticLogger**:  
  Outputs logs to static locations or files.

- **libmicroemu::NullLogger**:  
  Disables logging entirely, useful for performance-critical scenarios.

