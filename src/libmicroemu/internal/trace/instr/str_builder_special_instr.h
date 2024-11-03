#pragma once

#include "libmicroemu/internal/trace/instr/generic/relative_adr_builder.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
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
    const bool is_tbh = (iflags & InstrFlags::kTbh) != 0u;

    if (is_tbh) {
      mctx.builder.AddString("TBH")
          .AddString(It::GetConditionAsStr(mctx.pstates))
          .AddString(" [")
          .AddString(Reg::GetRegisterName(n))
          .AddString(", ")
          .AddString(Reg::GetRegisterName(m))
          .AddString(", LSL #1")
          .AddChar(']');
    } else {
      mctx.builder.AddString("TBB")
          .AddString(It::GetConditionAsStr(mctx.pstates))
          .AddString(" [")
          .AddString(Reg::GetRegisterName(n))
          .AddString(", ")
          .AddString(Reg::GetRegisterName(m))
          .AddChar(']');
    }

    mctx.builder.Terminate();
  }
  static void BuildCbNZ(const char *instr_spec, TContext &mctx,
                        const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                        const u8 &n, const u32 &imm) {
    static_cast<void>(instr_spec);
    static_cast<void>(bflags);
    const bool is_non_zero = (iflags & kNonZero) != 0u;
    // TODO: Separate into two
    mctx.builder.AddString("CB");
    if (is_non_zero == true) {
      mctx.builder.AddChar('N');
    }
    mctx.builder.AddString("Z ")
        .AddString(Reg::GetRegisterName(n))
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
        .AddString(Reg::GetRegisterName(d))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(n))
        .AddString(", #")
        .AddUInt(lsbit)
        .AddString(", #")
        .AddUInt(widthminus1 + 1u);

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
        .AddString(Reg::GetRegisterName(d))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(n))
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
        .AddString(Reg::GetRegisterName(t))
        .AddString(", [PC, #")
        .AddInt(imm32)
        .AddChar(']')
        .Terminate();
  }

  static void BuildMsrMrs(TContext &mctx, const u8 &mask, const u8 &SYSm) {
    const auto SYSm_7_3 = Bm32::Slice1R<7u, 3u>(SYSm);
    switch (SYSm_7_3) {
    case 0b00000u: {
      if (mask & 0x1) {
        mctx.builder.AddString("APSR_g"); // Application Program Status Register
      } else {
        mctx.builder.AddString("APSR_nzcvq"); // Application Program Status Register
      }
      break;
    }
    case 0b00001u: {
      const auto SYSm_2_0 = Bm32::Slice1R<2u, 0u>(SYSm);
      switch (SYSm_2_0) {
      case 0b000u:
        mctx.builder.AddString("MSP"); // Main Stack Pointer
        break;
      case 0b001u:
        mctx.builder.AddString("PSP"); // Process Stack Pointer
        break;
      default:
        mctx.builder.AddString("undefined");
        break;
      }
      break;
    }
    case 0b00010u: {
      const auto SYSm_2_0 = Bm32::Slice1R<2u, 0u>(SYSm);
      switch (SYSm_2_0) {
      case 0b000u:
        mctx.builder.AddString("PRIMASK"); // Priority Mask
        break;
      case 0b001u:
        mctx.builder.AddString("BASEPRI"); // Base Priority
        break;
      case 0b010u:
        mctx.builder.AddString("BASEPRI_MAX"); // Base Priority Max
        break;
      case 0b011u:
        mctx.builder.AddString("FAULTMASK"); // Fault Mask
        break;
      case 0b100u:
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
    mctx.builder.AddString(Reg::GetRegisterName(d)).AddString(", ");
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
    mctx.builder.AddString(", ").AddString(Reg::GetRegisterName(n)).Terminate();
  }

  static void BuildUmull(const char *instr_spec, TContext &mctx,
                         const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                         const u8 &d_lo, const u8 &d_hi, const u8 &n, const u8 &m) {
    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d_lo))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(d_hi))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(n))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m))
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
        .AddString(Reg::GetRegisterName(d_lo))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(d_hi))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(n))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m))
        .Terminate();
  }
  static void BuildLdrdStrd(const char *instr_spec, TContext &mctx,
                            const MnemonicBuilderFlagsSet &bflags, const InstrFlagsSet &iflags,
                            const u8 &n, const u8 &t, const u8 &t2, const u32 imm32) {
    static_cast<void>(bflags);
    const bool is_wback = (iflags & InstrFlags::kWBack) != 0u;
    const bool is_index = (iflags & InstrFlags::kIndex) != 0u;
    const bool is_add = (iflags & kAdd) != 0u;
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(t))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(t2))
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
      if (cond == 0x1u) {
        return "T";
      } else {
        return "E";
      }
    };
    const char *x_spec = "";
    const char *y_spec = "";
    const char *z_spec = "";
    switch (mask) {
    case 0b1000u:
      // do nothing
      break;
    case 0b0100u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      break;
    case 0b1100u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      break;
    case 0b0010u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      break;
    case 0b0110u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      break;
    case 0b1010u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      break;
    case 0b1110u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      break;
    case 0b0001u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      z_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      break;
    case 0b0011u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      z_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      break;
    case 0b0101u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      z_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      break;
    case 0b0111u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      z_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      break;
    case 0b1001u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      z_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      break;
    case 0b1011u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      z_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      break;
    case 0b1101u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      z_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) != 0x1u);
      break;
    case 0b1111u:
      x_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      y_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
      z_spec = then_or_else(Bm32::Slice1R<0u, 0u>(firstcond) == 0x1u);
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
} // namespace microemu