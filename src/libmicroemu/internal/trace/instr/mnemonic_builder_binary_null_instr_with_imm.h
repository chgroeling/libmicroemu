#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderBinaryNullInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &m, const u32 &imm32) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(m.Get()))
        .AddString(", #")
        .AddUInt(imm32)
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderBinaryNullInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderBinaryNullInstrWithImm() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderBinaryNullInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryNullInstrWithImm(const MnemonicBuilderBinaryNullInstrWithImm &r_src) =
      default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderBinaryNullInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryNullInstrWithImm &
  operator=(const MnemonicBuilderBinaryNullInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderBinaryNullInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryNullInstrWithImm(MnemonicBuilderBinaryNullInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderBinaryNullInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryNullInstrWithImm &
  operator=(MnemonicBuilderBinaryNullInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu