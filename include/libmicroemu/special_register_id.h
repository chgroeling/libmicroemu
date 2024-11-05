/**
 * @file special_register_id.h
 * @brief This file contains the declaration of the SpecialRegisterId enumeration and related
 * functions.
 */
#pragma once

#include "libmicroemu/types.h"

namespace microemu {

/**
 * @brief Enumeration of special register IDs.
 *
 * This enumeration represents the IDs of special registers.
 * Special registers are used for storing specific system information or control flags.
 */
enum class SpecialRegisterId : u8 {

  //---------------------
  // Persistent registers
  // These registers are part of the processor and have a state that persists.
  //---------------------

  kSysCtrl = 0u, ///< System Control Register, SYSCTRL
  kApsr,         ///< Application Program Status Register, APSR
  kIstate,       ///< If-Then Execution state bits
  kIpsr,         ///< Interrupt Program Status Register, IPSR
  kVtor,         ///< Vector Table Offset Register, VTOR
  kCcr,          ///< Configuration and Control Register, CCR
  kCfsr,         ///< Configurable Fault Status Register, CFSR
  kBfar,         ///< Bus Fault Address Register, BFAR
  kSpMain,       ///< Main Stack Pointer, MSP
  kSpProcess,    ///< Process Stack Pointer, PSP
  kSysTickCsr,   ///< SYSTICK_CSR - SysTick Control and Status Register
  kSysTickRvr,   ///< SYSTICK_RVR -  SysTick Reload Value Register
  kSysTickCvr,   ///< SYST_CVR - SysTick Current Value Register
  kSysTickCalib, ///< SYST_CALIB - SysTick Calibration Value Register

  //---------------------
  // Runtime registers
  // These registers are not persitent, but are built during runtime.
  //---------------------
  kEpsr,    ///< Execution Program Status Register, EPSR
  kXpsr,    ///< Program status register, XPSR
  kControl, ///< Control register, CONTROL
};

constexpr auto kLastPersistentSpecialRegister = SpecialRegisterId::kSysTickCalib;
constexpr auto kFirstRuntimeSpecialRegister = SpecialRegisterId::kEpsr;
constexpr auto kLastRuntimeSpecialRegister = SpecialRegisterId::kControl;
constexpr auto kLastSpecialRegister = kLastRuntimeSpecialRegister;

/**
 * @brief Returns the number of persistent special registers.
 *
 * This function returns the total number of persistent special registers,
 * i.e., registers that are part of the processor and have a state that persists.
 *
 * @return The number of persistent special registers.
 */
static constexpr u32 CountPersistentSpecialRegisters() noexcept {
  return static_cast<u32>(kLastPersistentSpecialRegister) + 1u;
};

/**
 * @brief Returns the number of special registers.
 *
 * This function returns the total number of special registers.
 *
 * @return The number of special registers.
 */
static constexpr u32 CountSpecialRegisters() noexcept {
  return static_cast<u32>(kLastSpecialRegister) + 1u;
};

} // namespace microemu