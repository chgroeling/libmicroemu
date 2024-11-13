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
 * @brief Cmp
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.224
 */
template <typename TInstrContext> class Cmp1ImmOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &imm32) {
    static_cast<void>(ictx);

    const u32 n_imm32 = ~imm32;
    const auto result = Alu32::AddWithCarry(rn, n_imm32, true);

    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

/**
 * @brief Cmn
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.221
 */
template <typename TInstrContext> class Cmn1ImmOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &imm32) {
    static_cast<void>(ictx);

    const auto result = Alu32::AddWithCarry(rn, imm32, false);

    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

template <typename TOp, typename TInstrContext> class BinaryNullInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const u32 &imm) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    auto result = TOp::Call(ictx, rn, imm);

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
  BinaryNullInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryNullInstrWithImm() = delete;

  /**
   * @brief Copy constructor for BinaryNullInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryNullInstrWithImm(const BinaryNullInstrWithImm &r_src) = default;

  /**
   * @brief Copy assignment operator for BinaryNullInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryNullInstrWithImm &operator=(const BinaryNullInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for BinaryNullInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryNullInstrWithImm(BinaryNullInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryNullInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryNullInstrWithImm &operator=(BinaryNullInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu