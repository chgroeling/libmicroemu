#pragma once

#include "libmicroemu/internal/trace/instr/generic/relative_adr_builder.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderSpecialInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void BuildTbbH(const char *instr_spec, TContext &mctx,
                        const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                        const u8 &n, const u8 &m) {
    static_cast<void>(instr_spec);
    static_cast<void>(bflags);
    // TODO: Separate into two
    const bool is_tbh = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kTbh)) != 0U;

    if (is_tbh) {
      mctx.builder.AddString("TBH")
          .AddString(It::GetConditionAsStr(mctx.pstates))
          .AddString(" [")
          .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
          .AddString(", ")
          .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)))
          .AddString(", LSL #1")
          .AddChar(']');
    } else {
      mctx.builder.AddString("TBB")
          .AddString(It::GetConditionAsStr(mctx.pstates))
          .AddString(" [")
          .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
          .AddString(", ")
          .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)))
          .AddChar(']');
    }

    mctx.builder.Terminate();
  }
  static void BuildCbNZ(const char *instr_spec, TContext &mctx,
                        const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                        const u8 &n, const u32 &imm) {
    static_cast<void>(instr_spec);
    static_cast<void>(bflags);
    const bool is_non_zero = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kNonZero)) != 0U;
    // TODO: Separate into two
    mctx.builder.AddString("CB");
    if (is_non_zero == true) {
      mctx.builder.AddChar('N');
    }
    mctx.builder.AddString("Z ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
        .AddString(", #")
        .AddInt(imm)
        .Terminate();
  }
  static void BuildUbfx(const char *instr_spec, TContext &mctx,
                        const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                        const u8 &d, const u8 &n, const u8 &lsbit, const u8 &widthminus1) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
        .AddString(", #")
        .AddUInt(lsbit)
        .AddString(", #")
        .AddUInt(widthminus1 + 1U);

    mctx.builder.Terminate();
  }

  static void BuildBfi(const char *instr_spec, TContext &mctx,
                       const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                       const u8 &d, const u8 &n, const u8 &lsbit, const u8 &msbit) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);

    auto width = msbit - lsbit + 1;
    mctx.builder.AddString(instr_spec)
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
        .AddString(", #")
        .AddUInt(lsbit)
        .AddString(", #")
        .AddUInt(width);

    mctx.builder.Terminate();
  }
  static void BuildLdrLiteral(const char *instr_spec, TContext &mctx,
                              const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                              const u8 &t, const u32 imm32) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(t)))
        .AddString(", [PC, #")
        .AddInt(imm32)
        .AddChar(']')
        .Terminate();
  }

  static void BuildMsrMrs(TContext &mctx, const u8 &mask, const u8 &SYSm) {
    const auto SYSm_7_3 = Bm32::Slice1R<7U, 3U>(SYSm);
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
      const auto SYSm_2_0 = Bm32::Slice1R<2U, 0U>(SYSm);
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
      const auto SYSm_2_0 = Bm32::Slice1R<2U, 0U>(SYSm);
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
  static void BuildMrs(const char *instr_spec, TContext &mctx,
                       const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                       const u8 &d, const u8 &mask, const u8 &SYSm) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec).AddString(It::GetConditionAsStr(mctx.pstates)).AddChar(' ');
    mctx.builder.AddString(Reg::GetRegisterName(static_cast<RegisterId>(d))).AddString(", ");
    BuildMsrMrs(mctx, mask, SYSm);
    mctx.builder.Terminate();
  }

  static void BuildMsr(const char *instr_spec, TContext &mctx,
                       const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                       const u8 &n, const u8 &mask, const u8 &SYSm) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec).AddString(It::GetConditionAsStr(mctx.pstates)).AddChar(' ');
    BuildMsrMrs(mctx, mask, SYSm);
    mctx.builder.AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
        .Terminate();
  }

  static void BuildUmull(const char *instr_spec, TContext &mctx,
                         const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                         const u8 &d_lo, const u8 &d_hi, const u8 &n, const u8 &m) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d_lo)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d_hi)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)))
        .Terminate();
  }

  // TODO: Introduce quatery instr
  static void BuildSmull(const char *instr_spec, TContext &mctx,
                         const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                         const u8 &d_lo, const u8 &d_hi, const u8 &n, const u8 &m) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d_lo)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d_hi)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)))
        .Terminate();
  }
  static void BuildLdrdStrd(const char *instr_spec, TContext &mctx,
                            const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                            const u8 &n, const u8 &t, const u8 &t2, const u32 imm32) {
    static_cast<void>(bflags);
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(t)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(t2)))
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
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b1100U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b0010U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b0110U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b1010U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b1110U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b0001U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      z_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b0011U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      z_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b0101U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      z_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b0111U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      z_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b1001U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      z_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b1011U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      z_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      break;
    case 0b1101U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      z_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) != 0x1U);
      break;
    case 0b1111U:
      x_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      y_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
      z_spec = then_or_else(Bm32::Slice1R<0U, 0U>(firstcond) == 0x1U);
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
  /// \brief Constructor
  StrBuilderSpecialInstr() = delete;

  /// \brief Destructor
  ~StrBuilderSpecialInstr() = delete;

  /// \brief Copy constructor for StrBuilderSpecialInstr.
  /// \param r_src the object to be copied
  StrBuilderSpecialInstr(const StrBuilderSpecialInstr &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderSpecialInstr.
  /// \param r_src the object to be copied
  StrBuilderSpecialInstr &operator=(const StrBuilderSpecialInstr &r_src) = delete;

  /// \brief Move constructor for StrBuilderSpecialInstr.
  /// \param r_src the object to be copied
  StrBuilderSpecialInstr(StrBuilderSpecialInstr &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderSpecialInstr.
  /// \param r_src the object to be copied
  StrBuilderSpecialInstr &operator=(StrBuilderSpecialInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu