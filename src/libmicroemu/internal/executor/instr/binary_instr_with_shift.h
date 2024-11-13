#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Asr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.341
 */
template <typename TInstrContext> class Asr1ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    auto r_shift_c = Alu32::Shift_C(rm, SRType::SRType_ASR, shift_res.value,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Lsl
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.282
 */
template <typename TInstrContext> class Lsl1ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    auto r_shift_c = Alu32::Shift_C(rm, SRType::SRType_LSL, shift_res.value,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Lsr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.282
 */
template <typename TInstrContext> class Lsr1ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    auto r_shift_c = Alu32::Shift_C(rm, SRType::SRType_LSR, shift_res.value,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Mvn
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.304
 */
template <typename TInstrContext> class Mvn1ShiftOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm,
                              const ImmShiftResults &shift_res) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    auto r_shift_c = Alu32::Shift_C(rm, shift_res.type, shift_res.value,
                                    (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{~r_shift_c.result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

template <typename TOp, typename TInstrContext> class BinaryInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_m, const TArg1 &arg_d,
                                 const ImmShiftResults &shift_res) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());
    auto result = TOp::Call(ictx, rm, shift_res);

    Reg::WriteRegister(ictx.pstates, arg_d.Get(), result.value);

    if ((iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U) {
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
  BinaryInstrWithShift(const BinaryInstrWithShift &r_src) = default;

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

} // namespace internal
} // namespace libmicroemu