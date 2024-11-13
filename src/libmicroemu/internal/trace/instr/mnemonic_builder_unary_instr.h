#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class MnemonicBuilderUnaryInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  template <typename TArg0, typename TArg1>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &d, const TArg1 &n) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(n.Get()))
        .Terminate();
  }

private:
  /// \brief Constructor
  MnemonicBuilderUnaryInstr() = delete;

  /// \brief Destructor
  ~MnemonicBuilderUnaryInstr() = delete;

  /// \brief Copy constructor for MnemonicBuilderUnaryInstr.
  /// \param r_src the object to be copied
  MnemonicBuilderUnaryInstr(const MnemonicBuilderUnaryInstr &r_src) = default;

  /// \brief Copy assignment operator for MnemonicBuilderUnaryInstr.
  /// \param r_src the object to be copied
  MnemonicBuilderUnaryInstr &operator=(const MnemonicBuilderUnaryInstr &r_src) = delete;

  /// \brief Move constructor for MnemonicBuilderUnaryInstr.
  /// \param r_src the object to be copied
  MnemonicBuilderUnaryInstr(MnemonicBuilderUnaryInstr &&r_src) = delete;

  /// \brief Move assignment operator for MnemonicBuilderUnaryInstr.
  /// \param r_src the object to be copied
  MnemonicBuilderUnaryInstr &operator=(MnemonicBuilderUnaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu