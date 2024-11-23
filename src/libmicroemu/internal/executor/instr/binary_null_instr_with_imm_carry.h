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
 * @brief Tst
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.419
 */
template <typename TInstrContext> class Tst1ImmCarryOp {
public:
  template <typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rn, const ThumbImmediateResult &imm_carry) {
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto result = Alu32::AND(n, imm_carry.out);

    const auto op_res =
        OpResult{result, imm_carry.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecSetFlags::Call(ictx, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Teq
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.417
 */
template <typename TInstrContext> class Teq1ImmCarryOp {
public:
  template <typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rn, const ThumbImmediateResult &imm_carry) {
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto result = Alu32::EOR(n, imm_carry.out);

    const auto op_res =
        OpResult{result, imm_carry.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecSetFlags::Call(ictx, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class BinaryNullInstrWithImmCarry {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rn, const ThumbImmediateResult &imm_carry) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rn, imm_carry));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryNullInstrWithImmCarry() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryNullInstrWithImmCarry() = delete;

  /**
   * @brief Copy constructor for BinaryNullInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  BinaryNullInstrWithImmCarry(const BinaryNullInstrWithImmCarry &r_src) = delete;

  /**
   * @brief Copy assignment operator for BinaryNullInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  BinaryNullInstrWithImmCarry &operator=(const BinaryNullInstrWithImmCarry &r_src) = delete;

  /**
   * @brief Move constructor for BinaryNullInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  BinaryNullInstrWithImmCarry(BinaryNullInstrWithImmCarry &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryNullInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  BinaryNullInstrWithImmCarry &operator=(BinaryNullInstrWithImmCarry &&r_src) = delete;
};

} // namespace libmicroemu::internal
