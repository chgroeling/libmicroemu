/**
 * @file
 * @brief Contains the reset logic for the processor.
 */
#pragma once

#include "libmicroemu/internal/logic/reg_access.h"
#include "libmicroemu/internal/logic/spec_reg_access.h"
#include "libmicroemu/internal/processor/step_flags.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/trace/intstr_to_mnemonic.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/machine.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"
#include <assert.h>
#include <cstddef>
#include <cstdint>

namespace libmicroemu::internal {

/**
 * @brief Reset logic for the processor.
 * @tparam TCpuAccessor The type of the CPU accessor object.
 * @tparam TBus The type of the bus object.
 * @tparam TProcessorOps The type of the processor operations object.
 * @tparam TLogger The type of the logger object.
 */
template <typename TCpuAccessor, typename TBus, typename TProcessorOps,
          typename TLogger = NullLogger>
class ResetLogic {
public:
  using Exc = typename TProcessorOps::Exc;
  using Pc = typename TProcessorOps::Pc;

  /**
   * @brief Reset the processor.
   * @param cpua The CPU accessor object.
   * @param bus The bus object.
   * @return Result<void> The result of the operation.
   */
  static Result<void> TakeReset(TCpuAccessor &cpua, TBus &bus) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.531
    LOG_INFO(TLogger, "Resetting processor");

    auto sys_ctrl = cpua.template ReadRegister<SpecialRegisterId::kSysCtrl>();

    // CurrentMode = Mode_Thread;
    sys_ctrl &= ~static_cast<u32>(SysCtrlRegister::kExecModeMsk);
    sys_ctrl |= static_cast<u32>(SysCtrlRegister::kExecModeThread);

    // PRIMASK<0> = '0';  /* priority mask cleared at reset */
    // FAULTMASK<0> = '0'; /* fault mask cleared at reset */
    // BASEPRI<7:0> = Zeros(8);  /* base priority disabled at reset */

    // if HaveFPExt() then /* initialize the Floating Point Extn */
    if (false) {
      // CONTROL<2:0> = '000';  /* FP inactive, stack is Main, thread is privileged */
      // CPACR.cp10 = '00';
      // CPACR.cp11 = '00';
      // FPDSCR.AHP = '0';
      // FPDSCR.DN = '0';
      // FPDSCR.FZ = '0';
      // FPDSCR.RMode = '00';
      // FPCCR.ASPEN = '1';
      // FPCCR.LSPEN = '1';
      // FPCCR.LSPACT = '0';
      // FPCAR = bits(32) UNKNOWN;
      // FPFSR = bits(32) UNKNOWN;
      // for i = 0 to 31
      //   S[i] = bits(32) UNKNOWN;
    } else {
      // CONTROL<1:0> = '00';    /* current stack is Main, thread is privileged */
      sys_ctrl &= ~static_cast<u32>(SysCtrlRegister::kControlSpSelMsk);
      sys_ctrl &= ~static_cast<u32>(SysCtrlRegister::kControlNPrivMsk);
    }

    cpua.template WriteRegister<SpecialRegisterId::kSysCtrl>(sys_ctrl);

    // for i = 0 to 511 /* all exceptions Inactive */
    //   ExceptionActive[i] = '0';
    Exc::InitDefaultExceptionStates(cpua);

    // ResetSCSRegs(); /* catch-all function for System Control Space reset */
    // ClearExclusiveLocal(ProcessorID()); /* Synchronization (LDREX* / STREX*) monitor support */
    // ClearEventRegister(); /* see WFE instruction for more details */

    // All registers are UNKNOWN
    // for i = 0 to 12
    //   R[i] = bits(32) UNKNOWN;

    // bits(32) vectortable = VTOR<31:7>:'0000000';
    const auto vtor = cpua.template ReadRegister<SpecialRegisterId::kVtor>();
    me_adr_t vectortable = vtor << 7 | 0x0U;

    // SP_main = MemA_with_priv[vectortable, 4, AccType_VECTABLE] AND 0xFFFFFFFC<31:0>;
    TRY_ASSIGN(sp_main, void,
               bus.template ReadOrRaise<u32>(cpua, vectortable,
                                             BusExceptionType::kRaisePreciseDataBusError));
    cpua.template WriteRegister<SpecialRegisterId::kSpMain>(sp_main);

    // SP_process = ((bits(30) UNKNOWN):'00');
    auto sp_process = cpua.template ReadRegister<SpecialRegisterId::kSpProcess>();
    sp_process &= ~0x3U; // clear the two least significant bits
    cpua.template WriteRegister<SpecialRegisterId::kSpProcess>(sp_process);

    // LR = 0xFFFFFFFF<31:0>; /* preset to an illegal exception return value */
    cpua.template WriteRegister<RegisterId::kLr>(0xFFFFFFFFU);

    // tmp = MemA_with_priv[vectortable+4, 4, AccType_VECTABLE];
    TRY_ASSIGN(tmp, void,
               bus.template ReadOrRaise<u32>(cpua, vectortable + 0x4U,
                                             BusExceptionType::kRaisePreciseDataBusError));

    // tbit = tmp<0>;
    auto tbit = tmp & 0x1U;

    // APSR = bits(32) UNKNOWN; /* flags UNPREDICTABLE from reset */

    // IPSR<8:0> = Zeros(9); /* Exception Number cleared */
    auto ipsr = cpua.template ReadRegister<SpecialRegisterId::kIpsr>();
    ipsr &= ~static_cast<u32>(IpsrRegister::kExceptionNumberMsk);
    cpua.template WriteRegister<SpecialRegisterId::kIpsr>(ipsr);

    // EPSR.T = tbit; /* T bit set from vector */
    // EPSR.IT<7:0> = Zeros(8); /* IT/ICI bits cleared */
    auto epsr = cpua.template ReadRegister<SpecialRegisterId::kEpsr>();
    epsr &= ~static_cast<u32>(EpsrRegister::kItMsk) & // clear it bits
            ~static_cast<u32>(EpsrRegister::kTMsk);   // clear t bit
    epsr |= tbit << EpsrRegister::kTPos;
    cpua.template WriteRegister<SpecialRegisterId::kEpsr>(epsr);

    // BranchTo(tmp AND 0xFFFFFFFE<31:0>); /* address of reset service routine */
    auto entry_point = tmp & 0xFFFFFFFEU;
    Pc::BranchTo(cpua, entry_point);
    LOG_DEBUG(TLogger, "Set entry Point to 0x%08X / tbit:%i", entry_point, tbit);

    // CSR.STKALIGN = '1'; <-- added as default
    /* stack alignment is 8-byte aligned per default*/
    auto ccr = cpua.template ReadRegister<SpecialRegisterId::kCcr>();
    ccr |= static_cast<u32>(CcrRegister::kStkAlignMsk);
    cpua.template WriteRegister<SpecialRegisterId::kCcr>(ccr);
    LOG_TRACE(TLogger, "CSR: 0x%08X", ccr);

#if IS_LOGLEVEL_TRACE_ENABLED == true // LOG TRACE apsr, ipsr, epsr, xpsr
    {
      auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
      auto ipsr = cpua.template ReadRegister<SpecialRegisterId::kIpsr>();
      auto epsr = cpua.template ReadRegister<SpecialRegisterId::kEpsr>();
      auto xpsr = cpua.template ReadRegister<SpecialRegisterId::kXpsr>();
      LOG_DEBUG(TLogger, "APSR: 0x%08X, IPSR: 0x%08X, EPSR: 0x%08X, XPSR: 0x%08X", apsr, ipsr, epsr,
                xpsr);
    }
#endif
    return Ok();
  }

private:
  ResetLogic() = delete;
  ~ResetLogic() = delete;
  ResetLogic(ResetLogic &r_src) = delete;
  ResetLogic &operator=(const ResetLogic &r_src) = delete;
  ResetLogic(ResetLogic &&r_src) = delete;
  ResetLogic &operator=(ResetLogic &&r_src) = delete;
};

} // namespace libmicroemu::internal