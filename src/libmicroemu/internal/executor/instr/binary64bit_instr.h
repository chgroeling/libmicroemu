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
 * @brief Umull
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p. 435
 */
template <typename TInstrContext> class Umull2Op {
public:
  template <typename TDestLo, typename TDestHi, typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDestLo &rd_lo, const TDestHi &rd_hi, const TArg0 &rn,
                                        const TArg1 &rm) {
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());

    const u64 result = (static_cast<u64>(n) * static_cast<u64>(m));
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);

    PostExecWriteRegPcExcluded::Call(ictx, rd_lo, result_lo);
    PostExecWriteRegPcExcluded::Call(ictx, rd_hi, result_hi);
    PostExecAdvancePcAndIt::Call(ictx, iflags);

    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Smull
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p. 372
 */
template <typename TInstrContext> class Smull2Op {
public:
  template <typename TDestLo, typename TDestHi, typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDestLo &rd_lo, const TDestHi &rd_hi, const TArg0 &rn,
                                        const TArg1 &rm) {
    const auto n = static_cast<i32>(ictx.cpua.ReadRegister(rn.Get()));
    const auto m = static_cast<i32>(ictx.cpua.ReadRegister(rm.Get()));
    const u64 result = static_cast<u64>(static_cast<i64>(n) * static_cast<i64>(m));
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);

    PostExecWriteRegPcExcluded::Call(ictx, rd_lo, result_lo);
    PostExecWriteRegPcExcluded::Call(ictx, rd_hi, result_hi);
    PostExecAdvancePcAndIt::Call(ictx, iflags);

    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Umlal
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p. 434
 */
template <typename TInstrContext> class Umlal2Op {
public:
  template <typename TDestLo, typename TDestHi, typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDestLo &rd_lo, const TDestHi &rd_hi, const TArg0 &rn,
                                        const TArg1 &rm) {

    const u32 rdhi = ictx.cpua.ReadRegister(rd_hi.Get());
    const u32 rdlo = ictx.cpua.ReadRegister(rd_lo.Get());
    const u32 rd = static_cast<u64>(rdhi) << 32U | static_cast<u64>(rdlo);

    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());

    const u64 result = (static_cast<u64>(n) * static_cast<u64>(m)) + rd;
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);

    PostExecWriteRegPcExcluded::Call(ictx, rd_lo, result_lo);
    PostExecWriteRegPcExcluded::Call(ictx, rd_hi, result_hi);
    PostExecAdvancePcAndIt::Call(ictx, iflags);

    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class Binary64bitInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TDestLo, typename TDestHi, typename TArg0, typename TArg1>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TDestLo &rd_lo, const TDestHi &rd_hi, const TArg0 &rn,
                                      const TArg1 &rm) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd_lo, rd_hi, rn, rm));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  Binary64bitInstr() = delete;

  /**
   * @brief Destructor
   */
  ~Binary64bitInstr() = delete;

  /**
   * @brief Copy constructor for Binary64bitInstr.
   * @param r_src the object to be copied
   */
  Binary64bitInstr(const Binary64bitInstr &r_src) = delete;

  /**
   * @brief Copy assignment operator for Binary64bitInstr.
   * @param r_src the object to be copied
   */
  Binary64bitInstr &operator=(const Binary64bitInstr &r_src) = delete;

  /**
   * @brief Move constructor for Binary64bitInstr.
   * @param r_src the object to be moved
   */
  Binary64bitInstr(Binary64bitInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  Binary64bitInstr.
   * @param r_src the object to be moved
   */
  Binary64bitInstr &operator=(Binary64bitInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal
