#pragma once

#include "libmicroemu/internal/fetcher/raw_instr.h"
#include "libmicroemu/internal/fetcher/raw_instr_flags_table.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>

namespace libmicroemu {
namespace internal {

template <typename TProcessorStates, typename TBus> class Fetcher {
public:
  static Result<RawInstr> Fetch(TProcessorStates &pstates, TBus &bus, const me_adr_t pc) {

    // make first 16 bit access to fetch instruction
    TRY_ASSIGN(
        instr_l, RawInstr,
        bus.template ReadOrRaise<u16>(pstates, pc, BusExceptionType::kRaiseInstructionBusError));

    const u32 opc = Bm32::ExtractBits1R<kFlagsOpCodeLast, kFlagsOpCodeFirst>(instr_l);

    // Check if a 32-bit instruction was loaded
    u16 instr_h{0x0U};
    const auto flags = kRawInstrFlagsTable[opc];

    if ((flags & static_cast<RawInstrFlagsSet>(RawInstrFlagsMsk::k32Bit)) != 0U) {
      // make the second access
      TRY_ASSIGN(instr_h_im, RawInstr,
                 bus.template ReadOrRaise<u16>(pstates, pc + 2U,
                                               BusExceptionType::kRaiseInstructionBusError));
      instr_h = instr_h_im;
    }

    return Ok(RawInstr{instr_l, instr_h, flags});
  }

private:
  /// \brief Constructs a Fetcher object
  Fetcher() = delete;

  /// \brief Destructor
  ~Fetcher() = delete;

  /// \brief Copy assignment operator for Fetcher.
  /// \param r_src the object to be copied
  Fetcher &operator=(const Fetcher &r_src) = delete;

  /// \brief Move constructor for Fetcher.
  /// \param r_src the object to be copied
  Fetcher(Fetcher &&r_src) = delete;

  /// \brief Move assignment operator for Fetcher.
  /// \param r_src the object to be copied
  Fetcher &operator=(Fetcher &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu
