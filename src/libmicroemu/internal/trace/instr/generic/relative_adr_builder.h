#pragma once

#include "libmicroemu/internal/logic/alu.h"

#include <cstdint>

namespace microemu {
namespace internal {

class RelativeAdrBuilder {
public:
  /// @brief Uses the given str builder to build a string representing the different addressing
  /// modes of instructions
  /// @tparam TStrBuilder
  /// @param builder
  /// @param is_add
  /// @param is_index
  /// @param is_wback
  /// @param n
  /// @param imm
  template <typename TStrBuilder, typename TRegOps>
  static void Build(TStrBuilder &builder, const bool &is_add, const bool &is_index,
                    const bool &is_wback, const u8 &n, const u32 &imm) {
    using Reg = TRegOps;
    assert(((is_index != false) || (is_wback != false))); // no viable option
    // TODO: Ommit imm if it is 0
    if ((is_index == true) && (is_wback == false)) { // Offset: index=TRUE, wback=FALSE
      builder.AddChar('[').AddString(Reg::GetRegisterName(n).data()).AddString(", ");
      AddImm(builder, is_add, imm);
      builder.AddChar(']');
    } else if ((is_index == false) && (is_wback == true)) { // Post indexed: index=FALSE, wback=TRUE
      builder.AddChar('[').AddString(Reg::GetRegisterName(n).data()).AddString("], ");
      AddImm(builder, is_add, imm);
    } else { // Pre indexed: index=TRUE, wback=TRUE
      builder.AddChar('[').AddString(Reg::GetRegisterName(n).data()).AddString(", ");
      AddImm(builder, is_add, imm);
      builder.AddString("]!");
    }
  }

private:
  template <typename TStrBuilder>
  static void AddImm(TStrBuilder &builder, const bool &is_add, const u32 &imm) {

    builder.AddChar('#');

    if (is_add == false) {
      builder.AddChar('-');
    }

    builder.AddUInt(imm);
  }

  /// \brief Constructs a RegIdToRegName object
  RelativeAdrBuilder() = delete;

  /// \brief Destructor
  ~RelativeAdrBuilder() = delete;

  /// \brief Copy assignment operator for RegIdToRegName.
  /// \param r_src the object to be copied
  RelativeAdrBuilder &operator=(const RelativeAdrBuilder &r_src) = delete;

  /// \brief Move constructor for RegIdToRegName.
  /// \param r_src the object to be copied
  RelativeAdrBuilder(RelativeAdrBuilder &&r_src) = delete;

  /// \brief Move assignment operator for RegIdToRegName.
  /// \param r_src the object to be copied
  RelativeAdrBuilder &operator=(RelativeAdrBuilder &&r_src) = delete;
};

} // namespace internal
} // namespace microemu