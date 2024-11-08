#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

// TODO: Include version to call an OP with flags..
namespace microemu {
namespace internal {
/// Add
/// see Armv7-M Architecture Reference Manual Issue E.e p.190
template <typename TInstrContext> class Add1ImmOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &imm32) {
    static_cast<void>(ictx);
    const auto result = Alu32::AddWithCarry(rn, imm32, false);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

/// Adc
/// see Armv7-M Architecture Reference Manual Issue E.e p.187
template <typename TInstrContext> class Adc1ImmOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &imm32) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto result =
        Alu32::AddWithCarry(rn, imm32, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

/// Subtract
/// see Armv7-M Architecture Reference Manual Issue E.e p.402
template <typename TInstrContext> class Sub1ImmOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &imm32) {
    static_cast<void>(ictx);
    const auto result = Alu32::AddWithCarry(rn, ~imm32, true);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

/// Subtract with carry
/// see Armv7-M Architecture Reference Manual Issue E.e p.346
template <typename TInstrContext> class Sbc1ImmOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &imm32) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto result =
        Alu32::AddWithCarry(rn, ~imm32, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

/// Reverse Subtract
/// see Armv7-M Architecture Reference Manual Issue E.e p.341
template <typename TInstrContext> class Rsb1ImmOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &imm32) {
    static_cast<void>(ictx);
    const auto result = Alu32::AddWithCarry(~rn, imm32, true);
    return OpResult{result.value, result.carry_out, result.overflow};
  }
};

template <typename TOp, typename TInstrContext> class BinaryInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const TArg1 &arg_d, const u32 &imm) {
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
    Reg::WriteRegister(ictx.pstates, arg_d.Get(), result.value);

    if ((iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U) {
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
      // Clear N, Z, C, V flags
      apsr &=
          ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

      apsr |= ((result.value >> 31U) & 0x1U) << ApsrRegister::kNPos;
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
  BinaryInstrWithImm() = delete;

  /// \brief Destructor
  ~BinaryInstrWithImm() = delete;

  /// \brief Copy constructor for BinaryInstrWithImm.
  /// \param r_src the object to be copied
  BinaryInstrWithImm(const BinaryInstrWithImm &r_src) = default;

  /// \brief Copy assignment operator for BinaryInstrWithImm.
  /// \param r_src the object to be copied
  BinaryInstrWithImm &operator=(const BinaryInstrWithImm &r_src) = delete;

  /// \brief Move constructor for BinaryInstrWithImm.
  /// \param r_src the object to be copied
  BinaryInstrWithImm(BinaryInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for BinaryInstrWithImm.
  /// \param r_src the object to be copied
  BinaryInstrWithImm &operator=(BinaryInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu