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
 * @brief Clz
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.220
 */
template <typename TInstrContext> class Clz1Op {
public:
  template <typename TDest, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg1 &rm) {
    using It = typename TInstrContext::It;
    using Pc = typename TInstrContext::Pc;

    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const u32 result = Bm32::CountLeadingZeros(m);
    const auto op_res = OpResult{result, false, false};

    if (rd.Get() == RegisterId::kPc) {
      Pc::ALUWritePC(ictx.cpua, op_res.value);
      It::ITAdvance(ictx.cpua);
      return Ok(kNoInstrExecFlags);
    } else {
      PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    }

    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Mov
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.293
 */
template <typename TInstrContext> class Mov1Op {
public:
  template <typename TDest, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg1 &rm) {
    using It = typename TInstrContext::It;
    using Pc = typename TInstrContext::Pc;

    auto apsr = ictx.cpua.template ReadSpecialRegister<SpecialRegisterId::kApsr>();
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto op_res = OpResult{m, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    if (rd.Get() == RegisterId::kPc) {
      Pc::ALUWritePC(ictx.cpua, op_res.value);
      It::ITAdvance(ictx.cpua);
      return Ok(kNoInstrExecFlags);
    } else {
      PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    }
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Rrx
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.340
 */
template <typename TInstrContext> class Rrx1Op {
public:
  template <typename TDest, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const TArg1 &rm) {
    using It = typename TInstrContext::It;
    using Pc = typename TInstrContext::Pc;

    auto apsr = ictx.cpua.template ReadSpecialRegister<SpecialRegisterId::kApsr>();
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    auto r_rrx = Alu32::Shift_C(m, SRType::SRType_RRX, 1U,
                                (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    const auto op_res = OpResult{r_rrx.result, r_rrx.carry_out,
                                 (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};

    if (rd.Get() == RegisterId::kPc) {
      Pc::ALUWritePC(ictx.cpua, op_res.value);
      It::ITAdvance(ictx.cpua);
      return Ok(kNoInstrExecFlags);
    } else {
      PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    }
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class UnaryInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TDest, typename TArg1>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TDest &rd, const TArg1 &rm) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd, rm));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryInstr() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryInstr() = delete;

  /**
   * @brief Copy constructor for UnaryInstr.
   * @param r_src the object to be copied
   */
  UnaryInstr(const UnaryInstr &r_src) = delete;

  /**
   * @brief Copy assignment operator for UnaryInstr.
   * @param r_src the object to be copied
   */
  UnaryInstr &operator=(const UnaryInstr &r_src) = delete;

  /**
   * @brief Move constructor for UnaryInstr.
   * @param r_src the object to be moved
   */
  UnaryInstr(UnaryInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryInstr.
   * @param r_src the object to be moved
   */
  UnaryInstr &operator=(UnaryInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal
