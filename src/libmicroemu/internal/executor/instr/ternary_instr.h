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
  template <typename TDest, typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rn, const TArg1 &rm,
                                        const TArg2 &ra) {
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto a = ictx.cpua.ReadRegister(ra.Get());

    const auto result =
        static_cast<u32>(static_cast<i32>(a) - static_cast<i32>(n) * static_cast<i32>(m));

    const auto op_result = OpResult{result, false, false};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_result.value);
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
  template <typename TDest, typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rn, const TArg1 &rm,
                                        const TArg2 &ra) {
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto a = ictx.cpua.ReadRegister(ra.Get());

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

    const auto result =
        static_cast<u32>(static_cast<i32>(n) * static_cast<i32>(m) + static_cast<i32>(a));

    const auto op_result = OpResult{result, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_result.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_result);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class TernaryInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TDest, typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TDest &rd, const TArg0 &rn, const TArg1 &rm,
                                      const TArg2 &ra) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd, rn, rm, ra));
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
