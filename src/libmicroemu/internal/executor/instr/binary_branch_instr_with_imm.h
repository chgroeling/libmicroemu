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
 * @brief Compare branch
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p. 216
 */
template <typename TInstrContext> class CbNZ1ImmOp {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rn, const u32 &imm) {
    const bool is_non_zero = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kNonZero)) != 0U;
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    const u32 new_pc = pc + imm;

    if ((n == 0U) && (is_non_zero == false)) {
      Pc::BranchWritePC(ictx.cpua, new_pc);
    } else if ((n != 0U) && (is_non_zero == true)) {
      Pc::BranchWritePC(ictx.cpua, new_pc);
    } else {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
    };

    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class BinaryBranchInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rn, const u32 &imm) {

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rn, imm));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryBranchInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryBranchInstrWithImm() = delete;

  /**
   * @brief Copy constructor for BinaryBranchInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryBranchInstrWithImm(const BinaryBranchInstrWithImm &r_src) = delete;

  /**
   * @brief Copy assignment operator for BinaryBranchInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryBranchInstrWithImm &operator=(const BinaryBranchInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for BinaryBranchInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryBranchInstrWithImm(BinaryBranchInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryBranchInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryBranchInstrWithImm &operator=(BinaryBranchInstrWithImm &&r_src) = delete;
};

} // namespace libmicroemu::internal
