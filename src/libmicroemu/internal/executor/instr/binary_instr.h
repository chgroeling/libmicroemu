#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Lsr
/// see Armv7-M Architecture Reference Manual Issue E.e p.285
template <typename TInstrContext> class Lsr2Op {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    static_cast<void>(ictx);
    const auto shift_n = rm & 0xFFu;

    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    auto r_shift_c = Alu32::Shift_C(rn, SRType::SRType_LSR, shift_n,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Asr
/// see Armv7-M Architecture Reference Manual Issue E.e p.204
template <typename TInstrContext> class Asr2Op {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    static_cast<void>(ictx);
    const auto shift_n = rm & 0xFFu;

    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    auto r_shift_c = Alu32::Shift_C(rn, SRType::SRType_ASR, shift_n,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Lsl
/// see Armv7-M Architecture Reference Manual Issue E.e p.283
template <typename TInstrContext> class Lsl2Op {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    const auto shift_n = rm & 0xFFu;

    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    auto r_shift_c = Alu32::Shift_C(rn, SRType::SRType_LSL, shift_n,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Mul
/// see Armv7-M Architecture Reference Manual Issue E.e p.302
template <typename TInstrContext> class Mul2Op {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto result = static_cast<u32>(static_cast<i32>(rn) * static_cast<i32>(rm));

    return OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Udiv
/// see Armv7-M Architecture Reference Manual Issue E.e p.426
template <typename TInstrContext> class UDiv2Op {
public:
  using SReg = typename TInstrContext::SReg;
  using ExcTrig = typename TInstrContext::ExcTrig;
  static inline bool IntegerZeroDivideTrappingEnabled(const TInstrContext &ictx) {
    const auto ccr = SReg::template ReadRegister<SpecialRegisterId::kCcr>(ictx.pstates);
    return (ccr & CcrRegister::kDivByZeroTrapEnableMsk) != 0U;
  }

  static inline void GenerateIntegerZeroDivide(const TInstrContext &ictx) {
    ExcTrig::SetPending(ictx.pstates, ExceptionType::kUsageFault);
    auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(ictx.pstates);
    cfsr |= CfsrUsageFault::kDivByZeroMsk;
    SReg::template WriteRegister<SpecialRegisterId::kCfsr>(ictx.pstates, cfsr);
  }

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
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

/// Sdiv
/// see Armv7-M Architecture Reference Manual Issue E.e p.350
template <typename TInstrContext> class SDiv2Op {
public:
  using SReg = typename TInstrContext::SReg;
  using ExcTrig = typename TInstrContext::ExcTrig;

  static inline bool IntegerZeroDivideTrappingEnabled(const TInstrContext &ictx) {
    const auto ccr = SReg::template ReadRegister<SpecialRegisterId::kCcr>(ictx.pstates);
    return (ccr & CcrRegister::kDivByZeroTrapEnableMsk) != 0U;
  }

  static inline void GenerateIntegerZeroDivide(const TInstrContext &ictx) {
    ExcTrig::SetPending(ictx.pstates, ExceptionType::kUsageFault);
    auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(ictx.pstates);
    cfsr |= CfsrUsageFault::kDivByZeroMsk;
    SReg::template WriteRegister<SpecialRegisterId::kCfsr>(ictx.pstates, cfsr);
  }
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
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
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const TArg1 &arg_m, const TArg2 &arg_d) {
    const auto is_32bit = (iflags & k32Bit) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    const auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());
    auto result = TOp::Call(ictx, rn, rm);

    Reg::WriteRegister(ictx.pstates, arg_d.Get(), result.value);

    if ((iflags & InstrFlags::kSetFlags) != 0U) {
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);

      // Clear N, Z, C, V flags
      apsr &=
          ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

      apsr |= ((result.value >> 31U) & 0x1U) << ApsrRegister::kNPos;       // N
      apsr |= Bm32::IsZeroBit(result.value) << ApsrRegister::kZPos;        // Z
      apsr |= (result.carry_out == true ? 1U : 0U) << ApsrRegister::kCPos; // C
      apsr |= (result.overflow == true ? 1U : 0U) << ApsrRegister::kVPos;  // V
      SReg::template WriteRegister<SpecialRegisterId::kApsr>(ictx.pstates, apsr);
    }
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  BinaryInstr() = delete;

  /// \brief Destructor
  ~BinaryInstr() = delete;

  /// \brief Copy constructor for BinaryInstr.
  /// \param r_src the object to be copied
  BinaryInstr(const BinaryInstr &r_src) = default;

  /// \brief Copy assignment operator for BinaryInstr.
  /// \param r_src the object to be copied
  BinaryInstr &operator=(const BinaryInstr &r_src) = delete;

  /// \brief Move constructor for BinaryInstr.
  /// \param r_src the object to be copied
  BinaryInstr(BinaryInstr &&r_src) = delete;

  /// \brief Move assignment operator for BinaryInstr.
  /// \param r_src the object to be copied
  BinaryInstr &operator=(BinaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace microemu