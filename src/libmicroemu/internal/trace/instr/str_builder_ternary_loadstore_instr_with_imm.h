#pragma once

#include "libmicroemu/internal/trace/instr/generic/relative_adr_builder.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderTernaryLoadStoreInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0, typename TArg1, typename TArg2>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &n, const TArg1 &t, const TArg2 &d,
                    const u32 &imm32) {
    static_cast<void>(bflags);
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(t.Get()))
        .AddString(", ");

    RelativeAdrBuilder::Build<decltype(mctx.builder), Reg>(mctx.builder, is_add, is_index, is_wback,
                                                           n, imm32);
    mctx.builder.Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderTernaryLoadStoreInstrWithImm() = delete;

  /// \brief Destructor
  ~StrBuilderTernaryLoadStoreInstrWithImm() = delete;

  /// \brief Copy constructor for StrBuilderTernaryLoadStoreInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderTernaryLoadStoreInstrWithImm(const StrBuilderTernaryLoadStoreInstrWithImm &r_src) =
      default;

  /// \brief Copy assignment operator for StrBuilderTernaryLoadStoreInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderTernaryLoadStoreInstrWithImm &
  operator=(const StrBuilderTernaryLoadStoreInstrWithImm &r_src) = delete;

  /// \brief Move constructor for StrBuilderTernaryLoadStoreInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderTernaryLoadStoreInstrWithImm(StrBuilderTernaryLoadStoreInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderTernaryLoadStoreInstrWithImm.
  /// \param r_src the object to be copied
  StrBuilderTernaryLoadStoreInstrWithImm &
  operator=(StrBuilderTernaryLoadStoreInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu