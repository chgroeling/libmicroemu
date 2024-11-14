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
 * @brief Clz
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.220
 */
template <typename TInstrContext> class Clz1Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm) {
    static_cast<void>(ictx);
    const u32 result = Bm32::CountLeadingZeros(rm);
    return OpResult{result, false, false};
  }
};

/**
 * @brief Mov
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.293
 */
template <typename TInstrContext> class Mov1Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    return OpResult{rm, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/**
 * @brief Rrx
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.340
 */
template <typename TInstrContext> class Rrx1Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm) {

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    auto r_rrx = Alu32::Shift_C(rm, SRType::SRType_RRX, 1U,
                                (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_rrx.result, r_rrx.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

template <typename TOp, typename TInstrContext> class UnaryInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_d, const TArg1 &arg_m) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    auto result = TOp::Call(ictx, rm);

    if (arg_d.Get() == RegisterId::kPc) {
      Pc::ALUWritePC(ictx.cpua, result.value);
      It::ITAdvance(ictx.cpua);
      return Ok(ExecResult{eflags});
    } else {
      ictx.cpua.WriteRegister(arg_d.Get(), result.value);
    }
    if ((iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U) {
      auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

      // Clear N, Z, C, V flags
      apsr &=
          ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

      apsr |= ((result.value >> 31U) & 0x1U) << ApsrRegister::kNPos;       // N
      apsr |= Bm32::IsZeroBit(result.value) << ApsrRegister::kZPos;        // Z
      apsr |= (result.carry_out == true ? 1U : 0U) << ApsrRegister::kCPos; // C
      apsr |= (result.overflow == true ? 1U : 0U) << ApsrRegister::kVPos;  // V
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
  UnaryInstr() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryInstr() = delete;

  /**
   * @brief Copy constructor for UnaryInstr.
   * @param r_src the object to be copied
   */
  UnaryInstr(const UnaryInstr &r_src) = default;

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

} // namespace internal
} // namespace libmicroemu