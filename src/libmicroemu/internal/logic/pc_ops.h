#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>

namespace libmicroemu {
namespace internal {

template <typename TProcessorStates, typename TBus, typename TRegOps, typename TSpecRegOps,
          typename TExceptionReturn, typename TLogger = NullLogger>
class PcOps {
public:
  using ExcRet = TExceptionReturn;
  using SReg = TSpecRegOps;

  static inline void BranchTo(TProcessorStates &pstates, const u32 &value) {
    TRegOps::ForceWritePC(pstates, value);
  }

  static inline void BranchWritePC(TProcessorStates &pstates, const me_adr_t &address) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.30
    BranchTo(pstates, address & (~0x1U));
  }

  static inline Result<void> BXWritePC(TProcessorStates &pstates, TBus &bus,
                                       const me_adr_t &address) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.31

    auto sys_ctrl = SReg::template ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);

    // TODO: Own functions for current mode and is_handler mode
    const auto current_mode = sys_ctrl & SysCtrlRegister::kExecModeMsk;
    const auto is_handler_mode = current_mode == SysCtrlRegister::kExecModeHandler;

    // if CurrentMode == Mode_Handler && address<31:28> == ‘1111’ then
    if (is_handler_mode && ((address & 0xF0000000U) == 0xF0000000U)) {
      LOG_TRACE(TLogger, "BXWritePC (Exception Return): address=0x%08X, is_handler_mode=%d",
                address, is_handler_mode);
      //     ExceptionReturn(address<27:0>);
      return ExcRet::Return(pstates, bus, address & 0x0FFFFFFFU);
    } else {
      auto epsr = SReg::template ReadRegister<SpecialRegisterId::kEpsr>(pstates);

      if ((address & 0x1) == 0U) {
        LOG_ERROR(TLogger, "BXWritePC: Set wrong execution state");
      }
      // TODO: Add UsageFault exception

      // EPSR.T = address<0>;
      // if EPSR.T == 0, a UsageFault(‘Invalid State’) is taken on the next instruction
      epsr &= ~EpsrRegister::kTMsk;
      epsr |= (address & 0x1U) << EpsrRegister::kTPos;
      SReg::template WriteRegister<SpecialRegisterId::kEpsr>(pstates, epsr);
      BranchTo(pstates, address & (~0x1));
      return Ok();
    }
    // should never reach here
  }

  static inline void BLXWritePC(TProcessorStates &pstates, const me_adr_t &address) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.31

    if (address & 0x1) {
      LOG_ERROR(TLogger, "BLXWritePC: Set wrong execution state");
    }
    // EPSR.T = address<0>;
    // if EPSR.T == 0, a UsageFault(‘Invalid State’) is taken on the next instruction
    auto epsr = SReg::template ReadRegister<SpecialRegisterId::kEpsr>(pstates);
    epsr &= ~EpsrRegister::kTMsk;
    epsr |= (address & 0x1U) << EpsrRegister::kTPos;
    SReg::template WriteRegister<SpecialRegisterId::kEpsr>(pstates, epsr);
    pstates.BranchTo(address & (~0x1U));
  }

  static inline Result<void> LoadWritePC(TProcessorStates &pstates, TBus &bus,
                                         const me_adr_t &address) {

    // see Armv7-M Architecture Reference Manual Issue E.e p.31
    return BXWritePC(pstates, bus, address);
  }

  static inline void ALUWritePC(TProcessorStates &pstates, const me_adr_t &address) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.31
    BranchWritePC(pstates, address);
  }

  /// @brief Advances the program counter to the next instruction
  /// @param is_32bit is the current instruction 32 bit in size?
  static inline void AdvanceInstr(TProcessorStates &pstates, bool is_32bit) {
    // The pc points to the current instruction +4. Therefore decrement 2
    // in case we have a 16 bit instruction

    u32 pc = TRegOps::ReadPC(pstates);
    pc += is_32bit ? 0U : -2U;
    BranchTo(pstates, pc);
  }

private:
  /// \brief Constructs a PcOps object
  PcOps() = delete;

  /// \brief Destructor
  ~PcOps() = delete;

  /// \brief Copy constructor for PcOps.
  /// \param r_src the object to be copied
  PcOps(PcOps &r_src) = delete;

  /// \brief Copy assignment operator for PcOps.
  /// \param r_src the object to be copied
  PcOps &operator=(const PcOps &r_src) = delete;

  /// \brief Move constructor for PcOps.
  /// \param r_src the object to be copied
  PcOps(PcOps &&r_src) = delete;

  /// \brief Move assignment operator for PcOps.
  /// \param r_src the object to be copied
  PcOps &operator=(PcOps &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu
