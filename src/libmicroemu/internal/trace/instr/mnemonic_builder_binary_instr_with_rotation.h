#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderBinaryInstrWithRotation {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  template <typename TArg0, typename TArg1>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &d, const TArg1 &m,
                    const u8 &rotation) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m.Get()));

    if (rotation != 0U) {
      mctx.builder.AddString(", ROR #").AddUInt(rotation);
    }

    mctx.builder.Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderBinaryInstrWithRotation() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderBinaryInstrWithRotation() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderBinaryInstrWithRotation.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryInstrWithRotation(const MnemonicBuilderBinaryInstrWithRotation &r_src) =
      default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderBinaryInstrWithRotation.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryInstrWithRotation &
  operator=(const MnemonicBuilderBinaryInstrWithRotation &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderBinaryInstrWithRotation.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryInstrWithRotation(MnemonicBuilderBinaryInstrWithRotation &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderBinaryInstrWithRotation.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryInstrWithRotation &
  operator=(MnemonicBuilderBinaryInstrWithRotation &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu