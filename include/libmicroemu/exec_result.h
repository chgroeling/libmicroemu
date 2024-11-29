/**
 * @file
 * @brief Contains the ExecResult class which is returned by the Executor::Exec method.
 */

#pragma once

#include "libmicroemu/status_code.h"
#include "libmicroemu/types.h"
#include <string_view>

namespace libmicroemu {

/**
 * @brief Returned by the Executor::Exec method. Contains the status code of the execution and the
 * program exit code.
 */
class ExecResult {
public:
  /**
   * @brief Constructs a new ExecResult object.
   */
  ExecResult(StatusCode status_code, int program_exit_code) noexcept
      : status_code(status_code), program_exit_code(program_exit_code) {}

  /**
   * @brief Move Constructor
   */
  ExecResult(ExecResult &&r) noexcept = default;

  /**
   * @brief Move Assignment
   */
  ExecResult &operator=(ExecResult &&r) = delete;

  /**
   * @brief Copy Constructor
   */
  ExecResult(const ExecResult &r) noexcept = default;

  /**
   * @brief Copy Assignment
   */
  ExecResult &operator=(const ExecResult &r) = delete;

  /**
   * @brief Checks if the execution result is a success.
   * @return true if the execution result is a success, false otherwise.
   */
  inline bool IsOk() const noexcept { return status_code == StatusCode::kSuccess; };

  /**
   * @brief Checks if the execution result is an error.
   * @return true if the execution result is an error, false otherwise.
   */
  inline bool IsErr() const noexcept { return status_code != StatusCode::kSuccess; };

  /**
   * @brief Checks if the maximum number of instructions has been reached.
   * @return true if the maximum number of instructions has been reached, false otherwise.
   */
  inline bool IsMaxInstructionsReached() const noexcept {
    return status_code == StatusCode::kMaxInstructionsReached;
  };

  /**
   * @brief Converts the contained status code to a string.
   * @return The string representation of the status code.
   */
  std::string_view ToString() const noexcept { return StatusCodeToString(status_code); }

  /**
   * @brief Gets the status code of the execution result.
   * @return The status code of the execution result.
   * @see StatusCode
   */
  StatusCode GetStatusCode() const noexcept { return status_code; }

  /**
   * @brief Gets the program exit code.
   * @return The program exit code.
   */
  u32 GetProgramExitCode() const noexcept { return program_exit_code; }

private:
  const StatusCode status_code;
  const int program_exit_code;
};

} // namespace libmicroemu