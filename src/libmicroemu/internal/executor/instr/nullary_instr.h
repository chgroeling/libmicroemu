#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr/post_exec.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Nop instruction
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.212
 */
template <typename TInstrContext> class Nop0Op {
public:
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags) {
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Dmb instruction
 *
 * A Data Memory Barrier operation completes when all explicit memory accesses before the DMB.
 * On this emulator, it is a NOP
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.230
 */
template <typename TInstrContext> class Dmb0Op {
public:
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags) {
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class NullaryInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  NullaryInstr() = delete;

  /**
   * @brief Destructor
   */
  ~NullaryInstr() = delete;

  /**
   * @brief Copy constructor for NullaryInstr.
   * @param r_src the object to be copied
   */
  NullaryInstr(const NullaryInstr &r_src) = delete;

  /**
   * @brief Copy assignment operator for NullaryInstr.
   * @param r_src the object to be copied
   */
  NullaryInstr &operator=(const NullaryInstr &r_src) = delete;

  /**
   * @brief Move constructor for NullaryInstr.
   * @param r_src the object to be moved
   */
  NullaryInstr(NullaryInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  NullaryInstr.
   * @param r_src the object to be moved
   */
  NullaryInstr &operator=(NullaryInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal