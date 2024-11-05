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
using FStateCallback = std::function<void(IRegisterAccess &, ISpecialRegAccess &)>;

class MicroEmu {
public:
  /// Constructor
  MicroEmu();

  /// Deconstructor
  ~MicroEmu();

  /// @brief Loads an elf file into memory
  /// @param elf_file
  /// @return
  Result<void> Load(const char *elf_file, bool set_entry_point = true);

  Result<void> Reset();

  /// @brief Executes number of max_instructions.
  /// @param max_instructions
  /// @param trace
  /// @return
  Result<EmuResult> Exec(i32 max_instructions = -1, FPreExecStepCallback cb_pre_exec = nullptr,
                         FPostExecStepCallback cb_post_exec = nullptr);

  void SetFlashSegment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr);
  void SetRam1Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr);
  void SetRam2Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr);

  void EvaluateState(FStateCallback cb);

  static void RegisterLoggerCallback(void (*callback)(microemu::LogLevel level, const char *, ...));

private:
  internal::Emulator<ProcessorStates> BuildEmulator();
  u8 *flash_{nullptr};
  me_size_t flash_size_{0u};
  me_adr_t flash_vadr_{0x0u};

  u8 *ram1_{nullptr};
  me_size_t ram1_size_{0u};
  me_adr_t ram1_vadr_{0x0u};

  u8 *ram2_{nullptr};
  me_size_t ram2_size_{0u};
  me_adr_t ram2_vadr_{0x0u};

  ProcessorStates pstates_{};
};

} // namespace microemu