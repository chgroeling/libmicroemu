#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderBinaryNullInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &m, const u32 &imm32) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)))
        .AddString(", #")
        .AddUInt(imm32)
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderBinaryNullInstrWithImm() = delete;

  /// \brief Destructor
  ~StrBuilderBinaryNullInstrWithImm() = delete;

  /// \brief Copy constructor for StrBuilderBinaryNullInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryNullInstrWithImm(const StrBuilderBinaryNullInstrWithImm &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderBinaryNullInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryNullInstrWithImm &
  operator=(const StrBuilderBinaryNullInstrWithImm &r_src) = delete;

  /// \brief Move constructor for StrBuilderBinaryNullInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryNullInstrWithImm(StrBuilderBinaryNullInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderBinaryNullInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryNullInstrWithImm &operator=(StrBuilderBinaryNullInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu