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
 * @brief Umull
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p. 435
 */
template <typename TInstrContext> class Umull2Op {
public:
  template <typename TDestLo, typename TDestHi, typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDestLo &arg_d_lo, const TDestHi &arg_d_hi,
                                        const TArg0 &arg_n, const TArg1 &arg_m) {
    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());

    const u64 result = (static_cast<u64>(rn) * static_cast<u64>(rm));
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);

    PostExecWriteRegPcExcluded::Call(ictx, arg_d_lo, result_lo);
    PostExecWriteRegPcExcluded::Call(ictx, arg_d_hi, result_hi);
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
                                        const TDestLo &arg_d_lo, const TDestHi &arg_d_hi,
                                        const TArg0 &arg_n, const TArg1 &arg_m) {
    const auto rn = static_cast<i32>(ictx.cpua.ReadRegister(arg_n.Get()));
    const auto rm = static_cast<i32>(ictx.cpua.ReadRegister(arg_m.Get()));
    const u64 result = static_cast<u64>(static_cast<i64>(rn) * static_cast<i64>(rm));
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);

    PostExecWriteRegPcExcluded::Call(ictx, arg_d_lo, result_lo);
    PostExecWriteRegPcExcluded::Call(ictx, arg_d_hi, result_hi);
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
                                        const TDestLo &arg_d_lo, const TDestHi &arg_d_hi,
                                        const TArg0 &arg_n, const TArg1 &arg_m) {

    const u32 rdhi = ictx.cpua.ReadRegister(arg_d_hi.Get());
    const u32 rdlo = ictx.cpua.ReadRegister(arg_d_lo.Get());
    const u32 rd = static_cast<u64>(rdhi) << 32U | static_cast<u64>(rdlo);

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());

    const u64 result = (static_cast<u64>(rn) * static_cast<u64>(rm)) + rd;
    const u32 result_lo = static_cast<u32>(result & 0xFFFFFFFFU);
    const u32 result_hi = static_cast<u32>((result >> 32U) & 0xFFFFFFFFU);

    PostExecWriteRegPcExcluded::Call(ictx, arg_d_lo, result_lo);
    PostExecWriteRegPcExcluded::Call(ictx, arg_d_hi, result_hi);
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
                                      const TDestLo &arg_d_lo, const TDestHi &arg_d_hi,
                                      const TArg0 &arg_n, const TArg1 &arg_m) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, arg_d_lo, arg_d_hi, arg_n, arg_m));
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
