#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Add to pc
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.190
 */
template <typename TInstrContext> class AddToPcImmOp {
public:
  static inline OpResult Call(const TInstrContext &ictx, const InstrFlagsSet &iflags,
                              const u32 &imm32) {
    static_cast<void>(ictx);
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    const auto apc = Bm32::AlignDown<4>(pc);
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;
    const auto result = is_add != false ? apc + imm32 : apc - imm32;
    return OpResult{result, false, false};
  }
};

template <typename TOp, typename TInstrContext> class UnaryInstrImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_d, const u32 &imm32) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    InstrExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(InstrExecResult{eflags});
    }

    auto result = TOp::Call(ictx, iflags, imm32);

    ictx.cpua.WriteRegister(arg_d.Get(), result.value);

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

    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryInstrImm() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryInstrImm() = delete;

  /**
   * @brief Copy constructor for UnaryInstrImm.
   * @param r_src the object to be copied
   */
  UnaryInstrImm(const UnaryInstrImm &r_src) = default;

  /**
   * @brief Copy assignment operator for UnaryInstrImm.
   * @param r_src the object to be copied
   */
  UnaryInstrImm &operator=(const UnaryInstrImm &r_src) = delete;

  /**
   * @brief Move constructor for UnaryInstrImm.
   * @param r_src the object to be moved
   */
  UnaryInstrImm(UnaryInstrImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryInstrImm.
   * @param r_src the object to be moved
   */
  UnaryInstrImm &operator=(UnaryInstrImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu