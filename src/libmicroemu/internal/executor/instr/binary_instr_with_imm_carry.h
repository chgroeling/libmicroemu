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

/// Orr
/// see Armv7-M Architecture Reference Manual Issue E.e p.309
template <typename TInstrContext> class Orr1ImmCarryOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn,
                              const ThumbImmediateResult &imm_carry) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto result = Alu32::OR(rn, imm_carry.out);
    return OpResult{result, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Eor
/// see Armv7-M Architecture Reference Manual Issue E.e p.232
template <typename TInstrContext> class Eor1ImmCarryOp {
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

/// And
/// see Armv7-M Architecture Reference Manual Issue E.e p.200
template <typename TInstrContext> class And1ImmCarryOp {
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

/// Bit clear
/// see Armv7-M Architecture Reference Manual Issue E.e p.190
template <typename TInstrContext> class Bic1ImmCarryOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn,
                              const ThumbImmediateResult &imm_carry) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto result = Alu32::AND(rn, ~imm_carry.out);
    return OpResult{result, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

template <typename TOp, typename TInstrContext> class BinaryInstrWithImmCarry {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const TArg1 &arg_d,
                                 const ThumbImmediateResult &imm_carry) {
    const auto is_32bit = (iflags & k32Bit) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    auto result = TOp::Call(ictx, rn, imm_carry);

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
  BinaryInstrWithImmCarry() = delete;

  /// \brief Destructor
  ~BinaryInstrWithImmCarry() = delete;

  /// \brief Copy constructor for BinaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  BinaryInstrWithImmCarry(const BinaryInstrWithImmCarry &r_src) = default;

  /// \brief Copy assignment operator for BinaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  BinaryInstrWithImmCarry &operator=(const BinaryInstrWithImmCarry &r_src) = delete;

  /// \brief Move constructor for BinaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  BinaryInstrWithImmCarry(BinaryInstrWithImmCarry &&r_src) = delete;

  /// \brief Move assignment operator for BinaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  BinaryInstrWithImmCarry &operator=(BinaryInstrWithImmCarry &&r_src) = delete;
};

} // namespace internal
} // namespace microemu