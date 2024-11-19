#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Mls
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.290
 */
template <typename TInstrContext> class Mls3Op {
public:
  template <typename TArg0, typename TArg1, typename TArg2, typename TArg3>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &arg_d, const TArg1 &arg_n, const TArg2 &arg_m,
                                        const TArg3 &arg_a) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    const auto ra = ictx.cpua.ReadRegister(arg_a.Get());

    const auto result =
        static_cast<u32>(static_cast<i32>(ra) - static_cast<i32>(rn) * static_cast<i32>(rm));

    const auto op_result = OpResult{result, false, false};

    PostExecWriteRegPcExcluded::Call(ictx, arg_d, op_result.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_result);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Mla
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.289
 */
template <typename TInstrContext> class Mla3Op {
public:
  template <typename TArg0, typename TArg1, typename TArg2, typename TArg3>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &arg_d, const TArg1 &arg_n, const TArg2 &arg_m,
                                        const TArg3 &arg_a) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    const auto ra = ictx.cpua.ReadRegister(arg_a.Get());

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

    const auto result =
        static_cast<u32>(static_cast<i32>(rn) * static_cast<i32>(rm) + static_cast<i32>(ra));

    const auto op_result = OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, arg_d, op_result.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_result);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class TernaryInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1, typename TArg2, typename TArg3>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_d, const TArg1 &arg_n, const TArg2 &arg_m,
                                      const TArg3 &arg_a) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, arg_d, arg_n, arg_m, arg_a));
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
  TernaryInstr(const TernaryInstr &r_src) = delete;

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

} // namespace libmicroemu::internal
