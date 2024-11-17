#pragma once
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
 * @brief Orr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.309
 */
template <typename TInstrContext> class Orr1ImmCarryOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn,
                              const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto result = Alu32::OR(rn, imm_carry.out);
    return OpResult{result, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Eor
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.232
 */
template <typename TInstrContext> class Eor1ImmCarryOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn,
                              const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto result = Alu32::EOR(rn, imm_carry.out);
    return OpResult{result, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief And
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.200
 */
template <typename TInstrContext> class And1ImmCarryOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn,
                              const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto result = Alu32::AND(rn, imm_carry.out);
    return OpResult{result, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Bit clear
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.190
 */
template <typename TInstrContext> class Bic1ImmCarryOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn,
                              const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto result = Alu32::AND(rn, ~imm_carry.out);
    return OpResult{result, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

template <typename TOp, typename TInstrContext> class BinaryInstrWithImmCarry {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_n, const TArg1 &arg_d,
                                      const ThumbImmediateResult &imm_carry) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    InstrExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(InstrExecResult{eflags});
    }

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    auto result = TOp::Call(ictx, rn, imm_carry);

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
  BinaryInstrWithImmCarry() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryInstrWithImmCarry() = delete;

  /**
   * @brief Copy constructor for BinaryInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  BinaryInstrWithImmCarry(const BinaryInstrWithImmCarry &r_src) = default;

  /**
   * @brief Copy assignment operator for BinaryInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  BinaryInstrWithImmCarry &operator=(const BinaryInstrWithImmCarry &r_src) = delete;

  /**
   * @brief Move constructor for BinaryInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  BinaryInstrWithImmCarry(BinaryInstrWithImmCarry &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  BinaryInstrWithImmCarry &operator=(BinaryInstrWithImmCarry &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu