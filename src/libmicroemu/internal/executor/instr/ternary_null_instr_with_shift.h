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
 * @brief Compare two registers
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.224
 */
template <typename TInstrContext> class Cmp2ShiftOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm, const u32 &rn,
                              const ImmShiftResults &shift_res) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

    const auto shifted = Alu32::Shift(rm, static_cast<SRType>(shift_res.type), shift_res.value,
                                      (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    const auto result = Alu32::AddWithCarry(rn, ~shifted, true);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

/**
 * @brief TST - Test two registers
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.420
 */
template <typename TInstrContext> class Tst2ShiftOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm, const u32 &rn,
                              const ImmShiftResults &shift_res) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

    const auto r_shift_c = Alu32::Shift_C(rm, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::AND(rn, r_shift_c.result);

    return OpResult{result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief TEQ - Test two registers
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.418
 */
template <typename TInstrContext> class Teq2ShiftOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm, const u32 &rn,
                              const ImmShiftResults &shift_res) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

    const auto r_shift_c = Alu32::Shift_C(rm, shift_res.type, shift_res.value,
                                          (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto result = Alu32::EOR(rn, r_shift_c.result);

    return OpResult{result, r_shift_c.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

template <typename TOp, typename TInstrContext> class TernaryNullInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_m, const TArg1 &arg_n,
                                 const ImmShiftResults &shift_res) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    auto result = TOp::Call(ictx, rm, rn, shift_res);

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

    // Clear N, Z, C, V flags
    apsr &=
        ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

    apsr |= ((result.value >> 31U) & 0x1U) << ApsrRegister::kNPos;       // N
    apsr |= Bm32::IsZeroBit(result.value) << ApsrRegister::kZPos;        // Z
    apsr |= (result.carry_out == true ? 1U : 0U) << ApsrRegister::kCPos; // C
    apsr |= (result.overflow == true ? 1U : 0U) << ApsrRegister::kVPos;  // V
    ictx.cpua.template WriteRegister<SpecialRegisterId::kApsr>(apsr);

    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);

    return Ok(ExecResult{eflags});
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
  TernaryNullInstrWithShift(const TernaryNullInstrWithShift &r_src) = default;

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