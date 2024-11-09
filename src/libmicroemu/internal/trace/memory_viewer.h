/**
 * \file
 * \brief In this file the MemoryViewer class is defined.
 */
#pragma once

#include "libmicroemu/types.h"
#include <ctype.h>

namespace libmicroemu {
namespace internal {

template <typename TProcessorStates, typename TBus> class MemoryViewer {
public:
  // TODO: Move reference to call
  MemoryViewer(const TBus &mem) : mem_(mem) {}

  ~MemoryViewer() = default;

  /// \brief Copy constructor for MemoryViewer.
  /// \param r_src the object to be copied
  MemoryViewer(const MemoryViewer &r_src) = default;

  /// \brief Copy assignment operator for MemoryViewer.
  /// \param r_src the object to be copied
  MemoryViewer &operator=(const MemoryViewer &r_src) = delete;

  /// \brief Move constructor for ByteIterator.
  /// \param r_src the object to be copied
  MemoryViewer(MemoryViewer &&r_src) = default;

  /// \brief Move assignment operator for ByteIterator.
  /// \param r_src the object to be copied
  MemoryViewer &operator=(MemoryViewer &&r_src) = delete;

  /// \brief Prints a hex dump of the given buffer to the console

  // TODO Google Style Guide
  void print(TProcessorStates &pstates, const me_offset_t vadr, const me_size_t size,
             const u32 indent = 0U) {
    const me_adr_t vadr_end = vadr + size;
    const auto alignment = 16U;

    const me_size_t no_filler_bytes = vadr % alignment;
    const me_adr_t vadr_filler = vadr - no_filler_bytes;

    // in case of addresses which do not fit into the alignment print
    // out the address and filler spaces
    if (no_filler_bytes > 0U) {

      print_section(pstates, vadr_filler, vadr_filler + no_filler_bytes, alignment, indent, true);
    }

    print_section(pstates, vadr, vadr_end, alignment, indent, false);
    printf("\n");
  }

private:
  const TBus &mem_;

  void print_section(TProcessorStates &pstates, const me_adr_t vadr_begin, const me_adr_t vadr_end,
                     const u32 alignment, u32 indent, bool skip_read) {

    for (me_adr_t ivadr = vadr_begin; ivadr < vadr_end; ivadr++) {
      // Print address first
      if (ivadr % alignment == 0U) {
        for (u32 spaces = 0U; spaces < indent; ++spaces) {
          printf(" ");
        }
        printf("%08x|", ivadr);
      }

      // Then the byte
      if (!skip_read) {
        auto res = mem_.template Read<u8>(pstates, ivadr);
        if (res.IsErr()) {
          printf("xx");
        } else {
          printf("%02x", res.content);
        }
      } else {
        printf("  ");
      }

      // Following by an delimiter
      printf(" ");

      // After x bytes add a newline
      // ... special condition do not add newline at the end
      if ((ivadr % alignment == alignment - 1) && (ivadr != vadr_end - 1)) {
        printf("\n");
      }
    }
  }
};
} // namespace internal
} // namespace libmicroemu