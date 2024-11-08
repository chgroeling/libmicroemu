#pragma once
#include "libmicroemu/internal/trace/mnemonic_builder.h"
#include "libmicroemu/microemu.h"
#include "libmicroemu/types.h"
#include <cstddef>

namespace microemu {
namespace internal {

template <typename TProcessorStates, typename TItOps, typename TRegOps, typename TSpecRegOps>
class InstrToMnemonic : public IInstrToMnemonic {
public:
  InstrToMnemonic(const TProcessorStates &pstates, const Instr &instr)
      : pstates_(pstates), instr_(instr) {}
  ~InstrToMnemonic() = default;
  InstrToMnemonic(const InstrToMnemonic &r_src) = delete;
  InstrToMnemonic &operator=(const InstrToMnemonic &r_src) = delete;
  InstrToMnemonic(InstrToMnemonic &&r_src) = delete;
  InstrToMnemonic &operator=(InstrToMnemonic &&r_src) = delete;

  void Build(char *buf, std::size_t buf_len) const override {
    MnemonicBuilder::Build<TProcessorStates, TItOps, TRegOps, TSpecRegOps>(pstates_, instr_, buf,
                                                                           buf_len);
  }

private:
  const TProcessorStates &pstates_;
  const Instr &instr_;
};

} // namespace internal
} // namespace microemu
