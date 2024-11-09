#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderUnaryInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &d, const u8 &n) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderUnaryInstr() = delete;

  /// \brief Destructor
  ~StrBuilderUnaryInstr() = delete;

  /// \brief Copy constructor for StrBuilderUnaryInstr.
  /// \param r_src the object to be copied
  StrBuilderUnaryInstr(const StrBuilderUnaryInstr &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderUnaryInstr.
  /// \param r_src the object to be copied
  StrBuilderUnaryInstr &operator=(const StrBuilderUnaryInstr &r_src) = delete;

  /// \brief Move constructor for StrBuilderUnaryInstr.
  /// \param r_src the object to be copied
  StrBuilderUnaryInstr(StrBuilderUnaryInstr &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderUnaryInstr.
  /// \param r_src the object to be copied
  StrBuilderUnaryInstr &operator=(StrBuilderUnaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu