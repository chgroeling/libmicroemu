#pragma once
#include "libmicroemu/internal/cpu_accessor.h"
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
 * @brief Lsr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.285
 */
template <typename TInstrContext> class Lsr2Op {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm) {
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto shift_n = m & 0xFFU;

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(n, SRType::SRType_LSR, shift_n,
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
 * @brief Asr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.204
 */
template <typename TInstrContext> class Asr2Op {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm) {
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto shift_n = m & 0xFFU;

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(n, SRType::SRType_ASR, shift_n,
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
 * see Armv7-M Architecture Reference Manual Issue E.e p.283
 */
template <typename TInstrContext> class Lsl2Op {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm) {

    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto shift_n = m & 0xFFU;

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(n, SRType::SRType_LSL, shift_n,
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
 * @brief Mul
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.302
 */
template <typename TInstrContext> class Mul2Op {
public:
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto result = static_cast<u32>(static_cast<i32>(n) * static_cast<i32>(m));

    const auto op_res = OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Udiv
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.426
 */
template <typename TInstrContext> class UDiv2Op {
public:
  using ExcTrig = typename TInstrContext::ExcTrig;
  static inline bool IntegerZeroDivideTrappingEnabled(const TInstrContext &ictx) {
    const auto ccr = ictx.cpua.template ReadRegister<SpecialRegisterId::kCcr>();
    return (ccr & CcrRegister::kDivByZeroTrapEnableMsk) != 0U;
  }

  static inline void GenerateIntegerZeroDivide(const TInstrContext &ictx) {
    ExcTrig::SetPending(ictx.cpua, ExceptionType::kUsageFault);
    auto cfsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kCfsr>();
    cfsr |= CfsrUsageFault::kDivByZeroMsk;
    ictx.cpua.template WriteRegister<SpecialRegisterId::kCfsr>(cfsr);
  }

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm) {

    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    u32 result = 0U;
    if (m == 0U) {

      if (IntegerZeroDivideTrappingEnabled(ictx)) {
        GenerateIntegerZeroDivide(ictx);
      } else {
        result = 0U;
      }
    } else {
      result = n / m;
    }

    const auto op_res = OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Sdiv
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.350
 */
template <typename TInstrContext> class SDiv2Op {
public:
  using ExcTrig = typename TInstrContext::ExcTrig;

  static inline bool IntegerZeroDivideTrappingEnabled(const TInstrContext &ictx) {
    const auto ccr = ictx.cpua.template ReadRegister<SpecialRegisterId::kCcr>();
    return (ccr & CcrRegister::kDivByZeroTrapEnableMsk) != 0U;
  }

  static inline void GenerateIntegerZeroDivide(const TInstrContext &ictx) {
    ExcTrig::SetPending(ictx.cpua, ExceptionType::kUsageFault);
    auto cfsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kCfsr>();
    cfsr |= CfsrUsageFault::kDivByZeroMsk;
    ictx.cpua.template WriteRegister<SpecialRegisterId::kCfsr>(cfsr);
  }

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rn, const TArg2 &rm) {

    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    u32 result = 0U;
    if (m == 0U) {
      if (IntegerZeroDivideTrappingEnabled(ictx)) {
        GenerateIntegerZeroDivide(ictx);
      } else {
        result = 0U;
      }
    } else {
      result = static_cast<u32>(static_cast<i32>(n) / static_cast<i32>(m));
    }

    const auto op_res = OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);

    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};
template <typename TOp, typename TInstrContext> class BinaryInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rd, const TArg1 &rn, const TArg2 &rm) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd, rn, rm));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryInstr() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryInstr() = delete;

  /**
   * @brief Copy constructor for BinaryInstr.
   * @param r_src the object to be copied
   */
  BinaryInstr(const BinaryInstr &r_src) = delete;

  /**
   * @brief Copy assignment operator for BinaryInstr.
   * @param r_src the object to be copied
   */
  BinaryInstr &operator=(const BinaryInstr &r_src) = delete;

  /**
   * @brief Move constructor for BinaryInstr.
   * @param r_src the object to be moved
   */
  BinaryInstr(BinaryInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryInstr.
   * @param r_src the object to be moved
   */
  BinaryInstr &operator=(BinaryInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal
