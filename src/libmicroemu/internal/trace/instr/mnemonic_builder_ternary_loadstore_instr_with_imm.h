#pragma once

#include "libmicroemu/internal/trace/instr/generic/relative_adr_builder.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderTernaryLoadStoreInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TDest, typename TTgt, typename TArg0>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TDest &rd, const TTgt &rt, const TArg0 &rn,
                    const u32 &imm32) {
    static_cast<void>(bflags);
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(rd.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(rt.Get()))
        .AddString(", ");

    RelativeAdrBuilder::Build<decltype(mctx.builder), Reg>(mctx.builder, is_add, is_index, is_wback,
                                                           rn, imm32);
    mctx.builder.Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderTernaryLoadStoreInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderTernaryLoadStoreInstrWithImm() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderTernaryLoadStoreInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderTernaryLoadStoreInstrWithImm(
      const MnemonicBuilderTernaryLoadStoreInstrWithImm &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderTernaryLoadStoreInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderTernaryLoadStoreInstrWithImm &
  operator=(const MnemonicBuilderTernaryLoadStoreInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderTernaryLoadStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderTernaryLoadStoreInstrWithImm(MnemonicBuilderTernaryLoadStoreInstrWithImm &&r_src) =
      delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderTernaryLoadStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderTernaryLoadStoreInstrWithImm &
  operator=(MnemonicBuilderTernaryLoadStoreInstrWithImm &&r_src) = delete;
};

} // namespace libmicroemu::internal