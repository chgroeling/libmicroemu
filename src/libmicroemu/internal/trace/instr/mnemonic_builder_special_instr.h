#pragma once

#include "libmicroemu/internal/trace/instr/generic/relative_adr_builder.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderSpecialInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0, typename TArg1>
  static void BuildTbbH(const char *instr_spec, TContext &mctx,
                        const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                        const TArg0 &n, const TArg1 &m) {
    static_cast<void>(instr_spec);
    static_cast<void>(bflags);
    const bool is_tbh = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kTbh)) != 0U;

    if (is_tbh) {
      mctx.builder.AddString("TBH")
          .AddString(It::GetConditionAsStr(mctx.cpua))
          .AddString(" [")
          .AddString(Reg::GetRegisterName(n.Get()))
          .AddString(", ")
          .AddString(Reg::GetRegisterName(m.Get()))
          .AddString(", LSL #1")
          .AddChar(']');
    } else {
      mctx.builder.AddString("TBB")
          .AddString(It::GetConditionAsStr(mctx.cpua))
          .AddString(" [")
          .AddString(Reg::GetRegisterName(n.Get()))
          .AddString(", ")
          .AddString(Reg::GetRegisterName(m.Get()))
          .AddChar(']');
    }

    mctx.builder.Terminate();
  }

  template <typename TArg0>
  static void BuildCbNZ(const char *instr_spec, TContext &mctx,
                        const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                        const TArg0 &n, const u32 &imm) {
    static_cast<void>(instr_spec);
    static_cast<void>(bflags);
    const bool is_non_zero = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kNonZero)) != 0U;

    mctx.builder.AddString("CB");
    if (is_non_zero == true) {
      mctx.builder.AddChar('N');
    }
    mctx.builder.AddString("Z ")
        .AddString(Reg::GetRegisterName(n.Get()))
        .AddString(", #")
        .AddInt(imm)
        .Terminate();
  }

  template <typename TArg0, typename TArg1>
  static void BuildUbfx(const char *instr_spec, TContext &mctx,
                        const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                        const TArg0 &d, const TArg1 &n, const u8 &lsbit, const u8 &widthminus1) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(n.Get()))
        .AddString(", #")
        .AddUInt(lsbit)
        .AddString(", #")
        .AddUInt(widthminus1 + 1U);

    mctx.builder.Terminate();
  }

  template <typename TArg0, typename TArg1>
  static void BuildBfi(const char *instr_spec, TContext &mctx,
                       const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                       const TArg0 &d, const TArg1 &n, const u8 &lsbit, const u8 &msbit) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);

    auto width = msbit - lsbit + 1;
    mctx.builder.AddString(instr_spec)
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(n.Get()))
        .AddString(", #")
        .AddUInt(lsbit)
        .AddString(", #")
        .AddUInt(width);

    mctx.builder.Terminate();
  }

  template <typename TArg0>
  static void BuildLdrLiteral(const char *instr_spec, TContext &mctx,
                              const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                              const TArg0 &t, const u32 imm32) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(t.Get()))
        .AddString(", [PC, #")
        .AddInt(imm32)
        .AddChar(']')
        .Terminate();
  }

  static void BuildMsrMrs(TContext &mctx, const u8 &mask, const u8 &SYSm) {
    const auto SYSm_7_3 = Bm32::ExtractBits1R<7U, 3U>(SYSm);
    switch (SYSm_7_3) {
    case 0b00000U: {
      if (mask & 0x1) {
        mctx.builder.AddString("APSR_g"); // Application Program Status Register
      } else {
        mctx.builder.AddString("APSR_nzcvq"); // Application Program Status Register
      }
      break;
    }
    case 0b00001U: {
      const auto SYSm_2_0 = Bm32::ExtractBits1R<2U, 0U>(SYSm);
      switch (SYSm_2_0) {
      case 0b000U:
        mctx.builder.AddString("MSP"); // Main Stack Pointer
        break;
      case 0b001U:
        mctx.builder.AddString("PSP"); // Process Stack Pointer
        break;
      default:
        mctx.builder.AddString("undefined");
        break;
      }
      break;
    }
    case 0b00010U: {
      const auto SYSm_2_0 = Bm32::ExtractBits1R<2U, 0U>(SYSm);
      switch (SYSm_2_0) {
      case 0b000U:
        mctx.builder.AddString("PRIMASK"); // Priority Mask
        break;
      case 0b001U:
        mctx.builder.AddString("BASEPRI"); // Base Priority
        break;
      case 0b010U:
        mctx.builder.AddString("BASEPRI_MAX"); // Base Priority Max
        break;
      case 0b011U:
        mctx.builder.AddString("FAULTMASK"); // Fault Mask
        break;
      case 0b100U:
        mctx.builder.AddString("CONTROL"); // Control
        break;
      default:
        mctx.builder.AddString("undefined");
        break;
      }
    } break;
    default:
      mctx.builder.AddString("undefined");
      break;
    }
  }

  template <typename TArg0>
  static void BuildMrs(const char *instr_spec, TContext &mctx,
                       const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                       const TArg0 &d, const u8 &mask, const u8 &SYSm) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec).AddString(It::GetConditionAsStr(mctx.cpua)).AddChar(' ');
    mctx.builder.AddString(Reg::GetRegisterName(d.Get())).AddString(", ");
    BuildMsrMrs(mctx, mask, SYSm);
    mctx.builder.Terminate();
  }

  template <typename TArg0>
  static void BuildMsr(const char *instr_spec, TContext &mctx,
                       const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                       const TArg0 &n, const u8 &mask, const u8 &SYSm) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec).AddString(It::GetConditionAsStr(mctx.cpua)).AddChar(' ');
    BuildMsrMrs(mctx, mask, SYSm);
    mctx.builder.AddString(", ").AddString(Reg::GetRegisterName(n.Get())).Terminate();
  }

  template <typename TArg0, typename TArg1, typename TArg2, typename TArg3>
  static void BuildUmull(const char *instr_spec, TContext &mctx,
                         const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                         const TArg0 &d_lo, const TArg1 &d_hi, const TArg2 &n, const TArg3 &m) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d_lo.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(d_hi.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(n.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m.Get()))
        .Terminate();
  }

  template <typename TArg0, typename TArg1, typename TArg2, typename TArg3>
  static void BuildSmull(const char *instr_spec, TContext &mctx,
                         const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                         const TArg0 &d_lo, const TArg1 &d_hi, const TArg2 &n, const TArg3 &m) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d_lo.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(d_hi.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(n.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m.Get()))
        .Terminate();
  }

  template <typename TArg0, typename TArg1, typename TArg2>
  static void BuildLdrdStrd(const char *instr_spec, TContext &mctx,
                            const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                            const TArg0 &t, const TArg1 &t2, const TArg2 &n, const u32 imm32) {
    static_cast<void>(bflags);
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(t.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(t2.Get()))
        .AddString(", ");

    RelativeAdrBuilder::Build<decltype(mctx.builder), Reg>(mctx.builder, is_add, is_index, is_wback,
                                                           n, imm32);
    mctx.builder.Terminate();
  }

  static void BuildBCond(const char *instr_spec, TContext &mctx,
                         const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                         const u8 &cond, const u32 &imm) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::ConditionToString(cond))
        .AddString(" #")
        .AddInt(static_cast<i32>(imm))
        .Terminate();
  }

  static void BuildIt(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                      const InstrFlagsSet &iflags, const u8 &firstcond, const u8 &mask) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    auto then_or_else = [](u8 cond) {
      if (cond == 0x1U) {
        return "T";
      } else {
        return "E";
      }
    };
    const char *x_spec = "";
    const char *y_spec = "";
    const char *z_spec = "";
    switch (mask) {
    case 0b1000U:
      // do nothing
      break;
    case 0b0100U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b1100U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b0010U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b0110U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b1010U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b1110U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b0001U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      z_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b0011U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      z_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b0101U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      z_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b0111U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      z_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b1001U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      z_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b1011U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      z_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b1101U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      z_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b1111U:
      x_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      z_spec = then_or_else(Bm32::ExtractBits1R<0U, 0U>(firstcond) == 0x1U);
      break;
    default:
      break;
    }

    mctx.builder.AddString(instr_spec)
        .AddString(x_spec)
        .AddString(y_spec)
        .AddString(z_spec)
        .AddChar(' ')
        .AddString(It::ConditionToString(firstcond))
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderSpecialInstr() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderSpecialInstr() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderSpecialInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderSpecialInstr(const MnemonicBuilderSpecialInstr &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderSpecialInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderSpecialInstr &operator=(const MnemonicBuilderSpecialInstr &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderSpecialInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderSpecialInstr(MnemonicBuilderSpecialInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderSpecialInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderSpecialInstr &operator=(MnemonicBuilderSpecialInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu