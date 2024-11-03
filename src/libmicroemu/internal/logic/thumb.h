#pragma once
#include "libmicroemu/internal/logic/alu.h"
#include "libmicroemu/internal/logic/thumb_immediate_result.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"
#include <assert.h>
#include <cstddef>
#include <cstdint>

namespace microemu {
namespace internal {

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
    u32 imm32{0u};
    bool carry_out{false};

    if (Bm32::Slice1R<11u, 10u>(in) == 0x0u) {
      switch (Bm32::Slice1R<9u, 8u>(in)) {

      case 0x0u: {
        imm32 = Bm32::ZeroExtend<u32>(in & 0xFFu);
        break;
      }
      case 0x1u: {
        if ((in & 0xFFu) == 0x0u) {
          return Err<ThumbImmediateResult>(StatusCode::kScExecutorUnpredictable);
        }
        imm32 = ((in & 0xFFu) << 16u) | (in & 0xFFu);
        break;
      }
      case 0x2u: {
        if ((in & 0xFFu) == 0x0u) {
          return Err<ThumbImmediateResult>(StatusCode::kScExecutorUnpredictable);
        }
        imm32 = ((in & 0xFFu) << 24u) | ((in & 0xFFu) << 8u);
        break;
      }
      case 0x3u: {
        if ((in & 0xFFu) == 0x0u) {
          return Err<ThumbImmediateResult>(StatusCode::kScExecutorUnpredictable);
        }
        imm32 = ((in & 0xFFu) << 24u) | ((in & 0xFFu) << 16u) | ((in & 0xFFu) << 8u) |
                ((in & 0xFFu) << 0u);
        break;
      }
      default:
        // should not happen
        assert(false);
        break;
      }
      carry_out = carry_in;
    } else {
      const u32 unrotated_value = Bm32::ZeroExtend<u32>(0x80u | (in & 0x7Fu));
      const auto r_ror = Alu32::ROR_C(unrotated_value, Bm32::Slice1R<11u, 7u>(in));

      imm32 = r_ror.result;
      carry_out = r_ror.carry_out;
    }
    return Ok(ThumbImmediateResult{imm32, carry_out});
  }

private:
  /// \brief Constructor
  Thumb() = delete;

  /// \brief Destructor
  ~Thumb() = delete;

  /// \brief Copy constructor for Thumb.
  /// \param r_src the object to be copied
  Thumb(const Thumb &r_src) = delete;

  /// \brief Copy assignment operator for Thumb.
  /// \param r_src the object to be copied
  Thumb &operator=(const Thumb &r_src) = delete;

  /// \brief Move constructor for Thumb.
  /// \param r_src the object to be copied
  Thumb(Thumb &&r_src) = delete;

  /// \brief Move assignment operator for Thumb.
  /// \param r_src the object to be copied
  Thumb &operator=(Thumb &&r_src) = delete;
};

} // namespace internal
} // namespace microemu