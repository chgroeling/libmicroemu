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
 * @brief Uxtb
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.452
 */
template <typename TInstrContext> class Uxtb1Rotation {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rm, const u8 &rotation) {
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const u32 rotated = Alu32::ROR(m, rotation);
    const u32 data = Bm8::ZeroExtend<u32>(static_cast<uint8_t>(rotated & 0xFFU));
    const auto op_res = OpResult{data, false, false};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Sxtb
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.413
 */
template <typename TInstrContext> class Sxtb1Rotation {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rm, const u8 &rotation) {
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const u32 rotated = Alu32::ROR(m, rotation);
    const u32 data = Bm8::SignExtend<u32, 7U>(static_cast<uint8_t>(rotated & 0xFFU));
    const auto op_res = OpResult{data, false, false};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Uxth
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.454
 */
template <typename TInstrContext> class Uxth1Rotation {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rm, const u8 &rotation) {
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const u32 rotated = Alu32::ROR(m, rotation);
    const u32 data = Bm16::ZeroExtend<u32>(static_cast<uint16_t>(rotated & 0xFFFFU));
    const auto op_res = OpResult{data, false, false};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

/**
 * @brief Sxth
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.414
 */
template <typename TInstrContext> class Sxth1Rotation {
public:
  template <typename TArg0, typename TArg1>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TArg0 &rd, const TArg1 &rm, const u8 &rotation) {
    const auto m = ictx.cpua.ReadRegister(rm.Get());
    const u32 rotated = Alu32::ROR(m, rotation);
    const u32 val = Bm16::SignExtend<u32, 15>(static_cast<uint16_t>(rotated & 0xFFFFU));
    const auto op_res = OpResult{val, false, false};

    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class BinaryInstrWithRotation {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rd, const TArg1 &rm, const u8 &rotation) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd, rm, rotation));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryInstrWithRotation() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryInstrWithRotation() = delete;

  /**
   * @brief Copy constructor for BinaryInstrWithRotation.
   * @param r_src the object to be copied
   */
  BinaryInstrWithRotation(const BinaryInstrWithRotation &r_src) = delete;

  /**
   * @brief Copy assignment operator for MeBinaryInstrWithRotationmoryRouter.
   * @param r_src the object to be copied
   */
  BinaryInstrWithRotation &operator=(const BinaryInstrWithRotation &r_src) = delete;

  /**
   * @brief Move constructor for BinaryInstrWithRotation.
   * @param r_src the object to be moved
   */
  BinaryInstrWithRotation(BinaryInstrWithRotation &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryInstrWithRotation.
   * @param r_src the object to be moved
   */
  BinaryInstrWithRotation &operator=(BinaryInstrWithRotation &&r_src) = delete;
};

} // namespace libmicroemu::internal
