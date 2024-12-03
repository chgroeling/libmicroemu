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
 * @brief Asr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.341
 */
template <typename TInstrContext> class Asr1ShiftOp {
public:
  template <typename TDest, typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rm,
                                        const ImmShiftResults &shift_res) {
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadSpecialRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(m, SRType::SRType_ASR, shift_res.value,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    const auto op_res = OpResult{r_shift_c.result, r_shift_c.carry_out,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Lsl
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.282
 */
template <typename TInstrContext> class Lsl1ShiftOp {
public:
  template <typename TDest, typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rm,
                                        const ImmShiftResults &shift_res) {
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadSpecialRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(m, SRType::SRType_LSL, shift_res.value,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    const auto op_res = OpResult{r_shift_c.result, r_shift_c.carry_out,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Lsr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.282
 */
template <typename TInstrContext> class Lsr1ShiftOp {
public:
  template <typename TDest, typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rm,
                                        const ImmShiftResults &shift_res) {
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadSpecialRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(m, SRType::SRType_LSR, shift_res.value,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    const auto op_res = OpResult{r_shift_c.result, r_shift_c.carry_out,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Mvn
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.304
 */
template <typename TInstrContext> class Mvn1ShiftOp {
public:
  template <typename TDest, typename TArg0>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rm,
                                        const ImmShiftResults &shift_res) {
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadSpecialRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(m, shift_res.type, shift_res.value,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    const auto op_res = OpResult{~r_shift_c.result, r_shift_c.carry_out,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class BinaryInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TDest, typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TDest &rd, const TArg0 &rm,
                                      const ImmShiftResults &shift_res) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd, rm, shift_res));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryInstrWithShift() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryInstrWithShift() = delete;

  /**
   * @brief Copy constructor for BinaryInstrWithShift.
   * @param r_src the object to be copied
   */
  BinaryInstrWithShift(const BinaryInstrWithShift &r_src) = delete;

  /**
   * @brief Copy assignment operator for BinaryInstrWithShift.
   * @param r_src the object to be copied
   */
  BinaryInstrWithShift &operator=(const BinaryInstrWithShift &r_src) = delete;

  /**
   * @brief Move constructor for BinaryInstrWithShift.
   * @param r_src the object to be moved
   */
  BinaryInstrWithShift(BinaryInstrWithShift &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryInstrWithShift.
   * @param r_src the object to be moved
   */
  BinaryInstrWithShift &operator=(BinaryInstrWithShift &&r_src) = delete;
};

} // namespace libmicroemu::internal
