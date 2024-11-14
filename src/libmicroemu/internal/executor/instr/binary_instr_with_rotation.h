#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Uxtb
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.452
 */
template <typename TInstrContext> class Uxtb1Rotation {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm, const u8 &rotation) {
    static_cast<void>(ictx);
    const u32 rotated = Alu32::ROR(rm, rotation);
    const u32 data = Bm8::ZeroExtend<u32>(static_cast<uint8_t>(rotated & 0xFF));
    return OpResult{data, false, false};
  }
};

/**
 * @brief Sxtb
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.413
 */
template <typename TInstrContext> class Sxtb1Rotation {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm, const u8 &rotation) {
    static_cast<void>(ictx);
    const u32 rotated = Alu32::ROR(rm, rotation);
    const u32 data = Bm8::SignExtend<u32, 7U>(static_cast<uint8_t>(rotated & 0xFF));
    return OpResult{data, false, false};
  }
};

/**
 * @brief Uxth
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.454
 */
template <typename TInstrContext> class Uxth1Rotation {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm, const u8 &rotation) {
    static_cast<void>(ictx);
    const u32 rotated = Alu32::ROR(rm, rotation);
    const u32 data = Bm16::ZeroExtend<u32>(static_cast<uint16_t>(rotated & 0xFFFFU));
    return OpResult{data, false, false};
  }
};

/**
 * @brief Sxth
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.414
 */
template <typename TInstrContext> class Sxth1Rotation {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm, const u8 &rotation) {
    static_cast<void>(ictx);
    const u32 rotated = Alu32::ROR(rm, rotation);
    const u32 val = Bm16::SignExtend<u32, 15>(static_cast<uint16_t>(rotated & 0xFFFFU));

    return OpResult{val, false, false};
  }
};

template <typename TOp, typename TInstrContext> class BinaryInstrWithRotation {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_m, const TArg1 &arg_d, const u8 &rotation) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    auto result = TOp::Call(ictx, rm, rotation);
    ictx.cpua.WriteRegister(arg_d.Get(), result.value);

    if ((iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U) {
      auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
      // Clear N, Z, C, V flags
      apsr &=
          ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

      apsr |= ((result.value >> 31U) & 0x1U) << ApsrRegister::kNPos;       // N
      apsr |= Bm32::IsZeroBit(result.value) << ApsrRegister::kZPos;        // Z
      apsr |= (result.carry_out == true ? 1U : 0U) << ApsrRegister::kCPos; // C

      apsr |= (result.overflow == true ? 1U : 0U) << ApsrRegister::kVPos; // V
      ictx.cpua.template WriteRegister<SpecialRegisterId::kApsr>(apsr);
    }
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);

    return Ok(ExecResult{eflags});
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
  BinaryInstrWithRotation(const BinaryInstrWithRotation &r_src) = default;

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

} // namespace internal
} // namespace libmicroemu