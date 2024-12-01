#pragma once
#include "libmicroemu/internal/cpu_accessor.h"
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
 * @brief Table branch
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p. 416
 */
template <typename TInstrContext> class TbbH2Op {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rn, const TArg1 &rm) {

    const bool is_tbh = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kTbh)) != 0U;
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    me_adr_t halfwords{0U};
    if (is_tbh) {
      me_adr_t adr = (n + Alu32::LSL(m, 1U));
      TRY_ASSIGN(out, InstrExecFlagsSet,
                 ictx.bus.template ReadOrRaise<u16>(ictx.cpua, adr,
                                                    BusExceptionType::kRaisePreciseDataBusError));
      halfwords = out;
    } else {
      me_adr_t adr = n + m;
      TRY_ASSIGN(out, InstrExecFlagsSet,
                 ictx.bus.template ReadOrRaise<u8>(ictx.cpua, adr,
                                                   BusExceptionType::kRaisePreciseDataBusError));
      halfwords = out;
    }

    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    Pc::BranchWritePC(ictx.cpua, pc + (halfwords << 1U));
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class BinaryBranchInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rn, const TArg1 &rm) {

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rn, rm));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryBranchInstr() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryBranchInstr() = delete;

  /**
   * @brief Copy constructor for BinaryBranchInstr.
   * @param r_src the object to be copied
   */
  BinaryBranchInstr(const BinaryBranchInstr &r_src) = delete;

  /**
   * @brief Copy assignment operator for BinaryBranchInstr.
   * @param r_src the object to be copied
   */
  BinaryBranchInstr &operator=(const BinaryBranchInstr &r_src) = delete;

  /**
   * @brief Move constructor for BinaryBranchInstr.
   * @param r_src the object to be moved
   */
  BinaryBranchInstr(BinaryBranchInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryBranchInstr.
   * @param r_src the object to be moved
   */
  BinaryBranchInstr &operator=(BinaryBranchInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal
