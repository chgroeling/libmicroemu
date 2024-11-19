#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load multiple
 *
 * LDM - see Armv7-M Architecture Reference Manual Issue E.e p.242
 * POP - see Armv7-M Architecture Reference Manual Issue E.e p.319
 */
template <typename TInstrContext> class VariadicLoadInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  using ExcTrig = typename TInstrContext::ExcTrig;

  template <typename TArg>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg &arg_n, const u32 &registers) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    auto address = static_cast<me_adr_t>(rn);

    for (u32 rid = 0U; rid <= 14U; ++rid) {
      const u32 bm = 0x1U << rid;
      if ((registers & bm) != 0U) {
        TRY_ASSIGN(rdat, InstrExecResult,
                   ictx.bus.template ReadOrRaise<u32>(ictx.cpua, address,
                                                      BusExceptionType::kRaisePreciseDataBusError));
        ictx.cpua.WriteRegister(static_cast<RegisterId>(rid), rdat);
        address += static_cast<me_adr_t>(4U);
      }
    }

    if (Bm32::ExtractBits1R<15U, 15U>(registers) == 0x1U) {
      TRY_ASSIGN(rdat, InstrExecResult,
                 ictx.bus.template ReadOrRaise<u32>(ictx.cpua, address,
                                                    BusExceptionType::kRaisePreciseDataBusError));
      TRY(InstrExecResult, PostExecLoadWritePc::Call(ictx, rdat));

      //  do not advance pc
    } else {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
    }

    // For debugging purposes
    // MemoryViewer<TMemAccess> mem_view(mem_acs_);
    // mem_view.Print(0x80000 - 0x32, 0x64);

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const auto n_mask = 1U << static_cast<u32>(arg_n.Get());
    const bool is_rn_in_set = (registers & n_mask) != 0;

    if ((is_wback) && (!is_rn_in_set)) {
      auto regcount = Bm32::BitCount(registers);
      const auto wback_val = rn + 4U * regcount;

      // Update n register
      ictx.cpua.WriteRegister(arg_n.Get(), wback_val);
    }

    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

private:
  /**
   * @brief Constructor
   */
  VariadicLoadInstr() = delete;

  /**
   * @brief Destructor
   */
  ~VariadicLoadInstr() = delete;

  /**
   * @brief Copy constructor for VariadicLoadInstr.
   * @param r_src the object to be copied
   */
  VariadicLoadInstr(const VariadicLoadInstr &r_src) = delete;

  /**
   * @brief Copy assignment operator for VariadicLoadInstr.
   * @param r_src the object to be copied
   */
  VariadicLoadInstr &operator=(const VariadicLoadInstr &r_src) = delete;

  /**
   * @brief Move constructor for VariadicLoadInstr.
   * @param r_src the object to be moved
   */
  VariadicLoadInstr(VariadicLoadInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  VariadicLoadInstr.
   * @param r_src the object to be moved
   */
  VariadicLoadInstr &operator=(VariadicLoadInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal