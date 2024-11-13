#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class MnemonicBuilderUnaryBranchInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &n) {
    static_cast<void>(bflags);
    static_cast<void>(iflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(n.Get()))
        .Terminate();
  }

private:
  /// \brief Constructor
  MnemonicBuilderUnaryBranchInstr() = delete;

  /// \brief Destructor
  ~MnemonicBuilderUnaryBranchInstr() = delete;

  /// \brief Copy constructor for MnemonicBuilderUnaryBranchInstr.
  /// \param r_src the object to be copied
  MnemonicBuilderUnaryBranchInstr(const MnemonicBuilderUnaryBranchInstr &r_src) = default;

  /// \brief Copy assignment operator for MnemonicBuilderUnaryBranchInstr.
  /// \param r_src the object to be copied
  MnemonicBuilderUnaryBranchInstr &operator=(const MnemonicBuilderUnaryBranchInstr &r_src) = delete;

  /// \brief Move constructor for MnemonicBuilderUnaryBranchInstr.
  /// \param r_src the object to be copied
  MnemonicBuilderUnaryBranchInstr(MnemonicBuilderUnaryBranchInstr &&r_src) = delete;

  /// \brief Move assignment operator for MnemonicBuilderUnaryBranchInstr.
  /// \param r_src the object to be copied
  MnemonicBuilderUnaryBranchInstr &operator=(MnemonicBuilderUnaryBranchInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu