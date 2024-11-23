#pragma once
#include "libmicroemu/internal/decoder/decoder.h"

#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Branch X
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.205
 */
template <typename TInstrContext> class Bx1Op {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rm) {
    static_cast<void>(iflags);
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto bres = static_cast<me_adr_t>(m);

    TRY(InstrExecFlagsSet, PostExecBxWritePc::Call(ictx, bres));
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Branch link X
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.205
 */
template <typename TInstrContext> class Blx1Op {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rm) {
    static_cast<void>(iflags);
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const me_adr_t next_instr_address = static_cast<me_adr_t>(pc - 2U);

    const auto lr_val = static_cast<uint32_t>((next_instr_address & ~0x1) | 0x1U);
    ictx.cpua.template WriteRegister<RegisterId::kLr>(lr_val);

    const auto bres = static_cast<me_adr_t>(m);
    TRY(InstrExecFlagsSet, PostExecBxWritePc::Call(ictx, bres));
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class UnaryBranchInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  using ExcTrig = typename TInstrContext::ExcTrig;

  template <typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rm) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rm));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryBranchInstr() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryBranchInstr() = delete;

  /**
   * @brief Copy constructor for UnaryBranchInstr.
   * @param r_src the object to be copied
   */
  UnaryBranchInstr(const UnaryBranchInstr &r_src) = delete;

  /**
   * @brief Copy assignment operator for UnaryBranchInstr.
   * @param r_src the object to be copied
   */
  UnaryBranchInstr &operator=(const UnaryBranchInstr &r_src) = delete;

  /**
   * @brief Move constructor for UnaryBranchInstr.
   * @param r_src the object to be moved
   */
  UnaryBranchInstr(UnaryBranchInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryBranchInstr.
   * @param r_src the object to be moved
   */
  UnaryBranchInstr &operator=(UnaryBranchInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal