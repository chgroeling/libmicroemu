#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr/post_exec.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Cmp
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.224
 */
template <typename TInstrContext> class Cmp1ImmOp {
public:
  template <typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &arg_n, const u32 &imm) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());

    const u32 n_imm32 = ~imm;
    const auto result = Alu32::AddWithCarry(rn, n_imm32, true);

    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecSetFlags::Call(ictx, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Cmn
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.221
 */
template <typename TInstrContext> class Cmn1ImmOp {
public:
  template <typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &arg_n, const u32 &imm) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto result = Alu32::AddWithCarry(rn, imm, false);

    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecSetFlags::Call(ictx, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class BinaryNullInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_n, const u32 &imm) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, arg_n, imm));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryNullInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryNullInstrWithImm() = delete;

  /**
   * @brief Copy constructor for BinaryNullInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryNullInstrWithImm(const BinaryNullInstrWithImm &r_src) = delete;

  /**
   * @brief Copy assignment operator for BinaryNullInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryNullInstrWithImm &operator=(const BinaryNullInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for BinaryNullInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryNullInstrWithImm(BinaryNullInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryNullInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryNullInstrWithImm &operator=(BinaryNullInstrWithImm &&r_src) = delete;
};

} // namespace libmicroemu::internal
