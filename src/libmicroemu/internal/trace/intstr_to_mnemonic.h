#pragma once
#include "libmicroemu/internal/trace/mnemonic_builder.h"
#include "libmicroemu/machine.h"
#include "libmicroemu/types.h"
#include <cstddef>

namespace libmicroemu {
namespace internal {

template <typename TCpuAccessor, typename TItOps> class InstrToMnemonic : public IInstrToMnemonic {
public:
  using RegOps = typename TCpuAccessor::Reg;
  using SRegOps = typename TCpuAccessor::SReg;
  InstrToMnemonic(const TCpuAccessor &cpua, const Instr &instr) : cpua_(cpua), instr_(instr) {}
  ~InstrToMnemonic() = default;
  InstrToMnemonic(const InstrToMnemonic &r_src) = delete;
  InstrToMnemonic &operator=(const InstrToMnemonic &r_src) = delete;
  InstrToMnemonic(InstrToMnemonic &&r_src) = delete;
  InstrToMnemonic &operator=(InstrToMnemonic &&r_src) = delete;

  void Build(char *buf, std::size_t buf_len) const override {
    MnemonicBuilder::Build<TCpuAccessor, TItOps, RegOps, SRegOps>(cpua_, instr_, buf, buf_len);
  }

private:
  const TCpuAccessor &cpua_;
  const Instr &instr_;
};

} // namespace internal
} // namespace libmicroemu
