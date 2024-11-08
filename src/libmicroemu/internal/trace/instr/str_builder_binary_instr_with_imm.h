#pragma once

#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderBinaryInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &d, const u8 &n, const u32 &imm32) {

    const bool is_setflags = (iflags & InstrFlags::kSetFlags) != 0u;
    const bool is_reduced_rd =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd)) != 0;

    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ');

    if ((d != n) || (!is_reduced_rd)) {
      mctx.builder.AddString(Reg::GetRegisterName(d).data()).AddString(", ");
    }
    mctx.builder.AddString(Reg::GetRegisterName(n).data())
        .AddString(", #")
        .AddUInt(imm32)
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderBinaryInstrWithImm() = delete;

  /// \brief Destructor
  ~StrBuilderBinaryInstrWithImm() = delete;

  /// \brief Copy constructor for StrBuilderBinaryInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithImm(const StrBuilderBinaryInstrWithImm &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderBinaryInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithImm &operator=(const StrBuilderBinaryInstrWithImm &r_src) = delete;

  /// \brief Move constructor for StrBuilderBinaryInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithImm(StrBuilderBinaryInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for ByteIterator.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithImm &operator=(StrBuilderBinaryInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu