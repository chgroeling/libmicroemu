# Programmer's Manual for `libmicroemu` {#manual}

\tableofcontents

## Introduction 

Welcome to the Programmer's Manual for the `libmicroemu` Library!  

It is a work in progress and will be updated regularly.

## Overview of the Library

### Namespaces
- **Public Namespace**:  
  All user-accessible features are within `::libmicroemu`.
  
- **Private Namespace**:  
  Internals of the library reside in `::libmicroemu::internal`. These are not accessible to library users.

### Key Classes
- **libmicroemu::Machine**:  
  The primary class for controlling the emulator. It allows configuration, ELF loading, and execution.

- **libmicroemu::CpuStates**:  
  Encapsulates the emulator's state, including CPU registers and exception states.

- **libmicroemu::ExecResult**:  
  Represents the result of an emulation run, detailing why execution stopped.

## Error Management

- **Consistent Error Handling**:  
  The error management system in `libmicroemu` ensures consistent and predictable handling of errors across all APIs. Focused on performance and simplicity, it avoids exceptions in favor of lightweight mechanisms.

- **Key Components**:
  - `libmicroemu::StatusCode`:  
    Enum class defining operation outcomes, including success (`libmicroemu::StatusCode::kSuccess`) and various error categories such as memory issues (`libmicroemu::StatusCode::kMemInaccesible`), decoding failures (`libmicroemu::StatusCode::kDecoderUnknownOpCode`), and ELF-related errors (`libmicroemu::StatusCode::kElfNotValid`).
  - `libmicroemu::StatusCodeToString`:  
    Converts a `StatusCode` into a human-readable string for debugging and logging purposes.
  - `libmicroemu::internal::Result` Class:  
    Used internally to encapsulate either a result or a `StatusCode` in case of failure. Provides flexibility for handling complex results.
  
- **Macros for Error Propagation**:
  - \ref TRY : Forwards results to the caller if a function returns an error.
  - \ref TRY_ASSIGN : Combines result assignment and error propagation in a single step.

## Logging

### Overview
- **Purpose**:  
  Logging in `libmicroemu` is used for tracking emulator activity, debugging, and performance monitoring.

### Logging Configuration
1. **Registering a Callback**:  
   - Use `libmicroemu::Machine::RegisterLoggerCallback`.  
   - Callback is static for performance reasons.  
   - Expected function signature is clearly defined.

2. **Log Levels**:  
   - Log levels are build-dependent and defined in `libmicroemu::LogLevel`:  
     - Debug: `libmicroemu::LogLevel::kTrace` and above.  
     - Release: `libmicroemu::LogLevel::kInfo` and above.

### Built-In Loggers
1. **libmicroemu::StaticLogger**:  
   Outputs logs to static files or predefined locations.

2. **libmicroemu::NullLogger**:  
   Disables logging entirely for performance-critical applications.



## Typical Usage

Here’s an example demonstrating a typical use case for the emulator:

### Example Code

@include ./examples/simple/main.cpp

### Step 1: Memory Setup
- Before the emulator can start operating, it must know the memory segments where the binary content will be stored.
- These segments are typically specified in the implementation documentation of the ARMv7-M architecture.
- Commonly used memory segment addresses:
  - Flash: Starts at `0x00000000`
  - SRAM: Starts at `0x20000000`
- Define FLASH memory using `libmicroemu::Machine::SetFlashSegment`.
- Set up RAM memory using `libmicroemu::Machine::SetRam1Segment` (optionally `libmicroemu::Machine::SetRam2Segment` for additional RAM).

### Step 2: Load ELF Files
- Load an ELF file using `libmicroemu::Machine::Load`.
- Ensure the allocated FLASH memory can accommodate the ELF file contents.
- Set the entry point either automatically (from the ELF file) or manually.
- If the entry point is not set from the ELF file, the reset vector specifies the entry point.

### Step 3: Start Emulation
- Start emulation with `libmicroemu::Machine::Exec`.
- Emulation stops under specific conditions:
  - Reaching a maximum instruction count (useful for coroutine-like workflows).
  - Program exit via:
    - **SVC call**.
    - **Semihosting**, e.g., `exit()` in the emulated program (handles termination automatically).

## Extensibility
TBD