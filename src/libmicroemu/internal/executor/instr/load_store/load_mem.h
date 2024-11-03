#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Loads u8 from memory and extends the result to u32
/// see Armv7-M Architecture Reference Manual Issue E.e p.252
/// see Armv7-M Architecture Reference Manual Issue E.e p.255
template <typename TInstrContext> class LoadMemU8 {
public:
  static inline Result<u32> Read(const TInstrContext &ictx, const me_adr_t &address) {
    TRY_ASSIGN(r_data, u32,
               ictx.bus.template ReadOrRaise<u8>(ictx.pstates, address,
                                                 BusExceptionType::kRaisePreciseDataBusError));
    const u32 data = Bm32::ZeroExtend<u8>(r_data);
    return Ok(data);
  }
};

/// Loads i8 from memory and extends the result to i32
/// see Armv7-M Architecture Reference Manual Issue E.e p.270
template <typename TInstrContext> class LoadMemI8 {
public:
  static inline Result<u32> Read(const TInstrContext &ictx, const me_adr_t &address) {
    TRY_ASSIGN(data, u32,
               ictx.bus.template ReadOrRaise<u8>(ictx.pstates, address,
                                                 BusExceptionType::kRaisePreciseDataBusError));
    const u32 sign_extended_data = Bm16::SignExtend<u32, 7u>(data);

    return Ok(sign_extended_data);
  }
};

/// Loads u16 from memory and extends the result to u32
/// see Armv7-M Architecture Reference Manual Issue E.e p.264
template <typename TInstrContext> class LoadMemU16 {
public:
  static inline Result<u32> Read(const TInstrContext &ictx, const me_adr_t &address) {
    TRY_ASSIGN(r_data, u32,
               ictx.bus.template ReadOrRaise<u16>(ictx.pstates, address,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    const u32 data = Bm16::ZeroExtend<u32>(r_data);
    return Ok(data);
  }
};

/// Loads i16 from memory and extends the result to u32
/// see Armv7-M Architecture Reference Manual Issue E.e p.275
template <typename TInstrContext> class LoadMemI16 {
public:
  static inline Result<u32> Read(const TInstrContext &ictx, const me_adr_t &address) {
    TRY_ASSIGN(r_data, u32,
               ictx.bus.template ReadOrRaise<u16>(ictx.pstates, address,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    const u32 sign_extended_data = Bm16::SignExtend<u32, 15u>(r_data);
    return Ok(sign_extended_data);
  }
};

/// Loads u32 from memory
/// see Armv7-M Architecture Reference Manual Issue E.e p.246
template <typename TInstrContext> class LoadMemU32 {
public:
  static inline Result<u32> Read(const TInstrContext &ictx, const me_adr_t &address) {
    TRY_ASSIGN(r_data, u32,
               ictx.bus.template ReadOrRaise<u32>(ictx.pstates, address,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    return Ok(r_data);
  }
};

/// Loads u32 (exlusive) from memory
/// see Armv7-M Architecture Reference Manual Issue E.e p.261
template <typename TInstrContext> class LoadMemExU32 {
public:
  static inline Result<u32> Read(const TInstrContext &ictx, const me_adr_t &address) {
    TRY_ASSIGN(r_data, u32,
               ictx.bus.template ReadOrRaise<u32>(ictx.pstates, address,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    return Ok(r_data);
  }
};
} // namespace internal
} // namespace microemu