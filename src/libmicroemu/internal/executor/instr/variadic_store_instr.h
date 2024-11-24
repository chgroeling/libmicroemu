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
 * @brief Store Multiple
 *
 * STM - see Armv7-M Architecture Reference Manual Issue E.e p.383
 */
template <typename TInstrContext> class VariadicStoreInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rn, const u32 &registers) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    const auto n = ictx.cpua.ReadRegister(rn.Get());
    auto reg_count = Bm32::BitCount(registers);
    u32 address = n;
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool lowest_bit_set = Bm8::LowestBitSet(registers);
    for (u32 reg = 0U; reg <= 14U; ++reg) {
      u32 bm = 0x1U << reg;
      if ((registers & bm) != 0U) {
        if ((reg == static_cast<u32>(rn.Get())) && (is_wback) && (lowest_bit_set == reg)) {
          return Err<InstrExecResult>(StatusCode::kExecutorUndefined);
        } else {
          const auto r = ictx.cpua.ReadRegister(static_cast<RegisterId>(reg));
          TRY(InstrExecResult,
              ictx.bus.template WriteOrRaise<u32>(ictx.cpua, address, r,
                                                  BusExceptionType::kRaisePreciseDataBusError));
        }

        address += 4U;
      }
    }

    // For debugging purposes
    // MemoryViewer<TMemAccess> mem_view(bus_);
    // mem_view.Print(0x80000 - 0x32, 0x64);

    if (is_wback) {
      const auto wback_val = n + 4U * reg_count;

      // Update n register
      PostExecWriteRegPcExcluded::Call(ictx, rn, wback_val);
    }
    PostExecAdvancePcAndIt::Call(ictx, iflags);

    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

private:
  /**
   * @brief Constructor
   */
  VariadicStoreInstr() = delete;

  /**
   * @brief Destructor
   */
  ~VariadicStoreInstr() = delete;

  /**
   * @brief Copy constructor for VariadicStoreInstr.
   * @param r_src the object to be copied
   */
  VariadicStoreInstr(const VariadicStoreInstr &r_src) = delete;

  /**
   * @brief Copy assignment operator for VariadicStoreInstr.
   * @param r_src the object to be copied
   */
  VariadicStoreInstr &operator=(const VariadicStoreInstr &r_src) = delete;

  /**
   * @brief Move constructor for VariadicStoreInstr.
   * @param r_src the object to be moved
   */
  VariadicStoreInstr(VariadicStoreInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  VariadicStoreInstr.
   * @param r_src the object to be moved
   */
  VariadicStoreInstr &operator=(VariadicStoreInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal