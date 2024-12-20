#pragma once

#include "libmicroemu/exception_states.h"
#include "libmicroemu/exception_type.h"
#include "libmicroemu/internal/logic/predicates.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/register_id.h"
#include "libmicroemu/special_register_id.h"
#include <assert.h>
#include <cstddef>
#include <cstdint>

namespace libmicroemu::internal {

struct ExceptionContext {
  u32 return_adr; // return address of the exception
};

enum class ProcessorMode {
  kThread = 0U,  // Thread mode
  kHandler = 1U, // Handler mode
};

enum class ExecutionInstant {
  kPreFetch = 0U,     // Before fetching the instruction
  kPostFetch = 1U,    // After fetching the instruction
  kPostExecution = 2U // After executing the instruction
};

class ExceptionPreFetch {
public:
  static constexpr ExecutionInstant kInstant = ExecutionInstant::kPreFetch;

  // there are no real asynchronous exceptions in the emulator. But for the sake of
  // completeness, we can consider them as asynchronous
  static constexpr bool isSynchronous = false;
};

class ExceptionPostFetch {
public:
  static constexpr ExecutionInstant kInstant = ExecutionInstant::kPostFetch;

  static constexpr bool isSynchronous = true;
};

class ExceptionPostExecution {
public:
  static constexpr ExecutionInstant kInstant = ExecutionInstant::kPostExecution;
  static constexpr bool isSynchronous = true;
};

template <typename TCpuAccessor, typename TPcOps, typename TLogger = NullLogger>
class ExceptionsOps {
public:
  using SId = SpecialRegisterId;
  using Pc = TPcOps;

  ExceptionsOps() = delete;
  ~ExceptionsOps() = delete;
  ExceptionsOps &operator=(const ExceptionsOps &r_src) = delete;
  ExceptionsOps(ExceptionsOps &&r_src) = delete;
  ExceptionsOps &operator=(ExceptionsOps &&r_src) = delete;
  ExceptionsOps(const ExceptionsOps &r_src) = delete;

  static void SetProcessorMode(TCpuAccessor &cpua, const ProcessorMode &mode) {
    auto sys_ctrl = cpua.template ReadSpecialRegister<SId::kSysCtrl>();

    sys_ctrl &= ~SysCtrlRegister::kExecModeMsk;
    sys_ctrl |= mode == ProcessorMode::kHandler ? SysCtrlRegister::kExecModeHandler
                                                : SysCtrlRegister::kExecModeThread;
    cpua.template WriteSpecialRegister<SId::kSysCtrl>(sys_ctrl);
  }

  static void LogImportantRegisters(TCpuAccessor &cpua, const char *preamble,
                                    const ExceptionType &exception_type) {
#if IS_LOGLEVEL_TRACE_ENABLED == true
    const auto is_handler_mode = Predicates::IsHandlerMode(cpua);
    const auto mode_str = is_handler_mode ? "Handler" : "Thread";

    auto &exception_states = cpua.GetExceptionStates();
    auto &selected_exception = exception_states.exception[static_cast<u32>(exception_type) - 1U];

    auto apsr = cpua.template ReadSpecialRegister<SId::kApsr>();
    auto ipsr = cpua.template ReadSpecialRegister<SId::kIpsr>();
    auto epsr = cpua.template ReadSpecialRegister<SId::kEpsr>();
    auto xpsr = cpua.template ReadSpecialRegister<SId::kXpsr>();
    auto sp = cpua.template ReadRegister<RegisterId::kSp>();
    auto stack_type = Predicates::IsMainStack(cpua) ? "Main" : "Process";
    LOG_TRACE(TLogger,
              "%s: "
              "type_id = %d, "
              "priority = %d, "
              "CurrentMode = \"%s\", "
              "APSR = 0x%08X, "
              "IPSR = 0x%08X, "
              "EPSR = 0x%08X, "
              "XPSR = 0x%08X, "
              "SP = 0x%08X (%s)",
              preamble, static_cast<uint32_t>(exception_type), selected_exception.GetPriority(),
              mode_str, apsr, ipsr, epsr, xpsr, sp, stack_type);
#else
    static_cast<void>(cpua);
    static_cast<void>(preamble);
    static_cast<void>(exception_type);
#endif
  }
  static void InitDefaultExceptionStates(TCpuAccessor &cpua) {
    auto &exception_states = cpua.GetExceptionStates();
    exception_states.pending_exceptions = 0U;
    auto &exceptions = exception_states.exception;
    for (u32 i = 0U; i < CountExceptions(); ++i) {
      auto e_type = static_cast<ExceptionType>(i + 1U);
      switch (e_type) {
      case ExceptionType::kReset: {
        exceptions[i].SetPriority(-3);
        break;
      }
      case ExceptionType::kNMI: {
        exceptions[i].SetPriority(-2);
        break;
      }
      case ExceptionType::kHardFault: {
        exceptions[i].SetPriority(-1);
        break;
      }
      // For testing purposes
      // case ExceptionType::kSysTick: {
      //   exceptions[i].priority = 1;
      //   break;
      // }
      default: {
        exceptions[i].SetPriority(0);
        break;
      }
      }
      exceptions[i].ClearFlags();
    }
  }

