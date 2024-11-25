#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderUnaryNullInstrWithImm {
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
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddString(" #")
        .AddUInt(imm)
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderUnaryNullInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderUnaryNullInstrWithImm() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderUnaryNullInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderUnaryNullInstrWithImm(const MnemonicBuilderUnaryNullInstrWithImm &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderUnaryNullInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderUnaryNullInstrWithImm &
  operator=(const MnemonicBuilderUnaryNullInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderUnaryNullInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderUnaryNullInstrWithImm(MnemonicBuilderUnaryNullInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderUnaryNullInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderUnaryNullInstrWithImm &
  operator=(MnemonicBuilderUnaryNullInstrWithImm &&r_src) = delete;
};

} // namespace libmicroemu::internal