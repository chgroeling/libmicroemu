#pragma once

#include "libmicroemu/internal/decoder/instr_flags.h"
#include "libmicroemu/internal/logic/imm_shift_results.h"
#include "libmicroemu/internal/logic/thumb_immediate_result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

enum class InstrId : u8 {
  kNop,
  kDmb,
  kAddPcPlusImmediate,
  kLslImmediate,
  kLslRegister,
  kClz,
  kSxth,
  kTbbH,
  kCmpRegister,
  kCmpImmediate,
  kCmnImmediate,
  kMovImmediate,
  kMvnImmediate,
  kMovRegister,
  kRrx,
  kLdrbImmediate,
  kLdrsbImmediate,
  kLdrhImmediate,
  kLdrshImmediate,
  kLdrImmediate,
  kLdrex,
  kLdrdImmediate,
  kLdrLiteral,
  kIt,
  kCbNZ,
  kBl,
  kBx,
  kBlx,
  kBCond,
  kB,
  kBfi,
  kUdiv,
  kSdiv,
  kMul,
  kUmlal,
  kUmull,
  kSmull,
  kMls,
  kMla,
  kSubImmediate,
  kSbcImmediate,
  kRsbImmediate,
  kSubSpMinusImmediate,
  kSubRegister,
  kRsbRegister,
  kAddImmediate,
  kAdcImmediate,
  kAddSpPlusImmediate,
  kTstImmediate,
  kTeqImmediate,
  kTstRegister,
  kTeqRegister,
  kAndImmediate,
  kAddRegister,
  kAdcRegister,
  kStmdb,
  kPush,
  kStm,
  kPop,
  kLdm,
  kUxtb,
  kSxtb,
  kUxth,
  kUbfx,
  kOrrImmediate,
  kOrrRegister,
  kEorImmediate,
  kEorRegister,
  kSbcRegister,
  kAndRegister,
  kBicRegister,
  kBicImmediate,
  kMvnRegister,
  kLsrImmediate,
  kAsrImmediate,
  kLsrRegister,
  kAsrRegister,
  kStrdImmediate,
  kStrRegister,
  kStrbRegister,
  kStrhRegister,
  kLdrbRegister,
  kLdrhRegister,
  kLdrRegister,
  kStrbImmediate,
  kStrhImmediate,
  kStrImmediate,
  kStrex,
  kSvc,
  kBkpt,
  kMsr,
  kMrs,
  kInvalid
};

struct InstrNop {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
};
struct InstrDmb {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
};
struct InstrAddPcPlusImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u32 imm32;
};
struct InstrLslImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 m;
  ImmShiftResults shift_res;
};
struct InstrLslRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u8 m;
};
struct InstrClz {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 m;
};
struct InstrSxth {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 d;
  u8 rotation;
};
struct InstrTbbH {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 n;
};
struct InstrCmpRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 m;
  ImmShiftResults shift_res;
};
struct InstrCmpImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u32 imm32;
};
struct InstrCmnImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u32 imm32;
};
struct InstrMovImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  ThumbImmediateResult imm32_carry;
};
struct InstrMvnImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  ThumbImmediateResult imm32_carry;
};
struct InstrMovRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 m;
};
struct InstrRrx {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 m;
};
struct InstrLdrbImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 t;
  u8 n;
  u32 imm32;
};
struct InstrLdrsbImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 t;
  u8 n;
  u32 imm32;
};
struct InstrLdrhImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 t;
  u8 n;
  u32 imm32;
};
struct InstrLdrshImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 t;
  u8 n;
  u32 imm32;
};
struct InstrLdrImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 t;
  u8 n;
  u32 imm32;
};
struct InstrLdrex {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 t;
  u8 n;
  u32 imm32;
};
struct InstrLdrdImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 t;
  u8 t2;
  u8 n;
  u32 imm32;
};
struct InstrLdrLiteral {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 t;
  u32 imm32;
};
struct InstrIt {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 firstcond;
  u8 mask;
};
struct InstrCbNZ {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  i32 imm32;
};
struct InstrBl {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  i32 imm32;
};
struct InstrBx {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
};
struct InstrBlx {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
};
struct InstrBCond {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 cond;
  i32 imm32;
};
struct InstrB {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  i32 imm32;
};
struct InstrBfi {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 n;
  u8 lsbit;
  u8 msbit;
};
struct InstrUdiv {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 m;
  u8 n;
};
struct InstrSdiv {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 m;
  u8 n;
};
struct InstrMul {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 m;
  u8 n;
};
struct InstrUmlal {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 dHi;
  u8 dLo;
  u8 m;
  u8 n;
};
struct InstrUmull {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 dHi;
  u8 dLo;
  u8 m;
  u8 n;
};
struct InstrSmull {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 dHi;
  u8 dLo;
  u8 m;
  u8 n;
};
struct InstrMls {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 a;
  u8 d;
  u8 m;
  u8 n;
};
struct InstrMla {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 a;
  u8 d;
  u8 m;
  u8 n;
};
struct InstrSubImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u32 imm32;
};
struct InstrSbcImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u32 imm32;
};
struct InstrRsbImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u32 imm32;
};
struct InstrSubSpMinusImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u32 imm32;
};
struct InstrSubRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  ImmShiftResults shift_res;
  u8 m;
  u8 n;
  u8 d;
};
struct InstrRsbRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  ImmShiftResults shift_res;
  u8 m;
  u8 n;
  u8 d;
};
struct InstrAddImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u32 imm32;
};
struct InstrAdcImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u32 imm32;
};
struct InstrAddSpPlusImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u32 imm32;
};
struct InstrTstImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  ThumbImmediateResult imm32_carry;
};
struct InstrTeqImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  ThumbImmediateResult imm32_carry;
};
struct InstrTstRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 m;
  ImmShiftResults shift_res;
};
struct InstrTeqRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 m;
  ImmShiftResults shift_res;
};
struct InstrAndImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  ThumbImmediateResult imm32_carry;
};
struct InstrAddRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  ImmShiftResults shift_res;
  u8 m;
  u8 n;
  u8 d;
};
struct InstrAdcRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  ImmShiftResults shift_res;
  u8 m;
  u8 n;
  u8 d;
};
struct InstrStmdb {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u32 registers;
};
struct InstrPush {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u32 registers;
};
struct InstrStm {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u32 registers;
  u8 n;
};
struct InstrPop {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u32 registers;
};
struct InstrLdm {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u32 registers;
  u8 n;
};
struct InstrUxtb {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 d;
  u8 rotation;
};
struct InstrSxtb {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 d;
  u8 rotation;
};
struct InstrUxth {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 d;
  u8 rotation;
};
struct InstrUbfx {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 n;
  u8 lsbit;
  u8 widthminus1;
};
struct InstrOrrImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  ThumbImmediateResult imm32_carry;
};
struct InstrOrrRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u8 m;
  ImmShiftResults shift_res;
};
struct InstrEorImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  ThumbImmediateResult imm32_carry;
};
struct InstrEorRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u8 m;
  ImmShiftResults shift_res;
};
struct InstrSbcRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u8 m;
  ImmShiftResults shift_res;
};
struct InstrAndRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u8 m;
  ImmShiftResults shift_res;
};
struct InstrBicRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u8 m;
  ImmShiftResults shift_res;
};
struct InstrBicImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  ThumbImmediateResult imm32_carry;
};
struct InstrMvnRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 d;
  ImmShiftResults shift_res;
};
struct InstrLsrImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 d;
  ImmShiftResults shift_res;
};
struct InstrAsrImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 d;
  ImmShiftResults shift_res;
};
struct InstrLsrRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u8 m;
};
struct InstrAsrRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 d;
  u8 m;
};
struct InstrStrdImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 t;
  u8 t2;
  u32 imm32;
};
struct InstrStrRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 m;
  u8 t;
  ImmShiftResults shift_res;
};
struct InstrStrbRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 m;
  u8 t;
  ImmShiftResults shift_res;
};
struct InstrStrhRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 m;
  u8 t;
  ImmShiftResults shift_res;
};
struct InstrLdrbRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 n;
  u8 t;
  ImmShiftResults shift_res;
};
struct InstrLdrhRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 n;
  u8 t;
  ImmShiftResults shift_res;
};
struct InstrLdrRegister {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 m;
  u8 n;
  u8 t;
  ImmShiftResults shift_res;
};
struct InstrStrbImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 t;
  u32 imm32;
};
struct InstrStrhImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 t;
  u32 imm32;
};
struct InstrStrImmediate {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 t;
  u32 imm32;
};
struct InstrStrex {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 t;
  u8 d;
  u32 imm32;
};
struct InstrSvc {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u32 imm32;
};
struct InstrBkpt {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u32 imm32;
};
struct InstrMsr {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 n;
  u8 mask;
  u8 SYSm;
};
struct InstrMrs {
  InstrId id;          // base_type: u8
  InstrFlagsSet flags; // base_type: u8
  u8 d;
  u8 mask;
  u8 SYSm;
};

union Instr {
  InstrNop nop;
  InstrDmb dmb;
  InstrAddPcPlusImmediate add_pc_plus_immediate;
  InstrLslImmediate lsl_immediate;
  InstrLslRegister lsl_register;
  InstrClz clz;
  InstrSxth sxth;
  InstrTbbH tbb_h;
  InstrCmpRegister cmp_register;
  InstrCmpImmediate cmp_immediate;
  InstrCmnImmediate cmn_immediate;
  InstrMovImmediate mov_immediate;
  InstrMvnImmediate mvn_immediate;
  InstrMovRegister mov_register;
  InstrRrx rrx;
  InstrLdrbImmediate ldrb_immediate;
  InstrLdrsbImmediate ldrsb_immediate;
  InstrLdrhImmediate ldrh_immediate;
  InstrLdrshImmediate ldrsh_immediate;
  InstrLdrImmediate ldr_immediate;
  InstrLdrex ldrex;
  InstrLdrdImmediate ldrd_immediate;
  InstrLdrLiteral ldr_literal;
  InstrIt it;
  InstrCbNZ cb_n_z;
  InstrBl bl;
  InstrBx bx;
  InstrBlx blx;
  InstrBCond b_cond;
  InstrB b;
  InstrBfi bfi;
  InstrUdiv udiv;
  InstrSdiv sdiv;
  InstrMul mul;
  InstrUmlal umlal;
  InstrUmull umull;
  InstrSmull smull;
  InstrMls mls;
  InstrMla mla;
  InstrSubImmediate sub_immediate;
  InstrSbcImmediate sbc_immediate;
  InstrRsbImmediate rsb_immediate;
  InstrSubSpMinusImmediate sub_sp_minus_immediate;
  InstrSubRegister sub_register;
  InstrRsbRegister rsb_register;
  InstrAddImmediate add_immediate;
  InstrAdcImmediate adc_immediate;
  InstrAddSpPlusImmediate add_sp_plus_immediate;
  InstrTstImmediate tst_immediate;
  InstrTeqImmediate teq_immediate;
  InstrTstRegister tst_register;
  InstrTeqRegister teq_register;
  InstrAndImmediate and_immediate;
  InstrAddRegister add_register;
  InstrAdcRegister adc_register;
  InstrStmdb stmdb;
  InstrPush push;
  InstrStm stm;
  InstrPop pop;
  InstrLdm ldm;
  InstrUxtb uxtb;
  InstrSxtb sxtb;
  InstrUxth uxth;
  InstrUbfx ubfx;
  InstrOrrImmediate orr_immediate;
  InstrOrrRegister orr_register;
  InstrEorImmediate eor_immediate;
  InstrEorRegister eor_register;
  InstrSbcRegister sbc_register;
  InstrAndRegister and_register;
  InstrBicRegister bic_register;
  InstrBicImmediate bic_immediate;
  InstrMvnRegister mvn_register;
  InstrLsrImmediate lsr_immediate;
  InstrAsrImmediate asr_immediate;
  InstrLsrRegister lsr_register;
  InstrAsrRegister asr_register;
  InstrStrdImmediate strd_immediate;
  InstrStrRegister str_register;
  InstrStrbRegister strb_register;
  InstrStrhRegister strh_register;
  InstrLdrbRegister ldrb_register;
  InstrLdrhRegister ldrh_register;
  InstrLdrRegister ldr_register;
  InstrStrbImmediate strb_immediate;
  InstrStrhImmediate strh_immediate;
  InstrStrImmediate str_immediate;
  InstrStrex strex;
  InstrSvc svc;
  InstrBkpt bkpt;
  InstrMsr msr;
  InstrMrs mrs;
  InstrId id; // the decoded opcode ... used to identify the struct
  Instr() : id(InstrId::kInvalid) {}
  Instr(const InstrId &arg) : id(arg) {}
  Instr(const InstrNop &arg) : nop(arg) {}
  Instr(const InstrDmb &arg) : dmb(arg) {}
  Instr(const InstrAddPcPlusImmediate &arg) : add_pc_plus_immediate(arg) {}
  Instr(const InstrLslImmediate &arg) : lsl_immediate(arg) {}
  Instr(const InstrLslRegister &arg) : lsl_register(arg) {}
  Instr(const InstrClz &arg) : clz(arg) {}
  Instr(const InstrSxth &arg) : sxth(arg) {}
  Instr(const InstrTbbH &arg) : tbb_h(arg) {}
  Instr(const InstrCmpRegister &arg) : cmp_register(arg) {}
  Instr(const InstrCmpImmediate &arg) : cmp_immediate(arg) {}
  Instr(const InstrCmnImmediate &arg) : cmn_immediate(arg) {}
  Instr(const InstrMovImmediate &arg) : mov_immediate(arg) {}
  Instr(const InstrMvnImmediate &arg) : mvn_immediate(arg) {}
  Instr(const InstrMovRegister &arg) : mov_register(arg) {}
  Instr(const InstrRrx &arg) : rrx(arg) {}
  Instr(const InstrLdrbImmediate &arg) : ldrb_immediate(arg) {}
  Instr(const InstrLdrsbImmediate &arg) : ldrsb_immediate(arg) {}
  Instr(const InstrLdrhImmediate &arg) : ldrh_immediate(arg) {}
  Instr(const InstrLdrshImmediate &arg) : ldrsh_immediate(arg) {}
  Instr(const InstrLdrImmediate &arg) : ldr_immediate(arg) {}
  Instr(const InstrLdrex &arg) : ldrex(arg) {}
  Instr(const InstrLdrdImmediate &arg) : ldrd_immediate(arg) {}
  Instr(const InstrLdrLiteral &arg) : ldr_literal(arg) {}
  Instr(const InstrIt &arg) : it(arg) {}
  Instr(const InstrCbNZ &arg) : cb_n_z(arg) {}
  Instr(const InstrBl &arg) : bl(arg) {}
  Instr(const InstrBx &arg) : bx(arg) {}
  Instr(const InstrBlx &arg) : blx(arg) {}
  Instr(const InstrBCond &arg) : b_cond(arg) {}
  Instr(const InstrB &arg) : b(arg) {}
  Instr(const InstrBfi &arg) : bfi(arg) {}
  Instr(const InstrUdiv &arg) : udiv(arg) {}
  Instr(const InstrSdiv &arg) : sdiv(arg) {}
  Instr(const InstrMul &arg) : mul(arg) {}
  Instr(const InstrUmlal &arg) : umlal(arg) {}
  Instr(const InstrUmull &arg) : umull(arg) {}
  Instr(const InstrSmull &arg) : smull(arg) {}
  Instr(const InstrMls &arg) : mls(arg) {}
  Instr(const InstrMla &arg) : mla(arg) {}
  Instr(const InstrSubImmediate &arg) : sub_immediate(arg) {}
  Instr(const InstrSbcImmediate &arg) : sbc_immediate(arg) {}
  Instr(const InstrRsbImmediate &arg) : rsb_immediate(arg) {}
  Instr(const InstrSubSpMinusImmediate &arg) : sub_sp_minus_immediate(arg) {}
  Instr(const InstrSubRegister &arg) : sub_register(arg) {}
  Instr(const InstrRsbRegister &arg) : rsb_register(arg) {}
  Instr(const InstrAddImmediate &arg) : add_immediate(arg) {}
  Instr(const InstrAdcImmediate &arg) : adc_immediate(arg) {}
  Instr(const InstrAddSpPlusImmediate &arg) : add_sp_plus_immediate(arg) {}
  Instr(const InstrTstImmediate &arg) : tst_immediate(arg) {}
  Instr(const InstrTeqImmediate &arg) : teq_immediate(arg) {}
  Instr(const InstrTstRegister &arg) : tst_register(arg) {}
  Instr(const InstrTeqRegister &arg) : teq_register(arg) {}
  Instr(const InstrAndImmediate &arg) : and_immediate(arg) {}
  Instr(const InstrAddRegister &arg) : add_register(arg) {}
  Instr(const InstrAdcRegister &arg) : adc_register(arg) {}
  Instr(const InstrStmdb &arg) : stmdb(arg) {}
  Instr(const InstrPush &arg) : push(arg) {}
  Instr(const InstrStm &arg) : stm(arg) {}
  Instr(const InstrPop &arg) : pop(arg) {}
  Instr(const InstrLdm &arg) : ldm(arg) {}
  Instr(const InstrUxtb &arg) : uxtb(arg) {}
  Instr(const InstrSxtb &arg) : sxtb(arg) {}
  Instr(const InstrUxth &arg) : uxth(arg) {}
  Instr(const InstrUbfx &arg) : ubfx(arg) {}
  Instr(const InstrOrrImmediate &arg) : orr_immediate(arg) {}
  Instr(const InstrOrrRegister &arg) : orr_register(arg) {}
  Instr(const InstrEorImmediate &arg) : eor_immediate(arg) {}
  Instr(const InstrEorRegister &arg) : eor_register(arg) {}
  Instr(const InstrSbcRegister &arg) : sbc_register(arg) {}
  Instr(const InstrAndRegister &arg) : and_register(arg) {}
  Instr(const InstrBicRegister &arg) : bic_register(arg) {}
  Instr(const InstrBicImmediate &arg) : bic_immediate(arg) {}
  Instr(const InstrMvnRegister &arg) : mvn_register(arg) {}
  Instr(const InstrLsrImmediate &arg) : lsr_immediate(arg) {}
  Instr(const InstrAsrImmediate &arg) : asr_immediate(arg) {}
  Instr(const InstrLsrRegister &arg) : lsr_register(arg) {}
  Instr(const InstrAsrRegister &arg) : asr_register(arg) {}
  Instr(const InstrStrdImmediate &arg) : strd_immediate(arg) {}
  Instr(const InstrStrRegister &arg) : str_register(arg) {}
  Instr(const InstrStrbRegister &arg) : strb_register(arg) {}
  Instr(const InstrStrhRegister &arg) : strh_register(arg) {}
  Instr(const InstrLdrbRegister &arg) : ldrb_register(arg) {}
  Instr(const InstrLdrhRegister &arg) : ldrh_register(arg) {}
  Instr(const InstrLdrRegister &arg) : ldr_register(arg) {}
  Instr(const InstrStrbImmediate &arg) : strb_immediate(arg) {}
  Instr(const InstrStrhImmediate &arg) : strh_immediate(arg) {}
  Instr(const InstrStrImmediate &arg) : str_immediate(arg) {}
  Instr(const InstrStrex &arg) : strex(arg) {}
  Instr(const InstrSvc &arg) : svc(arg) {}
  Instr(const InstrBkpt &arg) : bkpt(arg) {}
  Instr(const InstrMsr &arg) : msr(arg) {}
  Instr(const InstrMrs &arg) : mrs(arg) {}
};

} // namespace internal
} // namespace microemu