  template <typename ExcInstant, typename TBus>
  static Result<void> ExceptionEntry(TCpuAccessor &cpua, TBus &bus,
                                     const ExceptionType &exception_type,
                                     const ExceptionContext &context) {
#if IS_LOGLEVEL_TRACE_ENABLED == true
    if (constexpr auto instant = ExcInstant::kInstant; instant == ExecutionInstant::kPreFetch) {
      LogImportantRegisters(cpua, "[BEGIN] ExceptionEntry (PreFetch)", exception_type);
    } else if (instant == ExecutionInstant::kPostFetch) {
      LogImportantRegisters(cpua, "[BEGIN] ExceptionEntry (PostFetch)", exception_type);
    } else {
      LogImportantRegisters(cpua, "[BEGIN] ExceptionEntry (PostExec)", exception_type);
    }
#endif

    TRY(void, PushStack<ExcInstant>(cpua, bus, exception_type, context));

    TRY(void, ExceptionTaken(cpua, bus, exception_type));

    LOG_TRACE(TLogger, "[END] ExceptionEntry");
    return Ok();
  }

  template <typename ExcInstant, typename TBus>
  static Result<void> PushStack(TCpuAccessor &cpua, TBus &bus, const ExceptionType &exception_type,
                                const ExceptionContext &context) {
    // Taken from: Armv7-M Architecture Reference Manual Issue E.e p532

    static_cast<void>(exception_type);

    u32 framesize{0U};
    u32 forcealign{0U};

    // if HaveFPExt() && CONTROL.FPCA == '1' then
    if (false) {
      // framesize = 0x68;
      // forcealign = '1';
    } else {
      framesize = 0x20U;
      auto ccr = cpua.template ReadSpecialRegister<SId::kCcr>();
      forcealign = (ccr & CcrRegister::kStkAlignMsk) >> CcrRegister::kStkAlignPos;
    }

    auto spmask = ~static_cast<u32>(forcealign << 2U);

    u32 frameptralign{0U};
    u32 frameptr{0U};

    const bool is_thread_mode = Predicates::IsThreadMode(cpua);
    const bool is_process_stack = Predicates::IsProcessStack(cpua);

    // if CONTROL.SPSEL == '1' && CurrentMode == Mode_Thread then
    if (is_process_stack && is_thread_mode) {
      auto sp_process = cpua.template ReadSpecialRegister<SId::kSpProcess>();

      frameptralign = ((sp_process & 0x4U) >> 2U) & forcealign;

      sp_process = (sp_process - framesize) & spmask;
      LOG_TRACE(TLogger, "Setting processes stack pointer to = 0x%08X", sp_process);
      cpua.template WriteSpecialRegister<SId::kSpProcess>(sp_process);

      // frameptralign = SP_process<2> AND forcealign;
      // SP_process = (SP_process - framesize) AND spmask;
      // frameptr = SP_process;

      frameptr = sp_process;
    } else {
      auto sp_main = cpua.template ReadSpecialRegister<SId::kSpMain>();

      frameptralign = ((sp_main & 0x4U) >> 2U) & forcealign;

      sp_main = (sp_main - framesize) & spmask;
      LOG_TRACE(TLogger, "Setting main stack pointer to = 0x%08X", sp_main);
      cpua.template WriteSpecialRegister<SId::kSpMain>(sp_main);

      frameptr = sp_main;
    }

    /* only the stack locations, not the store order, are architected */

    // MemA[frameptr,4     ] = R[0];
    const auto r0 = cpua.template ReadRegister<RegisterId::kR0>();
    TRY(void, bus.template WriteOrRaise<u32>(cpua, frameptr, r0, BusExceptionType::kRaiseUnstkerr));

    // MemA[frameptr+0x4,4 ] = R[1];
    const auto r1 = cpua.template ReadRegister<RegisterId::kR1>();
    TRY(void, bus.template WriteOrRaise<u32>(cpua, frameptr + 0x4U, r1,
                                             BusExceptionType::kRaiseUnstkerr));

    // MemA[frameptr+0x8,4 ] = R[2];
    const auto r2 = cpua.template ReadRegister<RegisterId::kR2>();
    TRY(void, bus.template WriteOrRaise<u32>(cpua, frameptr + 0x8U, r2,
                                             BusExceptionType::kRaiseUnstkerr));

    // MemA[frameptr+0xC,4 ] = R[3];
    const auto r3 = cpua.template ReadRegister<RegisterId::kR3>();
    TRY(void, bus.template WriteOrRaise<u32>(cpua, frameptr + 0xCU, r3,
                                             BusExceptionType::kRaiseUnstkerr));

    // MemA[frameptr+0x10,4] = R[12];
    const auto r12 = cpua.template ReadRegister<RegisterId::kR12>();
    TRY(void, bus.template WriteOrRaise<u32>(cpua, frameptr + 0x10U, r12,
                                             BusExceptionType::kRaiseUnstkerr));

    // MemA[frameptr+0x14,4] = LR;
    const auto lr = cpua.template ReadRegister<RegisterId::kLr>();
    TRY(void, bus.template WriteOrRaise<u32>(cpua, frameptr + 0x14U, lr,
                                             BusExceptionType::kRaiseUnstkerr));

    // MemA[frameptr+0x18,4] = ReturnAddress(ExceptionType);
    const auto return_address = ReturnAddress<ExcInstant>(cpua, exception_type, context);

    TRY(void, bus.template WriteOrRaise<u32>(cpua, frameptr + 0x18U, return_address,
                                             BusExceptionType::kRaiseUnstkerr));

    // MemA[frameptr+0x1C,4] = (XPSR<31:10>:frameptralign:XPSR<8:0>);
    //                         //see ReturnAddress() in-line note for information on XPSR.IT bits
    const auto xpsr = cpua.template ReadSpecialRegister<SId::kXpsr>();
    const auto xpsr_adapt = (xpsr & Bm32::GenerateBitMask<8U, 0U>()) | (frameptralign << 9U) |
                            (xpsr & Bm32::GenerateBitMask<31U, 10U>());

    TRY(void, bus.template WriteOrRaise<u32>(cpua, frameptr + 0x1CU, xpsr_adapt,
                                             BusExceptionType::kRaiseUnstkerr));

    LOG_TRACE(TLogger,
              "Pushed R0 = 0x%08X, "
              "R1 = 0x%08X, "
              "R2 = 0x%08X, "
              "R3 = 0x%08X, "
              "R12 = 0x%08X, "
              "LR = 0x%08X, "
              "ReturnAddress = 0x%08X, "
              "XPSR = 0x%08X",
              r0, r1, r2, r3, r12, lr, return_address, xpsr_adapt);

    // if HaveFPExt() && CONTROL.FPCA == '1' then
    if (false) {
      // if FPCCR.LSPEN == '0' then
      //   CheckVFPEnabled();
      //   for i = 0 to 15
      //     MemA[frameptr+0x20+(4*i),4] = S[i];
      //   MemA[frameptr+0x60,4] = FPSCR;
      //   for i = 0 to 15
      //     S[i] = bits(32) UNKNOWN;
      //   FPSCR = bits(32) UNKNOWN;
      // else
      //   UpdateFPCCR(frameptr);
    }

    // if HaveFPExt() then
    if (false) {
      // if CurrentMode==Mode_Handler then
      //   LR = Ones(27):NOT(CONTROL.FPCA):'0001'; else
      //   LR = Ones(27):NOT(CONTROL.FPCA):'1':CONTROL.SPSEL:'01';
    } else {
      const auto is_handler_mode = Predicates::IsHandlerMode(cpua);
      if (is_handler_mode) {
        // LR = Ones(28):'0001';
        const auto lr = Bm32::GenerateBitMask<31U, 4U>() | 0b0001U;
        LOG_TRACE(TLogger, "Setting LR = 0x%08X (currently in Handler mode)", lr);
        cpua.template WriteRegister<RegisterId::kLr>(lr);
      } else { // Thread mode
        // LR = Ones(29):CONTROL.SPSEL:'01';
        const auto sctrl = cpua.template ReadSpecialRegister<SId::kSysCtrl>();
        const auto spsel =
            (sctrl & SysCtrlRegister::kControlSpSelMsk) >> SysCtrlRegister::kControlSpSelPos;
        const auto lr = Bm32::GenerateBitMask<31U, 3U>() | (spsel << 2U) | 0b01U;

        LOG_TRACE(TLogger, "Setting LR = 0x%08X (currently in Thread mode)", lr);
        cpua.template WriteRegister<RegisterId::kLr>(lr);
      }
    }
    return Ok();
  }

  // Call before the emulator fetches an instruction
  // This is typically the point where external exceptions are taken.
  // The return address is the address of the instruction which will be fetched next.
  template <typename ExcInstant,
            typename std::enable_if_t<ExcInstant::kInstant == ExecutionInstant::kPreFetch, int> = 0>
  static u32 ReturnAddress(TCpuAccessor &cpua, const ExceptionType &exception_type,
                           const ExceptionContext &context) {
    static_cast<void>(cpua);
    // Taken from: Armv7-M Architecture Reference Manual Issue E.e p534
    // bits(32) ReturnAddress(integer ExceptionType)
    // // Returns the following values based on the exception cause
    // // NOTE: ReturnAddress() is always halfword aligned, meaning bit<0> is always zero
    //          xPSR.IT bits saved to the stack are consistent with ReturnAddress()

    switch (exception_type) {
    case ExceptionType::kNMI:
    case ExceptionType::kSVCall:
    case ExceptionType::kPendSV:
    case ExceptionType::kSysTick: {
      return context.return_adr;
    }

    default: {
      if (static_cast<u32>(exception_type) >= 16U) {
        return context.return_adr;
      }
      assert(false &&
             "Return address calculation of these exceptions should not be called at this point");
      return context.return_adr;
    }
    }
    return 0x0U; // should not happen
  }

  template <typename ExcInstant, typename std::enable_if_t<
                                     ExcInstant::kInstant == ExecutionInstant::kPostFetch, int> = 0>
  static u32 ReturnAddress(TCpuAccessor &cpua, const ExceptionType &exception_type,
                           const ExceptionContext &context) {
    static_cast<void>(cpua);
    static_cast<void>(context);
    // Taken from: Armv7-M Architecture Reference Manual Issue E.e p534
    // bits(32) ReturnAddress(integer ExceptionType)
    // // Returns the following values based on the exception cause
    // // NOTE: ReturnAddress() is always halfword aligned, meaning bit<0> is always zero
    //          xPSR.IT bits saved to the stack are consistent with ReturnAddress()

    switch (exception_type) {
    case ExceptionType::kMemoryManagementFault:
    case ExceptionType::kUsageFault: // Invalid state
    case ExceptionType::kBusFault:   // instruction fetch from a location that does not allow that
    {
      return context.return_adr;
    }

    default: {
      assert(false &&
             "Return address calculation of these exceptions should not be called at this point");
      return context.return_adr;
    }
    }
    return 0x0U; // should not happen
  }

  // Call after the emulator executes an instruction
  // This is typically the point where internal exceptions are taken.
  // The return address is the address of the instruction which was executed.
  template <
      typename ExcInstant,
      typename std::enable_if_t<ExcInstant::kInstant == ExecutionInstant::kPostExecution, int> = 0>
  static u32 ReturnAddress(TCpuAccessor &cpua, const ExceptionType &exception_type,
                           const ExceptionContext &context) {
    static_cast<void>(cpua);
    // Taken from: Armv7-M Architecture Reference Manual Issue E.e p534
    // bits(32) ReturnAddress(integer ExceptionType)
    // // Returns the following values based on the exception cause
    // // NOTE: ReturnAddress() is always halfword aligned, meaning bit<0> is always zero
    //          xPSR.IT bits saved to the stack are consistent with ReturnAddress()

    switch (exception_type) {
    case ExceptionType::kHardFault:
    case ExceptionType::kMemoryManagementFault:
    case ExceptionType::kBusFault:
    case ExceptionType::kUsageFault:
    case ExceptionType::kDebugMonitor: {
      return context.return_adr;
    }
    default: {
      assert(false &&
             "Return address calculation of these exceptions should not be called at this point");
    }
    }
    return 0x0U; // should not happen
  }

  template <typename TBus>
  static Result<void> ExceptionTaken(TCpuAccessor &cpua, TBus &bus,
                                     const ExceptionType &exception_type) {
    // Taken from: Armv7-M Architecture Reference Manual Issue E.e p533
    // ExceptionTaken(integer ExceptionNumber)

    // bit tbit;
    // bits(32) tmp;
    // for i = 0 to 3
    //    R[i] = bits(32) UNKNOWN;
    // R[12] = bits(32) UNKNOWN;

    // bits(32) VectorTable = VTOR<31:7>:'0000000';
    const auto vector_table = cpua.template ReadSpecialRegister<SId::kVtor>() << 7U;

    // tmp = MemA[VectorTable+4*ExceptionNumber,4];
    TRY_ASSIGN(tmp, void,
               bus.template Read<u32>(cpua, vector_table + 4U * static_cast<u32>(exception_type)));

    // BranchTo(tmp AND 0xFFFFFFFE<31:0>);
    const auto exception_address = tmp & 0xFFFFFFFEU;
    LOG_TRACE(TLogger, "Branching to exception address = 0x%08X", exception_address);
    Pc::BranchTo(cpua, exception_address);

    const auto tbit = tmp & 0x1U; // tbit = tmp<0>;

    SetProcessorMode(cpua, ProcessorMode::kHandler);

    // APSR = bits(32) UNKNOWN; // Flags UNPREDICTABLE due to other activations

    auto ipsr = cpua.template ReadSpecialRegister<SId::kIpsr>();

    // IPSR SECTION
    // ------------
    const auto exception_number = static_cast<u32>(exception_type);
    ipsr &= ~static_cast<u32>(IpsrRegister::kExceptionNumberMsk);
    ipsr |= exception_number; // ExceptionNumber set in IPSR
    cpua.template WriteSpecialRegister<SId::kIpsr>(ipsr);

    // EPSR SECTION
    // ------------
    auto epsr = cpua.template ReadSpecialRegister<SId::kEpsr>();
    epsr &= ~static_cast<u32>(EpsrRegister::kTMsk);  // clear t bit
    epsr |= tbit << EpsrRegister::kTPos;             // T-bit set from vector
    epsr &= ~static_cast<u32>(EpsrRegister::kItMsk); // IT/ICI bits cleared
    cpua.template WriteSpecialRegister<SId::kEpsr>(epsr);

    auto sys_ctrl = cpua.template ReadSpecialRegister<SpecialRegisterId::kSysCtrl>();
    /* PRIMASK, FAULTMASK, BASEPRI unchanged on exception entry */
    sys_ctrl = cpua.template ReadSpecialRegister<SId::kSysCtrl>();

    // CONTROL.FPCA = '0'; // Mark Floating-point inactive

    // current Stack is Main, CONTROL.nPRIV unchanged
    sys_ctrl &= ~static_cast<u32>(SysCtrlRegister::kControlSpSelMsk);
    cpua.template WriteSpecialRegister<SId::kSysCtrl>(sys_ctrl);

    /* CONTROL.nPRIV unchanged */

    SetExceptionActive(cpua, exception_type);

    // SCS_UpdateStatusRegs(); // update SCS registers as appropriate
    // ClearExclusiveLocal(ProcessorID());
    // SetEventRegister();   // see WFE instruction for more details
    // InstructionSynchronizationBarrier('1111');
    return Ok();
  }

  static constexpr bool IsExceptionSynchronous() {
    // every exception is synchronous due to the nature of the emulator
    return true;
  }

  template <typename TBus>
  static Result<void> ExceptionReturn(TCpuAccessor &cpua, TBus &bus, u32 exc_return) {
    // Taken from: Armv7-M Architecture Reference Manual Issue E.e p541
    // ExceptionReturn(bits(28) EXC_RETURN)

    LOG_TRACE(TLogger, "[BEGIN] ExceptionReturn: exc_return = 0x%08X", exc_return);

    assert((Predicates::IsHandlerMode(cpua) == true) &&
           "ExceptionReturn should only be called in Handler mode");

    // if HaveFPExt() then
    if (false) {
      // if !IsOnes(EXC_RETURN<27:5>) then UNPREDICTABLE;
    } else {
      // if !IsOnes(EXC_RETURN<27:4>) then UNPREDICTABLE;
      if ((exc_return & 0x0FFFFFF0U) != 0x0FFFFFF0U) {
        return Err(StatusCode::kExecutorUnpredictable);
      }
    }

    // integer ReturningExceptionNumber = UInt(IPSR<8:0>);
    auto ret_exception_n =
        cpua.template ReadSpecialRegister<SId::kIpsr>() & IpsrRegister::kExceptionNumberMsk;

    // integer NestedActivation;
    // used for Handler => Thread check when value == 1

    // NestedActivation = ExceptionActiveBitCount(); // Number of active exceptions

    u32 frameptr{0U};

    // if ExceptionActive[ReturningExceptionNumber] == '0' then
    if (false) {
      // DeActivate(ReturningExceptionNumber);
      // UFSR.INVPC = '1';
      // LR = '1111':EXC_RETURN;
      // ExceptionTaken(UsageFault);  // returning from an inactive handler
      // return;
    } else {

      switch (exc_return & 0xFU) {
      case 0b0001U: { // return to Handler
        frameptr = cpua.template ReadSpecialRegister<SId::kSpMain>();
        SetProcessorMode(cpua, ProcessorMode::kHandler);

        auto sys_ctrl = cpua.template ReadSpecialRegister<SpecialRegisterId::kSysCtrl>();

        // CONTROL.SPSEL = '0';
        sys_ctrl &= ~static_cast<u32>(SysCtrlRegister::kControlSpSelMsk);

        cpua.template WriteSpecialRegister<SpecialRegisterId::kSysCtrl>(sys_ctrl);
        break;
      }

      case 0b1001U: // returning to Thread using Main stack
        // if NestedActivation != 1 && CCR.NONBASETHRDENA == '0' then
        if (false) {
          // DeActivate(ReturningExceptionNumber);
          // UFSR.INVPC = '1';
          // LR = '1111':EXC_RETURN;
          // ExceptionTaken(UsageFault); // return to Thread exception mismatch
          // return;
        } else {
          frameptr = cpua.template ReadSpecialRegister<SId::kSpMain>();
          SetProcessorMode(cpua, ProcessorMode::kThread);

          auto sys_ctrl = cpua.template ReadSpecialRegister<SpecialRegisterId::kSysCtrl>();

          // CONTROL.SPSEL = '0';
          sys_ctrl &= ~static_cast<u32>(SysCtrlRegister::kControlSpSelMsk);

          cpua.template WriteSpecialRegister<SpecialRegisterId::kSysCtrl>(sys_ctrl);
        }
        break;
      case 0b1101U: // returning to Thread using Process stack
        // if NestedActivation != 1 && CCR.NONBASETHRDENA == '0' then
        if (false) {
          // DeActivate(ReturningExceptionNumber);
          // UFSR.INVPC = '1';
          // LR = '1111':EXC_RETURN;
          // ExceptionTaken(UsageFault); // return to Thread exception mismatch
          // return;
        } else {
          frameptr = cpua.template ReadSpecialRegister<SId::kSpProcess>();
          SetProcessorMode(cpua, ProcessorMode::kThread);

          auto sys_ctrl = cpua.template ReadSpecialRegister<SpecialRegisterId::kSysCtrl>();

          // CONTROL.SPSEL = '1';
          sys_ctrl |= static_cast<u32>(SysCtrlRegister::kControlSpSelMsk);

          cpua.template WriteSpecialRegister<SpecialRegisterId::kSysCtrl>(sys_ctrl);
        }
        break;
      default:
        assert(false && "Not implemented");
        // DeActivate(ReturningExceptionNumber);
        // UFSR.INVPC = '1';
        // LR = '1111':EXC_RETURN;
        // ExceptionTaken(UsageFault); // illegal EXC_RETURN
        // return;
        return Err(StatusCode::kNotImplemented);
        break;
      }

      ClearExceptionActive(cpua, static_cast<ExceptionType>(ret_exception_n));

      // PopStack(frameptr, EXC_RETURN);
      TRY(void, PopStack(cpua, bus, frameptr, exc_return));

      const auto ipsr_8_0 =
          cpua.template ReadSpecialRegister<SId::kIpsr>() & IpsrRegister::kExceptionNumberMsk;

      const auto is_handler_mode = Predicates::IsHandlerMode(cpua);

      if (is_handler_mode && ipsr_8_0 == 0U) {
        // UFSR.INVPC = '1';
        // PushStack(UsageFault); // to negate PopStack()
        // LR = '1111':EXC_RETURN;
        // ExceptionTaken(UsageFault); // return IPSR is inconsistent
        // return;
        LOG_ERROR(TLogger, "Returning to Handler mode with IPSR inconsistent");
        return Err(StatusCode::kUsageFault);
      }

      const auto is_thread_mode = Predicates::IsThreadMode(cpua);
      if (is_thread_mode && ipsr_8_0 != 0U) {
        // UFSR.INVPC = '1';
        // PushStack(UsageFault); // to negate PopStack()
        // LR = '1111':EXC_RETURN;
        // ExceptionTaken(UsageFault); // return IPSR is inconsistent
        // return;
        LOG_ERROR(TLogger, "Returning to Thread mode with IPSR inconsistent");
        return Err(StatusCode::kUsageFault);
      }

      // ClearExclusiveLocal(ProcessorID());
      // SetEventRegister();  // see WFE instruction for more details
      // InstructionSynchronizationBarrier('1111');

      // if CurrentMode==Mode_Thread && NestedActivation == 0 && SCR.SLEEPONEXIT == '1' then
      //   SleepOnExit(); // IMPLEMENTATION DEFINED

#if IS_LOGLEVEL_TRACE_ENABLED == true
      LogImportantRegisters(cpua, "[END] ExceptionReturn",
                            static_cast<ExceptionType>(ret_exception_n));
#else
      static_cast<void>(ret_exception_n);
#endif

      return Ok();
    }
  }
  template <typename TBus>
  static Result<void> PopStack(TCpuAccessor &cpua, TBus &bus, u32 frameptr, u32 exc_return) {
    static_cast<void>(bus);

    // Taken from: Armv7-M Architecture Reference Manual Issue E.e p542
    // PopStack(bits(32) frameptr, bits(28) EXC_RETURN)

    /* only stack locations, not the load order, are architected */

    LOG_TRACE(TLogger, "Popping stack from 0x%08X", frameptr);
    u32 forcealign{0U};
    u32 framesize{0U};

    // if HaveFPExt() && EXC_RETURN<4> == '0' then
    if (false) {
      // framesize = 0x68;
      // forcealign = '1';
    } else {
      framesize = 0x20U;

      auto ccr = cpua.template ReadSpecialRegister<SId::kCcr>();
      forcealign = (ccr & CcrRegister::kStkAlignMsk) >> CcrRegister::kStkAlignPos;
    }

    // R[0] = MemA[frameptr,4];
    TRY_ASSIGN(r0, void,
               (bus.template ReadOrRaise<u32>(cpua, frameptr, BusExceptionType::kRaiseStkerr)));

    LOG_TRACE(TLogger, " R0 ADR = 0x%08X", frameptr);
    cpua.template WriteRegister<RegisterId::kR0>(r0);

    // R[1] = MemA[frameptr+0x4,4];
    TRY_ASSIGN(
        r1, void,
        (bus.template ReadOrRaise<u32>(cpua, frameptr + 0x4U, BusExceptionType::kRaiseStkerr)));
    cpua.template WriteRegister<RegisterId::kR1>(r1);

    // R[2] = MemA[frameptr+0x8,4];
    TRY_ASSIGN(
        r2, void,
        (bus.template ReadOrRaise<u32>(cpua, frameptr + 0x8U, BusExceptionType::kRaiseStkerr)));
    cpua.template WriteRegister<RegisterId::kR2>(r2);

    // R[3] = MemA[frameptr+0xC,4];
    TRY_ASSIGN(
        r3, void,
        (bus.template ReadOrRaise<u32>(cpua, frameptr + 0xCU, BusExceptionType::kRaiseStkerr)));
    cpua.template WriteRegister<RegisterId::kR3>(r3);

    // R[12] = MemA[frameptr+0x10,4];
    TRY_ASSIGN(
        r12, void,
        (bus.template ReadOrRaise<u32>(cpua, frameptr + 0x10U, BusExceptionType::kRaiseStkerr)));
    cpua.template WriteRegister<RegisterId::kR12>(r12);

    // LR = MemA[frameptr+0x14,4];
    TRY_ASSIGN(
        lr, void,
        (bus.template ReadOrRaise<u32>(cpua, frameptr + 0x14U, BusExceptionType::kRaiseStkerr)));
    cpua.template WriteRegister<RegisterId::kLr>(lr);

    // BranchTo(MemA[frameptr+0x18,4]); UNPREDICTABLE if the new PC not halfword aligned
    TRY_ASSIGN(
        return_adr, void,
        (bus.template ReadOrRaise<u32>(cpua, frameptr + 0x18U, BusExceptionType::kRaiseStkerr)));
    Pc::BranchTo(cpua, return_adr);

    // psr = MemA[frameptr+0x1C,4];
    TRY_ASSIGN(
        psr, void,
        (bus.template ReadOrRaise<u32>(cpua, frameptr + 0x1CU, BusExceptionType::kRaiseStkerr)));

    // Combine every LOG_TRACE into on single LOG_TRACE
    LOG_TRACE(TLogger,
              "Popped R0 = 0x%08X, "
              "R1 = 0x%08X, "
              "R2 = 0x%08X, "
              "R3 = 0x%08X, "
              "R12 = 0x%08X, "
              "LR = 0x%08X, "
              "ReturnAddress = 0x%08X, "
              "PSR = 0x%08X",
              r0, r1, r2, r3, r12, lr, return_adr, psr);

    // if HaveFPExt()
    if (false) {
      // if EXC_RETURN<4> == '0' then if FPCCR.LSPACT == '1' then
      if (false) {
        // FPCCR.LSPACT = '0'; // state in FP is still valid
        // else
        // CheckVFPEnabled();
        // for i = 0 to 15
        //   S[i] = MemA[frameptr+0x20+(4*i),4];
        // FPSCR = MemA[frameptr+0x60,4];
        // CONTROL.FPCA = NOT(EXC_RETURN<4>);
      }
    }

    // spmask = Zeros(29):(psr<9> AND forcealign):'00';
    const auto spmask = (Bm32::ExtractBits1R<9U, 9U>(psr) & forcealign) << 2U;

    switch (exc_return & 0xFU) {
    case 0b0001: { // returning to Handler  using Main stack
      // SP_main = (SP_main + framesize) OR spmask;
      auto sp_main = (cpua.template ReadSpecialRegister<SId::kSpMain>() + framesize) | spmask;
      LOG_TRACE(TLogger, "Returning to handler mode using main stack: SP_main = 0x%08X", sp_main);
      cpua.template WriteSpecialRegister<SId::kSpMain>(sp_main);
      break;
    }
    case 0b1001: { // returning to Thread using Main stack
      // SP_main = (SP_main + framesize) OR spmask;
      auto sp_main = (cpua.template ReadSpecialRegister<SId::kSpMain>() + framesize) | spmask;
      LOG_TRACE(TLogger, "Returning to thread mode using main stack: SP_main = 0x%08X", sp_main);
      cpua.template WriteSpecialRegister<SId::kSpMain>(sp_main);
      break;
    }

    case 0b1101: { // returning to Thread using Process stack
      auto sp_process = (cpua.template ReadSpecialRegister<SId::kSpProcess>() + framesize) | spmask;
      LOG_TRACE(TLogger, "Returning to thread mode using process stack: SP_process = 0x%08X",
                sp_process);
      cpua.template WriteSpecialRegister<SId::kSpProcess>(sp_process);
      break;
    }
    default: {
      return Err(StatusCode::kUnexpected);
    }
    }

    // APSR<31:27> = psr<31:27>; // valid APSR bits loaded from memory
    auto psr_31_27 = Bm32::ExtractBits1R<ApsrRegister::kNPos, ApsrRegister::kQPos>(psr);
    cpua.template WriteSpecialRegister<SId::kApsr>(psr_31_27 << ApsrRegister::kQPos);

    // if HaveDSPExt() then
    if (false) {
      // APSR<19:16> = psr<19:16>;
    }

    // IPSR<8:0> = psr<8:0>; // valid IPSR bits loaded from memory
    auto ipsr_8_0 = psr & IpsrRegister::kExceptionNumberMsk;

    cpua.template WriteSpecialRegister<SId::kIpsr>(ipsr_8_0);

    // EPSR<26:24,15:10> = psr<26:24,15:10>; // valid EPSR bits loaded from memory
    auto epsr_new = (Bm32::ExtractBits1R<EpsrRegister::kItBit1Pos, EpsrRegister::kTPos>(psr)
                     << EpsrRegister::kTPos) |
                    (Bm32::ExtractBits1R<EpsrRegister::kItBit7Pos, EpsrRegister::kItBit2Pos>(psr)
                     << EpsrRegister::kItBit2Pos);

    cpua.template WriteSpecialRegister<SId::kEpsr>(epsr_new);

    return Ok();
  }

  static void SetExceptionPending(TCpuAccessor &cpua, ExceptionType exception_type) {
#ifdef LOGLEVEL_ERROR
    switch (exception_type) {

    case ExceptionType::kHardFault:
      LOG_ERROR(TLogger, "Set HardFault exception pending");
      break;
    case ExceptionType::kMemoryManagementFault:
      LOG_ERROR(TLogger, "Set MemoryManagementFault exception pending");
      break;
    case ExceptionType::kBusFault:
      LOG_ERROR(TLogger, "Set BusFault exception pending");
      break;
    case ExceptionType::kUsageFault:
      LOG_ERROR(TLogger, "Set UsageFault exception pending");
      break;
    default:
      // do nothing
      break;
    }
#endif

    assert(static_cast<u32>(exception_type) >= 1U);
    assert(static_cast<u32>(exception_type) <= CountExceptions());

    auto &exception_states = cpua.GetExceptionStates();
    auto &selected_exception = exception_states.exception[static_cast<u32>(exception_type) - 1U];

    // cannot have multiple pending exceptions of the same type
    if (selected_exception.IsPending() == false) {
      exception_states.pending_exceptions += 1U;
    }

    selected_exception.SetPending();

    LOG_TRACE(TLogger, "SetExceptionPending: exception_type = %d, priority = %d",
              static_cast<uint32_t>(exception_type), selected_exception.GetPriority());
  }

  static void ClearExceptionPending(TCpuAccessor &cpua, ExceptionType exception_type) {
    assert(static_cast<u32>(exception_type) >= 1U);
    assert(static_cast<u32>(exception_type) <= CountExceptions());

    auto &exception_states = cpua.GetExceptionStates();
    auto &selected_exception = exception_states.exception[static_cast<u32>(exception_type) - 1U];

    // cannot clear a non-pending exception
    assert(selected_exception.IsPending() == true);

    selected_exception.ClearPending();
    exception_states.pending_exceptions -= 1U;

    LOG_TRACE(TLogger, "ClearExceptionPending: exception_type = %d, priority = %d",
              static_cast<uint32_t>(exception_type), selected_exception.GetPriority());
  }

  static void SetExceptionActive(TCpuAccessor &cpua, ExceptionType exception_type) {
    assert(static_cast<u32>(exception_type) >= 1U);
    assert(static_cast<u32>(exception_type) <= CountExceptions());

    auto &exception_states = cpua.GetExceptionStates();
    auto &selected_exception = exception_states.exception[static_cast<u32>(exception_type) - 1U];
    assert(selected_exception.IsActive() == false);
    selected_exception.SetActive();

    LOG_TRACE(TLogger, "SetExceptionActive: exception_type = %d, priority = %d",
              static_cast<uint32_t>(exception_type), selected_exception.GetPriority());
  }

  static void ClearExceptionActive(TCpuAccessor &cpua, ExceptionType exception_type) {
    assert(static_cast<u32>(exception_type) >= 1U);
    assert(static_cast<u32>(exception_type) <= CountExceptions());

    auto &exception_states = cpua.GetExceptionStates();
    auto &selected_exception = exception_states.exception[static_cast<u32>(exception_type) - 1U];
    assert(selected_exception.IsActive() == true);
    selected_exception.ClearActive();

    LOG_TRACE(TLogger, "ClearExceptionActive: exception_type = %d, priority = %d",
              static_cast<uint32_t>(exception_type), selected_exception.GetPriority());
  }

  template <typename ExcInstant,
            typename std::enable_if_t<ExcInstant::kInstant == ExecutionInstant::kPreFetch, int> = 0>
  static bool CanExceptionExecute(const ExceptionType &exception_type) {
    switch (exception_type) {
    case ExceptionType::kNMI:
    case ExceptionType::kSVCall:
    case ExceptionType::kSysTick:
    case ExceptionType::kPendSV:
      return true;
    default: {
      if (static_cast<u32>(exception_type) >= 16U) {
        return true;
      } else {
        return false;
      }
    }
    }
  }

  template <typename ExcInstant, typename std::enable_if_t<
                                     ExcInstant::kInstant == ExecutionInstant::kPostFetch, int> = 0>
  static bool CanExceptionExecute(const ExceptionType &exception_type) {
    switch (exception_type) {
    case ExceptionType::kMemoryManagementFault:
    case ExceptionType::kBusFault:   // After fetch bus fault and memory management fault can occur
    case ExceptionType::kUsageFault: // Invalid state
      return true;
    default: {
      return false;
    }
    }
  }

  template <
      typename ExcInstant,
      typename std::enable_if_t<ExcInstant::kInstant == ExecutionInstant::kPostExecution, int> = 0>
  static bool CanExceptionExecute(const ExceptionType &exception_type) {
    switch (exception_type) {
    case ExceptionType::kHardFault:
    case ExceptionType::kMemoryManagementFault:
    case ExceptionType::kBusFault:
    case ExceptionType::kUsageFault:
    case ExceptionType::kDebugMonitor:
      return true;
    default:
      return false;
    }
  }

  template <typename ExcInstant, typename TBus>
  static Result<bool> CheckExceptions(TCpuAccessor &cpua, TBus &bus,
                                      const ExceptionContext &context) {
    auto &exception_states = cpua.GetExceptionStates();
    auto &pending_exceptions = exception_states.pending_exceptions;

    // if no exceptions are pending, return
    if (pending_exceptions == 0U) {
      return Ok(false);
    }

    auto executing_exc_type =
        cpua.template ReadSpecialRegister<SId::kIpsr>() & IpsrRegister::kExceptionNumberMsk;

    i16 executing_exc_priority =
        kLowestExceptionPriority + 1U; // one lower than the lowest priority

    if (executing_exc_type != 0U) {
      assert(static_cast<u32>(executing_exc_type) >= 1U);
      assert(static_cast<u32>(executing_exc_type) <= CountExceptions());
      executing_exc_priority = exception_states.exception[executing_exc_type - 1U].GetPriority();
    }

    u32 preempt_exc_type = 0U;                                // 0 means no exception to preempt
    i16 preempt_exc_priority = kLowestExceptionPriority + 1U; // one lower than the lowest priority

    for (auto i = 0U; i < CountExceptions(); ++i) {

      auto &exception = exception_states.exception[i];

      if (exception.IsPending()) {
        if (exception.IsActive()) {
          // Exception is currently active ... skip
          continue;
        }

        if (exception.GetPriority() >= executing_exc_priority) {
          // Exception has lower or equal priority than the currently executing exception
          // ... skip
          continue;
        }

        if (exception.GetPriority() > preempt_exc_priority) {
          // Exception has lower priority than the last found exception to preempt ... skip

          // when multiple exceptions with the same priority are pending, the one with the lowest
          // exception number is taken
          continue;
        }
        preempt_exc_type = i + 1U;
        preempt_exc_priority = exception.GetPriority();
      }
    }

    // if no exceptions should preempt, return
    if (preempt_exc_type == 0U) {
      return Ok(false);
    }

    if (CanExceptionExecute<ExcInstant>(static_cast<ExceptionType>(preempt_exc_type)) == false) {
      // Will be processed later
      return Ok(false);
    }

    auto e_preemp_exc_type = static_cast<ExceptionType>(preempt_exc_type);

    ClearExceptionPending(cpua, e_preemp_exc_type);
    TRY(bool, ExceptionEntry<ExcInstant>(cpua, bus, e_preemp_exc_type, context));

    return Ok(true);
  };
};

} // namespace libmicroemu::internal