
#pragma once

#include <stdint.h>

// System Control Block (SCB) Registers (Teil des System Control and ID Space)
struct SCB {
  volatile uint32_t CPUID; // 0xE000ED00: CPUID Base Register (RO)
  volatile uint32_t ICSR;  // 0xE000ED04: Interrupt Control and State Register (RW)
  volatile uint32_t VTOR;  // 0xE000ED08: Vector Table Offset Register (RW)
  volatile uint32_t AIRCR; // 0xE000ED0C: Application Interrupt and Reset Control Register (RW)
  volatile uint32_t SCR;   // 0xE000ED10: System Control Register (RW)
  volatile uint32_t CCR;   // 0xE000ED14: Configuration and Control Register (RW)
  volatile uint32_t SHPR1; // 0xE000ED18: System Handler Priority Register 1 (RW)
  volatile uint32_t SHPR2; // 0xE000ED1C: System Handler Priority Register 2 (RW)
  volatile uint32_t SHPR3; // 0xE000ED20: System Handler Priority Register 3 (RW)
  volatile uint32_t SHCSR; // 0xE000ED24: System Handler Control and State Register (RW)
  volatile uint32_t CFSR;  // 0xE000ED28: Configurable Fault Status Register (RW)
  volatile uint32_t HFSR;  // 0xE000ED2C: HardFault Status Register (RW)
  volatile uint32_t DFSR;  // 0xE000ED30: Debug Fault Status Register (RW)
  volatile uint32_t MMFAR; // 0xE000ED34: MemManage Fault Address Register (RW)
  volatile uint32_t BFAR;  // 0xE000ED38: BusFault Address Register (RW)
  volatile uint32_t AFSR;  // 0xE000ED3C: Auxiliary Fault Status Register (RW)
  volatile uint32_t CPACR; // 0xE000ED88: Coprocessor Access Control Register (RW)
};

// BusFault Status Register (BFSR) flags
static constexpr uint32_t kCfsrBfarValidFlag{(1U << 7U) << 8U};    // BFAR valid flag
static constexpr uint32_t kCfsrLspErrFlag{(1U << 5U) << 8U};       // Lazy state preservation error
static constexpr uint32_t kCfsrStkErrFlag{(1U << 4U) << 8U};       // Stacking error
static constexpr uint32_t kCfsrUnstkErrFlag{(1U << 3U) << 8U};     // Unstacking error
static constexpr uint32_t kCfsrImpreciseErrFlag{(1U << 2U) << 8U}; // Imprecise data bus error
static constexpr uint32_t kCfsrPreciseErrFlag{(1U << 1U) << 8U};   // Precise data bus error
static constexpr uint32_t kCfsrIBusErrFlag{(1U << 0U) << 8U};      // Instruction bus error

// MemManage Fault Status Register (MMFSR) flags
static constexpr uint32_t kCfsrMmarValidFlag{(1U << 7U)}; // MMAR valid flag
static constexpr uint32_t kCfsrMLspErrFlag{(1U << 5U)};   // Lazy state preservation error (MMFSR)
static constexpr uint32_t kCfsrMStkErrFlag{(1U << 4U)};   // Stacking error (MMFSR)
static constexpr uint32_t kCfsrMUnstkErrFlag{(1U << 3U)}; // Unstacking error (MMFSR)
static constexpr uint32_t kCfsrDAccViolFlag{(1U << 1U)};  // Data access violation
static constexpr uint32_t kCfsrIAccViolFlag{(1U << 0U)};  // Instruction access violation

// UsageFault Status Register (UFSR) flags
static constexpr uint32_t kCfsrDivByZeroFlag{(1U << 9U) << 16U}; // Divide by zero
static constexpr uint32_t kCfsrUnalignedFlag{(1U << 8U) << 16U}; // Unaligned access
static constexpr uint32_t kCfsrNoCpFlag{(1U << 3U) << 16U};      // No coprocessor
static constexpr uint32_t kCfsrInvPcFlag{(1U << 2U) << 16U};     // Invalid PC load
static constexpr uint32_t kCfsrInvStateFlag{(1U << 1U) << 16U};  // Invalid state
static constexpr uint32_t kCfsrUndefInstrFlag{(1U << 0U) << 16U};

// Configuration and Control Register (CCR) flags
static constexpr uint32_t kCcrNonBaseThreadEnableFlag{(1U << 0U)}; // Non-base thread enable
static constexpr uint32_t kCcrUserSettablePendSvFlag{(1U << 1U)};  // User-settable PendSV
static constexpr uint32_t kCcrUnalignTrapEnableFlag{(1U << 3U)};   // Unaligned access trap enable
static constexpr uint32_t kCcrDivByZeroTrapEnableFlag{(1U << 4U)}; // Divide by zero trap enable
static constexpr uint32_t kCcrMainStackAlignFlag{
    (1U << 9U)}; // Main stack alignment on exception entry
static constexpr uint32_t kCcrIgnoreLsbOnDataFlag{
    (1U << 10U)};                                       // Ignore LSB in data address comparisons
static constexpr uint32_t kCcrStkAlignFlag{(1U << 9U)}; // Stack alignment on exception entry
static constexpr uint32_t kCcrBranchPredictionEnableFlag{(1U << 18U)}; // Branch prediction enable
static constexpr uint32_t kCcrL1CacheEnableFlag{(1U << 19U)};          // L1 cache enable

// SysTick register map base pointer
static inline volatile SCB *GetSCBBase() {
  return reinterpret_cast<SCB *>(reinterpret_cast<unsigned *>(0xE000ED00u));
};