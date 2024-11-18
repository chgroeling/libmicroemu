#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TOp, typename TInstrContext> class UnaryLoadInstrImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  using ExcTrig = typename TInstrContext::ExcTrig;
  template <typename TArg>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const u32 &imm32, const TArg &arg_t) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    InstrExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(InstrExecResult{eflags});
    }

    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    // unary loads always refer to the pc
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());

    const me_adr_t base = Bm32::AlignDown<4>(pc);
    const me_adr_t address = is_add == true ? base + imm32 : base - imm32;

    TRY_ASSIGN(data, InstrExecResult, TOp::Read(ictx, address));

    if (arg_t.Get() == RegisterId::kPc) {
      // When the given address was unaligend the behaviour is
      // unpredtictable
      if ((address & 0x3U) == 0U) {
        It::ITAdvance(ictx.cpua);
        TRY(InstrExecResult, Pc::LoadWritePC(ictx.cpua, ictx.bus, data));

        return Ok(InstrExecResult{eflags});
      } else {
        return Err<InstrExecResult>(StatusCode::kExecutorUnpredictable);
      }
    } else {
      ictx.cpua.WriteRegister(arg_t.Get(), data);
    }
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);

    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryLoadInstrImm() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryLoadInstrImm() = delete;

  /**
   * @brief Copy constructor for UnaryLoadInstrImm.
   * @param r_src the object to be copied
   */
  UnaryLoadInstrImm(const UnaryLoadInstrImm &r_src) = default;

  /**
   * @brief Copy assignment operator for UnaryLoadInstrImm.
   * @param r_src the object to be copied
   */
  UnaryLoadInstrImm &operator=(const UnaryLoadInstrImm &r_src) = delete;

  /**
   * @brief Move constructor for UnaryLoadInstrImm.
   * @param r_src the object to be moved
   */
  UnaryLoadInstrImm(UnaryLoadInstrImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryLoadInstrImm.
   * @param r_src the object to be moved
   */
  UnaryLoadInstrImm &operator=(UnaryLoadInstrImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu