#pragma once
#include "libmicroemu/internal/decoder/decoder.h"

#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Mov
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.291
 */
template <typename TInstrContext> class MovImmCarryOp {
public:
  template <typename TDest>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadSpecialRegister<SpecialRegisterId::kApsr>();
    const auto op_res = OpResult{imm_carry.out, imm_carry.carry_out,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);

    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Mvn
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.291
 */
template <typename TInstrContext> class MvnImmCarryOp {
public:
  template <typename TDest>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadSpecialRegister<SpecialRegisterId::kApsr>();
    const auto op_res = OpResult{~imm_carry.out, imm_carry.carry_out,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class UnaryInstrImmCarry {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TDest>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TDest &rd, const ThumbImmediateResult &imm_carry) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd, imm_carry));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryInstrImmCarry() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryInstrImmCarry() = delete;

  /**
   * @brief Copy constructor for UnaryInstrImmCarry.
   * @param r_src the object to be copied
   */
  UnaryInstrImmCarry(const UnaryInstrImmCarry &r_src) = delete;

  /**
   * @brief Copy assignment operator for UnaryInstrImmCarry.
   * @param r_src the object to be copied
   */
  UnaryInstrImmCarry &operator=(const UnaryInstrImmCarry &r_src) = delete;

  /**
   * @brief Move constructor for UnaryInstrImmCarry.
   * @param r_src the object to be moved
   */
  UnaryInstrImmCarry(UnaryInstrImmCarry &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryInstrImmCarry.
   * @param r_src the object to be moved
   */
  UnaryInstrImmCarry &operator=(UnaryInstrImmCarry &&r_src) = delete;
};

} // namespace libmicroemu::internal