#pragma once

#include "libmicroemu/cpu_states.h"
#include "libmicroemu/emu_context.h"
#include "libmicroemu/exec_result.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

#include <array>
#include <functional>

/**
 * @brief The \ref libmicroemu namespace contains all classes and functions of the libmicroemu which
 * are public.
 */
namespace libmicroemu {

/**
 * @brief The libmicroemu::internal namespace contains all classes and functions of libmicroemu
 * which are private.
 */
namespace internal {
// forward declarations
template <typename TCpuStates> class Emulator;
}; // namespace internal

using FPreExecStepCallback = std::function<void(EmuContext &)>;
using FPostExecStepCallback = std::function<void(EmuContext &)>;
using FStateCallback = std::function<void(IRegAccessor &, ISpecialRegAccessor &)>;

class Machine {
public:
  /**
   *  Constructor
   */
  Machine() noexcept;

  /**
   * Deconstructor
   */
  ~Machine() noexcept;

  /**
   * @brief Loads an elf file into memory
   * @param elf_file
   * @return
   */
  StatusCode Load(const char *elf_file, bool set_entry_point = true) noexcept;

  StatusCode Reset() noexcept;

  /**
   * @brief Executes the loaded program
   * @param max_instructions Maximum number of instructions to execute. -1 means infinite
   * @param cb_pre_exec Callback function to be called before each instruction is executed
   * @param cb_post_exec Callback function to be called after each instruction is executed
   * @return Result of the execution
   */
  ExecResult Exec(i32 max_instructions = -1, FPreExecStepCallback cb_pre_exec = nullptr,
                  FPostExecStepCallback cb_post_exec = nullptr) noexcept;

  /**
   * @brief Sets the Flash segment
   * The flash segment is where a microcontroller stores its program code. A flash
   * segment is read-only.
   * @param seg_ptr Pointer to the flash segment
   * @param seg_size Size of the flash segment
   * @param seg_vadr Virtual address of the flash segment
   */
  void SetFlashSegment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) noexcept;

  /**
   * @brief Sets the RAM1 segment
   * The RAM1 segment is where a microcontroller stores its data. A RAM1 segment
   * is read-write.
   * @param seg_ptr Pointer to the RAM1 segment
   * @param seg_size Size of the RAM1 segment
   * @param seg_vadr Virtual address of the RAM1 segment
   */
  void SetRam1Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) noexcept;

  /**
   * @brief Sets the RAM2 segment
   * The RAM2 segment is were a microcontroller stores its data. A RAM2 segment
   * is read-write.
   * @param seg_ptr Pointer to the RAM2 segment
   * @param seg_size Size of the RAM2 segment
   * @param seg_vadr Virtual address of the RAM2 segment
   */
  void SetRam2Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) noexcept;

  /**
   * @brief Evaluates the state of the processor
   * This function evaluates the state of the processor by calling a function which
   * gives access to the registers and special registers.
   * \attention The callback is not allowed to throw exceptions.
   * @param cb  Callback function to evaluate the state
   */
  void EvaluateState(FStateCallback cb) noexcept;

  static void RegisterLoggerCallback(void (*callback)(libmicroemu::LogLevel level, const char *,
                                                      ...) noexcept) noexcept;

  /**
   * @brief Gets the version of the library
   * @return The semver version of the library in the format "MAJOR.MINOR.PATCH"
   */
  std::string_view GetVersion() noexcept;

private:
  internal::Emulator<CpuStates> BuildEmulator();
  u8 *flash_{nullptr};
  me_size_t flash_size_{0U};
  me_adr_t flash_vadr_{0x0U};

  u8 *ram1_{nullptr};
  me_size_t ram1_size_{0U};
  me_adr_t ram1_vadr_{0x0U};

  u8 *ram2_{nullptr};
  me_size_t ram2_size_{0U};
  me_adr_t ram2_vadr_{0x0U};

  CpuStates cpu_states_{};
};

} // namespace libmicroemu