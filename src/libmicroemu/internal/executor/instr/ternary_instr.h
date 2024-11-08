#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Mls
/// see Armv7-M Architecture Reference Manual Issue E.e p.290
template <typename TInstrContext> class Mls3Op {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const u32 &ra) {
    static_cast<void>(ictx);
    const auto result =
        static_cast<u32>(static_cast<i32>(ra) - static_cast<i32>(rn) * static_cast<i32>(rm));

    return OpResult{result, false, false};
  }
};

/// Mla
/// see Armv7-M Architecture Reference Manual Issue E.e p.289
template <typename TInstrContext> class Mla3Op {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const u32 &ra) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    const auto result =
        static_cast<u32>(static_cast<i32>(rn) * static_cast<i32>(rm) + static_cast<i32>(ra));

    return OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

template <typename TOp, typename TInstrContext> class TernaryInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0, typename TArg1, typename TArg2, typename TArg3>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const TArg1 &arg_m, const TArg2 &arg_d,
                                 const TArg3 &arg_a) {
    const auto is_32bit = (iflags & k32Bit) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    const auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());
    const auto ra = Reg::ReadRegister(ictx.pstates, arg_a.Get());

    auto result = TOp::Call(ictx, rn, rm, ra);

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
  TernaryInstr() = delete;

  /// \brief Destructor
  ~TernaryInstr() = delete;

  /// \brief Copy constructor for TernaryInstr.
  /// \param r_src the object to be copied
  TernaryInstr(const TernaryInstr &r_src) = default;

  /// \brief Copy assignment operator for TernaryInstr.
  /// \param r_src the object to be copied
  TernaryInstr &operator=(const TernaryInstr &r_src) = delete;

  /// \brief Move constructor for TernaryInstr.
  /// \param r_src the object to be copied
  TernaryInstr(TernaryInstr &&r_src) = delete;

  /// \brief Move assignment operator for TernaryInstr.
  /// \param r_src the object to be copied
  TernaryInstr &operator=(TernaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace microemu