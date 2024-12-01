#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr/post_exec.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Orr
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.309
 */
template <typename TInstrContext> class Orr1ImmCarryOp {
public:
  template <typename TDest, typename TArg0>
  static Result<InstrExecFlagsSet> Call(const TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rn,
                                        const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto n = ictx.cpua.ReadRegister(rn.Get());

    const auto result = Alu32::OR(n, imm_carry.out);
    const auto op_res =
        OpResult{result, imm_carry.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Eor
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.232
 */
template <typename TInstrContext> class Eor1ImmCarryOp {
public:
  template <typename TDest, typename TArg0>
  static Result<InstrExecFlagsSet> Call(const TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rn,
                                        const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto result = Alu32::EOR(n, imm_carry.out);
    const auto op_res =
        OpResult{result, imm_carry.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief And
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.200
 */
template <typename TInstrContext> class And1ImmCarryOp {
public:
  template <typename TDest, typename TArg0>
  static Result<InstrExecFlagsSet> Call(const TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rn,
                                        const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto result = Alu32::AND(n, imm_carry.out);
    const auto op_res =
        OpResult{result, imm_carry.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Bit clear
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.190
 */
template <typename TInstrContext> class Bic1ImmCarryOp {
public:
  template <typename TDest, typename TArg0>
  static Result<InstrExecFlagsSet> Call(const TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg0 &rn,
                                        const ThumbImmediateResult &imm_carry) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto result = Alu32::AND(n, ~imm_carry.out);
    const auto op_res =
        OpResult{result, imm_carry.carry_out, (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class BinaryInstrWithImmCarry {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TDest, typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TDest &rd, const TArg0 &rn,
                                      const ThumbImmediateResult &imm_carry) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd, rn, imm_carry));
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
  BinaryInstrWithImmCarry(const BinaryInstrWithImmCarry &r_src) = delete;

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

} // namespace libmicroemu::internal
