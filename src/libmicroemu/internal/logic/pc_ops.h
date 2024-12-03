#pragma once

#include "libmicroemu/internal/logic/predicates.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>

namespace libmicroemu::internal {

template <typename TCpuAccessor, typename TBus, typename TExceptionReturn,
          typename TLogger = NullLogger>
class PcOps {
public:
  using ExcRet = TExceptionReturn;

  static inline void BranchTo(TCpuAccessor &cpua, const me_adr_t &pc) {
    auto &registers = cpua.GetRegisters();
    registers[static_cast<u8>(RegisterId::kPc)] = pc;
  }

  static inline void BranchWritePC(TCpuAccessor &cpua, const me_adr_t &address) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.30
    BranchTo(cpua, address & (~0x1U));
  }

  static inline Result<void> BXWritePC(TCpuAccessor &cpua, TBus &bus, const me_adr_t &address) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.31
    const auto is_handler_mode = Predicates::IsHandlerMode(cpua);

    // if CurrentMode == Mode_Handler && address<31:28> == ‘1111’ then
    if (is_handler_mode && ((address & 0xF0000000U) == 0xF0000000U)) {
      LOG_TRACE(TLogger, "BXWritePC (Exception Return): address=0x%08X, is_handler_mode=%d",
                address, is_handler_mode);
      //     ExceptionReturn(address<27:0>);
      return ExcRet::Return(cpua, bus, address & 0x0FFFFFFFU);
    } else {
      auto epsr = cpua.template ReadSpecialRegister<SpecialRegisterId::kEpsr>();

      if ((address & 0x1U) == 0U) {
        LOG_ERROR(TLogger, "BXWritePC: Set wrong execution state");
      }

      // EPSR.T = address<0>;
      // if EPSR.T == 0, a UsageFault(‘Invalid State’) is taken on the next instruction
      epsr &= ~EpsrRegister::kTMsk;
      epsr |= (address & 0x1U) << EpsrRegister::kTPos;
      cpua.template WriteSpecialRegister<SpecialRegisterId::kEpsr>(epsr);
      BranchTo(cpua, address & (~0x1));
      return Ok();
    }
    // should never reach here
  }

  static inline void BLXWritePC(TCpuAccessor &cpua, const me_adr_t &address) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.31

    if (address & 0x1) {
      LOG_ERROR(TLogger, "BLXWritePC: Set wrong execution state");
    }
    // EPSR.T = address<0>;
    // if EPSR.T == 0, a UsageFault(‘Invalid State’) is taken on the next instruction
    auto epsr = cpua.template ReadSpecialRegister<SpecialRegisterId::kEpsr>();
    epsr &= ~EpsrRegister::kTMsk;
    epsr |= (address & 0x1U) << EpsrRegister::kTPos;
    cpua.template WriteSpecialRegister<SpecialRegisterId::kEpsr>(epsr);
    BranchTo(cpua, address & (~0x1U));
  }

  static inline Result<void> LoadWritePC(TCpuAccessor &cpua, TBus &bus, const me_adr_t &address) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.31
    return BXWritePC(cpua, bus, address);
  }

  static inline void ALUWritePC(TCpuAccessor &cpua, const me_adr_t &address) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.31
    BranchWritePC(cpua, address);
  }

  /**
   * @brief Advances the program counter to the next instruction
   * @param is_32bit is the current instruction 32 bit in size?
   */
  static inline void AdvanceInstr(TCpuAccessor &cpua, bool is_32bit) {
    // The pc points to the current instruction +4. Therefore decrement 2
    // in case we have a 16 bit instruction

    me_adr_t pc = static_cast<me_adr_t>(cpua.template ReadRegister<RegisterId::kPc>());
    pc += is_32bit ? 0U : -2U;
    BranchTo(cpua, pc);
  }

private:
  /**
   * @brief Constructs a PcOps object
   */
  PcOps() = delete;

  /**
   * @brief Destructor
   */
  ~PcOps() = delete;

  /**
   * @brief Copy constructor for PcOps.
   * @param r_src the object to be copied
   */
  PcOps(PcOps &r_src) = delete;

  /**
   * @brief Copy assignment operator for PcOps.
   * @param r_src the object to be copied
   */
  PcOps &operator=(const PcOps &r_src) = delete;

  /**
   * @brief Move constructor for PcOps.
   * @param r_src the object to be moved
   */
  PcOps(PcOps &&r_src) = delete;

  /**
   * @brief Move assignment operator for  PcOps.
   * @param r_src the object to be moved
   */
  PcOps &operator=(PcOps &&r_src) = delete;
};

} // namespace libmicroemu::internal
