#pragma once

#include "libmicroemu/exception_states.h"
#include "libmicroemu/exception_type.h"
#include "libmicroemu/register_id.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"
#include <array>
#include <assert.h>
#include <cstddef>
#include <cstdint>

namespace microemu {

// TODO: Rename to TCpuStates

class ProcessorStates {

public:
  /// \brief Constructs a Registers object
  ProcessorStates() {
    registers_.fill(0u);
    special_registers_.fill(0u);
  };

  /// \brief Destructor
  ~ProcessorStates() = default;

  /// \brief Copy constructor for Registers.
  /// \param r_src the object to be copied
  ProcessorStates(ProcessorStates &r_src) = default;

  /// \brief Copy assignment operator for Registers.
  /// \param r_src the object to be copied
  ProcessorStates &operator=(const ProcessorStates &r_src) = default;

  /// \brief Move constructor for Registers.
  /// \param r_src the object to be copied
  ProcessorStates(ProcessorStates &&r_src) = default;

  /// \brief Move assignment operator for Registers.
  /// \param r_src the object to be copied
  ProcessorStates &operator=(ProcessorStates &&r_src) = default;

  auto &GetRegisters() { return registers_; }
  const auto &GetRegisters() const { return registers_; }

  auto &GetSpecialRegisters() { return special_registers_; }
  const auto &GetSpecialRegisters() const { return special_registers_; }

  auto &GetExceptionStates() { return exception_states_; }
  const auto &GetExceptionStates() const { return exception_states_; }

private:
  std::array<u32, kNoOfRegisters> registers_;
  std::array<u32, CountPersistentSpecialRegisters()> special_registers_;
  ExceptionStates exception_states_;
};

} // namespace microemu
