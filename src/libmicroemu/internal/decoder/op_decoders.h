#pragma once

#include "libmicroemu/internal/decoder/instr.h"
#include "libmicroemu/internal/fetcher/raw_instr.h"
#include "libmicroemu/internal/logic/thumb.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"
#include <assert.h>
#include <cstdint>

namespace microemu {
namespace internal {

using Bm8 = BitManip<u8>;
using Bm16 = BitManip<u16>;
using Bm32 = BitManip<u32>;

static constexpr u32 kDecodersOpCodeLast = 15U;
static constexpr u32 kDecodersOpCodeFirst = 11U;

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> InvalidInstrDecoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(rinstr);
  static_cast<void>(pstates); // prevents warning when pstate is not used
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> NopT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kNop};
  u8 flags = 0x0U;

  static_cast<void>(rinstr);

  assert(rinstr.low == 0b1011111100000000u);

  return Ok(Instr{InstrNop{
      iid,
      flags,
  }});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> DmbT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kDmb};
  u8 flags = 0x0U;

  assert(rinstr.low == 0b1111001110111111u);
  assert((Bm16::Slice1R<15u, 4u>(rinstr.high)) == 0b100011110101u);
  static_cast<void>(rinstr);

  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrDmb{
      iid,
      flags,
  }});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddPcPlusImmediateT1Decoder(const RawInstr &rinstr,
                                                 TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddPcPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b10100u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  const u16 Rd = Bm16::Slice1R<10u, 8u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2u);

  return Ok(Instr{InstrAddPcPlusImmediate{iid, flags, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LslImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLslImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b00000u);
  assert((Bm16::Slice1R<10u, 6u>(rinstr.low)) != 0x0u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 imm5 = Bm16::Slice1R<10u, 6u>(rinstr.low);
  const auto shift_res = Alu32::DecodeImmShift(0b00, imm5);

  assert(shift_res.type == SRType::SRType_LSL);

  return Ok(Instr{InstrLslImmediate{iid, flags, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LslImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLslImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010010u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1111u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::Slice1R<5u, 4u>(rinstr.high)) == 0b00u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u16 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(0b00u, (imm3 << 2u) | imm2);

  assert(shift_res.type == SRType::SRType_LSL);
  assert(((imm3 << 2u) | imm2) != 0b0u);
  if (d == 13u || d == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLslImmediate{iid, flags, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LslRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLslRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100000010u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  return Ok(Instr{InstrLslRegister{iid, flags, n, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LslRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLslRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11111010000u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0000u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if (d == 13u || d == 15 || n == 13u || n == 15 || m == 13u || m == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLslRegister{iid, flags, n, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> ClzT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kClz};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110101011u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b1000u);

  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm_1 = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u16 Rm_2 = Bm16::Slice1R<3u, 0u>(rinstr.high);
  if (Rm_1 != Rm_2) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  const u8 m = static_cast<u8>(Rm_1);

  if (d == 13u || d == 15 || m == 13u || m == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrClz{iid, flags, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AsrImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAsrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b00010u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm5 = Bm16::Slice1R<10u, 6u>(rinstr.low);
  const auto shift_res = Alu32::DecodeImmShift(0b10u, imm5);

  return Ok(Instr{InstrAsrImmediate{iid, flags, m, d, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> CmpRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kCmpRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<10u, 6u>(rinstr.low)) == 0b01010u);

  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0u};

  return Ok(Instr{InstrCmpRegister{iid, flags, n, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> CmpRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kCmpRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<10u, 8u>(rinstr.low)) == 0b101u);

  const u16 N = Bm16::IsolateBit<7u>(rinstr.low);
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(N << 3u | Rn);
  const u16 Rm = Bm16::Slice1R<6u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0u};

  if (n < 8u && m < 8u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (n == 15u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrCmpRegister{iid, flags, n, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> CmpRegisterT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kCmpRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111010111011u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::Slice1R<11u, 8u>(rinstr.high)) == 0b1111u);

  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 type = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u16 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const u16 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);

  if (n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrCmpRegister{iid, flags, n, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> CmpImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kCmpImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b00101u);

  const u16 Rn = Bm16::Slice1R<10u, 8u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrCmpImmediate{iid, flags, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> CmpImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kCmpImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 4u>(rinstr.low)) == 0b011011u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::Slice1R<11u, 8u>(rinstr.high)) == 0b1111u);

  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm((i << 11u) | (imm3 << 8u) | imm8));

  if (n == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrCmpImmediate{iid, flags, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> CmnImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kCmnImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 4u>(rinstr.low)) == 0b010001u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::Slice1R<11u, 8u>(rinstr.high)) == 0b1111u);

  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm((i << 11u) | (imm3 << 8u) | imm8));

  if (n == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrCmnImmediate{iid, flags, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MovImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMovImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b00100u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rd = Bm16::Slice1R<10u, 8u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  auto apsr = TSpecRegOps::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
  const u16 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const auto imm32_carry =
      ThumbImmediateResult{imm8, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk};

  return Ok(Instr{InstrMovImmediate{iid, flags, d, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MovImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMovImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b00010u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1111u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  auto apsr = TSpecRegOps::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13u || d == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMovImmediate{iid, flags, d, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MovImmediateT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMovImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 4u>(rinstr.low)) == 0b100100u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm4 = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const auto imm32_carry =
      ThumbImmediateResult{(imm4 << 12u) | (i << 11u) | (imm3 << 8u) | imm8, false};

  if (d == 13u || d == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMovImmediate{iid, flags, d, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MvnImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMvnImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b00011u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1111u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  auto apsr = TSpecRegOps::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13u || d == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMvnImmediate{iid, flags, d, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> TbbHT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kTbbH};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111010001101u);
  assert((Bm16::Slice1R<15u, 5u>(rinstr.high)) == 0b11110000000u);

  const u32 H = Bm16::IsolateBit<4u>(rinstr.high);
  flags |= H << static_cast<InstrFlagsSet>(InstrFlagsShift::kTbhShift);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (n == 13u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrTbbH{iid, flags, m, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> RsbImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kRsbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100001001u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm32 = 0x0u;

  return Ok(Instr{InstrRsbImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> RsbImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kRsbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b01110u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  if (d == 13u || d == 15u || n == 13u || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrRsbImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MovRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMovRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<11u, 8u>(rinstr.low)) == 0x6u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 Rd = Bm32::Slice2R<7u, 7u, 2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<6u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  if (d == 15u && TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrMovRegister{iid, flags, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MovRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMovRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0000000000u);

  flags |= InstrFlags::kSetFlags;
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  if (TItOps::InITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrMovRegister{iid, flags, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MovRegisterT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMovRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010010u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1111u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b0000u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0000u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if ((flags & InstrFlags::kSetFlags != 0u) && (d == 13u || d == 15u || m == 13u || m == 15u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if ((flags & InstrFlags::kSetFlags == 0u) && (d == 15u || m == 15u || (d == 13u && m == 13u))) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMovRegister{iid, flags, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> RrxT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kRrx};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010010u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1111u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b0000u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0011u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if ((d == 13u) || (d == 15u) || (m == 13u) || (m == 15u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrRrx{iid, flags, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrLiteralT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrLiteral};
  u8 flags = 0x0U;

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  const u16 Rt = Bm16::Slice1R<10u, 8u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u16 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8) << 2u;

  return Ok(Instr{InstrLdrLiteral{iid, flags, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrLiteralT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrLiteral};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 8u>(rinstr.low)) == 0b11111000u);
  assert((Bm16::Slice1R<6u, 0u>(rinstr.low)) == 0b1011111u);

  const u32 U = Bm16::IsolateBit<7u>(rinstr.low);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm12 = Bm16::Slice1R<11u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (t == 15u && TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrLiteral{iid, flags, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrbImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b01111u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~InstrFlags::kWBack;
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm5 = Bm16::Slice1R<10u, 6u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5);

  return Ok(Instr{InstrLdrbImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrbImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110001001u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~InstrFlags::kWBack;
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::Slice1R<11u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rt != 0b1111u);
  assert(Rn != 0b1111u);
  if (t == 13u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrbImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrbImmediateT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000001u);
  assert((Bm16::IsolateBit<11u>(rinstr.high)) == 0b1u);

  const u32 U = Bm16::IsolateBit<9u>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10u>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8u>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert((Rt != 0b1111u) || (P != 1u) || (U != 0u) || (W != 0u));
  assert(Rn != 0b1111u);
  assert((P != 0b1u) || (U != 0b1u) || (W != 0b0u));
  assert((Rn != 0b1101u) || (P != 0x0u) || (U != 0x1u) || (W != 0x1u) || (imm8 != 0b00000100u));
  if ((P == 0x0u) && (W == 0x0u)) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 13u || (flags & InstrFlags::kWBack && n == t)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (t == 15u && ((P == 0u) || (U == 1u) || (W == 1u))) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrbImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrsbImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrsbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110011001u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~InstrFlags::kWBack;
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::Slice1R<11u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rt != 0b1111u);
  assert(Rn != 0b1111u);
  if (t == 13u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrsbImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrsbImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrsbImmediate};
  u8 flags = 0x0U;

  assert(false); // not tested
  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110010001u);
  assert((Bm16::IsolateBit<11u>(rinstr.high)) == 0b1u);

  const u32 U = Bm16::IsolateBit<9u>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10u>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8u>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert((Rt != 0b1111u) || (P != 1u) || (U != 0u) || (W != 0u));
  assert(Rn != 0b1111u);
  assert((P != 0b1u) || (U != 0b1u) || (W != 0b0u));
  assert((Rn != 0b1101u) || (P != 0x0u) || (U != 0x1u) || (W != 0x1u) || (imm8 != 0b00000100u));
  if ((P == 0x0u) && (W == 0x0u)) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 13u || (t == 15u && (W == 1u)) || (flags & InstrFlags::kWBack && n == t)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrsbImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrshImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrshImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110011011u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::Slice1R<11u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rn != 0b1111u);
  assert(Rt != 0b1111u);
  if (t == 13u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrshImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrshImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrshImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110010011u);
  assert((Bm16::IsolateBit<11u>(rinstr.high)) == 0b1u);

  const u32 U = static_cast<u32>(Bm16::IsolateBit<9u>(rinstr.high));
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = static_cast<u32>(Bm16::IsolateBit<10u>(rinstr.high));
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = static_cast<u32>(Bm16::IsolateBit<8u>(rinstr.high));
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = static_cast<u32>(Bm16::Slice1R<7u, 0u>(rinstr.high));
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert(Rn != 0b1111u);
  assert(Rt != 0b1111u || P != 1u || U != 0u || W != 0u);
  assert(P != 0b1u || U != 0b1u || W != 0b0u);
  if (P == 0b0u && W == 0b0u) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 13u || (t == 15u && W == 1u) || (flags & InstrFlags::kWBack && n == t)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrshImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrhImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b10001u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~InstrFlags::kWBack;
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm5 = Bm16::Slice1R<10u, 6u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5 << 1u);

  return Ok(Instr{InstrLdrhImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrhImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110001011u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= ~InstrFlags::kWBack;
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::Slice1R<11u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rt != 0b1111u);
  assert(Rn != 0b1111u);
  if (t == 13u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrhImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrhImmediateT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000011u);
  assert((Bm16::IsolateBit<11u>(rinstr.high)) == 0b1u);

  const u32 U = static_cast<u32>(Bm16::IsolateBit<9u>(rinstr.high));
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = static_cast<u32>(Bm16::IsolateBit<10u>(rinstr.high));
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = static_cast<u32>(Bm16::IsolateBit<8u>(rinstr.high));
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = static_cast<u32>(Bm16::Slice1R<7u, 0u>(rinstr.high));
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert(Rn != 0b1111u);
  assert(Rt != 0b1111u || P != 1u || U != 0u || W != 0u);
  assert(P != 0b1u || U != 0b1u || W != 0b0u);
  if (P == 0b0u && W == 0b0u) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 13u || (t == 15u && W == 1u) || (flags & InstrFlags::kWBack && n == t)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrhImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> PopT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kPop};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b1011110u);

  flags &= ~InstrFlags::kUnalignedAllow;
  const u32 P = Bm16::IsolateBit<8u>(rinstr.low);
  const u32 register_list = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 registers = (P << 15u) | register_list;

  if (Bm32::BitCount(registers) < 1u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if ((Bm32::Slice1R<15u, 15u>(registers) == 0x1u) && (TItOps::InITBlock(pstates)) &&
      (!TItOps::LastInITBlock(pstates))) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrPop{iid, flags, registers}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> PopT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kPop};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 0u>(rinstr.low)) == 0b1110100010111101u);
  assert((Bm16::IsolateBit<13u>(rinstr.high)) == 0b0u);

  flags &= ~InstrFlags::kUnalignedAllow;
  const u32 P = Bm16::IsolateBit<15u>(rinstr.high);
  const u32 M = Bm16::IsolateBit<14u>(rinstr.high);
  const u32 register_list = Bm16::Slice1R<12u, 0u>(rinstr.high);
  const u32 registers = (P << 15u) | (M << 14u) | register_list;

  if ((Bm32::BitCount(registers) < 2u) || ((P == 1u) && M == 1u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if ((Bm32::Slice1R<15u, 15u>(registers) == 0x1u) && (TItOps::InITBlock(pstates)) &&
      (!TItOps::LastInITBlock(pstates))) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrPop{iid, flags, registers}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> PopT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kPop};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 0u>(rinstr.low)) == 0b1111100001011101u);
  assert((Bm16::Slice1R<11u, 0u>(rinstr.high)) == 0b101100000100u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kUnalignedAllow);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 registers = 1u << Rt;

  if (t == 13u || (t == 15u && TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates))) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrPop{iid, flags, registers}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b01101u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm5 = Bm16::Slice1R<10u, 6u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5 << 2u);

  return Ok(Instr{InstrLdrImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b10011u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rt = Bm16::Slice1R<10u, 8u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u8 n = 13u;
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2u);

  return Ok(Instr{InstrLdrImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrImmediateT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110001101u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm12 = Bm16::Slice1R<11u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (t == 15u && TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrImmediateT4Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000101u);
  assert((Bm16::IsolateBit<11u>(rinstr.high)) == 0b1u);

  const u32 U = Bm16::IsolateBit<9u>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10u>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8u>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert(Rn != 0b1111u);
  assert((P != 0b1u) || (U != 0b1u) || (W != 0b0u));
  if ((Rn == 0b1101u) && (P == 0x0u) && (U == 0x1u) && (W == 0x1u) && (imm8 == 0b00000100u)) {
    // SEE POP
    return PopT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  if ((P == 0x0u) && (W == 0x0u)) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if ((flags & InstrFlags::kWBack && n == t) ||
      t == 15u && TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrImmediate{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrexT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrex};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111010000101u);
  assert((Bm16::Slice1R<11u, 8u>(rinstr.high)) == 0b1111u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  if (t == 13u || t == 15u || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrex{iid, flags, t, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrdImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrdImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b1110100u);
  assert((Bm16::IsolateBit<6u>(rinstr.low)) == 0b1u);
  assert((Bm16::IsolateBit<4u>(rinstr.low)) == 0b1u);

  const u32 U = Bm16::IsolateBit<7u>(rinstr.low);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<8u>(rinstr.low);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<5u>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 Rt2 = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 t2 = static_cast<u8>(Rt2);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2u);

  assert((P != 0u) || (W != 0));
  assert(Rn != 0b1111u);
  if (flags & InstrFlags::kWBack && ((n == t) || (n == t2))) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (t == 13u || t == 15 || (t2 == 13u) || (t2 == 15) || (t == t2)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrdImmediate{iid, flags, t, t2, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> ItT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kIt};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<11u, 8u>(rinstr.low)) == 0xFu);

  const u32 firstcond_32 = Bm16::Slice1R<7u, 4u>(rinstr.low);
  const u8 firstcond = static_cast<u8>(firstcond_32);
  const u32 mask_32 = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 mask = static_cast<u8>(mask_32);

  assert(mask != 0x0u);
  if (firstcond == 0xFu || (firstcond == 0b1110u && Bm8::BitCount(mask) != 1u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  if (TItOps::InITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrIt{iid, flags, firstcond, mask}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BlT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBl};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 14u>(rinstr.high)) == 0x3u);
  assert((Bm16::IsolateBit<12u>(rinstr.high)) == 0x1u);
  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);

  const u32 s = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm10 = Bm16::Slice1R<9u, 0u>(rinstr.low);
  const u32 j1 = Bm16::IsolateBit<13u>(rinstr.high);     // bit 13
  const u32 j2 = Bm16::IsolateBit<11u>(rinstr.high);     // bit 11
  const u32 imm11 = Bm16::Slice1R<10u, 0u>(rinstr.high); // bit 10..0
  const u32 i1 = (~(j1 ^ s)) & 0x1u;
  const u32 i2 = (~(j2 ^ s)) & 0x1u;
  const u32 imm32_us = (s << 24u) | (i1 << 23u) | (i2 << 22u) | (imm10 << 12u) | imm11 << 1u;
  const i32 imm32 = static_cast<int32_t>(Bm32::SignExtend<u32, 24u>(imm32_us));

  if (TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBl{iid, flags, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BxT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBx};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 7u>(rinstr.low) == 0b010001110u));

  const u16 Rm = Bm16::Slice1R<6u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  if (TItOps::InITBlock(pstates) && (!TItOps::LastInITBlock(pstates))) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrBx{iid, flags, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BlxT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBlx};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 7u>(rinstr.low) == 0b010001111u));

  const u16 Rm = Bm16::Slice1R<6u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  if (m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrBlx{iid, flags, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBCond};
  u8 flags = 0x0U;

  if (TItOps::InITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  const u32 cond_32 = Bm16::Slice1R<11u, 8u>(rinstr.low);
  const u8 cond = static_cast<u8>(cond_32);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const i32 imm32 = Bm32::SignExtend<u32, 8u>(imm8 << 1u);

  assert(cond != 0b1110u);
  assert(cond != 0b1111u);

  return Ok(Instr{InstrBCond{iid, flags, cond, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kB};
  u8 flags = 0x0U;

  const u32 imm_11_32 = Bm16::Slice1R<10u, 0u>(rinstr.low);
  const i32 imm32 = Bm32::SignExtend<u32, 11u>((imm_11_32) << 1u);

  if (TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrB{iid, flags, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBCond};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<15u, 14u>(rinstr.high)) == 0b10u);
  assert((Bm16::IsolateBit<12u>(rinstr.high)) == 0b0u);

  const u32 cond_32 = Bm16::Slice1R<9u, 6u>(rinstr.low);
  const u8 cond = static_cast<u8>(cond_32);
  const u32 imm11 = Bm16::Slice1R<10u, 0u>(rinstr.high);
  const u32 imm6 = Bm16::Slice1R<5u, 0u>(rinstr.low);
  const u32 J1 = Bm16::IsolateBit<13u>(rinstr.high); // bit 13
  const u32 J2 = Bm16::IsolateBit<11u>(rinstr.high); // bit 11
  const u32 S = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm32_us = (S << 20u) | (J2 << 19u) | (J1 << 18u) | (imm6 << 12u) | (imm11 << 1u);
  const i32 imm32 = static_cast<int32_t>(Bm32::SignExtend<u32, 20u>(imm32_us));

  if (TItOps::InITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBCond{iid, flags, cond, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BT4Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kB};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<15u, 14u>(rinstr.high)) == 0b10u);
  assert((Bm16::IsolateBit<12u>(rinstr.high)) == 0b1u);

  const u32 imm11 = Bm16::Slice1R<10u, 0u>(rinstr.high);
  const u32 imm10 = Bm16::Slice1R<9u, 0u>(rinstr.low);
  const u32 J1 = Bm16::IsolateBit<13u>(rinstr.high); // bit 13
  const u32 J2 = Bm16::IsolateBit<11u>(rinstr.high); // bit 11
  const u32 S = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 I1 = (~(J1 ^ S)) & 0x1u;
  const u32 I2 = (~(J2 ^ S)) & 0x1u;
  const u32 imm32_us = (S << 24u) | (I1 << 23u) | (I2 << 22u) | (imm10 << 12u) | imm11 << 1u;
  const i32 imm32 = static_cast<int32_t>(Bm32::SignExtend<u32, 24u>(imm32_us));

  if (TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrB{iid, flags, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SubSpMinusImmediateT1Decoder(const RawInstr &rinstr,
                                                  TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSubSpMinusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 7u>(rinstr.low)) == 0b101100001u);

  flags &= ~InstrFlags::kSetFlags;
  const u8 d = 13u;
  const u32 imm7 = Bm16::Slice1R<6u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm7 << 2);

  return Ok(Instr{InstrSubSpMinusImmediate{iid, flags, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SubSpMinusImmediateT2Decoder(const RawInstr &rinstr,
                                                  TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSubSpMinusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b01101u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1101u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert((d != 0b1111u) || (S != 1u));
  if (d == 15u && (S == 0x0u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubSpMinusImmediate{iid, flags, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SubSpMinusImmediateT3Decoder(const RawInstr &rinstr,
                                                  TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSubSpMinusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 0u>(rinstr.low)) == 0b1010101101u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (d == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubSpMinusImmediate{iid, flags, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SubImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSubImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b0001111u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u8 n = static_cast<u8>(Bm16::Slice1R<5u, 3u>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::Slice1R<2u, 0u>(rinstr.low));
  const u32 imm3 = Bm16::Slice1R<8u, 6u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm3);

  return Ok(Instr{InstrSubImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SubImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSubImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b00111u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u8 n = static_cast<u8>(Bm16::Slice1R<10u, 8u>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::Slice1R<10u, 8u>(rinstr.low));
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrSubImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SubImmediateT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSubImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b01101u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert(!((d == 0xFu) && (S == 1u)));
  if (n == 0b1101u) {
    return SubSpMinusImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  if (d == 13u || (d == 15u && S == 0x0u) || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SubImmediateT4Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSubImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 4u>(rinstr.low)) == 0b101010u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rn != 0b1111u);
  if (Rn == 0b1101u) {
    return SubSpMinusImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  if (d == 13u || d == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SbcImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSbcImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b01011u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert(!((d == 0xFu) && (S == 1u)));
  assert(n != 0xDu);
  if (d == 13u || (d == 15u && S == 0x0u) || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSbcImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddSpPlusImmediateT1Decoder(const RawInstr &rinstr,
                                                 TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddSpPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b10101u);

  flags &= ~InstrFlags::kSetFlags;
  const u8 d = static_cast<u8>(Bm16::Slice1R<10u, 8u>(rinstr.low));
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2u);

  return Ok(Instr{InstrAddSpPlusImmediate{iid, flags, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddSpPlusImmediateT2Decoder(const RawInstr &rinstr,
                                                 TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddSpPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 7u>(rinstr.low)) == 0b101100000u);

  flags &= ~InstrFlags::kSetFlags;
  const u8 d = 13u;
  const u32 imm7 = Bm16::Slice1R<6u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm7 << 2u);

  return Ok(Instr{InstrAddSpPlusImmediate{iid, flags, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddSpPlusImmediateT3Decoder(const RawInstr &rinstr,
                                                 TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddSpPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b01000u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1101u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert((Rd != 0b1111u) || (S != 1));
  if (d == 15u && S == 0x0u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddSpPlusImmediate{iid, flags, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddSpPlusImmediateT4Decoder(const RawInstr &rinstr,
                                                 TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddSpPlusImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 0u>(rinstr.low)) == 0b1000001101u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (d == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddSpPlusImmediate{iid, flags, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b0001110u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u8 n = static_cast<u8>(Bm16::Slice1R<5u, 3u>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::Slice1R<2u, 0u>(rinstr.low));
  const u32 imm3 = Bm16::Slice1R<8u, 6u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm3);

  return Ok(Instr{InstrAddImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddImmediate};
  u8 flags = 0x0U;

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u8 n = static_cast<u8>(Bm16::Slice1R<10u, 8u>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::Slice1R<10u, 8u>(rinstr.low));
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrAddImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddImmediateT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b01000u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0x0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  assert(!((d == 0xFu) && (S == 1u)));
  if (Rn == 0b1101u) {
    return AddSpPlusImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  if (d == 13u || (d == 15u && S == 0x0u) || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddImmediateT4Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddImmediate};
  u8 flags = 0x0U;

  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0x0u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b10000u);
  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  assert(Rn != 0b1111u);
  if (Rn == 0b1101u) {
    return AddSpPlusImmediateT4Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  if (d == 13u || d == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AdcImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAdcImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b01010u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0x0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);

  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  TRY_ASSIGN(imm32, Instr, Thumb::ThumbExpandImm(imm12));

  if (d == 13u || d == 15u || n == 13u || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAdcImmediate{iid, flags, n, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> TstImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kTstImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 4u>(rinstr.low)) == 0b000001u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::Slice1R<11u, 8u>(rinstr.high)) == 0b1111u);

  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  auto apsr = TSpecRegOps::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (n == 13u || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrTstImmediate{iid, flags, n, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> TeqImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kTeqImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 4u>(rinstr.low)) == 0b001001u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::Slice1R<11u, 8u>(rinstr.high)) == 0b1111u);

  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  auto apsr = TSpecRegOps::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (n == 13u || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrTeqImmediate{iid, flags, n, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> TstRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kTstRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100001000u);

  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0u};

  return Ok(Instr{InstrTstRegister{iid, flags, n, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> TeqRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kTeqRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111010101001u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::Slice1R<11u, 8u>(rinstr.high)) == 0b1111u);

  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = static_cast<u32>(Bm16::Slice1R<5u, 4u>(rinstr.high));
  const u32 imm2 = static_cast<u32>(Bm16::Slice1R<7u, 6u>(rinstr.high));
  const u32 imm3 = static_cast<u32>(Bm16::Slice1R<14u, 12u>(rinstr.high));
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);

  if (n == 13u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrTeqRegister{iid, flags, n, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AndImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAndImmediate};
  u8 flags = 0x0U;

  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert(((Bm16::Slice1R<11u, 8u>(rinstr.high)) != 0b1111u) ||
         ((Bm16::IsolateBit<4u>(rinstr.low)) != 0b1u));
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b00000u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  auto apsr = TSpecRegOps::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13u || (d == 15u && S == 0u) || n == 13u || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAndImmediate{iid, flags, n, d, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> OrrImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kOrrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b00010u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  auto apsr = TSpecRegOps::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  assert(Rn != 0b1111u);
  if (d == 13u || d == 15u || n == 13u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrOrrImmediate{iid, flags, n, d, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> EorImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kEorImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11110u);
  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b00100u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  auto apsr = TSpecRegOps::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13u || (d == 15u && (S == 0u)) || n == 13u || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrEorImmediate{iid, flags, n, d, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SubRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSubRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b0001101u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0u};
  const u8 m = static_cast<u8>(Bm16::Slice1R<8u, 6u>(rinstr.low));
  const u8 n = static_cast<u8>(Bm16::Slice1R<5u, 3u>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::Slice1R<2u, 0u>(rinstr.low));

  return Ok(Instr{InstrSubRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SubRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSubRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101011101u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = static_cast<u32>(Bm16::IsolateBit<4u>(rinstr.low));
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u32 type = static_cast<u32>(Bm16::Slice1R<5u, 4u>(rinstr.high));
  const u32 imm2 = static_cast<u32>(Bm16::Slice1R<7u, 6u>(rinstr.high));
  const u32 imm3 = static_cast<u32>(Bm16::Slice1R<14u, 12u>(rinstr.high));
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);

  assert((Rd != 0b1111u) || (S != 1u));
  assert(Rn != 0b1101u);
  if (d == 13u || (d == 15u && S == 0u) || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSubRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> RsbRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kRsbRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101011110u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = static_cast<u32>(Bm16::IsolateBit<4u>(rinstr.low));
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u32 type = static_cast<u32>(Bm16::Slice1R<5u, 4u>(rinstr.high));
  const u32 imm2 = static_cast<u32>(Bm16::Slice1R<7u, 6u>(rinstr.high));
  const u32 imm3 = static_cast<u32>(Bm16::Slice1R<14u, 12u>(rinstr.high));
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);

  if (d == 13u || d == 15u || n == 13u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrRsbRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> UmlalT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kUmlal};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110111110u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0000u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 RdHi = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 dHi = static_cast<u8>(RdHi);
  const u16 RdLo = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 dLo = static_cast<u8>(RdLo);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (dLo == 13u || dLo == 15u || dHi == 13u || dHi == 15u || n == 13u || n == 15u || m == 13u ||
      m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (dHi == dLo) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUmlal{iid, flags, dHi, dLo, m, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> UmullT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kUmull};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110111010u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0000u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 RdHi = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 dHi = static_cast<u8>(RdHi);
  const u16 RdLo = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 dLo = static_cast<u8>(RdLo);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (dLo == 13u || dLo == 15u || dHi == 13u || dHi == 15u || n == 13u || n == 15u || m == 13u ||
      m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (dHi == dLo) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUmull{iid, flags, dHi, dLo, m, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SmullT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSmull};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110111000u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0000u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 RdHi = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 dHi = static_cast<u8>(RdHi);
  const u16 RdLo = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 dLo = static_cast<u8>(RdLo);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (dHi == dLo) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSmull{iid, flags, dHi, dLo, m, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MulT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMul};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110110000u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0000u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (d == 13u || d == 15u || n == 13u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMul{iid, flags, d, m, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> UdivT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kUdiv};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110111011u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b1111u);

  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (d == 13u || d == 15u || n == 13u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUdiv{iid, flags, d, m, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SdivT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSdiv};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110111001u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b1111u);

  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (d == 13u || d == 15u || n == 13u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSdiv{iid, flags, d, m, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MlsT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMls};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110110000u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0001u);

  const u16 Ra = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 a = static_cast<u8>(Ra);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (d == 13u || d == 15u || n == 13u || n == 15u || m == 13u || m == 15u || (a == 13u) ||
      (a == 15u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMls{iid, flags, a, d, m, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MlaT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMla};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110110000u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0000u);

  flags &= ~InstrFlags::kSetFlags;
  const u16 Ra = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 a = static_cast<u8>(Ra);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  assert(Ra != 0b1111u);
  if (d == 13u || d == 15u || n == 13u || n == 15u || m == 13u || m == 15u || (a == 13u) ||
      (a == 15u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMla{iid, flags, a, d, m, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b0001100u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0u};
  const u8 m = static_cast<u8>(Bm16::Slice1R<8u, 6u>(rinstr.low));
  const u8 n = static_cast<u8>(Bm16::Slice1R<5u, 3u>(rinstr.low));
  const u8 d = static_cast<u8>(Bm16::Slice1R<2u, 0u>(rinstr.low));

  return Ok(Instr{InstrAddRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<10u, 8u>(rinstr.low)) == 0b100u);

  flags &= ~InstrFlags::kSetFlags;
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0u};
  const u8 m = static_cast<u8>(Bm16::Slice1R<6u, 3u>(rinstr.low));
  const u32 dn_1 = Bm16::IsolateBit<7u>(rinstr.low);
  const u8 n = static_cast<u8>(dn_1 << 3u | Bm16::Slice1R<2u, 0u>(rinstr.low));
  const u32 dn_2 = Bm16::IsolateBit<7u>(rinstr.low);
  const u8 d = static_cast<u8>(dn_2 << 3u | Bm16::Slice1R<2u, 0u>(rinstr.low));

  assert(d != 0b1101u);
  assert(n != 0b1101u);
  if (d == 15u && TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (d == 15u && m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrAddRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AddRegisterT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAddRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101011000u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u32 type = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u32 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);

  assert((d != 0b1111u) || (S != 1u));
  assert(m != 0b1101u);
  if (d == 13u || (d == 15u && S == 0u) || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAddRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AdcRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAdcRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100000101u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(0u)};
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);

  return Ok(Instr{InstrAdcRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AdcRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAdcRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101011010u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u32 type = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u32 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2) | imm2);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);

  if (d == 13u || d == 15u || n == 13u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAdcRegister{iid, flags, shift_res, m, n, d}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StmdbT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStmdb};
  u8 flags = 0x0U;

  assert(false); // not implemented
  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b1110100100u);
  assert((Bm16::IsolateBit<4u>(rinstr.low)) == 0b0u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::IsolateBit<13u>(rinstr.high)) == 0b0u);

  const u32 W = Bm16::IsolateBit<5u>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 M = Bm16::IsolateBit<14u>(rinstr.high);
  const u32 register_list = Bm16::Slice1R<12u, 0u>(rinstr.high);
  const u32 registers = (M << 14u) | register_list;

  assert((W != 0x1u) || (Rn != 0b1101u));
  if (n == 15 || (Bm32::BitCount(registers) < 2u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  // if wback && registers<n> == '1' then UNPREDICTABLE;
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStmdb{iid, flags, n, registers}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> PushT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kPush};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<10u, 9u>(rinstr.low)) == 0b10u);

  flags &= ~InstrFlags::kUnalignedAllow;
  const u32 M = Bm16::IsolateBit<8u>(rinstr.low);
  const u32 register_list = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 registers = (M << 14u) | register_list;

  if (Bm32::BitCount(registers) < 1u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrPush{iid, flags, registers}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> PushT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kPush};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 0u>(rinstr.low)) == 0b1110100100101101u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::IsolateBit<13u>(rinstr.high)) == 0b0u);

  flags &= ~InstrFlags::kUnalignedAllow;
  const u32 M = Bm16::IsolateBit<14u>(rinstr.high);
  const u32 register_list = Bm16::Slice1R<12u, 0u>(rinstr.high);
  const u32 registers = (M << 14u) | register_list;

  if (Bm32::BitCount(registers) < 2u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrPush{iid, flags, registers}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> PushT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kPush};
  u8 flags = 0x0U;

  assert(rinstr.low == 0b1111100001001101u);
  assert((Bm16::Slice1R<11u, 0u>(rinstr.high)) == 0b110100000100u);

  flags |= InstrFlags::kUnalignedAllow;
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 registers = 1u << Rt;

  if (t == 13u || t == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrPush{iid, flags, registers}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdmT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdm};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11001u);

  const u8 Rn_flag = static_cast<u8>(Bm16::Slice1R<10u, 8u>(rinstr.low));
  const u16 register_list_flag = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u16 n_mask = 1u << Rn_flag;
  flags |= (register_list_flag & n_mask) == 0 ? static_cast<InstrFlagsSet>(InstrFlags::kWBack) : 0u;
  const u16 register_list = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 registers = static_cast<u32>(register_list);
  const u16 Rn = Bm16::Slice1R<10u, 8u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (Bm32::BitCount(registers) < 1u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrLdm{iid, flags, registers, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdmT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdm};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b1110100010u);
  assert((Bm16::IsolateBit<4u>(rinstr.low)) == 0b1u);
  assert((Bm16::IsolateBit<13u>(rinstr.high)) == 0b0u);

  const u32 W = Bm16::IsolateBit<5u>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u32 P = Bm16::IsolateBit<15u>(rinstr.high);
  const u32 M = Bm16::IsolateBit<14u>(rinstr.high);
  const u32 register_list = Bm16::Slice1R<12u, 0u>(rinstr.high);
  const u32 registers = (P << 15u) | (M << 14u) | register_list;
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  assert((W != 1) || (Rn != 0b1101u));
  if (n == 15 || (Bm32::BitCount(registers) < 2u) || ((P == 1u) && M == 1u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if ((Bm32::Slice1R<15u, 15u>(registers) == 0x1u) && (TItOps::InITBlock(pstates)) &&
      (!TItOps::LastInITBlock(pstates))) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (flags & InstrFlags::kWBack && ((registers & (1 << n)) != 0u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdm{iid, flags, registers, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StmT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStm};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b11000u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kWBack);
  const u16 register_list = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 registers = static_cast<u32>(register_list);
  const u16 Rn = Bm16::Slice1R<10u, 8u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  return Ok(Instr{InstrStm{iid, flags, registers, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StmT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStm};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b1110100010u);
  assert((Bm16::IsolateBit<4u>(rinstr.low)) == 0b0u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::IsolateBit<13u>(rinstr.high)) == 0b0u);

  flags &= ~InstrFlags::kUnalignedAllow;
  const u32 W = Bm16::IsolateBit<5u>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u32 M = Bm16::IsolateBit<14u>(rinstr.high);
  const u32 register_list = Bm16::Slice1R<12u, 0u>(rinstr.high);
  const u32 registers = (M << 14u) | register_list;
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);

  if (n == 15 || (Bm32::BitCount(registers) < 2u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (flags & InstrFlags::kWBack && ((registers & (1 << n)) != 0u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStm{iid, flags, registers, n}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SxthT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSxth};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b1011001000u);

  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u8 rotation = 0u;

  return Ok(Instr{InstrSxth{iid, flags, m, d, rotation}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SxthT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSxth};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 0u>(rinstr.low)) == 0b1111101000001111u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 6u>(rinstr.high)) == 0b10u);

  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 rotate = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u8 rotation = static_cast<u8>(rotate << 3u);

  if (d == 13u || d == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSxth{iid, flags, m, d, rotation}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> UxtbT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kUxtb};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<11u, 6u>(rinstr.low)) == 0b001011u);

  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u8 rotation = 0u;

  return Ok(Instr{InstrUxtb{iid, flags, m, d, rotation}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> UxtbT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kUxtb};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 0u>(rinstr.low)) == 0b1111101001011111u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 6u>(rinstr.high)) == 0b10u);

  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 rotate = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u8 rotation = static_cast<u8>(rotate << 3u);

  if (d == 13u || d == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUxtb{iid, flags, m, d, rotation}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SxtbT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSxtb};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b1011001001u);

  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u8 rotation = 0u;

  return Ok(Instr{InstrSxtb{iid, flags, m, d, rotation}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SxtbT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSxtb};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 0u>(rinstr.low)) == 0b1111101001001111u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 6u>(rinstr.high)) == 0b10u);

  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 rotate = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u8 rotation = static_cast<u8>(rotate << 3u);

  if (d == 13u || d == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSxtb{iid, flags, m, d, rotation}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> UxthT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kUxth};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b1011001010u);

  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u8 rotation = 0u;

  return Ok(Instr{InstrUxth{iid, flags, m, d, rotation}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> UxthT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kUxth};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 0u>(rinstr.low)) == 0b1111101000011111u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 6u>(rinstr.high)) == 0b10u);

  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 rotate = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u8 rotation = rotate << 3u;

  if (d == 13u || d == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUxth{iid, flags, m, d, rotation}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BfiT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBfi};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111100110110u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::IsolateBit<5u>(rinstr.high)) == 0b0u);

  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm2 = static_cast<u32>(Bm16::Slice1R<7u, 6u>(rinstr.high));
  const u32 imm3 = static_cast<u32>(Bm16::Slice1R<14u, 12u>(rinstr.high));
  const u8 lsbit = static_cast<u8>((imm3 << 2u) | imm2);
  const u16 msb = Bm16::Slice1R<4u, 0u>(rinstr.high);
  const u8 msbit = static_cast<u8>(msb);

  if (d == 13u || d == 15u || n == 13u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBfi{iid, flags, d, n, lsbit, msbit}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> UbfxT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kUbfx};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111100111100u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::IsolateBit<5u>(rinstr.high)) == 0b0u);

  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u8 lsbit = static_cast<u8>((imm3 << 2u) | imm2);
  const u32 widthm1 = Bm16::Slice1R<4u, 0u>(rinstr.high);
  const u8 widthminus1 = static_cast<u8>(widthm1);

  if (d == 13u || d == 15u || n == 13u || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrUbfx{iid, flags, d, n, lsbit, widthminus1}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> EorRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kEorRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100000001u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  // TODO: use ImmShiftResults for all occureces
  const auto shift_res = Alu32::DecodeImmShift(0b00, 0);

  return Ok(Instr{InstrEorRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> EorRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kEorRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010100u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u32 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2u) | imm2);

  assert((Rd != 0b1111u) || (S != 1));
  if (d == 13u || (d == 15u && (S == 0)) || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrEorRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SbcRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSbcRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101011011u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u32 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2u) | imm2);

  if (d == 13u || d == 15u || n == 13u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrSbcRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> OrrRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kOrrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100001100u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = Alu32::DecodeImmShift(0b00, 0);

  return Ok(Instr{InstrOrrRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> OrrRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kOrrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010010u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u32 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2u) | imm2);

  assert(Rn != 0b1111u);
  if (d == 13u || d == 15u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrOrrRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AndRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAndRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100000000u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = Alu32::DecodeImmShift(0b00, 0);

  return Ok(Instr{InstrAndRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AndRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAndRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010000u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u32 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2u) | imm2);

  assert((Rd != 0b1111u) || (S != 0b1u));
  if (d == 13u || (d == 15u && (S == 0u)) || n == 13u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAndRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BicImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBicImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<9u, 5u>(rinstr.low)) == 0b00001u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 i = Bm16::IsolateBit<10u>(rinstr.low);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm12 = (i << 11u) | (imm3 << 8u) | imm8;
  auto apsr = TSpecRegOps::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
  TRY_ASSIGN(imm32_carry, Instr,
             Thumb::ThumbExpandImm_C(imm12, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk));

  if (d == 13u || d == 15u || n == 13u || n == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBicImmediate{iid, flags, n, d, imm32_carry}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BicRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBicRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100001110u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0u};

  return Ok(Instr{InstrBicRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BicRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBicRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010001u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u32 type = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const u32 imm2 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const u32 imm3 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2u) | imm2);

  if (d == 13u || d == 15u || n == 13u || n == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrBicRegister{iid, flags, n, d, m, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LsrRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLsrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100000011u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  return Ok(Instr{InstrLsrRegister{iid, flags, n, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LsrRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLsrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11111010001u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0000u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if (d == 13u || d == 15 || n == 13u || n == 15 || m == 13u || m == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLsrRegister{iid, flags, n, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AsrRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAsrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 6u>(rinstr.low)) == 0b0100000100u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);

  return Ok(Instr{InstrAsrRegister{iid, flags, n, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AsrRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAsrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11111010010u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1111u);
  assert((Bm16::Slice1R<7u, 4u>(rinstr.high)) == 0b0000u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);

  if (d == 13u || d == 15 || n == 13u || n == 15 || m == 13u || m == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAsrRegister{iid, flags, n, d, m}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LsrImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLsrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b00001u);

  flags |=
      TItOps::InITBlock(pstates) == false ? static_cast<InstrFlagsSet>(InstrFlags::kSetFlags) : 0u;
  const u16 Rm = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm5 = Bm16::Slice1R<10u, 6u>(rinstr.low);
  const auto shift_res = Alu32::DecodeImmShift(0b01u, imm5);

  return Ok(Instr{InstrLsrImmediate{iid, flags, m, d, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LsrImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLsrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010010u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1111u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::Slice1R<5u, 4u>(rinstr.high)) == 0b01u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm3_32 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm2_32 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(0b01u, (imm3_32 << 2u) | imm2_32);

  assert(shift_res.type == SRType::SRType_LSR);
  if (d == 13u || d == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLsrImmediate{iid, flags, m, d, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MvnRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMvnRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010011u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1111u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm3 = static_cast<u32>(Bm16::Slice1R<14u, 12u>(rinstr.high));
  const u32 imm2 = static_cast<u32>(Bm16::Slice1R<7u, 6u>(rinstr.high));
  const u32 type = static_cast<u32>(Bm16::Slice1R<5u, 4u>(rinstr.high));
  const auto shift_res = Alu32::DecodeImmShift(type, (imm3 << 2u) | imm2);

  if (d == 13u || d == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMvnRegister{iid, flags, m, d, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> AsrImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kAsrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 5u>(rinstr.low)) == 0b11101010010u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) == 0b1111u);
  assert((Bm16::IsolateBit<15u>(rinstr.high)) == 0b0u);
  assert((Bm16::Slice1R<5u, 4u>(rinstr.high)) == 0b10u);

  const u32 S = Bm16::IsolateBit<4u>(rinstr.low);
  flags |= S << static_cast<InstrFlagsSet>(InstrFlagsShift::kSetFlagsShift);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm3_32 = Bm16::Slice1R<14u, 12u>(rinstr.high);
  const u32 imm2_32 = Bm16::Slice1R<7u, 6u>(rinstr.high);
  const auto shift_res = Alu32::DecodeImmShift(0b10u, (imm3_32 << 2u) | imm2_32);

  assert(shift_res.type == SRType::SRType_ASR);
  if (d == 13u || d == 15u || m == 13u || m == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrAsrImmediate{iid, flags, m, d, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15U, 9U>(rinstr.low)) == 0b0101000u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::Slice1R<8u, 6u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0u};

  return Ok(Instr{InstrStrRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000100u);
  assert((Bm16::Slice1R<11u, 6u>(rinstr.high)) == 0b000000u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  if (t == 15u || m == 13u || m == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrbRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrbRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b0101010u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::Slice1R<8u, 6u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0u};

  return Ok(Instr{InstrStrbRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrbRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrbRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000000u);
  assert((Bm16::Slice1R<11u, 6u>(rinstr.high)) == 0b000000u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  if (Rn == 0b1111u) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 13u || t == 15u || m == 13u || m == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrbRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrhRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrhRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000010u);
  assert((Bm16::Slice1R<11u, 6u>(rinstr.high)) == 0b000000u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  if (Rn == 0b1111u) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 13u || t == 15u || m == 13u || m == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrhRegister{iid, flags, n, m, t, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrbRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrbRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b0101110u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rm = Bm16::Slice1R<8u, 6u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0x0u};

  return Ok(Instr{InstrLdrbRegister{iid, flags, m, n, t, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrRegisterT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b0101100u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rm = Bm16::Slice1R<8u, 6u>(rinstr.low);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, 0x0u};

  return Ok(Instr{InstrLdrRegister{iid, flags, m, n, t, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000101u);
  assert((Bm16::Slice1R<11u, 6u>(rinstr.high)) == 0b000000u);
  assert((Bm16::Slice1R<3u, 0u>(rinstr.low)) != 0b1111u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  if (m == 13u || m == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (t == 15u && TItOps::InITBlock(pstates) && !TItOps::LastInITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrRegister{iid, flags, m, n, t, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> LdrhRegisterT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kLdrhRegister};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000011u);
  assert((Bm16::Slice1R<11u, 6u>(rinstr.high)) == 0b000000u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rm = Bm16::Slice1R<3u, 0u>(rinstr.high);
  const u8 m = static_cast<u8>(Rm);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm2 = Bm16::Slice1R<5u, 4u>(rinstr.high);
  const auto shift_res = ImmShiftResults{SRType::SRType_LSL, static_cast<u8>(imm2)};

  assert(Rn != 0b1111u);
  assert(Rt != 0b1111u);
  if (t == 13u || m == 13u || m == 15) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrLdrhRegister{iid, flags, m, n, t, shift_res}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrdImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrdImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 9u>(rinstr.low)) == 0b1110100u);
  assert((Bm16::IsolateBit<6u>(rinstr.low)) == 0b1u);
  assert((Bm16::IsolateBit<4u>(rinstr.low)) == 0b0u);

  const u32 U = Bm16::IsolateBit<7u>(rinstr.low);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<8u>(rinstr.low);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<5u>(rinstr.low);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rt2 = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 t2 = static_cast<u8>(Rt2);
  const u32 imm8_32 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8_32 << 2u);

  assert((P != 0u) || (W != 0u));
  if (flags & InstrFlags::kWBack && (n == t || n == t2)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if (n == 15u || t == 13u || t == 15u || t2 == 13u || t2 == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrdImmediate{iid, flags, n, t, t2, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrhImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b10000u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm5 = Bm16::Slice1R<10u, 6u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5 << 1u);

  return Ok(Instr{InstrStrhImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrhImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110001010u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm12 = Bm16::Slice1R<11u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (t == 13u || t == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrhImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrhImmediateT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrhImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000010u);
  assert((Bm16::IsolateBit<11u>(rinstr.high)) == 0b1u);

  const u32 U = Bm16::IsolateBit<9u>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10u>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8u>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert(P != 0b1u || U != 0b1u || W != 0b0u);
  if ((Rn == 0b1111u) || (P == 0b0 && W == 0b0u)) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if ((t == 13u || t == 15u) && (flags & InstrFlags::kWBack && n == t)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrhImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrbImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b01110u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm5 = Bm16::Slice1R<10u, 6u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5);

  return Ok(Instr{InstrStrbImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrbImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110001000u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm12 = Bm16::Slice1R<11u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (Rn == 0b1111u) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 13u || t == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrbImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrbImmediateT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrbImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000000u);
  assert((Bm16::IsolateBit<11u>(rinstr.high)) == 0b1u);

  const u32 U = Bm16::IsolateBit<9u>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10u>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8u>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  assert((P != 1u) || (U != 1) || (W != 0u));
  if (n == 15u || ((P == 0u) && (W == 0u))) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 13u || t == 15u || (flags & InstrFlags::kWBack && n == t)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrbImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrImmediateT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b01100u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<5u, 3u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm5 = Bm16::Slice1R<10u, 6u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm5 << 2);

  return Ok(Instr{InstrStrImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrImmediateT2Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 11u>(rinstr.low)) == 0b10010u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u8 n = 13u;
  const u16 Rt = Bm16::Slice1R<10u, 8u>(rinstr.low);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8 << 2u);

  return Ok(Instr{InstrStrImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrImmediateT3Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110001100u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm12 = Bm16::Slice1R<11u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm12);

  if (n == 0b1111u) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 15u) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrImmediateT4Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrImmediate};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111110000100u);
  assert((Bm16::IsolateBit<11u>(rinstr.high)) == 0b1u);

  const u32 U = Bm16::IsolateBit<9u>(rinstr.high);
  flags |= U << static_cast<InstrFlagsSet>(InstrFlagsShift::kAddShift);
  const u32 P = Bm16::IsolateBit<10u>(rinstr.high);
  flags |= P << static_cast<InstrFlagsSet>(InstrFlagsShift::kIndexShift);
  const u32 W = Bm16::IsolateBit<8u>(rinstr.high);
  flags |= W << static_cast<InstrFlagsSet>(InstrFlagsShift::kWBackShift);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u32 imm8_32 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8_32);

  assert((P != 1u) || (U != 1) || (W != 0u));
  if ((n == 13u) && (P == 1) && (U == 0u) && (W == 1u) && (imm8_32 == 0b00000100u)) {
    return PushT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  if (n == 15u || ((P == 0u) && (W == 0u))) {
    return Err<Instr>(StatusCode::kScDecoderUndefined);
  }
  if (t == 15u || (flags & InstrFlags::kWBack && n == t)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrImmediate{iid, flags, n, t, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> StrexT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kStrex};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111010000100u);

  flags |= static_cast<InstrFlagsSet>(InstrFlags::kAdd);
  flags |= static_cast<InstrFlagsSet>(InstrFlags::kIndex);
  flags &= static_cast<InstrFlagsSet>(~InstrFlags::kWBack);
  const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u16 Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
  const u8 t = static_cast<u8>(Rt);
  const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
  const u8 d = static_cast<u8>(Rd);
  const u32 imm8_32 = Bm16::Slice1R<7u, 0u>(rinstr.high);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8_32);

  if (d == n || d == t) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrStrex{iid, flags, n, t, d, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> CbNZT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kCbNZ};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 12u>(rinstr.low)) == 0b1011u);
  assert((Bm16::IsolateBit<10u>(rinstr.low)) == 0b0u);
  assert((Bm16::IsolateBit<8u>(rinstr.low)) == 0b1u);

  const u32 op = Bm16::IsolateBit<11u>(rinstr.low);
  flags |= op << static_cast<InstrFlagsSet>(InstrFlagsShift::kNonZeroShift);
  const u16 Rn = Bm16::Slice1R<2u, 0u>(rinstr.low);
  const u8 n = static_cast<u8>(Rn);
  const u32 imm5 = Bm16::Slice1R<7u, 3u>(rinstr.low);
  const u32 i = Bm16::IsolateBit<9u>(rinstr.low);
  const i32 imm32 = Bm32::ZeroExtend<u32>((i << 6u) | (imm5 << 1u));

  if (TItOps::InITBlock(pstates)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }

  return Ok(Instr{InstrCbNZ{iid, flags, n, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> SvcT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kSvc};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 8u>(rinstr.low)) == 0b11011111u);

  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrSvc{iid, flags, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> BkptT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kBkpt};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 8u>(rinstr.low)) == 0b10111110u);

  const u32 imm8 = Bm16::Slice1R<7u, 0u>(rinstr.low);
  const u32 imm32 = Bm32::ZeroExtend<u32>(imm8);

  return Ok(Instr{InstrBkpt{iid, flags, imm32}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MsrT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMsr};
  u8 flags = 0x0U;

  assert((Bm16::Slice1R<15u, 4u>(rinstr.low)) == 0b111100111000u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1000u);
  assert((Bm16::Slice1R<9u, 8u>(rinstr.high)) == 0b00u);

  const u8 n = static_cast<u8>(Bm16::Slice1R<3u, 0u>(rinstr.low));
  const u8 mask = static_cast<u8>(Bm16::Slice1R<11u, 10u>(rinstr.high));
  const u8 SYSm = static_cast<u8>(Bm16::Slice1R<7u, 0u>(rinstr.high));

  if (mask == 0b00u || (mask != 0b10U && !(SYSm == 0u || SYSm == 1u || SYSm == 2u || SYSm == 3u))) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  if ((n == 13u) || (n == 15u) ||
      !(SYSm == 0u || SYSm == 1u || SYSm == 2u || SYSm == 3u || SYSm == 5u || SYSm == 6u ||
        SYSm == 7u || SYSm == 8u || SYSm == 9u || SYSm == 16u || SYSm == 17u || SYSm == 18u ||
        SYSm == 19u || SYSm == 20u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMsr{iid, flags, n, mask, SYSm}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> MrsT1Decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  static_cast<void>(pstates); // prevents warning when pstates is not used
  const InstrId iid{InstrId::kMrs};
  u8 flags = 0x0U;

  assert(rinstr.low == 0b1111001111101111u);
  assert((Bm16::Slice1R<15u, 12u>(rinstr.high)) == 0b1000u);

  const u8 d = static_cast<u8>(Bm16::Slice1R<11u, 8u>(rinstr.high));
  const u8 mask = static_cast<u8>(Bm16::Slice1R<11u, 10u>(rinstr.high));
  const u8 SYSm = static_cast<u8>(Bm16::Slice1R<7u, 0u>(rinstr.high));

  if ((d == 13u) || (d == 15u) ||
      !(SYSm == 0u || SYSm == 1u || SYSm == 2u || SYSm == 3u || SYSm == 5u || SYSm == 6u ||
        SYSm == 7u || SYSm == 8u || SYSm == 9u || SYSm == 16u || SYSm == 17u || SYSm == 18u ||
        SYSm == 19u || SYSm == 20u)) {
    return Err<Instr>(StatusCode::kScDecoderUnpredictable);
  }
  flags |= static_cast<u8>(InstrFlags::k32Bit);

  return Ok(Instr{InstrMrs{iid, flags, d, mask, SYSm}});
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter16bit_op00000(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129
  //
  // # Shift (immediate), add, subtract, move, and compare
  const u16 opcode = Bm16::Slice1R<13u, 9u>(rinstr.low);
  if (Bm16::Slice1R<4u, 2u>(opcode) == 0b000u) {
    if (Bm16::Slice1R<10u, 6u>(rinstr.low) == 0b00000u) {
      return MovRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else {
      return LslImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter16bit_op00011(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129
  //
  // # Shift (immediate), add, subtract, move, and compare
  //  see Armv7-M Architecture Reference Manual Issue E.e p130
  const u16 opcode = Bm16::Slice1R<13u, 9u>(rinstr.low);
  if (opcode == 0b01100u) {
    return AddRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (opcode == 0b01110u) {
    return AddImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (opcode == 0b01101u) {
    return SubRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (opcode == 0b01111u) {
    return SubImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter16bit_op01010(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # Load/store single data item
  // see Armv7-M Architecture Reference Manual Issue E.e p133
  //
  const u16 op_a = Bm16::Slice1R<15u, 12u>(rinstr.low);
  const u16 op_b = Bm16::Slice1R<11u, 9u>(rinstr.low);
  if (op_a == 0b0101U && op_b == 0b000U) {
    return StrRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (op_a == 0b0101U && op_b == 0b010U) {
    return StrbRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter16bit_op10111(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # Miscellaneous 16-bit instructions
  // see Armv7-M Architecture Reference Manual Issue E.e p134
  //
  const u16 opcode = Bm16::Slice1R<11u, 5u>(rinstr.low);
  if (Bm16::Slice1R<6u, 3u>(opcode) == 0b1001u) {
    return CbNZT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 3u>(opcode) == 0b1011u) {
    return CbNZT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 4u>(opcode) == 0b110u) {
    return PopT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 3u>(opcode) == 0b1110u) {
    return BkptT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 3u>(opcode) == 0b1111u) {
    // # If-Then, and hints
    // see Armv7-M Architecture Reference Manual Issue E.e p135
    if (Bm16::Slice1R<3u, 0u>(rinstr.low) != 0x0u) {
      return ItT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
    if ((Bm16::Slice1R<3u, 0u>(rinstr.low) == 0x0u) &&
        (Bm16::Slice1R<7u, 4u>(rinstr.low) == 0x0u)) {
      return NopT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter16bit_op10110(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129
  //
  // ## Miscellaneous 16-bit instructions
  // see Armv7-M Architecture Reference Manual Issue E.e p134
  const u16 opcode = Bm16::Slice1R<11u, 5u>(rinstr.low);
  if (Bm16::Slice1R<6u, 2u>(opcode) == 0b00000u) {
    return AddSpPlusImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 3u>(opcode) == 0b0001u) {
    return CbNZT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 2u>(opcode) == 0b00001u) {
    return SubSpMinusImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 3u>(opcode) == 0b0011u) {
    return CbNZT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 1u>(opcode) == 0b001001u) {
    return SxtbT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 1u>(opcode) == 0b001011u) {
    return UxtbT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 1u>(opcode) == 0b001010u) {
    return UxthT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 1u>(opcode) == 0b001000u) {
    return SxthT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (Bm16::Slice1R<6u, 4u>(opcode) == 0b010u) {
    return PushT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter16bit_op01000(const RawInstr &rinstr, TProcessorStates &pstates) {
  if (Bm16::IsolateBit<10u>(rinstr.low) == 0x1) {
    // ## Special data instructions and branch and exchange
    // see Armv7-M Architecture Reference Manual Issue E.e p132
    if (Bm16::Slice1R<9u, 8u>(rinstr.low) == 0b00u) {
      return AddRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (Bm16::Slice1R<9u, 8u>(rinstr.low) == 0b10u) {
      return MovRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (Bm16::Slice1R<9u, 8u>(rinstr.low) == 0b01u) {
      return CmpRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (Bm16::Slice1R<9u, 7u>(rinstr.low) == 0b110u) {
      return BxT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (Bm16::Slice1R<9u, 7u>(rinstr.low) == 0b111u) {
      return BlxT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  } else {
    // ## Data processing
    // see Armv7-M Architecture Reference Manual Issue E.e p131
    const u16 _opcode = Bm16::Slice1R<9u, 6u>(rinstr.low);
    if (_opcode == 0b0000u) {
      return AndRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b0010u) {
      return LslRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b0011u) {
      return LsrRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b0100u) {
      return AsrRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b0101u) {
      return AdcRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b0001u) {
      return EorRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b1000u) {
      return TstRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b1001u) {
      return RsbImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b1010u) {
      return CmpRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b1100u) {
      return OrrRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_opcode == 0b1110u) {
      return BicRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter16bit_op01011(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129

  const u16 opB = Bm16::Slice1R<11u, 9u>(rinstr.low);
  // # Load/store single data item
  // see Armv7-M Architecture Reference Manual Issue E.e p133
  if (opB == 0b100u) {
    return LdrRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (opB == 0b110u) {
    return LdrbRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter16bit_op01101(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129

  const u16 opB = Bm16::Slice1R<11u, 9u>(rinstr.low);
  // # Load/store single data item
  // see Armv7-M Architecture Reference Manual Issue E.e p133
  if (Bm16::Slice1R<2u, 2u>(opB) == 0b1u) {
    return LdrImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter16bit_op11011(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # 16-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p129
  //
  // ## Conditional branch, and Supervisor Call
  // see Armv7-M Architecture Reference Manual Issue E.e p136
  const u16 opcode = Bm16::Slice1R<11u, 8u>(rinstr.low);
  if (Bm16::Slice1R<3u, 1u>(opcode) != 0b111u) {
    return BT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  } else if (opcode == 0b1111u) {
    return SvcT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter32bit_op11101(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # 32-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p137
  if (Bm16::Slice1R<10u, 9u>(rinstr.low) == 0b01u) {
    // ## Data processing (shifted register)
    // see Armv7-M Architecture Reference Manual Issue E.e p150
    const u16 op = Bm16::Slice1R<8u, 5u>(rinstr.low);
    const u16 Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
    const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
    const u16 S = Bm16::IsolateBit<4u>(rinstr.low);
    if ((op == 0b0000u) && (Rd != 0b1111u)) {
      return AndRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b0001u) {
      return BicRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b0011u && Rn == 0b1111u) {
      return MvnRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b1000u && Rd != 0b1111u) {
      return AddRegisterT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b1110u) {
      return RsbRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b0010u && Rn == 0b1111u) {
      // ### Move register and immediate shifts
      // see Armv7-M Architecture Reference Manual Issue E.e p151
      const u32 type = static_cast<u32>(Bm16::Slice1R<5u, 4u>(rinstr.high));
      const u32 imm2 = static_cast<u32>(Bm16::Slice1R<7u, 6u>(rinstr.high));
      const u32 imm3 = static_cast<u32>(Bm16::Slice1R<14u, 12u>(rinstr.high));
      const u32 imm3_2 = imm3 << 2u | imm2;
      if (type == 0b00u && imm3_2 == 0b00000u) {
        return MovRegisterT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
      } else if (type == 0b01u) {
        return LsrImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
      } else if (type == 0b10u) {
        return AsrImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
      } else if (type == 0b00u && imm3_2 != 0b00000u) {
        return LslImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
      } else if (type == 0b11u && imm3_2 == 0b00000u) {
        return RrxT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
      }
    } else if (op == 0b0010u && Rn != 0b1111u) {
      return OrrRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b0100u && Rd != 0b1111u) {
      return EorRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b0100u && Rd == 0b1111u && S == 1) {
      return TeqRegisterT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b0100u && Rd == 0b1111u && S == 0) {
      return Err<Instr>(StatusCode::kScDecoderUnpredictable);
    } else if (op == 0b1101u && Rd != 0b1111u) {
      return SubRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b1010u) {
      return AdcRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b1011u) {
      return SbcRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op == 0b1101u && Rd == 0b1111u) {
      return CmpRegisterT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  } else if ((Bm16::Slice1R<10u, 9u>(rinstr.low) == 0b00u) &&
             (Bm16::IsolateBit<6u>(rinstr.low) == 0b0u)) {
    // ## Load Multiple and Store Multiple
    // see Armv7-M Architecture Reference Manual Issue E.e p144
    const u16 op = Bm16::Slice1R<8u, 7u>(rinstr.low);
    const u16 W = Bm16::IsolateBit<5u>(rinstr.low);
    const u16 Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
    const u16 L = Bm16::IsolateBit<4u>(rinstr.low);
    const u16 W_Rn = (W << 4u) | Rn;
    if ((op == 0b10u) && (L == 0u) && (W_Rn == 0b11101u)) {
      return PushT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((op == 0b01u) && (L == 0u)) {
      return StmT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((op == 0b10u) && (L == 0u) && (W_Rn != 0b11101u)) {
      return StmdbT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((op == 0b01u) && (L == 1u) && (W_Rn != 0b11101u)) {
      return LdmT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((op == 0b01u) && (L == 1u) && (W_Rn == 0b11101u)) {
      return PopT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  } else if ((Bm16::Slice1R<10u, 9u>(rinstr.low) == 0b00u) &&
             (Bm16::IsolateBit<6u>(rinstr.low) == 0b1u)) {
    // ## Load/store dual or exclusive, table branch
    // see Armv7-M Architecture Reference Manual Issue E.e p145
    const u16 op1 = Bm16::Slice1R<8u, 7u>(rinstr.low);
    const u16 op2 = Bm16::Slice1R<5u, 4u>(rinstr.low);
    const u16 op3 = Bm16::Slice1R<7u, 4u>(rinstr.high);
    if (op1 == 0b00u && op2 == 0b00u) {
      return StrexT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (op1 == 0b00u && op2 == 0b01u) {
      return LdrexT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<1u, 1u>(op1) == 0b0u) && (op2 == 0b10u)) {
      return StrdImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<1u, 1u>(op1) == 0b1u) && (Bm16::Slice1R<0u, 0u>(op2) == 0b0u)) {
      return StrdImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<1u, 1u>(op1) == 0b0u) && (op2 == 0b11u)) {
      return LdrdImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<1u, 1u>(op1) == 0b1u) && (Bm16::Slice1R<0u, 0u>(op2) == 0b1u)) {
      return LdrdImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((op1 == 0b01u) && (op2 == 0b01u) && (op3 == 0b0000u)) {
      return TbbHT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((op1 == 0b01u) && (op2 == 0b01u) && (op3 == 0b0001u)) {
      return TbbHT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter32bit_op11110(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # 32-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p137
  const u16 op1 = Bm16::Slice1R<12u, 11u>(rinstr.low);
  const u16 op2 = Bm16::Slice1R<10u, 4u>(rinstr.low);
  const u16 op = Bm16::IsolateBit<15u>(rinstr.high);
  if ((op1 == 0b10u) && (op == 0b1u)) {
    // ## Branches and miscellaneous control
    // see Armv7-M Architecture Reference Manual Issue E.e p142
    const u16 _op1 = Bm16::Slice1R<14u, 12u>(rinstr.high);
    const u16 _op = Bm16::Slice1R<10u, 4u>(rinstr.low);
    if ((_op1 == 0b000u || _op1 == 0b010u) && (Bm16::Slice1R<5u, 3u>(_op) != 0b111u)) {
      return BT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((_op1 == 0b000u || _op1 == 0b010u) && (Bm16::Slice1R<6u, 1u>(_op) == 0b011100u)) {
      return MsrT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((_op1 == 0b000u || _op1 == 0b010u) && (Bm16::Slice1R<6u, 1u>(_op) == 0b011111u)) {
      return MrsT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b001u || _op1 == 0b011u) { // attention the x in the docs does not mean hex
      return BT4Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b101u || _op1 == 0b111u) {
      return BlT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((_op1 == 0b000u || _op1 == 0b010u) && (_op == 0b0111011u)) {
      // ### Miscellaneous control instructions
      // see Armv7-M Architecture Reference Manual Issue E.e p143
      const u16 _opc = Bm16::Slice1R<7u, 4u>(rinstr.high);
      // const u16 _option = Bm16::Slice1R<3u, 0u>(rinstr.high);
      if (_opc == 0b0101u) {
        return DmbT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
      }
    }
  } else if ((op1 == 0b10u) && (Bm16::IsolateBit<5u>(op2) == 0b0u) && (op == 0x0u)) {
    // ## Data processing (modified immediate)
    // see Armv7-M Architecture Reference Manual Issue E.e p138
    const u16 _op = Bm16::Slice1R<8u, 4u>(rinstr.low);
    const u16 _Rd = Bm16::Slice1R<11u, 8u>(rinstr.high);
    const u16 _Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
    if ((Bm16::Slice1R<4u, 1u>(_op) == 0b0000u) && _Rd == 0b1111u) {
      return TstImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b0000u) && _Rd != 0b1111u) {
      return AndImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b0010u) && _Rn != 0b1111u) {
      return OrrImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b0010u) && _Rn == 0b1111u) {
      return MovImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b0011u) && _Rn == 0b1111u) {
      return MvnImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b0100u) && _Rd != 0b1111u) {
      return EorImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b0100u) && _Rd == 0b1111u) {
      return TeqImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (Bm16::Slice1R<4u, 1u>(_op) == 0b0001u) {
      return BicImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b1000u) && _Rd != 0b1111u) {
      return AddImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b1000u) && _Rd == 0b1111u) {
      return CmnImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (Bm16::Slice1R<4u, 1u>(_op) == 0b1010u) {
      return AdcImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b1101u) && _Rd != 0b1111u) {
      return SubImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<4u, 1u>(_op) == 0b1101u) && _Rd == 0b1111u) {
      return CmpImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (Bm16::Slice1R<4u, 1u>(_op) == 0b1011u) {
      return SbcImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (Bm16::Slice1R<4u, 1u>(_op) == 0b1110u) {
      return RsbImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  } else if ((op1 == 0b10u) && (Bm16::IsolateBit<5u>(op2) == 0b1u) && (op == 0x0u)) {
    // ## Data processing (plain binary immediate)
    // see Armv7-M Architecture Reference Manual Issue E.e p141
    const u16 _op = Bm16::Slice1R<8u, 4u>(rinstr.low);
    const u16 _Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
    if (_op == 0b00100u) {
      return MovImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op == 0b00000u && _Rn != 0b1111u) {
      return AddImmediateT4Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op == 0b01010u && _Rn != 0b1111u) {
      return SubImmediateT4Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op == 0b10110u && _Rn != 0b1111u) {
      return BfiT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op == 0b11100u) {
      return UbfxT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}
template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> Splitter32bit_op11111(const RawInstr &rinstr, TProcessorStates &pstates) {
  // # 32-bit Thumb instruction encoding
  // see Armv7-M Architecture Reference Manual Issue E.e p137
  // op1 is set by jump table
  const u16 op2 = Bm16::Slice1R<10u, 4u>(rinstr.low);
  if ((Bm16::Slice1R<6u, 4u>(op2) == 0b000u) && (Bm16::Slice1R<0u, 0u>(op2) == 0b0u)) {
    // ## Store single data item
    // see Armv7-M Architecture Reference Manual Issue E.e p149
    const u16 _op1 = Bm16::Slice1R<7u, 5u>(rinstr.low);
    const u16 _op2 = Bm16::Slice1R<11u, 6u>(rinstr.high);
    if (_op1 == 0b100u) {
      return StrbImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b101u) {
      return StrhImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b001u && Bm16::IsolateBit<5u>(_op2) == 0b1u) {
      return StrhImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b000u && Bm16::IsolateBit<5u>(_op2) == 0b1u) {
      return StrbImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b010u && Bm16::IsolateBit<5u>(_op2) == 0b1u) {
      return StrImmediateT4Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b010u && Bm16::IsolateBit<5u>(_op2) == 0b0u) {
      return StrRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b001u && Bm16::IsolateBit<5u>(_op2) == 0b0u) {
      return StrhRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b000u && Bm16::IsolateBit<5u>(_op2) == 0b0u) {
      return StrbRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b110u) {
      return StrImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  } else if ((Bm16::Slice1R<6u, 5u>(op2) == 0b00u) && (Bm16::Slice1R<2u, 0u>(op2) == 0b001u)) {
    // ## Load byte, memory hints
    // see Armv7-M Architecture Reference Manual Issue E.e p148
    const u16 _op1 = Bm16::Slice1R<8u, 7u>(rinstr.low);
    const u16 _op2 = Bm16::Slice1R<11u, 6u>(rinstr.high);
    const u16 _Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
    const u16 _Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
    if (_op1 == 0b01u && _Rn != 0b1111 && _Rt != 0b1111u) {
      return LdrbImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b00u && (Bm16::IsolateBit<5u>(_op2) == 0b1u) &&
               (Bm16::Slice1R<2u, 2u>(_op2) == 0b1u) && _Rn != 0b1111 && _Rt != 0b1111u) {
      return LdrbImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b00u && Bm16::Slice1R<5u, 2u>(_op2) == 0b1100u && _Rn != 0b1111 &&
               _Rt != 0b1111u) {
      return LdrbImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b11u && _Rn != 0b1111 && _Rt != 0b1111u) {
      return LdrsbImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  } else if ((Bm16::Slice1R<6u, 5u>(op2) == 0b00u) && (Bm16::Slice1R<2u, 0u>(op2) == 0b011u)) {
    // ## Load halfword, memory hints
    // see Armv7-M Architecture Reference Manual Issue E.e p147
    const u16 _op1 = Bm16::Slice1R<8u, 7u>(rinstr.low);
    const u16 _op2 = Bm16::Slice1R<11u, 6u>(rinstr.high);
    const u16 _Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
    const u16 _Rt = Bm16::Slice1R<15u, 12u>(rinstr.high);
    if (_op1 == 0b01u && _Rn != 0b1111u && _Rt != 0b1111u) {
      return LdrhImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b00u && (_op2 == 0b000000u) && _Rn != 0b1111u && _Rt != 0b1111u) {
      return LdrhRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b11u && _Rn != 0b1111u && _Rt != 0b1111u) {
      return LdrshImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b00u && Bm16::Slice1R<2u, 2u>(_op2) == 0b1u &&
               Bm16::IsolateBit<5u>(_op2) == 0b1u && _Rn != 0b1111u && _Rt != 0b1111u) {
      return LdrhImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b00u && Bm16::Slice1R<5u, 2u>(_op2) == 0b1100u && _Rn != 0b1111u &&
               _Rt != 0b1111u) {
      return LdrhImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b10u && Bm16::Slice1R<2u, 2u>(_op2) == 0b1u &&
               Bm16::IsolateBit<5u>(_op2) == 0b1u && _Rn != 0b1111u && _Rt != 0b1111u) {
      return LdrshImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b10u && Bm16::Slice1R<5u, 2u>(_op2) == 0b1100u && _Rn != 0b1111u &&
               _Rt != 0b1111u) {
      return LdrshImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  } else if ((Bm16::Slice1R<6u, 5u>(op2) == 0b00u) && (Bm16::Slice1R<2u, 0u>(op2) == 0b101u)) {
    // ## Load word
    // see Armv7-M Architecture Reference Manual Issue E.e p146
    const u16 _op1 = Bm16::Slice1R<8u, 7u>(rinstr.low);
    const u16 _op2 = Bm16::Slice1R<11u, 6u>(rinstr.high);
    const u16 _Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
    if ((Bm16::Slice1R<1u, 1u>(_op1) == 0b0u) && _Rn == 0b1111u) {
      return LdrLiteralT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b01u && _Rn != 0b1111u) {
      return LdrImmediateT3Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b00u && Bm16::IsolateBit<5u>(_op2) == 0b1u &&
               (Bm16::Slice1R<2u, 2u>(_op2) == 0b1u) && _Rn != 0b1111u) {
      return LdrImmediateT4Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b00u && Bm16::Slice1R<5u, 2u>(_op2) == 0b1100u && _Rn != 0b1111u) {
      return LdrImmediateT4Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b00u && _op2 == 0b000000u && _Rn != 0b1111u) {
      return LdrRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  } else if (Bm16::Slice1R<6u, 4u>(op2) == 0b010u) {
    // ## Data processing (register)
    // see Armv7-M Architecture Reference Manual Issue E.e p152
    const u16 _op1 = Bm16::Slice1R<7u, 4u>(rinstr.low);
    const u16 _op2 = Bm16::Slice1R<7u, 4u>(rinstr.high);
    const u16 _Rn = Bm16::Slice1R<3u, 0u>(rinstr.low);
    if ((Bm16::Slice1R<3u, 1u>(_op1) == 0b000u) && _op2 == 0b0000u) {
      return LslRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<3u, 1u>(_op1) == 0b001u) && _op2 == 0b0000u) {
      return LsrRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<3u, 1u>(_op1) == 0b010u) && _op2 == 0b0000u) {
      return AsrRegisterT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b0001u && (Bm16::Slice1R<3u, 3u>(_op2) == 0b1u) && _Rn == 0b1111u) {
      return UxthT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b0100u && (Bm16::Slice1R<3u, 3u>(_op2) == 0b1u) && _Rn == 0b1111u) {
      return SxtbT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b0000u && (Bm16::Slice1R<3u, 3u>(_op2) == 0b1u) && _Rn == 0b1111u) {
      return SxthT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b0101u && (Bm16::Slice1R<3u, 3u>(_op2) == 0b1u) && _Rn == 0b1111u) {
      return UxtbT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if ((Bm16::Slice1R<3u, 2u>(_op1) == 0b10u) && (Bm16::Slice1R<3u, 2u>(_op2) == 0b10u)) {
      // ## Miscellaneous operations
      // see Armv7-M Architecture Reference Manual Issue E.e p155
      const u32 __op1 = Bm16::Slice1R<5u, 4u>(rinstr.low);
      const u32 __op2 = Bm16::Slice1R<5u, 4u>(rinstr.high);
      if ((__op1 == 0b11u) && (__op2 == 0b00u)) {
        return ClzT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
      }
    }
  } else if (Bm16::Slice1R<6u, 3u>(op2) == 0b0110u) {
    // ## Multiply, multiply accumulate, and absolute difference
    // see Armv7-M Architecture Reference Manual Issue E.e p156
    const u16 _op1 = Bm16::Slice1R<6u, 4u>(rinstr.low);
    const u16 _op2 = Bm16::Slice1R<5u, 4u>(rinstr.high);
    const u16 _Ra = Bm16::Slice1R<15u, 12u>(rinstr.high);
    if (_op1 == 0b000u && _op2 == 0b00u && _Ra == 0b1111u) {
      return MulT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b000u && _op2 == 0b00u && _Ra != 0b1111u) {
      return MlaT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b000u && _op2 == 0b01u) {
      return MlsT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  } else if (Bm16::Slice1R<6u, 3u>(op2) == 0b0111u) {
    // ## Long multiply, long multiply accumulate, and divide
    // see Armv7-M Architecture Reference Manual Issue E.e p156
    const u16 _op1 = Bm16::Slice1R<6u, 4u>(rinstr.low);
    const u16 _op2 = Bm16::Slice1R<7u, 4u>(rinstr.high);
    if (_op1 == 0b010u && _op2 == 0b0000u) {
      return UmullT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b000u && _op2 == 0b0000u) {
      return SmullT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b011u && _op2 == 0b1111u) {
      return UdivT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b001u && _op2 == 0b1111u) {
      return SdivT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    } else if (_op1 == 0b110u && _op2 == 0b0000u) {
      return UmlalT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
    }
  }
  return Err<Instr>(StatusCode::kScDecoderUnknownOpCode);
}

template <typename TProcessorStates, typename TItOps, typename TSpecRegOps>
static Result<Instr> call_decoder(const RawInstr &rinstr, TProcessorStates &pstates) {
  const u16 opc = Bm32::Slice1R<kDecodersOpCodeLast, kDecodersOpCodeFirst>(rinstr.low);
  switch (opc) {
  case 0b00000u: {
    return Splitter16bit_op00000<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b00001u: {
    return LsrImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b00010u: {
    return AsrImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b00011u: {
    return Splitter16bit_op00011<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b00100u: {
    return MovImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b00101u: {
    return CmpImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b00110u: {
    return AddImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b00111u: {
    return SubImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b01000u: {
    return Splitter16bit_op01000<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b01001u: {
    return LdrLiteralT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b01010u: {
    return Splitter16bit_op01010<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b01011u: {
    return Splitter16bit_op01011<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b01100u: {
    return StrImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b01101u: {
    return Splitter16bit_op01101<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b01110u: {
    return StrbImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b01111u: {
    return LdrbImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b10000u: {
    return StrhImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b10001u: {
    return LdrhImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b10010u: {
    return StrImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b10011u: {
    return LdrImmediateT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b10100u: {
    return AddPcPlusImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b10101u: {
    return AddSpPlusImmediateT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b10110u: {
    return Splitter16bit_op10110<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b10111u: {
    return Splitter16bit_op10111<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b11000u: {
    return StmT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b11001u: {
    return LdmT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b11010u: {
    return BT1Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b11011u: {
    return Splitter16bit_op11011<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b11100u: {
    return BT2Decoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b11101u: {
    return Splitter32bit_op11101<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b11110u: {
    return Splitter32bit_op11110<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  case 0b11111u: {
    return Splitter32bit_op11111<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
  }
  default: {
    assert(false);
    // should not happen
    break;
  }
  }
  return InvalidInstrDecoder<TProcessorStates, TItOps, TSpecRegOps>(rinstr, pstates);
};

} // namespace internal
} // namespace microemu