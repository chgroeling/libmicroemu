#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderUnaryBranchInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u32 &imm) {
    static_cast<void>(bflags);
    static_cast<void>(iflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates).data())
        .AddString(" #")
        .AddInt(static_cast<int>(imm))
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderUnaryBranchInstrWithImm() = delete;

  /// \brief Destructor
  ~StrBuilderUnaryBranchInstrWithImm() = delete;

  /// \brief Copy constructor for StrBuilderUnaryBranchInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderUnaryBranchInstrWithImm(const StrBuilderUnaryBranchInstrWithImm &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderUnaryBranchInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderUnaryBranchInstrWithImm &
  operator=(const StrBuilderUnaryBranchInstrWithImm &r_src) = delete;

  /// \brief Move constructor for StrBuilderUnaryBranchInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderUnaryBranchInstrWithImm(StrBuilderUnaryBranchInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderUnaryBranchInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderUnaryBranchInstrWithImm &operator=(StrBuilderUnaryBranchInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu