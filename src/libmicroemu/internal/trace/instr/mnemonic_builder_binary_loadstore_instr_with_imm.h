#pragma once

#include "libmicroemu/internal/trace/instr/generic/relative_adr_builder.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderBinaryLoadStoreInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TTgt, typename TArg0>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TTgt &rt, const TArg0 &rn,
                    const u32 &imm32) {
    static_cast<void>(bflags);
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
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
  MnemonicBuilderBinaryLoadStoreInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderBinaryLoadStoreInstrWithImm() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderBinaryLoadStoreInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryLoadStoreInstrWithImm(
      const MnemonicBuilderBinaryLoadStoreInstrWithImm &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderBinaryLoadStoreInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryLoadStoreInstrWithImm &
  operator=(const MnemonicBuilderBinaryLoadStoreInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderBinaryLoadStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryLoadStoreInstrWithImm(MnemonicBuilderBinaryLoadStoreInstrWithImm &&r_src) =
      delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderBinaryLoadStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryLoadStoreInstrWithImm &
  operator=(MnemonicBuilderBinaryLoadStoreInstrWithImm &&r_src) = delete;
};

}