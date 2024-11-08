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
    registers_.fill(0U);
    special_registers_.fill(0U);
  };

  /// \brief Destructor
  ~ProcessorStates() noexcept = default;

  /// \brief Copy constructor for Registers.
  /// \param r_src the object to be copied
  ProcessorStates(ProcessorStates &r_src) noexcept = default;

  /// \brief Move constructor for Registers.
  /// \param r_src the object to be copied
  ProcessorStates(ProcessorStates &&r_src) noexcept = default;

  inline auto &GetRegisters() noexcept { return registers_; }
  inline const auto &GetRegisters() const noexcept { return registers_; }

  inline auto &GetSpecialRegisters() noexcept { return special_registers_; }
  inline const auto &GetSpecialRegisters() const noexcept { return special_registers_; }

  inline auto &GetExceptionStates() noexcept { return exception_states_; }
  inline const auto &GetExceptionStates() const noexcept { return exception_states_; }

private:
  /// \brief Copy assignment operator for Registers.
  /// \param r_src the object to be copied
  ProcessorStates &operator=(const ProcessorStates &r_src) noexcept = delete;

  /// \brief Move assignment operator for Registers.
  /// \param r_src the object to be copied
  ProcessorStates &operator=(ProcessorStates &&r_src) noexcept = delete;

  std::array<u32, CountRegisters()> registers_;
  std::array<u32, CountPersistentSpecialRegisters()> special_registers_;
  ExceptionStates exception_states_;
};

} // namespace microemu
