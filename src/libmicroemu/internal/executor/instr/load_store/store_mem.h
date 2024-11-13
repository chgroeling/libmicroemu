#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Store u8 to memory
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.389
 */
template <typename TInstrContext> class StoreMemU8 {
public:
  static inline Result<void> Write(const TInstrContext &ictx, const u32 &address, const u32 &rt) {
    const auto &rt8 = static_cast<u8>(rt & 0xFFU);
    TRY(void, ictx.bus.template WriteOrRaise<u8>(ictx.pstates, address, rt8,
                                                 BusExceptionType::kRaisePreciseDataBusError));
    return Ok();
  }
};

/**
 * @brief Store u16 to memory
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.389
 */
template <typename TInstrContext> class StoreMemU16 {
public:
  static inline Result<void> Write(const TInstrContext &ictx, const u32 &address, const u32 &rt) {
    const auto &rt16 = static_cast<u16>(rt & 0xFFFFU);
    TRY(void, ictx.bus.template WriteOrRaise<u16>(ictx.pstates, address, rt16,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    return Ok();
  }
};

/**
 * @brief Store u32 to memory
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.386
 */
template <typename TInstrContext> class StoreMemU32 {
public:
  static inline Result<void> Write(const TInstrContext &ictx, const u32 &address, const u32 &rt) {
    TRY(void, ictx.bus.template WriteOrRaise<u32>(ictx.pstates, address, rt,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    return Ok();
  }
};

/**
 * @brief Store (exclusive) u32 to memory
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.394
 */
template <typename TInstrContext> class StoreMemExU32 {
public:
  static inline Result<void> Write(const TInstrContext &ictx, const u32 &address, const u32 &rt,
                                   u32 &rd) {
    TRY(void, ictx.bus.template WriteOrRaise<u32>(ictx.pstates, address, rt,
                                                  BusExceptionType::kRaisePreciseDataBusError));
    rd = 0x0U; // operation always succeeds ... updates memory
    return Ok();
  }
};

} // namespace internal
} // namespace libmicroemu