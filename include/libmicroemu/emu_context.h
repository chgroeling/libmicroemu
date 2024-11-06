#pragma once

#include "libmicroemu/processor_states.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"
#include <cstddef> // Add header for size_t

namespace microemu {

struct OpCode {
  u16 low;
  u16 high;
  bool is_32bit;
};

class IInstrToMnemonic {
public:
  // TODO: Use std::size_t instead of size_t
  virtual void Build(char *buf, size_t buf_len) const = 0;
};

class IRegAccess {
public:
  // TODO: Add methods for raw types
  virtual const char *GetRegisterName(const uint8_t &reg_id) const = 0;

  virtual u32 ReadRegister(const uint8_t &reg_id) const = 0;
  virtual void WriteRegister(const uint8_t &reg_id, u32 value) = 0;
};

class ISpecialRegAccess {
public:
  virtual const char *GetRegisterName(const uint8_t &reg_id) const = 0;
  virtual const char *GetRegisterName(const SpecialRegisterId &reg_id) const = 0;

  virtual u32 ReadRegister(const SpecialRegisterId &reg_id) const = 0;
  virtual void WriteRegister(const SpecialRegisterId &reg_id, u32 value) = 0;
};

class EmuContext {
public:
  EmuContext(const me_adr_t &pc, const OpCode &op_code, const IInstrToMnemonic &instr_decoder,
             IRegAccess &reg_access, ISpecialRegAccess &spec_reg_access) noexcept

      : pc_(pc), op_code_(op_code), instr_decoder_(instr_decoder), reg_access_(reg_access),
        spec_reg_access_(spec_reg_access) {}

  inline me_adr_t GetPc() const noexcept { return pc_; }
  inline IRegAccess &GetRegisterAccess() noexcept { return reg_access_; }
  inline ISpecialRegAccess &GetSpecialRegisterAccess() noexcept { return spec_reg_access_; }
  inline const OpCode &GetOpCode() const noexcept { return op_code_; }

  /// @brief  Builds the mnemonic of the instruction
  /// This takes the current instruction and decodes it into a human readable string.
  /// Attention this is a resource intensive operation.
  /// @param buf
  /// @param buf_len
  inline void BuildMnemonic(char *buf, size_t buf_len) const noexcept {
    instr_decoder_.Build(buf, buf_len);
  }

private:
  const me_adr_t &pc_;
  const OpCode &op_code_;
  const IInstrToMnemonic &instr_decoder_;
  IRegAccess &reg_access_;

  //  TODO: Rename to accessor
  ISpecialRegAccess &spec_reg_access_;
};

} // namespace microemu