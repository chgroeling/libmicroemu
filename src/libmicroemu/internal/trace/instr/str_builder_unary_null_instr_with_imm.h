#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderUnaryNullInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u32 &imm) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddString(" #")
        .AddUInt(imm)
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderUnaryNullInstrWithImm() = delete;

  /// \brief Destructor
  ~StrBuilderUnaryNullInstrWithImm() = delete;

  /// \brief Copy constructor for StrBuilderUnaryNullInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderUnaryNullInstrWithImm(const StrBuilderUnaryNullInstrWithImm &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderUnaryNullInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderUnaryNullInstrWithImm &operator=(const StrBuilderUnaryNullInstrWithImm &r_src) = delete;

  /// \brief Move constructor for StrBuilderUnaryNullInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderUnaryNullInstrWithImm(StrBuilderUnaryNullInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderUnaryNullInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderUnaryNullInstrWithImm &operator=(StrBuilderUnaryNullInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu