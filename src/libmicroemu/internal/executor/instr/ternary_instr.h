#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Mls
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.290
 */
template <typename TInstrContext> class Mls3Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const u32 &ra) {
    static_cast<void>(ictx);
    const auto result =
        static_cast<u32>(static_cast<i32>(ra) - static_cast<i32>(rn) * static_cast<i32>(rm));

    return OpResult{result, false, false};
  }
};

/**
 * @brief Mla
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.289
 */
template <typename TInstrContext> class Mla3Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rn, const u32 &rm,
                              const u32 &ra) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
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

  template <typename TArg0, typename TArg1, typename TArg2, typename TArg3>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_n, const TArg1 &arg_m, const TArg2 &arg_d,
                                      const TArg3 &arg_a) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    InstrExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(InstrExecResult{eflags});
    }

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    const auto ra = ictx.cpua.ReadRegister(arg_a.Get());

    auto result = TOp::Call(ictx, rn, rm, ra);

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
  TernaryInstr() = delete;

  /**
   * @brief Destructor
   */
  ~TernaryInstr() = delete;

  /**
   * @brief Copy constructor for TernaryInstr.
   * @param r_src the object to be copied
   */
  TernaryInstr(const TernaryInstr &r_src) = default;

  /**
   * @brief Copy assignment operator for TernaryInstr.
   * @param r_src the object to be copied
   */
  TernaryInstr &operator=(const TernaryInstr &r_src) = delete;

  /**
   * @brief Move constructor for TernaryInstr.
   * @param r_src the object to be moved
   */
  TernaryInstr(TernaryInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  TernaryInstr.
   * @param r_src the object to be moved
   */
  TernaryInstr &operator=(TernaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu