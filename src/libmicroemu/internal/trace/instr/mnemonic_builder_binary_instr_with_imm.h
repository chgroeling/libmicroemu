#pragma once

#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"

#include "libmicroemu/internal/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderBinaryInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TDest, typename TArg0>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TDest &rd, const TArg0 &rn,
                    const u32 &imm32) {

    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    const bool is_reduced_rd =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd)) != 0U;

    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ');

    if ((rd != rn) || (!is_reduced_rd)) {
      mctx.builder.AddString(Reg::GetRegisterName(rd.Get())).AddString(", ");
    }
    mctx.builder.AddString(Reg::GetRegisterName(rn.Get()))
        .AddString(", #")
        .AddUInt(imm32)
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderBinaryInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderBinaryInstrWithImm() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderBinaryInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryInstrWithImm(const MnemonicBuilderBinaryInstrWithImm &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderBinaryInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryInstrWithImm &
  operator=(const MnemonicBuilderBinaryInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderBinaryInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryInstrWithImm(MnemonicBuilderBinaryInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  ByteIterator.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryInstrWithImm &operator=(MnemonicBuilderBinaryInstrWithImm &&r_src) = delete;
};

} // namespace libmicroemu::internal