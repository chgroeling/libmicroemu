#pragma once

#include "libmicroemu/internal/decoder/op_decoders.h"
#include "libmicroemu/internal/fetcher/raw_instr.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>

namespace microemu {
namespace internal {

template <typename TProcessorStates, typename TSpecRegOps, typename TItOps>

class Decoder {
public:
  static Result<Instr> Decode(TProcessorStates &pstates, RawInstr instr) {

    // TODO: Change order of arguments
    auto result = call_decoder<TProcessorStates, TItOps, TSpecRegOps>(instr, pstates);
    return result;
  }

private:
  /// \brief Constructs a Decoder object
  Decoder() = delete;

  /// \brief Destructor
  ~Decoder() = delete;

  /// \brief Copy assignment operator for Decoder.
  /// \param r_src the object to be copied
  Decoder &operator=(const Decoder &r_src) = delete;

  /// \brief Move constructor for Decoder.
  /// \param r_src the object to be copied
  Decoder(Decoder &&r_src) = delete;

  /// \brief Move assignment operator for Decoder.
  /// \param r_src the object to be copied
  Decoder &operator=(Decoder &&r_src) = delete;
};

} // namespace internal
} // namespace microemu
