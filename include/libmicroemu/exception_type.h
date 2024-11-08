#pragma once

#include "libmicroemu/types.h"

namespace microemu {

/// @brief Exception enumeration
///
/// The enumeration of ExceptionsOps that can be triggered by the processor.
enum class ExceptionType {

  kReset = 1U, ///< Special exception to reset the processor

  /// Highest priority exception (for except the reset) that cannot be ever masked away.
  /// Can be triggered by a peripheral or triggered by software.
  kNMI = 2U,

  /// Hard fault exception, triggered by faults that cannot be handled by any other exception.
  kHardFault = 3U,

  /// Memory related fault (bus access error either for instructions or data)
  kMemoryManagementFault = 4U,

  /// Bus fault exception, triggered by bus errors on instruction fetches and data accesses.
  kBusFault = 5U,

  /// Usage fault exception, triggered by errors in the usage of the processor.
  kUsageFault = 6U,

  /// Reserved for future use
  kReserved1 = 7U,

  /// Reserved for future use
  kReserved2 = 8U,

  /// Reserved for future use
  kReserved3 = 9U,

  /// Reserved for future use
  kReserved4 = 9U,

  /// Reserved for future use
  kReserved5 = 10U,

  /// Exception for system calls
  kSVCall = 11U,

  /// Debugging related ExceptionsOps
  kDebugMonitor = 12U,

  /// Reserved for future use
  kReserved6 = 13U,

  /// PendSV exception, used for context switching.
  kPendSV = 14U,

  /// SysTick exception, used for system timer.
  kSysTick = 15U

  /// Exception from a peripheral or software triggered interrupt
  /// ...

};

static constexpr auto kNoOfExternalIrqs = 32U;
static constexpr auto kLastInternalException = ExceptionType::kSysTick;

static constexpr u32 CountInternalExceptions() noexcept {
  return static_cast<u32>(kLastInternalException) + 1U;
};

static constexpr u32 CountExceptions() noexcept {
  return CountInternalExceptions() + kNoOfExternalIrqs;
};

} // namespace microemu