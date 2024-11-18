#pragma once
#include "libmicroemu/internal/cpu_accessor.h"
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Lsr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.285
 */
template <typename TInstrContext> class Lsr2Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    static_cast<void>(ictx);
    const auto shift_n = rm & 0xFFU;

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(rn, SRType::SRType_LSR, shift_n,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Asr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.204
 */
template <typename TInstrContext> class Asr2Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    static_cast<void>(ictx);
    const auto shift_n = rm & 0xFFU;

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(rn, SRType::SRType_ASR, shift_n,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Lsl
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.283
 */
template <typename TInstrContext> class Lsl2Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    const auto shift_n = rm & 0xFFU;

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    auto r_shift_c = Alu32::Shift_C(rn, SRType::SRType_LSL, shift_n,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Mul
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.302
 */
template <typename TInstrContext> class Mul2Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto result = static_cast<u32>(static_cast<i32>(rn) * static_cast<i32>(rm));

    return OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
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

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    u32 result = 0U;
    if (rm == 0U) {

      if (IntegerZeroDivideTrappingEnabled(ictx)) {
        GenerateIntegerZeroDivide(ictx);
      } else {
        result = 0U;
      }
    } else {
      result = rn / rm;
    }

    return OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
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
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    u32 result = 0U;
    if (rm == 0U) {
      if (IntegerZeroDivideTrappingEnabled(ictx)) {
        GenerateIntegerZeroDivide(ictx);
      } else {
        result = 0U;
      }
    } else {
      result = static_cast<u32>(static_cast<i32>(rn) / static_cast<i32>(rm));
    }

    return OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};
template <typename TOp, typename TInstrContext> class BinaryInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_d, const TArg1 &arg_n, const TArg2 &arg_m) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    InstrExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(InstrExecResult{eflags});
    }

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    auto result = TOp::Call(ictx, rn, rm);

    ictx.cpua.WriteRegister(arg_d.Get(), result.value);

    if ((iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U) {
      auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

      // Clear N, Z, C, V flags
      apsr &=
          ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

      apsr |= ((result.value >> 31U) & 0x1U) << ApsrRegister::kNPos;       // N
      apsr |= Bm32::IsZeroBit(result.value) << ApsrRegister::kZPos;        // Z
      apsr |= (result.carry_out == true ? 1U : 0U) << ApsrRegister::kCPos; // C
      apsr |= (result.overflow == true ? 1U : 0U) << ApsrRegister::kVPos;  // V
      ictx.cpua.template WriteRegister<SpecialRegisterId::kApsr>(apsr);
    }
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);

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
  BinaryInstr(const BinaryInstr &r_src) = default;

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

} // namespace internal
} // namespace libmicroemu