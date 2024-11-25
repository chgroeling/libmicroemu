#pragma once
#include "libmicroemu/internal/utils/traits.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace libmicroemu::internal {

class MemoryHelpers {

public:
  template <typename TCpuStates, typename TBus, typename... Args,
            typename = std::enable_if_t<(std::is_same_v<Args, uint32_t> && ...)>>
  static Result<std::array<u32, sizeof...(Args)>> ReadMemory(TCpuStates &cpua, TBus &bus,
                                                             Args... value) {
    using TElement = typename std::tuple_element<0, std::tuple<Args...>>::type;
    std::array<TElement, sizeof...(Args)> result_ma;
    auto sc = AssignReadResult(cpua, bus, result_ma, value...);
    return Result<std::array<u32, sizeof...(Args)>>(sc, result_ma);
  }

  // @brief Copies data from an memory address used by the emulator to an local array.
  // @param dest_ptr The character array to store the data.
  // @param dest_len The maximum length of the character array.
  // @param mem_acs object to access the emulated memory.
  // @param src_ptr The memory address to read the data from.
  // @param len The length of data to be read.
  // @return Result<void> The result of the write operation.
  template <typename TCpuStates, typename TBus>
  static Result<void> CpyFromEmuMem(TCpuStates &cpua, TBus &bus, char *dest_ptr,
                                    std::size_t dest_len, me_adr_t src_ptr, me_size_t src_len) {
    auto res_len = src_len <= dest_len ? src_len : dest_len;

    for (u32 i = 0U; i < res_len; ++i) {
      auto r_ch = bus.template Read<u8>(cpua, src_ptr);
      if (r_ch.IsErr()) {
        return Err<u8, void>(r_ch);
      };
      const auto &ch = r_ch.content;
      dest_ptr[i] = ch;
      src_ptr++;
    }

    return Ok();
  }

  // @brief Copies data from an local array to an memory address used by the emulator.
  template <typename TCpuStates, typename TBus>
  static Result<u32> CpyToEmuMem(TCpuStates &cpua, TBus &bus, me_adr_t dest_ptr, me_size_t dest_len,
                                 char *src_ptr, std::size_t src_len) {
    auto res_len = dest_len <= src_len ? dest_len : src_len;

    for (u32 i = 0U; i < res_len; ++i) {
      auto r_ch = bus.template Write<u8>(cpua, dest_ptr, src_ptr[i]);
      if (r_ch.IsErr()) {
        return Err<void, u32>(r_ch);
      };
      dest_ptr++;
    }

    return Ok<u32>(res_len);
  }

private:
  // ------------------

  template <std::size_t I = 0, std::size_t N, typename TCpuStates, typename TBus, typename T,
            typename... Args>
      static typename std::enable_if <
      I<(N - 1), StatusCode>::type AssignReadResult(TCpuStates &cpua, TBus &bus,
                                                    std::array<uint32_t, N> &result_ma, T value,
                                                    Args... args) {
    auto r_w1 = bus.template Read<u32>(cpua, value);

    // abort if an error occured
    if (r_w1.IsErr()) {
      return r_w1.status_code;
    }

    result_ma[I] = r_w1.content;
    auto sc = AssignReadResult<I + 1, N, TCpuStates, TBus, Args...>(cpua, bus, result_ma, args...);
    return sc;
  }

  template <std::size_t I = 0, std::size_t N, typename TCpuStates, typename TBus, typename T>
  static typename std::enable_if_t<I == (N - 1), StatusCode>
  AssignReadResult(TCpuStates &cpua, TBus &bus, std::array<uint32_t, N> &result_ma, T value) {
    auto r_w1 = bus.template Read<u32>(cpua, value);
    // abort if an error occured
    if (r_w1.IsErr()) {
      return r_w1.status_code;
    }
    result_ma[I] = r_w1.content;
    return StatusCode::kSuccess;
  }

  /**
   * @brief Constructor
   */
  MemoryHelpers() = delete;

  /**
   * @brief Destructor
   */
  ~MemoryHelpers() = delete;

  /**
   * @brief Copy constructor for MemoryHelpers.
   * @param r_src the object to be copied
   */
  MemoryHelpers(const MemoryHelpers &r_src) = delete;

  /**
   * @brief Copy assignment operator for MemoryHelpers.
   * @param r_src the object to be copied
   */
  MemoryHelpers &operator=(const MemoryHelpers &r_src) = delete;

  /**
   * @brief Move constructor for MemoryHelpers.
   * @param r_src the object to be moved
   */
  MemoryHelpers(MemoryHelpers &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MemoryHelpers.
   * @param r_src the object to be moved
   */
  MemoryHelpers &operator=(MemoryHelpers &&r_src) = delete;
};

} // namespace libmicroemu::internal