/**
 * @file
 * @brief This file contains the declaration of the Predicates class
 */
#pragma once

#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace libmicroemu {
namespace internal {

/**
 * @brief Provides predicates for the processor
 */
class Predicates {
public:
  /**
   * @brief Check if the processor is in thread mode
   * @param pstates the processor states
   * @return true if the processor is in thread mode, false otherwise
   */
  template <typename TProcessorStates, typename TSpecRegOps>
  static bool IsThreadMode(const TProcessorStates &pstates) {
    const auto sys_ctrl = TSpecRegOps::template ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);
    const auto exec_mode = sys_ctrl & SysCtrlRegister::kExecModeMsk;
    return exec_mode == SysCtrlRegister::kExecModeThread;
  }

  /**
   * @brief Check if the processor is in handler mode
   * @param pstates the processor states
   * @return true if the processor is in handler mode, false otherwise
   */
  template <typename TProcessorStates, typename TSpecRegOps>
  static bool IsHandlerMode(const TProcessorStates &pstates) {
    const auto sys_ctrl = TSpecRegOps::template ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);
    const auto exec_mode = sys_ctrl & SysCtrlRegister::kExecModeMsk;
    return exec_mode == SysCtrlRegister::kExecModeHandler;
  }

  /**
   * @brief Check if the processor is using the main stack
   * @param pstates the processor states
   * @return true if the processor is using the main stack, false otherwise
   */
  template <typename TProcessorStates, typename TSpecRegOps>
  static bool IsMainStack(const TProcessorStates &pstates) {
    using SId = SpecialRegisterId;
    auto sys_ctrl = TSpecRegOps::template ReadRegister<SId::kSysCtrl>(pstates);
    auto spsel = sys_ctrl & SysCtrlRegister::kControlSpSelMsk;
    return spsel == 0U;
  }

  /**
   * @brief Check if the processor is using the process stack
   * @param pstates the processor states
   * @return true if the processor is using the process stack, false otherwise
   */
  template <typename TProcessorStates, typename TSpecRegOps>
  static bool IsProcessStack(const TProcessorStates &pstates) {
    using SId = SpecialRegisterId;
    auto sys_ctrl = TSpecRegOps::template ReadRegister<SId::kSysCtrl>(pstates);
    auto spsel = sys_ctrl & SysCtrlRegister::kControlSpSelMsk;
    return spsel != 0U;
  }

  /**
   * @brief Check if the processor is in privileged mode
   * @param pstates the processor states
   * @return true if the processor is in privileged mode, false otherwise
   */
  template <typename TProcessorStates, typename TSpecRegOps>
  static bool IsCurrentModePrivileged(TProcessorStates &pstates) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.512
    auto sys_ctrl = TSpecRegOps::template ReadRegister<SpecialRegisterId::kSysCtrl>(pstates);
    const auto exec_mode = sys_ctrl & SysCtrlRegister::kExecModeMsk;
    const auto is_handler_mode = exec_mode == SysCtrlRegister::kExecModeHandler;
    const auto is_privileged = (sys_ctrl & SysCtrlRegister::kControlNPrivMsk) == 0U;

    return (is_handler_mode || is_privileged);
  }

private:
  Predicates() = delete;
  ~Predicates() = delete;
  Predicates(const Predicates &r_src) = delete;
  Predicates &operator=(const Predicates &r_src) = delete;
  Predicates(Predicates &&r_src) = delete;
  Predicates &operator=(Predicates &&r_src) = delete;
};
} // namespace internal
} // namespace libmicroemu