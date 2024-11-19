#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr/post_exec.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {
/**
 * @brief Add
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.190
 */
template <typename TInstrContext> class Add1ImmOp {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &arg_d, const TArg1 &arg_n, const u32 &imm) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto result = Alu32::AddWithCarry(rn, imm, false);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecWriteRegPcExcluded::Call(ictx, arg_d, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Adc
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.187
 */
template <typename TInstrContext> class Adc1ImmOp {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &arg_d, const TArg1 &arg_n, const u32 &imm) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto result =
        Alu32::AddWithCarry(rn, imm, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};

    PostExecWriteRegPcExcluded::Call(ictx, arg_d, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Subtract
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.402
 */
template <typename TInstrContext> class Sub1ImmOp {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &arg_d, const TArg1 &arg_n, const u32 &imm) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto result = Alu32::AddWithCarry(rn, ~imm, true);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};

    PostExecWriteRegPcExcluded::Call(ictx, arg_d, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Subtract with carry
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.346
 */
template <typename TInstrContext> class Sbc1ImmOp {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &arg_d, const TArg1 &arg_n, const u32 &imm) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    const auto result =
        Alu32::AddWithCarry(rn, ~imm, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};

    PostExecWriteRegPcExcluded::Call(ictx, arg_d, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Reverse Subtract
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.341
 */
template <typename TInstrContext> class Rsb1ImmOp {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &arg_d, const TArg1 &arg_n, const u32 &imm) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto result = Alu32::AddWithCarry(~rn, imm, true);
    const auto op_res = OpResult{result.value, result.carry_out, result.overflow};
    PostExecWriteRegPcExcluded::Call(ictx, arg_d, op_res.value);

    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class BinaryInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_d, const TArg1 &arg_n, const u32 &imm) {

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, arg_d, arg_n, imm));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryInstrWithImm() = delete;

  /**
   * @brief Copy constructor for BinaryInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryInstrWithImm(const BinaryInstrWithImm &r_src) = delete;

  /**
   * @brief Copy assignment operator for BinaryInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryInstrWithImm &operator=(const BinaryInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for BinaryInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryInstrWithImm(BinaryInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryInstrWithImm &operator=(BinaryInstrWithImm &&r_src) = delete;
};

} // namespace libmicroemu::internal
