#pragma once

#include "libmicroemu/emu_context.h"
#include "libmicroemu/emu_flags.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/processor_states.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

#include <array>
#include <functional>

namespace microemu {

// forward declarations
namespace internal {
template <typename TProcessorStates> class Emulator;
};

using FPreExecStepCallback = std::function<void(EmuContext &)>;
using FPostExecStepCallback = std::function<void(EmuContext &)>;
using FStateCallback = std::function<void(IRegAccessor &, ISpecialRegAccessor &)>;

class MicroEmu {
public:
  /// Constructor
  MicroEmu() noexcept;

  /// Deconstructor
  ~MicroEmu() noexcept;

  /// @brief Loads an elf file into memory
  /// @param elf_file
  /// @return
  Result<void> Load(const char *elf_file, bool set_entry_point = true) noexcept;

  Result<void> Reset() noexcept;

  /// @brief Executes the loaded program
  /// @param max_instructions Maximum number of instructions to execute. -1 means infinite
  /// @param cb_pre_exec Callback function to be called before each instruction is executed
  /// @param cb_post_exec Callback function to be called after each instruction is executed
  /// @return Result of the execution
  Result<EmuResult> Exec(i32 max_instructions = -1, FPreExecStepCallback cb_pre_exec = nullptr,
                         FPostExecStepCallback cb_post_exec = nullptr) noexcept;

  /// @brief Sets the Flash segment
  /// The flash segment is were a microcontroller stores its program code. A flash
  /// segment is read-only.
  /// @param seg_ptr Pointer to the flash segment
  /// @param seg_size Size of the flash segment
  /// @param seg_vadr Virtual address of the flash segment
  void SetFlashSegment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) noexcept;

  /// @brief Sets the RAM1 segment
  /// The RAM1 segment is were a microcontroller stores its data. A RAM1 segment
  /// is read-write.
  /// @param seg_ptr Pointer to the RAM1 segment
  /// @param seg_size Size of the RAM1 segment
  /// @param seg_vadr Virtual address of the RAM1 segment
  void SetRam1Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) noexcept;

  /// @brief Sets the RAM2 segment
  /// The RAM2 segment is were a microcontroller stores its data. A RAM2 segment
  /// is read-write.
  /// @param seg_ptr Pointer to the RAM2 segment
  /// @param seg_size Size of the RAM2 segment
  /// @param seg_vadr Virtual address of the RAM2 segment
  void SetRam2Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) noexcept;

  /// @brief Evaluates the state of the processor
  /// This function evaluates the state of the processor by calling a function which
  /// gives access to the registers and special registers.
  /// \attention The callback is not allowed to throw exceptions.
  /// @param cb  Callback function to evaluate the state
  void EvaluateState(FStateCallback cb) noexcept;

  static void RegisterLoggerCallback(void (*callback)(microemu::LogLevel level, const char *,
                                                      ...) noexcept) noexcept;

private:
  internal::Emulator<ProcessorStates> BuildEmulator();
  u8 *flash_{nullptr};
  me_size_t flash_size_{0U};
  me_adr_t flash_vadr_{0x0U};

  u8 *ram1_{nullptr};
  me_size_t ram1_size_{0U};
  me_adr_t ram1_vadr_{0x0U};

  u8 *ram2_{nullptr};
  me_size_t ram2_size_{0U};
  me_adr_t ram2_vadr_{0x0U};

  ProcessorStates pstates_{};
};

} // namespace microemu