#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/arg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Exclusive Or operation
/// see Armv7-M Architecture Reference Manual Issue E.e p.233
template <typename TInstrContext> class Eor2ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto shift_val = Alu32::Shift_C(rm, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto &shifted = shift_val.result;

    const u32 result = Alu32::EOR(rn, shifted);
    return OpResult{result, shift_val.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Orr operation
/// see Armv7-M Architecture Reference Manual Issue E.e p.310
template <typename TInstrContext> class Orr2ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto shift_val = Alu32::Shift_C(rm, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto &shifted = shift_val.result;

    const u32 result = Alu32::OR(rn, shifted);
    return OpResult{result, shift_val.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// And operation
/// see Armv7-M Architecture Reference Manual Issue E.e p.201
template <typename TInstrContext> class And2ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto shift_val = Alu32::Shift_C(rm, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto &shifted = shift_val.result;

    const u32 result = Alu32::AND(rn, shifted);
    return OpResult{result, shift_val.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Bit clear operation
/// see Armv7-M Architecture Reference Manual Issue E.e p.309
template <typename TInstrContext> class Bic2ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto shift_val = Alu32::Shift_C(rm, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto &shifted = shift_val.result;

    const u32 result = Alu32::AND(rn, ~shifted);
    return OpResult{result, shift_val.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Subtract with carry
/// see Armv7-M Architecture Reference Manual Issue E.e p.347
template <typename TInstrContext> class Sbc2ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto shifted = Alu32::Shift(rm, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result =
        Alu32::AddWithCarry(rn, ~shifted, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

/// Subtract
/// see Armv7-M Architecture Reference Manual Issue E.e p.404
template <typename TInstrContext> class Sub2ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto shifted = Alu32::Shift(rm, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::AddWithCarry(rn, ~shifted, true);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

/// Reverse Subtract
/// see Armv7-M Architecture Reference Manual Issue E.e p.342
template <typename TInstrContext> class Rsb2ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto shifted = Alu32::Shift(rm, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::AddWithCarry(~rn, shifted, true);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};
/// Add
/// see Armv7-M Architecture Reference Manual Issue E.e p.192
template <typename TInstrContext> class Add2ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto shifted = Alu32::Shift(rm, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::AddWithCarry(rn, shifted, false);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};
/// Adc
/// see Armv7-M Architecture Reference Manual Issue E.e p.188
template <typename TInstrContext> class Adc2ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto shifted = Alu32::Shift(rm, shift_res.type, shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result =
        Alu32::AddWithCarry(rn, shifted, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

template <typename TOp, typename TInstrContext> class TernaryInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const TArg1 &arg_m, const TArg2 &arg_d,
                                 const ImmShiftResults &shift_res) {
    const auto is_32bit = (iflags & k32Bit) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());
    auto result = TOp::Call(ictx, rn, rm, shift_res);

    Reg::WriteRegister(ictx.pstates, arg_d.Get(), result.value);

    if ((iflags & InstrFlags::kSetFlags) != 0u) {
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);

      // Clear N, Z, C, V flags
      apsr &=
          ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

      apsr |= ((result.value >> 31u) & 0x1u) << ApsrRegister::kNPos;       // N
      apsr |= Bm32::IsZeroBit(result.value) << ApsrRegister::kZPos;        // Z
      apsr |= (result.carry_out == true ? 1u : 0u) << ApsrRegister::kCPos; // C
      apsr |= (result.overflow == true ? 1u : 0u) << ApsrRegister::kVPos;  // V
      SReg::template WriteRegister<SpecialRegisterId::kApsr>(ictx.pstates, apsr);
    }
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  TernaryInstrWithShift() = delete;

  /// \brief Destructor
  ~TernaryInstrWithShift() = delete;

  /// \brief Copy constructor for MemorTernaryInstrWithShiftyRouter.
  /// \param r_src the object to be copied
  TernaryInstrWithShift(const TernaryInstrWithShift &r_src) = default;

  /// \brief Copy assignment operator for TernaryInstrWithShift.
  /// \param r_src the object to be copied
  TernaryInstrWithShift &operator=(const TernaryInstrWithShift &r_src) = delete;

  /// \brief Move constructor for TernaryInstrWithShift.
  /// \param r_src the object to be copied
  TernaryInstrWithShift(TernaryInstrWithShift &&r_src) = delete;

  /// \brief Move assignment operator for TernaryInstrWithShift.
  /// \param r_src the object to be copied
  TernaryInstrWithShift &operator=(TernaryInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace microemu