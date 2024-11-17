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

template <typename TOp, typename TInstrContext> class TernaryStoreInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_n, const TArg1 &arg_t, const TArg2 &arg_d,
                                      const u32 &imm32) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    InstrExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(InstrExecResult{eflags});
    }

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());

    const me_adr_t offset_addr = is_add == true ? rn + imm32 : rn - imm32;
    const me_adr_t address = is_index == true ? offset_addr : rn;

    const auto rt = ictx.cpua.ReadRegister(arg_t.Get());
    u32 rd{0U};

    TRY(InstrExecResult, TOp::Write(ictx, address, rt, rd));

    // write back if write succeeded
    ictx.cpua.WriteRegister(arg_d.Get(), rd);

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    if (is_wback == true) {
      ictx.cpua.WriteRegister(arg_n.Get(), offset_addr);
    }
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);

    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  TernaryStoreInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~TernaryStoreInstrWithImm() = delete;

  /**
   * @brief Copy constructor for MemoTernaryStoreInstrWithImmryRouter.
   * @param r_src the object to be copied
   */
  TernaryStoreInstrWithImm(const TernaryStoreInstrWithImm &r_src) = default;

  /**
   * @brief Copy assignment operator for TernaryStoreInstrWithImm.
   * @param r_src the object to be copied
   */
  TernaryStoreInstrWithImm &operator=(const TernaryStoreInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for TernaryStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  TernaryStoreInstrWithImm(TernaryStoreInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  TernaryStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  TernaryStoreInstrWithImm &operator=(TernaryStoreInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu