#pragma once
#include "libmicroemu/internal/logic/alu.h"
#include "libmicroemu/internal/logic/thumb_immediate_result.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/types.h"
#include <assert.h>
#include <cstddef>
#include <cstdint>

namespace libmicroemu::internal {

class Thumb {
public:
  static Result<u32> ThumbExpandImm(u32 imm12) {
    // APSR.C argument to following function call does not affect the imm32
    // therefore fake it
    const bool apsr_c_dummy = false;
    const auto result = ThumbExpandImm_C(imm12, apsr_c_dummy);
    return Result<u32>(result.status_code, result.content.out);
  }

  static Result<ThumbImmediateResult> ThumbExpandImm_C(const uint32_t &in, const bool carry_in) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.139 - 140
    u32 imm32{0U};
    bool carry_out{false};

    if (Bm32::ExtractBits1R<11U, 10U>(in) == 0x0U) {
      switch (Bm32::ExtractBits1R<9U, 8U>(in)) {

      case 0x0U: {
        imm32 = Bm32::ZeroExtend<u32>(in & 0xFFU);
        break;
      }
      case 0x1U: {
        if ((in & 0xFFU) == 0x0U) {
          return Err<ThumbImmediateResult>(StatusCode::kExecutorUnpredictable);
        }
        imm32 = ((in & 0xFFU) << 16U) | (in & 0xFFU);
        break;
      }
      case 0x2U: {
        if ((in & 0xFFU) == 0x0U) {
          return Err<ThumbImmediateResult>(StatusCode::kExecutorUnpredictable);
        }
        imm32 = ((in & 0xFFU) << 24U) | ((in & 0xFFU) << 8U);
        break;
      }
      case 0x3U: {
        if ((in & 0xFFU) == 0x0U) {
          return Err<ThumbImmediateResult>(StatusCode::kExecutorUnpredictable);
        }
        imm32 = ((in & 0xFFU) << 24U) | ((in & 0xFFU) << 16U) | ((in & 0xFFU) << 8U) |
                ((in & 0xFFU) << 0U);
        break;
      }
      default:
        // should not happen
        assert(false);
        break;
      }
      carry_out = carry_in;
    } else {
      const u32 unrotated_value = Bm32::ZeroExtend<u32>(0x80U | (in & 0x7FU));
      const auto r_ror = Alu32::ROR_C(unrotated_value, Bm32::ExtractBits1R<11U, 7U>(in));

      imm32 = r_ror.result;
      carry_out = r_ror.carry_out;
    }
    return Ok(ThumbImmediateResult{imm32, carry_out});
  }

private:
  /**
   * @brief Constructor
   */
  Thumb() = delete;

  /**
   * @brief Destructor
   */
  ~Thumb() = delete;

  /**
   * @brief Copy constructor for Thumb.
   * @param r_src the object to be copied
   */
  Thumb(const Thumb &r_src) = delete;

  /**
   * @brief Copy assignment operator for Thumb.
   * @param r_src the object to be copied
   */
  Thumb &operator=(const Thumb &r_src) = delete;

  /**
   * @brief Move constructor for Thumb.
   * @param r_src the object to be moved
   */
  Thumb(Thumb &&r_src) = delete;

  /**
   * @brief Move assignment operator for  Thumb.
   * @param r_src the object to be moved
   */
  Thumb &operator=(Thumb &&r_src) = delete;
};

} // namespace libmicroemu::internal