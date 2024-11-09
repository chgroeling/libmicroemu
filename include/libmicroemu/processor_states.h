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

namespace libmicroemu {

/** @brief Processor states class.
 *
 * This class holds all the states necessary for emulating a processor.
 * Mainly these are the registers, special registers and exception states.
 */
class ProcessorStates {

public:
  /** @brief Constructs a ProcessorStates object.
   */
  ProcessorStates() : registers_{}, special_registers_{}, exception_states_{} {
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

  /** @brief Get a reference on the raw register array.
   *
   * This function returns a reference to the raw register array.
   *
   * @return The raw register array.
   */
  inline auto &GetRegisters() noexcept { return registers_; }

  /** @brief Get a const reference on the raw register array.
   *
   * This function returns a const reference to the raw register array.
   *
   * @return The raw register array.
   */
  inline const auto &GetRegisters() const noexcept { return registers_; }

  /** @brief Get a reference on the raw special register array.
   *
   * This function returns a reference to the raw special register array.
   *
   * @return The raw special register array.
   */
  inline auto &GetSpecialRegisters() noexcept { return special_registers_; }

  /** @brief Get a const reference on the raw special register array.
   *
   * This function returns a const reference to the raw special register array.
   *
   * @return The raw special register array.
   */
  inline const auto &GetSpecialRegisters() const noexcept { return special_registers_; }

  /** @brief Get a reference on the exception states.
   *
   * This function returns a reference to the exception states.
   *
   * @return The exception states.
   */
  inline auto &GetExceptionStates() noexcept { return exception_states_; }

  /** @brief Get a const reference on the exception states.
   *
   * This function returns a const reference to the exception states.
   *
   * @return The exception states.
   */
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

} // namespace libmicroemu
