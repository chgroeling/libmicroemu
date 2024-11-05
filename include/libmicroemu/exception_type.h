#pragma once

#include "libmicroemu/types.h"

namespace microemu {

/// @brief Exception enumeration
///
/// The enumeration of ExceptionsOps that can be triggered by the processor.
enum class ExceptionType {

  kReset = 1u, ///< Special exception to reset the processor

  /// Highest priority exception (for except the reset) that cannot be ever masked away.
  /// Can be triggered by a peripheral or triggered by software.
  kNMI = 2u,

  /// Hard fault exception, triggered by faults that cannot be handled by any other exception.
  kHardFault = 3u,

  /// Memory related fault (bus access error either for instructions or data)
  kMemoryManagementFault = 4u,

  /// Bus fault exception, triggered by bus errors on instruction fetches and data accesses.
  kBusFault = 5u,

  /// Usage fault exception, triggered by errors in the usage of the processor.
  kUsageFault = 6u,

  /// Reserved for future use
  kReserved1 = 7u,

  /// Reserved for future use
  kReserved2 = 8u,

  /// Reserved for future use
  kReserved3 = 9u,

  /// Reserved for future use
  kReserved4 = 9u,

  /// Reserved for future use
  kReserved5 = 10u,

  /// Exception for system calls
  kSVCall = 11u,

  /// Debugging related ExceptionsOps
  kDebugMonitor = 12u,

  /// Reserved for future use
  kReserved6 = 13u,

  /// PendSV exception, used for context switching.
  kPendSV = 14u,

  /// SysTick exception, used for system timer.
  kSysTick = 15u

  /// Exception from a peripheral or software triggered interrupt
  /// ...

};

static constexpr auto kNoOfExternalIrqs = 32u;
static constexpr auto kLastInternalException = ExceptionType::kSysTick;

static constexpr u32 CountInternalExceptions() noexcept {
  return static_cast<u32>(kLastInternalException) + 1u;
};

static constexpr u32 CountExceptions() noexcept {
  return CountInternalExceptions() + kNoOfExternalIrqs;
};

} // namespace microemu