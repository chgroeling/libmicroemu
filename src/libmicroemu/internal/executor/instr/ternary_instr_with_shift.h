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
 * @brief Exclusive Or operation
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.233
 */
template <typename TInstrContext> class Eor2ShiftOp {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                        const ImmShiftResults &shift_res) {
    auto n = ictx.cpua.ReadRegister(rn.Get());
    auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto shift_val = Alu32::Shift_C(m, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto &shifted = shift_val.result;

    const u32 result = Alu32::EOR(n, shifted);
    const auto op_res =
        OpResult{result, shift_val.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Orr operation
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.310
 */
template <typename TInstrContext> class Orr2ShiftOp {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                        const ImmShiftResults &shift_res) {
    auto n = ictx.cpua.ReadRegister(rn.Get());
    auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto shift_val = Alu32::Shift_C(m, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto &shifted = shift_val.result;

    const u32 result = Alu32::OR(n, shifted);
    const auto op_res =
        OpResult{result, shift_val.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief And operation
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.201
 */
template <typename TInstrContext> class And2ShiftOp {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                        const ImmShiftResults &shift_res) {
    auto n = ictx.cpua.ReadRegister(rn.Get());
    auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto shift_val = Alu32::Shift_C(m, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto &shifted = shift_val.result;

    const u32 result = Alu32::AND(n, shifted);
    const auto op_res =
        OpResult{result, shift_val.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Bit clear operation
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.309
 */
template <typename TInstrContext> class Bic2ShiftOp {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                        const ImmShiftResults &shift_res) {
    auto n = ictx.cpua.ReadRegister(rn.Get());
    auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto shift_val = Alu32::Shift_C(m, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto &shifted = shift_val.result;

    const u32 result = Alu32::AND(n, ~shifted);
    const auto op_res =
        OpResult{result, shift_val.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Subtract with carry
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.347
 */
template <typename TInstrContext> class Sbc2ShiftOp {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                        const ImmShiftResults &shift_res) {
    auto n = ictx.cpua.ReadRegister(rn.Get());
    auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto shifted = Alu32::Shift(m, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result =
        Alu32::AddWithCarry(n, ~shifted, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Subtract
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.404
 */
template <typename TInstrContext> class Sub2ShiftOp {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                        const ImmShiftResults &shift_res) {
    auto n = ictx.cpua.ReadRegister(rn.Get());
    auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto shifted = Alu32::Shift(m, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::AddWithCarry(n, ~shifted, true);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Reverse Subtract
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.342
 */
template <typename TInstrContext> class Rsb2ShiftOp {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                        const ImmShiftResults &shift_res) {
    auto n = ictx.cpua.ReadRegister(rn.Get());
    auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto shifted = Alu32::Shift(m, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::AddWithCarry(~n, shifted, true);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};
/**
 * @brief Add
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.192
 */
template <typename TInstrContext> class Add2ShiftOp {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                        const ImmShiftResults &shift_res) {
    auto n = ictx.cpua.ReadRegister(rn.Get());
    auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto shifted = Alu32::Shift(m, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::AddWithCarry(n, shifted, false);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};
/**
 * @brief Adc
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.188
 */
template <typename TInstrContext> class Adc2ShiftOp {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                        const ImmShiftResults &shift_res) {
    auto n = ictx.cpua.ReadRegister(rn.Get());
    auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto shifted = Alu32::Shift(m, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result =
        Alu32::AddWithCarry(n, shifted, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class TernaryInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rd, const TArg1 &rn, const TArg2 &rm,
                                      const ImmShiftResults &shift_res) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd, rn, rm, shift_res));

    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  TernaryInstrWithShift() = delete;

  /**
   * @brief Destructor
   */
  ~TernaryInstrWithShift() = delete;

  /**
   * @brief Copy constructor for MemorTernaryInstrWithShiftyRouter.
   * @param r_src the object to be copied
   */
  TernaryInstrWithShift(const TernaryInstrWithShift &r_src) = delete;

  /**
   * @brief Copy assignment operator for TernaryInstrWithShift.
   * @param r_src the object to be copied
   */
  TernaryInstrWithShift &operator=(const TernaryInstrWithShift &r_src) = delete;

  /**
   * @brief Move constructor for TernaryInstrWithShift.
   * @param r_src the object to be moved
   */
  TernaryInstrWithShift(TernaryInstrWithShift &&r_src) = delete;

  /**
   * @brief Move assignment operator for  TernaryInstrWithShift.
   * @param r_src the object to be moved
   */
  TernaryInstrWithShift &operator=(TernaryInstrWithShift &&r_src) = delete;
};

} // namespace libmicroemu::internal
