#pragma once

#include "libmicroemu/internal/trace/instr/generic/relative_adr_builder.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderBinaryLoadStoreInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &n, const u8 &t, const u32 &imm32) {
    static_cast<void>(bflags);
    const bool is_wback = (iflags & InstrFlags::kWBack) != 0u;
    const bool is_index = (iflags & InstrFlags::kIndex) != 0u;
    const bool is_add = (iflags & kAdd) != 0u;

    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(static_cast<RegisterId>(t))).data())
        .AddString(", ");

    RelativeAdrBuilder::Build<decltype(mctx.builder), Reg>(mctx.builder, is_add, is_index, is_wback,
                                                           n, imm32);
    mctx.builder.Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderBinaryLoadStoreInstrWithImm() = delete;

  /// \brief Destructor
  ~StrBuilderBinaryLoadStoreInstrWithImm() = delete;

  /// \brief Copy constructor for StrBuilderBinaryLoadStoreInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryLoadStoreInstrWithImm(const StrBuilderBinaryLoadStoreInstrWithImm &r_src) =
      default;

  /// \brief Copy assignment operator for StrBuilderBinaryLoadStoreInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryLoadStoreInstrWithImm &
  operator=(const StrBuilderBinaryLoadStoreInstrWithImm &r_src) = delete;

  /// \brief Move constructor for StrBuilderBinaryLoadStoreInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryLoadStoreInstrWithImm(StrBuilderBinaryLoadStoreInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderBinaryLoadStoreInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderBinaryLoadStoreInstrWithImm &
  operator=(StrBuilderBinaryLoadStoreInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu