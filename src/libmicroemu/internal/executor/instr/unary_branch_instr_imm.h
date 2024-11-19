#pragma once
#include "libmicroemu/internal/decoder/decoder.h"

#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu ::internal {

/**
 * @brief Branch
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.205
 */
template <typename TInstrContext> class B1ImmOp {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const u32 &imm32) {
    static_cast<void>(iflags);
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    auto branch_res = pc + imm32;
    PostExecBranch::Call(ictx, branch_res);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Branch Link
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.213
 */
template <typename TInstrContext> class Bl1ImmOp {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const u32 &imm32) {
    static_cast<void>(iflags);
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    ictx.cpua.template WriteRegister<RegisterId::kLr>(static_cast<uint32_t>((pc & ~0x1U) | 0x1U));
    auto branch_res = pc + imm32;
    PostExecBranch::Call(ictx, branch_res);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class UnaryBranchInstrImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const u32 &imm32) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, imm32));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryBranchInstrImm() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryBranchInstrImm() = delete;

  /**
   * @brief Copy constructor for UnaryBranchInstrImm.
   * @param r_src the object to be copied
   */
  UnaryBranchInstrImm(const UnaryBranchInstrImm &r_src) = delete;

  /**
   * @brief Copy assignment operator for UnaryBranchInstrImm.
   * @param r_src the object to be copied
   */
  UnaryBranchInstrImm &operator=(const UnaryBranchInstrImm &r_src) = delete;

  /**
   * @brief Move constructor for UnaryBranchInstrImm.
   * @param r_src the object to be moved
   */
  UnaryBranchInstrImm(UnaryBranchInstrImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryBranchInstrImm.
   * @param r_src the object to be moved
   */
  UnaryBranchInstrImm &operator=(UnaryBranchInstrImm &&r_src) = delete;
};

} // namespace libmicroemu::internal
