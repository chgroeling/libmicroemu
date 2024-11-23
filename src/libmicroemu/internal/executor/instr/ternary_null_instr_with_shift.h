#pragma once
#include "libmicroemu/internal/decoder/decoder.h"

#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Compare two registers
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.224
 */
template <typename TInstrContext> class Cmp2ShiftOp {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rm, const TArg1 &rn,
                                        const ImmShiftResults &shift_res) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto n = ictx.cpua.ReadRegister(rn.Get());

    const auto shifted = Alu32::Shift(m, static_cast<SRType>(shift_res.type), shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    const auto result = Alu32::AddWithCarry(n, ~shifted, true);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecSetFlags::Call(ictx, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief TST - Test two registers
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.420
 */
template <typename TInstrContext> class Tst2ShiftOp {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rm, const TArg1 &rn,
                                        const ImmShiftResults &shift_res) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto n = ictx.cpua.ReadRegister(rn.Get());

    const auto r_shift_c = Alu32::Shift_C(m, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::AND(n, r_shift_c.result);

    const auto op_res =
        OpResult{result, r_shift_c.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecSetFlags::Call(ictx, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief TEQ - Test two registers
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.418
 */
template <typename TInstrContext> class Teq2ShiftOp {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rm, const TArg1 &rn,
                                        const ImmShiftResults &shift_res) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto n = ictx.cpua.ReadRegister(rn.Get());

    const auto r_shift_c = Alu32::Shift_C(m, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::EOR(n, r_shift_c.result);
    const auto op_res =
        OpResult{result, r_shift_c.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecSetFlags::Call(ictx, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class TernaryNullInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rm, const TArg1 &rn,
                                      const ImmShiftResults &shift_res) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rm, rn, shift_res));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  TernaryNullInstrWithShift() = delete;

  /**
   * @brief Destructor
   */
  ~TernaryNullInstrWithShift() = delete;

  /**
   * @brief Copy constructor for TernaryNullInstrWithShift.
   * @param r_src the object to be copied
   */
  TernaryNullInstrWithShift(const TernaryNullInstrWithShift &r_src) = delete;

  /**
   * @brief Copy assignment operator for TernaryNullInstrWithShift.
   * @param r_src the object to be copied
   */
  TernaryNullInstrWithShift &operator=(const TernaryNullInstrWithShift &r_src) = delete;

  /**
   * @brief Move constructor for TernaryNullInstrWithShift.
   * @param r_src the object to be moved
   */
  TernaryNullInstrWithShift(TernaryNullInstrWithShift &&r_src) = delete;

  /**
   * @brief Move assignment operator for  TernaryNullInstrWithShift.
   * @param r_src the object to be moved
   */
  TernaryNullInstrWithShift &operator=(TernaryNullInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu