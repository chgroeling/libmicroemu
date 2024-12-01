#pragma once

#include "libmicroemu/internal/decoder/instr.h"
#include "libmicroemu/internal/fetcher/raw_instr.h"
#include "libmicroemu/internal/logic/thumb.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/types.h"
#include <assert.h>
#include <cstdint>

namespace libmicroemu::internal {

using Bm8 = BitManip<u8>;
using Bm16 = BitManip<u16>;
using Bm32 = BitManip<u32>;

static constexpr u32 kDecodersOpCodeLast = 15U;
static constexpr u32 kDecodersOpCodeFirst = 11U;

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> InvalidInstrDecoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(rinstr);
  static_cast<void>(cpua); // prevents warning when cpua is not used
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> NopT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kNop};
  u8 flags = 0x0U;

  static_cast<void>(rinstr);

  assert(rinstr.low == 0b1011111100000000U);

  return Ok(Instr{InstrNop{
      iid,
      flags,
  }});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> DmbT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kDmb};
  u8 flags = 0x0U;

  assert(rinstr.low == 0b1111001110111111U);
  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.high)) == 0b100011110101U);
  static_cast<void>(rinstr);

  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrDmb{
      iid,
      flags,
  }});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddPcPlusImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddPcPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b10100U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  const u16 Rd = Bm16::ExtractBits1R<10U, 8U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2U);

  return Ok(Instr{InstrAddPcPlusImmediate{iid, flags, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LslImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLslImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b00000U);
  assert((Bm16::ExtractBits1R<10U, 6U>(rinstr.low)) != 0x0U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 imm5 = Bm16::ExtractBits1R<10U, 6U>(rinstr.low);
  const auto shift_res = Alu32::DecodeImmShift(0b00, imm5);

  assert(shift_res.type == SRType::SRType_LSL);

  return Ok(Instr{InstrLslImmediate{iid, flags, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LslImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLslImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010010U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1111U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::ExtractBits1R<5U, 4U>(rinstr.high)) == 0b00U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u16 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(0b00U, (imm3 << 2U) | imm2);

  assert(shift_res.type == SRType::SRType_LSL);
  assert(((imm3 << 2U) | imm2) != 0b0U);
  if (d == 13U || d == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLslImmediate{iid, flags, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LslRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLslRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100000010U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  return Ok(Instr{InstrLslRegister{iid, flags, n, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LslRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLslRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11111010000U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0000U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if (d == 13U || d == 15 || n == 13U || n == 15 || m == 13U || m == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLslRegister{iid, flags, n, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> ClzT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kClz};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110101011U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b1000U);

  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm_1 = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u16 Rm_2 = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  if (Rm_1 != Rm_2) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  const u8 m = static_cast<u8>(Rm_1);

  if (d == 13U || d == 15 || m == 13U || m == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrClz{iid, flags, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AsrImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAsrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b00010U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm5 = Bm16::ExtractBits1R<10U, 6U>(rinstr.low);
  const auto shift_res = Alu32::DecodeImmShift(0b10U, imm5);

  return Ok(Instr{InstrAsrImmediate{iid, flags, m, d, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> CmpRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kCmpRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<10U, 6U>(rinstr.low)) == 0b01010U);

  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};

  return Ok(Instr{InstrCmpRegister{iid, flags, n, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> CmpRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kCmpRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<10U, 8U>(rinstr.low)) == 0b101U);

  const u16 N = Bm16::IsolateBit<7U>(rinstr.low);
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(N << 3U | Rn);
  const u16 Rm = Bm16::ExtractBits1R<6U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};

  if (n < 8U && m < 8U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (n == 15U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrCmpRegister{iid, flags, n, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> CmpRegisterT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kCmpRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111010111011U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::ExtractBits1R<11U, 8U>(rinstr.high)) == 0b1111U);

  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 type = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u16 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const u16 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);

  if (n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrCmpRegister{iid, flags, n, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> CmpImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kCmpImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b00101U);

  const u16 Rn = Bm16::ExtractBits1R<10U, 8U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrCmpImmediate{iid, flags, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> CmpImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kCmpImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 4U>(rinstr.low)) == 0b011011U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::ExtractBits1R<11U, 8U>(rinstr.high)) == 0b1111U);

  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm((i << 11U) | (imm3 << 8U) | imm8));

  if (n == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrCmpImmediate{iid, flags, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> CmnImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kCmnImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 4U>(rinstr.low)) == 0b010001U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::ExtractBits1R<11U, 8U>(rinstr.high)) == 0b1111U);

  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm((i << 11U) | (imm3 << 8U) | imm8));

  if (n == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrCmnImmediate{iid, flags, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MovImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMovImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b00100U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rd = Bm16::ExtractBits1R<10U, 8U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
  const u16 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const auto imm32_carry =
      ThumbImmediateResult{imm8, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk};

  return Ok(Instr{InstrMovImmediate{iid, flags, d, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MovImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMovImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b00010U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1111U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13U || d == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMovImmediate{iid, flags, d, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MovImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMovImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 4U>(rinstr.low)) == 0b100100U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm4 = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const auto imm32_carry =
      ThumbImmediateResult{(imm4 << 12U) | (i << 11U) | (imm3 << 8U) | imm8, false};

  if (d == 13U || d == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMovImmediate{iid, flags, d, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MvnImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMvnImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b00011U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1111U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13U || d == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMvnImmediate{iid, flags, d, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> TbbHT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kTbbH};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111010001101U);
  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.high)) == 0b11110000000U);

  const u32 H = Bm16::IsolateBit<4U>(rinstr.high);
  flags |= H << static_cast<InstrFlagsSet>(InstrFlagsShift::kTbhShift);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (n == 13U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrTbbH{iid, flags, m, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> RsbImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kRsbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100001001U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm32 = 0x0U;

  return Ok(Instr{InstrRsbImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> RsbImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kRsbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b01110U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  if (d == 13U || d == 15U || n == 13U || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrRsbImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MovRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMovRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<11U, 8U>(rinstr.low)) == 0x6U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 Rd = Bm32::ExtractBits2R<7U, 7U, 2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<6U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  if (d == 15U && TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrMovRegister{iid, flags, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MovRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMovRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0000000000U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  if (TItOps::InITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrMovRegister{iid, flags, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MovRegisterT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMovRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010010U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1111U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b0000U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0000U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if ((flags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) != 0U) &&
      (d == 13U || d == 15U || m == 13U || m == 15U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if ((flags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) == 0U) &&
      (d == 15U || m == 15U || (d == 13U && m == 13U))) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMovRegister{iid, flags, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> RrxT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kRrx};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010010U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1111U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b0000U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0011U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if ((d == 13U) || (d == 15U) || (m == 13U) || (m == 15U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrRrx{iid, flags, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrLiteralT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrLiteral};
  u8 flags = 0x0U;

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  const u16 Rt = Bm16::ExtractBits1R<10U, 8U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u16 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8) << 2U;

  return Ok(Instr{InstrLdrLiteral{iid, flags, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrLiteralT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrLiteral};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 8U>(rinstr.low)) == 0b11111000U);
  assert((Bm16::ExtractBits1R<6U, 0U>(rinstr.low)) == 0b1011111U);

  const u32 U = Bm16::IsolateBit<7U>(rinstr.low);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm12 = Bm16::ExtractBits1R<11U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (t == 15U && TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrLiteral{iid, flags, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrbImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b01111U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm5 = Bm16::ExtractBits1R<10U, 6U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5);

  return Ok(Instr{InstrLdrbImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrbImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110001001U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::ExtractBits1R<11U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rt != 0b1111U);
  assert(Rn != 0b1111U);
  if (t == 13U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrbImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrbImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000001U);
  assert((Bm16::IsolateBit<11U>(rinstr.high)) == 0b1U);

  const u32 U = Bm16::IsolateBit<9U>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10U>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8U>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert((Rt != 0b1111U) || (P != 1U) || (U != 0U) || (W != 0U));
  assert(Rn != 0b1111U);
  assert((P != 0b1U) || (U != 0b1U) || (W != 0b0U));
  assert((Rn != 0b1101U) || (P != 0x0U) || (U != 0x1U) || (W != 0x1U) || (imm8 != 0b00000100U));
  if ((P == 0x0U) && (W == 0x0U)) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 13U || (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && n == t)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (t == 15U && ((P == 0U) || (U == 1U) || (W == 1U))) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrbImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrsbImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrsbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110011001U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::ExtractBits1R<11U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rt != 0b1111U);
  assert(Rn != 0b1111U);
  if (t == 13U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrsbImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrsbImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrsbImmediate};
  u8 flags = 0x0U;

  assert(false); // not tested
  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110010001U);
  assert((Bm16::IsolateBit<11U>(rinstr.high)) == 0b1U);

  const u32 U = Bm16::IsolateBit<9U>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10U>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8U>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert((Rt != 0b1111U) || (P != 1U) || (U != 0U) || (W != 0U));
  assert(Rn != 0b1111U);
  assert((P != 0b1U) || (U != 0b1U) || (W != 0b0U));
  assert((Rn != 0b1101U) || (P != 0x0U) || (U != 0x1U) || (W != 0x1U) || (imm8 != 0b00000100U));
  if ((P == 0x0U) && (W == 0x0U)) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 13U || (t == 15U && (W == 1U)) ||
      (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && n == t)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrsbImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrshImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrshImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110011011U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::ExtractBits1R<11U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rn != 0b1111U);
  assert(Rt != 0b1111U);
  if (t == 13U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrshImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrshImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrshImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110010011U);
  assert((Bm16::IsolateBit<11U>(rinstr.high)) == 0b1U);

  const u32 U = static_cast<u32>(Bm16::IsolateBit<9U>(rinstr.high));
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = static_cast<u32>(Bm16::IsolateBit<10U>(rinstr.high));
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = static_cast<u32>(Bm16::IsolateBit<8U>(rinstr.high));
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = static_cast<u32>(Bm16::ExtractBits1R<7U, 0U>(rinstr.high));
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert(Rn != 0b1111U);
  assert(Rt != 0b1111U || P != 1U || U != 0U || W != 0U);
  assert(P != 0b1U || U != 0b1U || W != 0b0U);
  if (P == 0b0U && W == 0b0U) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 13U || (t == 15U && W == 1U) ||
      (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && n == t)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrshImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrhImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b10001U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm5 = Bm16::ExtractBits1R<10U, 6U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5 << 1U);

  return Ok(Instr{InstrLdrhImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrhImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110001011U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::ExtractBits1R<11U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rt != 0b1111U);
  assert(Rn != 0b1111U);
  if (t == 13U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrhImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrhImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000011U);
  assert((Bm16::IsolateBit<11U>(rinstr.high)) == 0b1U);

  const u32 U = static_cast<u32>(Bm16::IsolateBit<9U>(rinstr.high));
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = static_cast<u32>(Bm16::IsolateBit<10U>(rinstr.high));
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = static_cast<u32>(Bm16::IsolateBit<8U>(rinstr.high));
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = static_cast<u32>(Bm16::ExtractBits1R<7U, 0U>(rinstr.high));
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert(Rn != 0b1111U);
  assert(Rt != 0b1111U || P != 1U || U != 0U || W != 0U);
  assert(P != 0b1U || U != 0b1U || W != 0b0U);
  if (P == 0b0U && W == 0b0U) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 13U || (t == 15U && W == 1U) ||
      (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && n == t)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrhImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> PopT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kPop};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b1011110U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kUnalignedAllow);
  const u32 P = Bm16::IsolateBit<8U>(rinstr.low);
  const u32 register_list = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 registers = (P << 15U) | register_list;

  if (Bm32::BitCount(registers) < 1U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if ((Bm32::ExtractBits1R<15U, 15U>(registers) == 0x1U) && (TItOps::InITBlock(cpua)) &&
      (!TItOps::LastInITBlock(cpua))) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrPop{iid, flags, registers}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> PopT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kPop};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 0U>(rinstr.low)) == 0b1110100010111101U);
  assert((Bm16::IsolateBit<13U>(rinstr.high)) == 0b0U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kUnalignedAllow);
  const u32 P = Bm16::IsolateBit<15U>(rinstr.high);
  const u32 M = Bm16::IsolateBit<14U>(rinstr.high);
  const u32 register_list = Bm16::ExtractBits1R<12U, 0U>(rinstr.high);
  const u32 registers = (P << 15U) | (M << 14U) | register_list;

  if ((Bm32::BitCount(registers) < 2U) || ((P == 1U) && M == 1U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if ((Bm32::ExtractBits1R<15U, 15U>(registers) == 0x1U) && (TItOps::InITBlock(cpua)) &&
      (!TItOps::LastInITBlock(cpua))) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrPop{iid, flags, registers}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> PopT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kPop};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 0U>(rinstr.low)) == 0b1111100001011101U);
  assert((Bm16::ExtractBits1R<11U, 0U>(rinstr.high)) == 0b101100000100U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kUnalignedAllow);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 registers = 1U << Rt;

  if (t == 13U || (t == 15U && TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua))) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrPop{iid, flags, registers}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b01101U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm5 = Bm16::ExtractBits1R<10U, 6U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5 << 2U);

  return Ok(Instr{InstrLdrImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b10011U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<10U, 8U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u8 n = 13U;
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2U);

  return Ok(Instr{InstrLdrImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110001101U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::ExtractBits1R<11U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (t == 15U && TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrImmediateT4Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000101U);
  assert((Bm16::IsolateBit<11U>(rinstr.high)) == 0b1U);

  const u32 U = Bm16::IsolateBit<9U>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10U>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8U>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert(Rn != 0b1111U);
  assert((P != 0b1U) || (U != 0b1U) || (W != 0b0U));
  if ((Rn == 0b1101U) && (P == 0x0U) && (U == 0x1U) && (W == 0x1U) && (imm8 == 0b00000100U)) {
    // SEE POP
    return PopT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  if ((P == 0x0U) && (W == 0x0U)) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if ((flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && n == t) ||
      t == 15U && TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrImmediate{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrexT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrex};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111010000101U);
  assert((Bm16::ExtractBits1R<11U, 8U>(rinstr.high)) == 0b1111U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  if (t == 13U || t == 15U || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrex{iid, flags, t, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrdImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrdImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b1110100U);
  assert((Bm16::IsolateBit<6U>(rinstr.low)) == 0b1U);
  assert((Bm16::IsolateBit<4U>(rinstr.low)) == 0b1U);

  const u32 U = Bm16::IsolateBit<7U>(rinstr.low);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<8U>(rinstr.low);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<5U>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 Rt2 = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 t2 = static_cast<u8>(Rt2);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2U);

  assert((P != 0U) || (W != 0));
  assert(Rn != 0b1111U);
  if (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && ((n == t) || (n == t2))) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (t == 13U || t == 15 || (t2 == 13U) || (t2 == 15) || (t == t2)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrdImmediate{iid, flags, t, t2, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> ItT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kIt};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<11U, 8U>(rinstr.low)) == 0xFU);

  const u32 firstcond_32 = Bm16::ExtractBits1R<7U, 4U>(rinstr.low);
  const u8 firstcond = static_cast<u8>(firstcond_32);
  const u32 mask_32 = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 mask = static_cast<u8>(mask_32);

  assert(mask != 0x0U);
  if (firstcond == 0xFU || (firstcond == 0b1110U && Bm8::BitCount(mask) != 1U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  if (TItOps::InITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrIt{iid, flags, firstcond, mask}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BlT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBl};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 14U>(rinstr.high)) == 0x3U);
  assert((Bm16::IsolateBit<12U>(rinstr.high)) == 0x1U);
  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);

  const u32 s = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm10 = Bm16::ExtractBits1R<9U, 0U>(rinstr.low);
  const u32 j1 = Bm16::IsolateBit<13U>(rinstr.high);           // bit 13
  const u32 j2 = Bm16::IsolateBit<11U>(rinstr.high);           // bit 11
  const u32 imm11 = Bm16::ExtractBits1R<10U, 0U>(rinstr.high); // bit 10..0
  const u32 i1 = (~(j1 ^ s)) & 0x1U;
  const u32 i2 = (~(j2 ^ s)) & 0x1U;
  const u32 imm32_us = (s << 24U) | (i1 << 23U) | (i2 << 22U) | (imm10 << 12U) | imm11 << 1U;
  const i32 imm32 = static_cast<int32_t>(Bm32::SignExtend<u32, 24U>(imm32_us));

  if (TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBl{iid, flags, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BxT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBx};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 7U>(rinstr.low) == 0b010001110U));

  const u16 Rm = Bm16::ExtractBits1R<6U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  if (TItOps::InITBlock(cpua) && (!TItOps::LastInITBlock(cpua))) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrBx{iid, flags, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BlxT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBlx};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 7U>(rinstr.low) == 0b010001111U));

  const u16 Rm = Bm16::ExtractBits1R<6U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  if (m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrBlx{iid, flags, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBCond};
  u8 flags = 0x0U;

  if (TItOps::InITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  const u32 cond_32 = Bm16::ExtractBits1R<11U, 8U>(rinstr.low);
  const u8 cond = static_cast<u8>(cond_32);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const i32 imm32 = Bm32::SignExtend<u32, 8U>(imm8 << 1U);

  assert(cond != 0b1110U);
  assert(cond != 0b1111U);

  return Ok(Instr{InstrBCond{iid, flags, cond, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kB};
  u8 flags = 0x0U;

  const u32 imm_11_32 = Bm16::ExtractBits1R<10U, 0U>(rinstr.low);
  const i32 imm32 = Bm32::SignExtend<u32, 11U>((imm_11_32) << 1U);

  if (TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrB{iid, flags, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBCond};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<15U, 14U>(rinstr.high)) == 0b10U);
  assert((Bm16::IsolateBit<12U>(rinstr.high)) == 0b0U);

  const u32 cond_32 = Bm16::ExtractBits1R<9U, 6U>(rinstr.low);
  const u8 cond = static_cast<u8>(cond_32);
  const u32 imm11 = Bm16::ExtractBits1R<10U, 0U>(rinstr.high);
  const u32 imm6 = Bm16::ExtractBits1R<5U, 0U>(rinstr.low);
  const u32 J1 = Bm16::IsolateBit<13U>(rinstr.high); // bit 13
  const u32 J2 = Bm16::IsolateBit<11U>(rinstr.high); // bit 11
  const u32 S = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm32_us = (S << 20U) | (J2 << 19U) | (J1 << 18U) | (imm6 << 12U) | (imm11 << 1U);
  const i32 imm32 = static_cast<int32_t>(Bm32::SignExtend<u32, 20U>(imm32_us));

  if (TItOps::InITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBCond{iid, flags, cond, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BT4Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kB};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<15U, 14U>(rinstr.high)) == 0b10U);
  assert((Bm16::IsolateBit<12U>(rinstr.high)) == 0b1U);

  const u32 imm11 = Bm16::ExtractBits1R<10U, 0U>(rinstr.high);
  const u32 imm10 = Bm16::ExtractBits1R<9U, 0U>(rinstr.low);
  const u32 J1 = Bm16::IsolateBit<13U>(rinstr.high); // bit 13
  const u32 J2 = Bm16::IsolateBit<11U>(rinstr.high); // bit 11
  const u32 S = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 I1 = (~(J1 ^ S)) & 0x1U;
  const u32 I2 = (~(J2 ^ S)) & 0x1U;
  const u32 imm32_us = (S << 24U) | (I1 << 23U) | (I2 << 22U) | (imm10 << 12U) | imm11 << 1U;
  const i32 imm32 = static_cast<int32_t>(Bm32::SignExtend<u32, 24U>(imm32_us));

  if (TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrB{iid, flags, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SubSpMinusImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSubSpMinusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 7U>(rinstr.low)) == 0b101100001U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u8 d = 13U;
  const u32 imm7 = Bm16::ExtractBits1R<6U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm7 << 2);

  return Ok(Instr{InstrSubSpMinusImmediate{iid, flags, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SubSpMinusImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSubSpMinusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b01101U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1101U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert((d != 0b1111U) || (S != 1U));
  if (d == 15U && (S == 0x0U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubSpMinusImmediate{iid, flags, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SubSpMinusImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSubSpMinusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 0U>(rinstr.low)) == 0b1010101101U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (d == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubSpMinusImmediate{iid, flags, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SubImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSubImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b0001111U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u8 n = static_cast<u8>(Bm16::ExtractBits1R<5U, 3U>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::ExtractBits1R<2U, 0U>(rinstr.low));
  const u32 imm3 = Bm16::ExtractBits1R<8U, 6U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm3);

  return Ok(Instr{InstrSubImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SubImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSubImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b00111U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u8 n = static_cast<u8>(Bm16::ExtractBits1R<10U, 8U>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::ExtractBits1R<10U, 8U>(rinstr.low));
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrSubImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SubImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSubImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b01101U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert(!((d == 0xFU) && (S == 1U)));
  if (n == 0b1101U) {
    return SubSpMinusImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  if (d == 13U || (d == 15U && S == 0x0U) || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SubImmediateT4Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSubImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 4U>(rinstr.low)) == 0b101010U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rn != 0b1111U);
  if (Rn == 0b1101U) {
    return SubSpMinusImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  if (d == 13U || d == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SbcImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSbcImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b01011U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert(!((d == 0xFU) && (S == 1U)));
  assert(n != 0xDU);
  if (d == 13U || (d == 15U && S == 0x0U) || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSbcImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddSpPlusImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddSpPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b10101U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u8 d = static_cast<u8>(Bm16::ExtractBits1R<10U, 8U>(rinstr.low));
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2U);

  return Ok(Instr{InstrAddSpPlusImmediate{iid, flags, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddSpPlusImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddSpPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 7U>(rinstr.low)) == 0b101100000U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u8 d = 13U;
  const u32 imm7 = Bm16::ExtractBits1R<6U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm7 << 2U);

  return Ok(Instr{InstrAddSpPlusImmediate{iid, flags, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddSpPlusImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddSpPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b01000U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1101U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert((Rd != 0b1111U) || (S != 1));
  if (d == 15U && S == 0x0U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddSpPlusImmediate{iid, flags, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddSpPlusImmediateT4Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddSpPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 0U>(rinstr.low)) == 0b1000001101U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (d == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddSpPlusImmediate{iid, flags, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b0001110U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u8 n = static_cast<u8>(Bm16::ExtractBits1R<5U, 3U>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::ExtractBits1R<2U, 0U>(rinstr.low));
  const u32 imm3 = Bm16::ExtractBits1R<8U, 6U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm3);

  return Ok(Instr{InstrAddImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddImmediate};
  u8 flags = 0x0U;

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u8 n = static_cast<u8>(Bm16::ExtractBits1R<10U, 8U>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::ExtractBits1R<10U, 8U>(rinstr.low));
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrAddImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b01000U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0x0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert(!((d == 0xFU) && (S == 1U)));
  if (Rn == 0b1101U) {
    return AddSpPlusImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  if (d == 13U || (d == 15U && S == 0x0U) || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddImmediateT4Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddImmediate};
  u8 flags = 0x0U;

  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0x0U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b10000U);
  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rn != 0b1111U);
  if (Rn == 0b1101U) {
    return AddSpPlusImmediateT4Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  if (d == 13U || d == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AdcImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAdcImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b01010U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0x0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);

  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  if (d == 13U || d == 15U || n == 13U || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAdcImmediate{iid, flags, n, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> TstImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kTstImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 4U>(rinstr.low)) == 0b000001U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::ExtractBits1R<11U, 8U>(rinstr.high)) == 0b1111U);

  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (n == 13U || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrTstImmediate{iid, flags, n, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> TeqImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kTeqImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 4U>(rinstr.low)) == 0b001001U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::ExtractBits1R<11U, 8U>(rinstr.high)) == 0b1111U);

  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (n == 13U || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrTeqImmediate{iid, flags, n, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> TstRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kTstRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100001000U);

  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};

  return Ok(Instr{InstrTstRegister{iid, flags, n, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> TeqRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kTeqRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111010101001U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::ExtractBits1R<11U, 8U>(rinstr.high)) == 0b1111U);

  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = static_cast<u32>(Bm16::ExtractBits1R<5U, 4U>(rinstr.high));
  const u32 imm2 = static_cast<u32>(Bm16::ExtractBits1R<7U, 6U>(rinstr.high));
  const u32 imm3 = static_cast<u32>(Bm16::ExtractBits1R<14U, 12U>(rinstr.high));
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);

  if (n == 13U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrTeqRegister{iid, flags, n, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AndImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAndImmediate};
  u8 flags = 0x0U;

  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert(((Bm16::ExtractBits1R<11U, 8U>(rinstr.high)) != 0b1111U) ||
         ((Bm16::IsolateBit<4U>(rinstr.low)) != 0b1U));
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b00000U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13U || (d == 15U && S == 0U) || n == 13U || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAndImmediate{iid, flags, n, d, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> OrrImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kOrrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b00010U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  assert(Rn != 0b1111U);
  if (d == 13U || d == 15U || n == 13U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrOrrImmediate{iid, flags, n, d, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> EorImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kEorImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11110U);
  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b00100U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13U || (d == 15U && (S == 0U)) || n == 13U || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrEorImmediate{iid, flags, n, d, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SubRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSubRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b0001101U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};
  const u8 m = static_cast<u8>(Bm16::ExtractBits1R<8U, 6U>(rinstr.low));
  const u8 n = static_cast<u8>(Bm16::ExtractBits1R<5U, 3U>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::ExtractBits1R<2U, 0U>(rinstr.low));

  return Ok(Instr{InstrSubRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SubRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSubRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101011101U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = static_cast<u32>(Bm16::IsolateBit<4U>(rinstr.low));
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u32 type = static_cast<u32>(Bm16::ExtractBits1R<5U, 4U>(rinstr.high));
  const u32 imm2 = static_cast<u32>(Bm16::ExtractBits1R<7U, 6U>(rinstr.high));
  const u32 imm3 = static_cast<u32>(Bm16::ExtractBits1R<14U, 12U>(rinstr.high));
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);

  assert((Rd != 0b1111U) || (S != 1U));
  assert(Rn != 0b1101U);
  if (d == 13U || (d == 15U && S == 0U) || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> RsbRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kRsbRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101011110U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = static_cast<u32>(Bm16::IsolateBit<4U>(rinstr.low));
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u32 type = static_cast<u32>(Bm16::ExtractBits1R<5U, 4U>(rinstr.high));
  const u32 imm2 = static_cast<u32>(Bm16::ExtractBits1R<7U, 6U>(rinstr.high));
  const u32 imm3 = static_cast<u32>(Bm16::ExtractBits1R<14U, 12U>(rinstr.high));
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);

  if (d == 13U || d == 15U || n == 13U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrRsbRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> UmlalT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kUmlal};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110111110U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0000U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 RdHi = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 dHi = static_cast<u8>(RdHi);
  const u16 RdLo = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 dLo = static_cast<u8>(RdLo);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (dLo == 13U || dLo == 15U || dHi == 13U || dHi == 15U || n == 13U || n == 15U || m == 13U ||
      m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (dHi == dLo) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUmlal{iid, flags, dHi, dLo, m, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> UmullT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kUmull};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110111010U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0000U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 RdHi = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 dHi = static_cast<u8>(RdHi);
  const u16 RdLo = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 dLo = static_cast<u8>(RdLo);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (dLo == 13U || dLo == 15U || dHi == 13U || dHi == 15U || n == 13U || n == 15U || m == 13U ||
      m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (dHi == dLo) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUmull{iid, flags, dHi, dLo, m, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SmullT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSmull};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110111000U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0000U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 RdHi = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 dHi = static_cast<u8>(RdHi);
  const u16 RdLo = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 dLo = static_cast<u8>(RdLo);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (dHi == dLo) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSmull{iid, flags, dHi, dLo, m, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MulT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMul};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110110000U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0000U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (d == 13U || d == 15U || n == 13U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMul{iid, flags, d, m, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> UdivT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kUdiv};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110111011U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b1111U);

  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (d == 13U || d == 15U || n == 13U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUdiv{iid, flags, d, m, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SdivT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSdiv};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110111001U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b1111U);

  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (d == 13U || d == 15U || n == 13U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSdiv{iid, flags, d, m, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MlsT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMls};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110110000U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0001U);

  const u16 Ra = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 a = static_cast<u8>(Ra);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (d == 13U || d == 15U || n == 13U || n == 15U || m == 13U || m == 15U || (a == 13U) ||
      (a == 15U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMls{iid, flags, a, d, m, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MlaT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMla};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110110000U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0000U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const u16 Ra = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 a = static_cast<u8>(Ra);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  assert(Ra != 0b1111U);
  if (d == 13U || d == 15U || n == 13U || n == 15U || m == 13U || m == 15U || (a == 13U) ||
      (a == 15U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMla{iid, flags, a, d, m, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b0001100U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};
  const u8 m = static_cast<u8>(Bm16::ExtractBits1R<8U, 6U>(rinstr.low));
  const u8 n = static_cast<u8>(Bm16::ExtractBits1R<5U, 3U>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::ExtractBits1R<2U, 0U>(rinstr.low));

  return Ok(Instr{InstrAddRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<10U, 8U>(rinstr.low)) == 0b100U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kSetFlags);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};
  const u8 m = static_cast<u8>(Bm16::ExtractBits1R<6U, 3U>(rinstr.low));
  const u32 dn_1 = Bm16::IsolateBit<7U>(rinstr.low);
  const u8 n = static_cast<u8>(dn_1 << 3U | Bm16::ExtractBits1R<2U, 0U>(rinstr.low));
  const u32 dn_2 = Bm16::IsolateBit<7U>(rinstr.low);
  const u8 d = static_cast<u8>(dn_2 << 3U | Bm16::ExtractBits1R<2U, 0U>(rinstr.low));

  assert(d != 0b1101U);
  assert(n != 0b1101U);
  if (d == 15U && TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (d == 15U && m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrAddRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AddRegisterT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAddRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101011000U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u32 type = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u32 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);

  assert((d != 0b1111U) || (S != 1U));
  assert(m != 0b1101U);
  if (d == 13U || (d == 15U && S == 0U) || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AdcRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAdcRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100000101U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(0U)};
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);

  return Ok(Instr{InstrAdcRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AdcRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAdcRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101011010U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u32 type = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u32 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);

  if (d == 13U || d == 15U || n == 13U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAdcRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StmdbT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStmdb};
  u8 flags = 0x0U;

  assert(false); // not implemented
  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b1110100100U);
  assert((Bm16::IsolateBit<4U>(rinstr.low)) == 0b0U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::IsolateBit<13U>(rinstr.high)) == 0b0U);

  const u32 W = Bm16::IsolateBit<5U>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 M = Bm16::IsolateBit<14U>(rinstr.high);
  const u32 register_list = Bm16::ExtractBits1R<12U, 0U>(rinstr.high);
  const u32 registers = (M << 14U) | register_list;

  assert((W != 0x1U) || (Rn != 0b1101U));
  if (n == 15 || (Bm32::BitCount(registers) < 2U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  // if wback && registers<n> == '1' then UNPREDICTABLE;
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStmdb{iid, flags, n, registers}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> PushT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kPush};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<10U, 9U>(rinstr.low)) == 0b10U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kUnalignedAllow);
  const u32 M = Bm16::IsolateBit<8U>(rinstr.low);
  const u32 register_list = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 registers = (M << 14U) | register_list;

  if (Bm32::BitCount(registers) < 1U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrPush{iid, flags, registers}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> PushT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kPush};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 0U>(rinstr.low)) == 0b1110100100101101U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::IsolateBit<13U>(rinstr.high)) == 0b0U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kUnalignedAllow);
  const u32 M = Bm16::IsolateBit<14U>(rinstr.high);
  const u32 register_list = Bm16::ExtractBits1R<12U, 0U>(rinstr.high);
  const u32 registers = (M << 14U) | register_list;

  if (Bm32::BitCount(registers) < 2U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrPush{iid, flags, registers}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> PushT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kPush};
  u8 flags = 0x0U;

  assert(rinstr.low == 0b1111100001001101U);
  assert((Bm16::ExtractBits1R<11U, 0U>(rinstr.high)) == 0b110100000100U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kUnalignedAllow);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 registers = 1U << Rt;

  if (t == 13U || t == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrPush{iid, flags, registers}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdmT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdm};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11001U);

  const u8 Rn_flag = static_cast<u8>(Bm16::ExtractBits1R<10U, 8U>(rinstr.low));
  const u16 register_list_flag = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u16 n_mask = 1U << Rn_flag;
  flags |= (register_list_flag & n_mask) == 0 ? static_cast<InstrFlagsSet>(InstrFlags::kWBack) : 0U;
  const u16 register_list = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 registers = static_cast<u32>(register_list);
  const u16 Rn = Bm16::ExtractBits1R<10U, 8U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (Bm32::BitCount(registers) < 1U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrLdm{iid, flags, registers, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdmT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdm};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b1110100010U);
  assert((Bm16::IsolateBit<4U>(rinstr.low)) == 0b1U);
  assert((Bm16::IsolateBit<13U>(rinstr.high)) == 0b0U);

  const u32 W = Bm16::IsolateBit<5U>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u32 P = Bm16::IsolateBit<15U>(rinstr.high);
  const u32 M = Bm16::IsolateBit<14U>(rinstr.high);
  const u32 register_list = Bm16::ExtractBits1R<12U, 0U>(rinstr.high);
  const u32 registers = (P << 15U) | (M << 14U) | register_list;
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  assert((W != 1) || (Rn != 0b1101U));
  if (n == 15 || (Bm32::BitCount(registers) < 2U) || ((P == 1U) && M == 1U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if ((Bm32::ExtractBits1R<15U, 15U>(registers) == 0x1U) && (TItOps::InITBlock(cpua)) &&
      (!TItOps::LastInITBlock(cpua))) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && ((registers & (1 << n)) != 0U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdm{iid, flags, registers, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StmT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStm};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b11000U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 register_list = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 registers = static_cast<u32>(register_list);
  const u16 Rn = Bm16::ExtractBits1R<10U, 8U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  return Ok(Instr{InstrStm{iid, flags, registers, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StmT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStm};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b1110100010U);
  assert((Bm16::IsolateBit<4U>(rinstr.low)) == 0b0U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::IsolateBit<13U>(rinstr.high)) == 0b0U);

  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kUnalignedAllow);
  const u32 W = Bm16::IsolateBit<5U>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u32 M = Bm16::IsolateBit<14U>(rinstr.high);
  const u32 register_list = Bm16::ExtractBits1R<12U, 0U>(rinstr.high);
  const u32 registers = (M << 14U) | register_list;
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (n == 15 || (Bm32::BitCount(registers) < 2U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && ((registers & (1 << n)) != 0U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStm{iid, flags, registers, n}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SxthT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSxth};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b1011001000U);

  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u8 rotation = 0U;

  return Ok(Instr{InstrSxth{iid, flags, m, d, rotation}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SxthT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSxth};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 0U>(rinstr.low)) == 0b1111101000001111U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 6U>(rinstr.high)) == 0b10U);

  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 rotate = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u8 rotation = static_cast<u8>(rotate << 3U);

  if (d == 13U || d == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSxth{iid, flags, m, d, rotation}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> UxtbT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kUxtb};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<11U, 6U>(rinstr.low)) == 0b001011U);

  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u8 rotation = 0U;

  return Ok(Instr{InstrUxtb{iid, flags, m, d, rotation}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> UxtbT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kUxtb};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 0U>(rinstr.low)) == 0b1111101001011111U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 6U>(rinstr.high)) == 0b10U);

  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 rotate = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u8 rotation = static_cast<u8>(rotate << 3U);

  if (d == 13U || d == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUxtb{iid, flags, m, d, rotation}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SxtbT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSxtb};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b1011001001U);

  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u8 rotation = 0U;

  return Ok(Instr{InstrSxtb{iid, flags, m, d, rotation}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SxtbT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSxtb};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 0U>(rinstr.low)) == 0b1111101001001111U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 6U>(rinstr.high)) == 0b10U);

  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 rotate = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u8 rotation = static_cast<u8>(rotate << 3U);

  if (d == 13U || d == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSxtb{iid, flags, m, d, rotation}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> UxthT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kUxth};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b1011001010U);

  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u8 rotation = 0U;

  return Ok(Instr{InstrUxth{iid, flags, m, d, rotation}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> UxthT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kUxth};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 0U>(rinstr.low)) == 0b1111101000011111U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 6U>(rinstr.high)) == 0b10U);

  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 rotate = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u8 rotation = rotate << 3U;

  if (d == 13U || d == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUxth{iid, flags, m, d, rotation}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BfiT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBfi};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111100110110U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::IsolateBit<5U>(rinstr.high)) == 0b0U);

  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm2 = static_cast<u32>(Bm16::ExtractBits1R<7U, 6U>(rinstr.high));
  const u32 imm3 = static_cast<u32>(Bm16::ExtractBits1R<14U, 12U>(rinstr.high));
  const u8 lsbit = static_cast<u8>((imm3 << 2U) | imm2);
  const u16 msb = Bm16::ExtractBits1R<4U, 0U>(rinstr.high);
  const u8 msbit = static_cast<u8>(msb);

  if (d == 13U || d == 15U || n == 13U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBfi{iid, flags, d, n, lsbit, msbit}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> UbfxT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kUbfx};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111100111100U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::IsolateBit<5U>(rinstr.high)) == 0b0U);

  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u8 lsbit = static_cast<u8>((imm3 << 2U) | imm2);
  const u32 widthm1 = Bm16::ExtractBits1R<4U, 0U>(rinstr.high);
  const u8 widthminus1 = static_cast<u8>(widthm1);

  if (d == 13U || d == 15U || n == 13U || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUbfx{iid, flags, d, n, lsbit, widthminus1}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> EorRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kEorRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100000001U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};

  return Ok(Instr{InstrEorRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> EorRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kEorRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010100U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u32 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2U) | imm2);

  assert((Rd != 0b1111U) || (S != 1));
  if (d == 13U || (d == 15U && (S == 0)) || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrEorRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SbcRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSbcRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101011011U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u32 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2U) | imm2);

  if (d == 13U || d == 15U || n == 13U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSbcRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> OrrRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kOrrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100001100U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};

  return Ok(Instr{InstrOrrRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> OrrRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kOrrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010010U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u32 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2U) | imm2);

  assert(Rn != 0b1111U);
  if (d == 13U || d == 15U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrOrrRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AndRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAndRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100000000U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};

  return Ok(Instr{InstrAndRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AndRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAndRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010000U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u32 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2U) | imm2);

  assert((Rd != 0b1111U) || (S != 0b1U));
  if (d == 13U || (d == 15U && (S == 0U)) || n == 13U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAndRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BicImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBicImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<9U, 5U>(rinstr.low)) == 0b00001U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10U>(rinstr.low);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm12 = (i << 11U) | (imm3 << 8U) | imm8;
  auto apsr = cpua.template ReadRegister<SpecialRegisterId::kApsr>();
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13U || d == 15U || n == 13U || n == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBicImmediate{iid, flags, n, d, imm32_carry}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BicRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBicRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100001110U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};

  return Ok(Instr{InstrBicRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BicRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBicRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010001U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const u32 imm2 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const u32 imm3 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2U) | imm2);

  if (d == 13U || d == 15U || n == 13U || n == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBicRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LsrRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLsrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100000011U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  return Ok(Instr{InstrLsrRegister{iid, flags, n, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LsrRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLsrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11111010001U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0000U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if (d == 13U || d == 15 || n == 13U || n == 15 || m == 13U || m == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLsrRegister{iid, flags, n, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AsrRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAsrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 6U>(rinstr.low)) == 0b0100000100U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  return Ok(Instr{InstrAsrRegister{iid, flags, n, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AsrRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAsrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11111010010U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1111U);
  assert((Bm16::ExtractBits1R<7U, 4U>(rinstr.high)) == 0b0000U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if (d == 13U || d == 15 || n == 13U || n == 15 || m == 13U || m == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAsrRegister{iid, flags, n, d, m}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LsrImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLsrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b00001U);

  flags |=
      TItOps::InITBlock(cpua) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0U;
  const u16 Rm = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm5 = Bm16::ExtractBits1R<10U, 6U>(rinstr.low);
  const auto shift_res = Alu32::DecodeImmShift(0b01U, imm5);

  return Ok(Instr{InstrLsrImmediate{iid, flags, m, d, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LsrImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLsrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010010U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1111U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::ExtractBits1R<5U, 4U>(rinstr.high)) == 0b01U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm3_32 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm2_32 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(0b01U, (imm3_32 << 2U) | imm2_32);

  assert(shift_res.type == SRType::SRType_LSR);
  if (d == 13U || d == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLsrImmediate{iid, flags, m, d, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MvnRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMvnRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010011U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1111U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm3 = static_cast<u32>(Bm16::ExtractBits1R<14U, 12U>(rinstr.high));
  const u32 imm2 = static_cast<u32>(Bm16::ExtractBits1R<7U, 6U>(rinstr.high));
  const u32 type = static_cast<u32>(Bm16::ExtractBits1R<5U, 4U>(rinstr.high));
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2U) | imm2);

  if (d == 13U || d == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMvnRegister{iid, flags, m, d, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> AsrImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kAsrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 5U>(rinstr.low)) == 0b11101010010U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) == 0b1111U);
  assert((Bm16::IsolateBit<15U>(rinstr.high)) == 0b0U);
  assert((Bm16::ExtractBits1R<5U, 4U>(rinstr.high)) == 0b10U);

  const u32 S = Bm16::IsolateBit<4U>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm3_32 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
  const u32 imm2_32 = Bm16::ExtractBits1R<7U, 6U>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(0b10U, (imm3_32 << 2U) | imm2_32);

  assert(shift_res.type == SRType::SRType_ASR);
  if (d == 13U || d == 15U || m == 13U || m == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAsrImmediate{iid, flags, m, d, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b0101000U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::ExtractBits1R<8U, 6U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};

  return Ok(Instr{InstrStrRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000100U);
  assert((Bm16::ExtractBits1R<11U, 6U>(rinstr.high)) == 0b000000U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  if (t == 15U || m == 13U || m == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrbRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrbRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b0101010U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::ExtractBits1R<8U, 6U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0U};

  return Ok(Instr{InstrStrbRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrbRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrbRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000000U);
  assert((Bm16::ExtractBits1R<11U, 6U>(rinstr.high)) == 0b000000U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  if (Rn == 0b1111U) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 13U || t == 15U || m == 13U || m == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrbRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrhRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrhRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000010U);
  assert((Bm16::ExtractBits1R<11U, 6U>(rinstr.high)) == 0b000000U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  if (Rn == 0b1111U) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 13U || t == 15U || m == 13U || m == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrhRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrbRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrbRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b0101110U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rm = Bm16::ExtractBits1R<8U, 6U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0x0U};

  return Ok(Instr{InstrLdrbRegister{iid, flags, m, n, t, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrRegisterT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b0101100U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rm = Bm16::ExtractBits1R<8U, 6U>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0x0U};

  return Ok(Instr{InstrLdrRegister{iid, flags, m, n, t, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000101U);
  assert((Bm16::ExtractBits1R<11U, 6U>(rinstr.high)) == 0b000000U);
  assert((Bm16::ExtractBits1R<3U, 0U>(rinstr.low)) != 0b1111U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  if (m == 13U || m == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (t == 15U && TItOps::InITBlock(cpua) && !TItOps::LastInITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrRegister{iid, flags, m, n, t, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> LdrhRegisterT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kLdrhRegister};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000011U);
  assert((Bm16::ExtractBits1R<11U, 6U>(rinstr.high)) == 0b000000U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rm = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  assert(Rn != 0b1111U);
  assert(Rt != 0b1111U);
  if (t == 13U || m == 13U || m == 15) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrhRegister{iid, flags, m, n, t, shift_res}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrdImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrdImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 9U>(rinstr.low)) == 0b1110100U);
  assert((Bm16::IsolateBit<6U>(rinstr.low)) == 0b1U);
  assert((Bm16::IsolateBit<4U>(rinstr.low)) == 0b0U);

  const u32 U = Bm16::IsolateBit<7U>(rinstr.low);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<8U>(rinstr.low);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<5U>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rt2 = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 t2 = static_cast<u8>(Rt2);
  const u32 imm8_32 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8_32 << 2U);

  assert((P != 0U) || (W != 0U));
  if (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && (n == t || n == t2)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if (n == 15U || t == 13U || t == 15U || t2 == 13U || t2 == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrdImmediate{iid, flags, n, t, t2, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrhImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b10000U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm5 = Bm16::ExtractBits1R<10U, 6U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5 << 1U);

  return Ok(Instr{InstrStrhImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrhImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110001010U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm12 = Bm16::ExtractBits1R<11U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (t == 13U || t == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrhImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrhImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000010U);
  assert((Bm16::IsolateBit<11U>(rinstr.high)) == 0b1U);

  const u32 U = Bm16::IsolateBit<9U>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10U>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8U>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert(P != 0b1U || U != 0b1U || W != 0b0U);
  if ((Rn == 0b1111U) || (P == 0b0 && W == 0b0U)) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if ((t == 13U || t == 15U) &&
      (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && n == t)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrhImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrbImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b01110U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm5 = Bm16::ExtractBits1R<10U, 6U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5);

  return Ok(Instr{InstrStrbImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrbImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110001000U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm12 = Bm16::ExtractBits1R<11U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (Rn == 0b1111U) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 13U || t == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrbImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrbImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000000U);
  assert((Bm16::IsolateBit<11U>(rinstr.high)) == 0b1U);

  const u32 U = Bm16::IsolateBit<9U>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10U>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8U>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert((P != 1U) || (U != 1) || (W != 0U));
  if (n == 15U || ((P == 0U) && (W == 0U))) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 13U || t == 15U || (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && n == t)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrbImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrImmediateT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b01100U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<5U, 3U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm5 = Bm16::ExtractBits1R<10U, 6U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5 << 2);

  return Ok(Instr{InstrStrImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrImmediateT2Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 11U>(rinstr.low)) == 0b10010U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u8 n = 13U;
  const u16 Rt = Bm16::ExtractBits1R<10U, 8U>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2U);

  return Ok(Instr{InstrStrImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrImmediateT3Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110001100U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm12 = Bm16::ExtractBits1R<11U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (n == 0b1111U) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 15U) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrImmediateT4Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111110000100U);
  assert((Bm16::IsolateBit<11U>(rinstr.high)) == 0b1U);

  const u32 U = Bm16::IsolateBit<9U>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10U>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8U>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm8_32 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8_32);

  assert((P != 1U) || (U != 1) || (W != 0U));
  if ((n == 13U) && (P == 1) && (U == 0U) && (W == 1U) && (imm8_32 == 0b00000100U)) {
    return PushT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  if (n == 15U || ((P == 0U) && (W == 0U))) {
    return Err<Instr>(StatusCode::kDecoderUndefined);
  }
  if (t == 15U || (flags & static_cast<InstrFlagsSet>(InstrFlags::kWBack) && n == t)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrImmediate{iid, flags, n, t, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> StrexT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kStrex};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111010000100U);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm8_32 = Bm16::ExtractBits1R<7U, 0U>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8_32);

  if (d == n || d == t) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrex{iid, flags, n, t, d, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> CbNZT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kCbNZ};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.low)) == 0b1011U);
  assert((Bm16::IsolateBit<10U>(rinstr.low)) == 0b0U);
  assert((Bm16::IsolateBit<8U>(rinstr.low)) == 0b1U);

  const u32 op = Bm16::IsolateBit<11U>(rinstr.low);
  flags |= op << static_cast<InstrFlagsSet>(InstrFlagsShift::kNonZeroShift);
  const u16 Rn = Bm16::ExtractBits1R<2U, 0U>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm5 = Bm16::ExtractBits1R<7U, 3U>(rinstr.low);
  const u32 i = Bm16::IsolateBit<9U>(rinstr.low);
  const i32 imm32 = Bm32::ZeroExtend<u32>((i << 6U) | (imm5 << 1U));

  if (TItOps::InITBlock(cpua)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }

  return Ok(Instr{InstrCbNZ{iid, flags, n, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> SvcT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kSvc};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 8U>(rinstr.low)) == 0b11011111U);

  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrSvc{iid, flags, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> BkptT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kBkpt};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 8U>(rinstr.low)) == 0b10111110U);

  const u32 imm8 = Bm16::ExtractBits1R<7U, 0U>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrBkpt{iid, flags, imm32}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MsrT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMsr};
  u8 flags = 0x0U;

  assert((Bm16::ExtractBits1R<15U, 4U>(rinstr.low)) == 0b111100111000U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1000U);
  assert((Bm16::ExtractBits1R<9U, 8U>(rinstr.high)) == 0b00U);

  const u8 n = static_cast<u8>(Bm16::ExtractBits1R<3U, 0U>(rinstr.low));
  const u8 mask = static_cast<u8>(Bm16::ExtractBits1R<11U, 10U>(rinstr.high));
  const u8 SYSm = static_cast<u8>(Bm16::ExtractBits1R<7U, 0U>(rinstr.high));

  if (mask == 0b00U || (mask != 0b10U && !(SYSm == 0U || SYSm == 1U || SYSm == 2U || SYSm == 3U))) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  if ((n == 13U) || (n == 15U) ||
      !(SYSm == 0U || SYSm == 1U || SYSm == 2U || SYSm == 3U || SYSm == 5U || SYSm == 6U ||
        SYSm == 7U || SYSm == 8U || SYSm == 9U || SYSm == 16U || SYSm == 17U || SYSm == 18U ||
        SYSm == 19U || SYSm == 20U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMsr{iid, flags, n, mask, SYSm}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> MrsT1Decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  static_cast<void>(cpua); // prevents warning when cpua is not used
  const InstrId iid{InstrId::kMrs};
  u8 flags = 0x0U;

  assert(rinstr.low == 0b1111001111101111U);
  assert((Bm16::ExtractBits1R<15U, 12U>(rinstr.high)) == 0b1000U);

  const u8 d = static_cast<u8>(Bm16::ExtractBits1R<11U, 8U>(rinstr.high));
  const u8 mask = static_cast<u8>(Bm16::ExtractBits1R<11U, 10U>(rinstr.high));
  const u8 SYSm = static_cast<u8>(Bm16::ExtractBits1R<7U, 0U>(rinstr.high));

  if ((d == 13U) || (d == 15U) ||
      !(SYSm == 0U || SYSm == 1U || SYSm == 2U || SYSm == 3U || SYSm == 5U || SYSm == 6U ||
        SYSm == 7U || SYSm == 8U || SYSm == 9U || SYSm == 16U || SYSm == 17U || SYSm == 18U ||
        SYSm == 19U || SYSm == 20U)) {
    return Err<Instr>(StatusCode::kDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMrs{iid, flags, d, mask, SYSm}});
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter16bit_op00000(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129
  //
  // # Shift (immediate), add, subtract, move, and compare
  const u16 opcode = Bm16::ExtractBits1R<13U, 9U>(rinstr.low);
  if (Bm16::ExtractBits1R<4U, 2U>(opcode) == 0b000U) {
    if (Bm16::ExtractBits1R<10U, 6U>(rinstr.low) == 0b00000U) {
      return MovRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else {
      return LslImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter16bit_op00011(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129
  //
  // # Shift (immediate), add, subtract, move, and compare
  //  see Armv7-M Architecture Reference Manual Issue E.e p130
  const u16 opcode = Bm16::ExtractBits1R<13U, 9U>(rinstr.low);
  if (opcode == 0b01100U) {
    return AddRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (opcode == 0b01110U) {
    return AddImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (opcode == 0b01101U) {
    return SubRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (opcode == 0b01111U) {
    return SubImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter16bit_op01010(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # Load/store single data item
  // see Armv7-M Architecture Reference Manual Issue E.e p133
  //
  const u16 op_a = Bm16::ExtractBits1R<15U, 12U>(rinstr.low);
  const u16 op_b = Bm16::ExtractBits1R<11U, 9U>(rinstr.low);
  if (op_a == 0b0101U && op_b == 0b000U) {
    return StrRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (op_a == 0b0101U && op_b == 0b010U) {
    return StrbRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter16bit_op10111(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # Miscellaneous 16-bit instructions
  // see Armv7-M Architecture Reference Manual Issue E.e p134
  //
  const u16 opcode = Bm16::ExtractBits1R<11U, 5U>(rinstr.low);
  if (Bm16::ExtractBits1R<6U, 3U>(opcode) == 0b1001U) {
    return CbNZT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 3U>(opcode) == 0b1011U) {
    return CbNZT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 4U>(opcode) == 0b110U) {
    return PopT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 3U>(opcode) == 0b1110U) {
    return BkptT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 3U>(opcode) == 0b1111U) {
    // # If-Then, and hints
    // see Armv7-M Architecture Reference Manual Issue E.e p135
    if (Bm16::ExtractBits1R<3U, 0U>(rinstr.low) != 0x0U) {
      return ItT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
    if ((Bm16::ExtractBits1R<3U, 0U>(rinstr.low) == 0x0U) &&
        (Bm16::ExtractBits1R<7U, 4U>(rinstr.low) == 0x0U)) {
      return NopT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter16bit_op10110(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129
  //
  // ## Miscellaneous 16-bit instructions
  // see Armv7-M Architecture Reference Manual Issue E.e p134
  const u16 opcode = Bm16::ExtractBits1R<11U, 5U>(rinstr.low);
  if (Bm16::ExtractBits1R<6U, 2U>(opcode) == 0b00000U) {
    return AddSpPlusImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 3U>(opcode) == 0b0001U) {
    return CbNZT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 2U>(opcode) == 0b00001U) {
    return SubSpMinusImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 3U>(opcode) == 0b0011U) {
    return CbNZT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 1U>(opcode) == 0b001001U) {
    return SxtbT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 1U>(opcode) == 0b001011U) {
    return UxtbT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 1U>(opcode) == 0b001010U) {
    return UxthT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 1U>(opcode) == 0b001000U) {
    return SxthT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (Bm16::ExtractBits1R<6U, 4U>(opcode) == 0b010U) {
    return PushT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter16bit_op01000(const RawInstr &rinstr, TCpuAccessor &cpua) {
  if (Bm16::IsolateBit<10U>(rinstr.low) == 0x1) {
    // ## Special data instructions and branch and exchange
    // see Armv7-M Architecture Reference Manual Issue E.e p132
    if (Bm16::ExtractBits1R<9U, 8U>(rinstr.low) == 0b00U) {
      return AddRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (Bm16::ExtractBits1R<9U, 8U>(rinstr.low) == 0b10U) {
      return MovRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (Bm16::ExtractBits1R<9U, 8U>(rinstr.low) == 0b01U) {
      return CmpRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (Bm16::ExtractBits1R<9U, 7U>(rinstr.low) == 0b110U) {
      return BxT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (Bm16::ExtractBits1R<9U, 7U>(rinstr.low) == 0b111U) {
      return BlxT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  } else {
    // ## Data processing
    // see Armv7-M Architecture Reference Manual Issue E.e p131
    const u16 _opcode = Bm16::ExtractBits1R<9U, 6U>(rinstr.low);
    if (_opcode == 0b0000U) {
      return AndRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b0010U) {
      return LslRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b0011U) {
      return LsrRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b0100U) {
      return AsrRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b0101U) {
      return AdcRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b0001U) {
      return EorRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b1000U) {
      return TstRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b1001U) {
      return RsbImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b1010U) {
      return CmpRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b1100U) {
      return OrrRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_opcode == 0b1110U) {
      return BicRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter16bit_op01011(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129

  const u16 opB = Bm16::ExtractBits1R<11U, 9U>(rinstr.low);
  // # Load/store single data item
  // see Armv7-M Architecture Reference Manual Issue E.e p133
  if (opB == 0b100U) {
    return LdrRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (opB == 0b110U) {
    return LdrbRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter16bit_op01101(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129

  const u16 opB = Bm16::ExtractBits1R<11U, 9U>(rinstr.low);
  // # Load/store single data item
  // see Armv7-M Architecture Reference Manual Issue E.e p133
  if (Bm16::ExtractBits1R<2U, 2U>(opB) == 0b1U) {
    return LdrImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter16bit_op11011(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129
  //
  // ## Conditional branch, and Supervisor Call
  // see Armv7-M Architecture Reference Manual Issue E.e p136
  const u16 opcode = Bm16::ExtractBits1R<11U, 8U>(rinstr.low);
  if (Bm16::ExtractBits1R<3U, 1U>(opcode) != 0b111U) {
    return BT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  } else if (opcode == 0b1111U) {
    return SvcT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter32bit_op11101(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # 32-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p137
  if (Bm16::ExtractBits1R<10U, 9U>(rinstr.low) == 0b01U) {
    // ## Data processing (shifted register)
    // see Armv7-M Architecture Reference Manual Issue E.e p150
    const u16 op = Bm16::ExtractBits1R<8U, 5U>(rinstr.low);
    const u16 Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
    const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
    const u16 S = Bm16::IsolateBit<4U>(rinstr.low);
    if ((op == 0b0000U) && (Rd != 0b1111U)) {
      return AndRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b0001U) {
      return BicRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b0011U && Rn == 0b1111U) {
      return MvnRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b1000U && Rd != 0b1111U) {
      return AddRegisterT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b1110U) {
      return RsbRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b0010U && Rn == 0b1111U) {
      // ### Move register and immediate shifts
      // see Armv7-M Architecture Reference Manual Issue E.e p151
      const u32 type = static_cast<u32>(Bm16::ExtractBits1R<5U, 4U>(rinstr.high));
      const u32 imm2 = static_cast<u32>(Bm16::ExtractBits1R<7U, 6U>(rinstr.high));
      const u32 imm3 = static_cast<u32>(Bm16::ExtractBits1R<14U, 12U>(rinstr.high));
      const u32 imm3_2 = imm3 << 2U | imm2;
      if (type == 0b00U && imm3_2 == 0b00000U) {
        return MovRegisterT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
      } else if (type == 0b01U) {
        return LsrImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
      } else if (type == 0b10U) {
        return AsrImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
      } else if (type == 0b00U && imm3_2 != 0b00000U) {
        return LslImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
      } else if (type == 0b11U && imm3_2 == 0b00000U) {
        return RrxT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
      }
    } else if (op == 0b0010U && Rn != 0b1111U) {
      return OrrRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b0100U && Rd != 0b1111U) {
      return EorRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b0100U && Rd == 0b1111U && S == 1) {
      return TeqRegisterT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b0100U && Rd == 0b1111U && S == 0) {
      return Err<Instr>(StatusCode::kDecoderUnpredictable);
    } else if (op == 0b1101U && Rd != 0b1111U) {
      return SubRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b1010U) {
      return AdcRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b1011U) {
      return SbcRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op == 0b1101U && Rd == 0b1111U) {
      return CmpRegisterT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  } else if ((Bm16::ExtractBits1R<10U, 9U>(rinstr.low) == 0b00U) &&
             (Bm16::IsolateBit<6U>(rinstr.low) == 0b0U)) {
    // ## Load Multiple and Store Multiple
    // see Armv7-M Architecture Reference Manual Issue E.e p144
    const u16 op = Bm16::ExtractBits1R<8U, 7U>(rinstr.low);
    const u16 W = Bm16::IsolateBit<5U>(rinstr.low);
    const u16 Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
    const u16 L = Bm16::IsolateBit<4U>(rinstr.low);
    const u16 W_Rn = (W << 4U) | Rn;
    if ((op == 0b10U) && (L == 0U) && (W_Rn == 0b11101U)) {
      return PushT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((op == 0b01U) && (L == 0U)) {
      return StmT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((op == 0b10U) && (L == 0U) && (W_Rn != 0b11101U)) {
      return StmdbT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((op == 0b01U) && (L == 1U) && (W_Rn != 0b11101U)) {
      return LdmT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((op == 0b01U) && (L == 1U) && (W_Rn == 0b11101U)) {
      return PopT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  } else if ((Bm16::ExtractBits1R<10U, 9U>(rinstr.low) == 0b00U) &&
             (Bm16::IsolateBit<6U>(rinstr.low) == 0b1U)) {
    // ## Load/store dual or exclusive, table branch
    // see Armv7-M Architecture Reference Manual Issue E.e p145
    const u16 op1 = Bm16::ExtractBits1R<8U, 7U>(rinstr.low);
    const u16 op2 = Bm16::ExtractBits1R<5U, 4U>(rinstr.low);
    const u16 op3 = Bm16::ExtractBits1R<7U, 4U>(rinstr.high);
    if (op1 == 0b00U && op2 == 0b00U) {
      return StrexT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (op1 == 0b00U && op2 == 0b01U) {
      return LdrexT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<1U, 1U>(op1) == 0b0U) && (op2 == 0b10U)) {
      return StrdImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<1U, 1U>(op1) == 0b1U) &&
               (Bm16::ExtractBits1R<0U, 0U>(op2) == 0b0U)) {
      return StrdImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<1U, 1U>(op1) == 0b0U) && (op2 == 0b11U)) {
      return LdrdImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<1U, 1U>(op1) == 0b1U) &&
               (Bm16::ExtractBits1R<0U, 0U>(op2) == 0b1U)) {
      return LdrdImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((op1 == 0b01U) && (op2 == 0b01U) && (op3 == 0b0000U)) {
      return TbbHT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((op1 == 0b01U) && (op2 == 0b01U) && (op3 == 0b0001U)) {
      return TbbHT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter32bit_op11110(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # 32-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p137
  const u16 op1 = Bm16::ExtractBits1R<12U, 11U>(rinstr.low);
  const u16 op2 = Bm16::ExtractBits1R<10U, 4U>(rinstr.low);
  const u16 op = Bm16::IsolateBit<15U>(rinstr.high);
  if ((op1 == 0b10U) && (op == 0b1U)) {
    // ## Branches and miscellaneous control
    // see Armv7-M Architecture Reference Manual Issue E.e p142
    const u16 _op1 = Bm16::ExtractBits1R<14U, 12U>(rinstr.high);
    const u16 _op = Bm16::ExtractBits1R<10U, 4U>(rinstr.low);
    if ((_op1 == 0b000U || _op1 == 0b010U) && (Bm16::ExtractBits1R<5U, 3U>(_op) != 0b111U)) {
      return BT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((_op1 == 0b000U || _op1 == 0b010U) &&
               (Bm16::ExtractBits1R<6U, 1U>(_op) == 0b011100U)) {
      return MsrT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((_op1 == 0b000U || _op1 == 0b010U) &&
               (Bm16::ExtractBits1R<6U, 1U>(_op) == 0b011111U)) {
      return MrsT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b001U || _op1 == 0b011U) { // attention the x in the docs does not mean hex
      return BT4Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b101U || _op1 == 0b111U) {
      return BlT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((_op1 == 0b000U || _op1 == 0b010U) && (_op == 0b0111011U)) {
      // ### Miscellaneous control instructions
      // see Armv7-M Architecture Reference Manual Issue E.e p143
      const u16 _opc = Bm16::ExtractBits1R<7U, 4U>(rinstr.high);
      // const u16 _option = Bm16::ExtractBits1R<3U, 0U>(rinstr.high);
      if (_opc == 0b0101U) {
        return DmbT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
      }
    }
  } else if ((op1 == 0b10U) && (Bm16::IsolateBit<5U>(op2) == 0b0U) && (op == 0x0U)) {
    // ## Data processing (modified immediate)
    // see Armv7-M Architecture Reference Manual Issue E.e p138
    const u16 _op = Bm16::ExtractBits1R<8U, 4U>(rinstr.low);
    const u16 _Rd = Bm16::ExtractBits1R<11U, 8U>(rinstr.high);
    const u16 _Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
    if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b0000U) && _Rd == 0b1111U) {
      return TstImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b0000U) && _Rd != 0b1111U) {
      return AndImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b0010U) && _Rn != 0b1111U) {
      return OrrImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b0010U) && _Rn == 0b1111U) {
      return MovImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b0011U) && _Rn == 0b1111U) {
      return MvnImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b0100U) && _Rd != 0b1111U) {
      return EorImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b0100U) && _Rd == 0b1111U) {
      return TeqImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (Bm16::ExtractBits1R<4U, 1U>(_op) == 0b0001U) {
      return BicImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b1000U) && _Rd != 0b1111U) {
      return AddImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b1000U) && _Rd == 0b1111U) {
      return CmnImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (Bm16::ExtractBits1R<4U, 1U>(_op) == 0b1010U) {
      return AdcImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b1101U) && _Rd != 0b1111U) {
      return SubImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<4U, 1U>(_op) == 0b1101U) && _Rd == 0b1111U) {
      return CmpImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (Bm16::ExtractBits1R<4U, 1U>(_op) == 0b1011U) {
      return SbcImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (Bm16::ExtractBits1R<4U, 1U>(_op) == 0b1110U) {
      return RsbImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  } else if ((op1 == 0b10U) && (Bm16::IsolateBit<5U>(op2) == 0b1U) && (op == 0x0U)) {
    // ## Data processing (plain binary immediate)
    // see Armv7-M Architecture Reference Manual Issue E.e p141
    const u16 _op = Bm16::ExtractBits1R<8U, 4U>(rinstr.low);
    const u16 _Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
    if (_op == 0b00100U) {
      return MovImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op == 0b00000U && _Rn != 0b1111U) {
      return AddImmediateT4Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op == 0b01010U && _Rn != 0b1111U) {
      return SubImmediateT4Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op == 0b10110U && _Rn != 0b1111U) {
      return BfiT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op == 0b11100U) {
      return UbfxT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}
template <typename TCpuAccessor, typename TItOps>
static Result<Instr> Splitter32bit_op11111(const RawInstr &rinstr, TCpuAccessor &cpua) {
  // # 32-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p137
  // op1 is set by jump table
  const u16 op2 = Bm16::ExtractBits1R<10U, 4U>(rinstr.low);
  if ((Bm16::ExtractBits1R<6U, 4U>(op2) == 0b000U) && (Bm16::ExtractBits1R<0U, 0U>(op2) == 0b0U)) {
    // ## Store single data item
    // see Armv7-M Architecture Reference Manual Issue E.e p149
    const u16 _op1 = Bm16::ExtractBits1R<7U, 5U>(rinstr.low);
    const u16 _op2 = Bm16::ExtractBits1R<11U, 6U>(rinstr.high);
    if (_op1 == 0b100U) {
      return StrbImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b101U) {
      return StrhImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b001U && Bm16::IsolateBit<5U>(_op2) == 0b1U) {
      return StrhImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b000U && Bm16::IsolateBit<5U>(_op2) == 0b1U) {
      return StrbImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b010U && Bm16::IsolateBit<5U>(_op2) == 0b1U) {
      return StrImmediateT4Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b010U && Bm16::IsolateBit<5U>(_op2) == 0b0U) {
      return StrRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b001U && Bm16::IsolateBit<5U>(_op2) == 0b0U) {
      return StrhRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b000U && Bm16::IsolateBit<5U>(_op2) == 0b0U) {
      return StrbRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b110U) {
      return StrImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  } else if ((Bm16::ExtractBits1R<6U, 5U>(op2) == 0b00U) &&
             (Bm16::ExtractBits1R<2U, 0U>(op2) == 0b001U)) {
    // ## Load byte, memory hints
    // see Armv7-M Architecture Reference Manual Issue E.e p148
    const u16 _op1 = Bm16::ExtractBits1R<8U, 7U>(rinstr.low);
    const u16 _op2 = Bm16::ExtractBits1R<11U, 6U>(rinstr.high);
    const u16 _Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
    const u16 _Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
    if (_op1 == 0b01U && _Rn != 0b1111 && _Rt != 0b1111U) {
      return LdrbImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b00U && (Bm16::IsolateBit<5U>(_op2) == 0b1U) &&
               (Bm16::ExtractBits1R<2U, 2U>(_op2) == 0b1U) && _Rn != 0b1111 && _Rt != 0b1111U) {
      return LdrbImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b00U && Bm16::ExtractBits1R<5U, 2U>(_op2) == 0b1100U && _Rn != 0b1111 &&
               _Rt != 0b1111U) {
      return LdrbImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b11U && _Rn != 0b1111 && _Rt != 0b1111U) {
      return LdrsbImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  } else if ((Bm16::ExtractBits1R<6U, 5U>(op2) == 0b00U) &&
             (Bm16::ExtractBits1R<2U, 0U>(op2) == 0b011U)) {
    // ## Load halfword, memory hints
    // see Armv7-M Architecture Reference Manual Issue E.e p147
    const u16 _op1 = Bm16::ExtractBits1R<8U, 7U>(rinstr.low);
    const u16 _op2 = Bm16::ExtractBits1R<11U, 6U>(rinstr.high);
    const u16 _Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
    const u16 _Rt = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
    if (_op1 == 0b01U && _Rn != 0b1111U && _Rt != 0b1111U) {
      return LdrhImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b00U && (_op2 == 0b000000U) && _Rn != 0b1111U && _Rt != 0b1111U) {
      return LdrhRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b11U && _Rn != 0b1111U && _Rt != 0b1111U) {
      return LdrshImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b00U && Bm16::ExtractBits1R<2U, 2U>(_op2) == 0b1U &&
               Bm16::IsolateBit<5U>(_op2) == 0b1U && _Rn != 0b1111U && _Rt != 0b1111U) {
      return LdrhImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b00U && Bm16::ExtractBits1R<5U, 2U>(_op2) == 0b1100U && _Rn != 0b1111U &&
               _Rt != 0b1111U) {
      return LdrhImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b10U && Bm16::ExtractBits1R<2U, 2U>(_op2) == 0b1U &&
               Bm16::IsolateBit<5U>(_op2) == 0b1U && _Rn != 0b1111U && _Rt != 0b1111U) {
      return LdrshImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b10U && Bm16::ExtractBits1R<5U, 2U>(_op2) == 0b1100U && _Rn != 0b1111U &&
               _Rt != 0b1111U) {
      return LdrshImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  } else if ((Bm16::ExtractBits1R<6U, 5U>(op2) == 0b00U) &&
             (Bm16::ExtractBits1R<2U, 0U>(op2) == 0b101U)) {
    // ## Load word
    // see Armv7-M Architecture Reference Manual Issue E.e p146
    const u16 _op1 = Bm16::ExtractBits1R<8U, 7U>(rinstr.low);
    const u16 _op2 = Bm16::ExtractBits1R<11U, 6U>(rinstr.high);
    const u16 _Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
    if ((Bm16::ExtractBits1R<1U, 1U>(_op1) == 0b0U) && _Rn == 0b1111U) {
      return LdrLiteralT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b01U && _Rn != 0b1111U) {
      return LdrImmediateT3Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b00U && Bm16::IsolateBit<5U>(_op2) == 0b1U &&
               (Bm16::ExtractBits1R<2U, 2U>(_op2) == 0b1U) && _Rn != 0b1111U) {
      return LdrImmediateT4Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b00U && Bm16::ExtractBits1R<5U, 2U>(_op2) == 0b1100U && _Rn != 0b1111U) {
      return LdrImmediateT4Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b00U && _op2 == 0b000000U && _Rn != 0b1111U) {
      return LdrRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  } else if (Bm16::ExtractBits1R<6U, 4U>(op2) == 0b010U) {
    // ## Data processing (register)
    // see Armv7-M Architecture Reference Manual Issue E.e p152
    const u16 _op1 = Bm16::ExtractBits1R<7U, 4U>(rinstr.low);
    const u16 _op2 = Bm16::ExtractBits1R<7U, 4U>(rinstr.high);
    const u16 _Rn = Bm16::ExtractBits1R<3U, 0U>(rinstr.low);
    if ((Bm16::ExtractBits1R<3U, 1U>(_op1) == 0b000U) && _op2 == 0b0000U) {
      return LslRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<3U, 1U>(_op1) == 0b001U) && _op2 == 0b0000U) {
      return LsrRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<3U, 1U>(_op1) == 0b010U) && _op2 == 0b0000U) {
      return AsrRegisterT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b0001U && (Bm16::ExtractBits1R<3U, 3U>(_op2) == 0b1U) && _Rn == 0b1111U) {
      return UxthT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b0100U && (Bm16::ExtractBits1R<3U, 3U>(_op2) == 0b1U) && _Rn == 0b1111U) {
      return SxtbT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b0000U && (Bm16::ExtractBits1R<3U, 3U>(_op2) == 0b1U) && _Rn == 0b1111U) {
      return SxthT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b0101U && (Bm16::ExtractBits1R<3U, 3U>(_op2) == 0b1U) && _Rn == 0b1111U) {
      return UxtbT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if ((Bm16::ExtractBits1R<3U, 2U>(_op1) == 0b10U) &&
               (Bm16::ExtractBits1R<3U, 2U>(_op2) == 0b10U)) {
      // ## Miscellaneous operations
      // see Armv7-M Architecture Reference Manual Issue E.e p155
      const u32 __op1 = Bm16::ExtractBits1R<5U, 4U>(rinstr.low);
      const u32 __op2 = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
      if ((__op1 == 0b11U) && (__op2 == 0b00U)) {
        return ClzT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
      }
    }
  } else if (Bm16::ExtractBits1R<6U, 3U>(op2) == 0b0110U) {
    // ## Multiply, multiply accumulate, and absolute difference
    // see Armv7-M Architecture Reference Manual Issue E.e p156
    const u16 _op1 = Bm16::ExtractBits1R<6U, 4U>(rinstr.low);
    const u16 _op2 = Bm16::ExtractBits1R<5U, 4U>(rinstr.high);
    const u16 _Ra = Bm16::ExtractBits1R<15U, 12U>(rinstr.high);
    if (_op1 == 0b000U && _op2 == 0b00U && _Ra == 0b1111U) {
      return MulT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b000U && _op2 == 0b00U && _Ra != 0b1111U) {
      return MlaT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b000U && _op2 == 0b01U) {
      return MlsT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  } else if (Bm16::ExtractBits1R<6U, 3U>(op2) == 0b0111U) {
    // ## Long multiply, long multiply accumulate, and divide
    // see Armv7-M Architecture Reference Manual Issue E.e p156
    const u16 _op1 = Bm16::ExtractBits1R<6U, 4U>(rinstr.low);
    const u16 _op2 = Bm16::ExtractBits1R<7U, 4U>(rinstr.high);
    if (_op1 == 0b010U && _op2 == 0b0000U) {
      return UmullT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b000U && _op2 == 0b0000U) {
      return SmullT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b011U && _op2 == 0b1111U) {
      return UdivT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b001U && _op2 == 0b1111U) {
      return SdivT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    } else if (_op1 == 0b110U && _op2 == 0b0000U) {
      return UmlalT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
    }
  }
  return Err<Instr>(StatusCode::kDecoderUnknownOpCode);
}

template <typename TCpuAccessor, typename TItOps>
static Result<Instr> call_decoder(const RawInstr &rinstr, TCpuAccessor &cpua) {
  const u16 opc = Bm32::ExtractBits1R<kDecodersOpCodeLast, kDecodersOpCodeFirst>(rinstr.low);
  switch (opc) {
  case 0b00000U: {
    return Splitter16bit_op00000<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b00001U: {
    return LsrImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b00010U: {
    return AsrImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b00011U: {
    return Splitter16bit_op00011<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b00100U: {
    return MovImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b00101U: {
    return CmpImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b00110U: {
    return AddImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b00111U: {
    return SubImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b01000U: {
    return Splitter16bit_op01000<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b01001U: {
    return LdrLiteralT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b01010U: {
    return Splitter16bit_op01010<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b01011U: {
    return Splitter16bit_op01011<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b01100U: {
    return StrImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b01101U: {
    return Splitter16bit_op01101<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b01110U: {
    return StrbImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b01111U: {
    return LdrbImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b10000U: {
    return StrhImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b10001U: {
    return LdrhImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b10010U: {
    return StrImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b10011U: {
    return LdrImmediateT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b10100U: {
    return AddPcPlusImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b10101U: {
    return AddSpPlusImmediateT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b10110U: {
    return Splitter16bit_op10110<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b10111U: {
    return Splitter16bit_op10111<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b11000U: {
    return StmT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b11001U: {
    return LdmT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b11010U: {
    return BT1Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b11011U: {
    return Splitter16bit_op11011<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b11100U: {
    return BT2Decoder<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b11101U: {
    return Splitter32bit_op11101<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b11110U: {
    return Splitter32bit_op11110<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  case 0b11111U: {
    return Splitter32bit_op11111<TCpuAccessor, TItOps>(rinstr, cpua);
  }
  default: {
    assert(false);
    // should not happen
    break;
  }
  }
  return InvalidInstrDecoder<TCpuAccessor, TItOps>(rinstr, cpua);
};

} // namespace libmicroemu::internal