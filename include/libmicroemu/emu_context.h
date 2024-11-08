#pragma once

#include "libmicroemu/processor_states.h"
#include "libmicroemu/register_id.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <string_view>

namespace microemu {

struct OpCode {
  u16 low;
  u16 high;
  bool is_32bit;
};

class IInstrToMnemonic {
public:
  virtual void Build(char *buf, std::size_t buf_len) const = 0;
};

/** @brief Interface for accessing general-purpose registers.
 *
 */
class IRegAccessor {
public:
  /**
   * @brief Gets the name of the specified register.
   * @param id The ID of the register.
   * @return The name of the register.
   */
  virtual std::string_view GetRegisterName(const RegisterId &reg_id) const = 0;

  /**
   * @brief Reads the value of the specified register.
   * @param reg_id The ID of the register to be read.
   * @return The value of the register.
   */
  virtual u32 ReadRegister(const RegisterId &reg_id) const = 0;

  /**
   * @brief Writes the value to the specified register.
   * @param reg_id The ID of the register to be written.
   * @param value The value to be written to the register.
   */
  virtual void WriteRegister(const RegisterId &reg_id, u32 value) = 0;
};

/** @brief Interface for accessing special registers.
 *
 */
class ISpecialRegAccessor {
public:
  /**
   * @brief Gets the name of the specified special register.
   *
   * @param reg_id The ID of the special register.
   * @return The name of the special register.
   */
  virtual std::string_view GetRegisterName(const SpecialRegisterId &reg_id) const = 0;

  /**
   * @brief Reads the value of the specified special register.
   *
   * @param reg_id The ID of the special register to be read.
   * @return The value of the special register.
   */
  virtual u32 ReadRegister(const SpecialRegisterId &reg_id) const = 0;

  /**
   * @brief Writes the value to the specified special register.
   *
   * @param reg_id The ID of the special register to be written.
   * @param value The value to be written to the special register.
   */
  virtual void WriteRegister(const SpecialRegisterId &reg_id, u32 value) = 0;
};

class EmuContext {
public:
  EmuContext(const me_adr_t &pc, const OpCode &op_code, const IInstrToMnemonic &instr_decoder,
             IRegAccessor &reg_access, ISpecialRegAccessor &spec_reg_access) noexcept

      : pc_(pc), op_code_(op_code), instr_decoder_(instr_decoder), reg_access_(reg_access),
        spec_reg_access_(spec_reg_access) {}

  /// @brief Gets the program counter of the current instruction.
  inline me_adr_t GetPc() const noexcept { return pc_; }

  /// @brief Gets the register accessor.
  inline IRegAccessor &GetRegisterAccessor() noexcept { return reg_access_; }

  /// @brief Gets the special register accessor.
  inline ISpecialRegAccessor &GetSpecialRegisterAccessor() noexcept { return spec_reg_access_; }

  /// @brief Gets the opcode of the current instruction.
  inline const OpCode &GetOpCode() const noexcept { return op_code_; }

  /// @brief  Builds the mnemonic of the current instruction
  /// This takes the current instruction and decodes it into a human readable string.
  /// Attention this is a resource intensive operation.
  /// @param buf The buffer to store the mnemonic.
  /// @param buf_len The length of the buffer.
  inline void BuildMnemonic(char *buf, std::size_t buf_len) const noexcept {
    instr_decoder_.Build(buf, buf_len);
  }

private:
  const me_adr_t &pc_;
  const OpCode &op_code_;
  const IInstrToMnemonic &instr_decoder_;

  IRegAccessor &reg_access_;
  ISpecialRegAccessor &spec_reg_access_;
};

} // namespace microemu