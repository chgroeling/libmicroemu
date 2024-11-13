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
 * @brief Tst
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.419
 */
template <typename TInstrContext> class Tst1ImmCarryOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn,
                              const ThumbImmediateResult &imm_carry) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto result = Alu32::AND(rn, imm_carry.out);

    return OpResult{result, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Teq
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.417
 */
template <typename TInstrContext> class Teq1ImmCarryOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn,
                              const ThumbImmediateResult &imm_carry) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto result = Alu32::EOR(rn, imm_carry.out);

    return OpResult{result, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

template <typename TOp, typename TInstrContext> class BinaryNullInstrWithImmCarry {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const ThumbImmediateResult &imm_carry) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    auto result = TOp::Call(ictx, rn, imm_carry);

    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);

    // Clear N, Z, C, V flags
    apsr &=
        ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

    apsr |= ((result.value >> 31U) & 0x1U) << ApsrRegister::kNPos;       // N
    apsr |= Bm32::IsZeroBit(result.value) << ApsrRegister::kZPos;        // Z
    apsr |= (result.carry_out == true ? 1U : 0U) << ApsrRegister::kCPos; // C
    apsr |= (result.overflow == true ? 1U : 0U) << ApsrRegister::kVPos;  // V
    SReg::template WriteRegister<SpecialRegisterId::kApsr>(ictx.pstates, apsr);

    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);

    return Ok(ExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryNullInstrWithImmCarry() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryNullInstrWithImmCarry() = delete;

  /**
   * @brief Copy constructor for BinaryNullInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  BinaryNullInstrWithImmCarry(const BinaryNullInstrWithImmCarry &r_src) = default;

  /**
   * @brief Copy assignment operator for BinaryNullInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  BinaryNullInstrWithImmCarry &operator=(const BinaryNullInstrWithImmCarry &r_src) = delete;

  /**
   * @brief Move constructor for BinaryNullInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  BinaryNullInstrWithImmCarry(BinaryNullInstrWithImmCarry &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryNullInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  BinaryNullInstrWithImmCarry &operator=(BinaryNullInstrWithImmCarry &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu