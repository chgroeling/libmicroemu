#pragma once

#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace microemu {
namespace internal {

/// @brief A class to build a const string in a buffer
class ConstStringBuilder {
public:
  ConstStringBuilder(char *buf, std::size_t buf_len) : buf_(buf), buf_len_(buf_len) {}

  /// @brief Add a string to the buffer
  /// @param str The string to add
  /// @return A reference to the current object
  ConstStringBuilder &AddString(const char *str) {
    for (std::size_t i = 0U; i < strlen(str); ++i) {
      if (act_pos_ >= (buf_len_ - 1U)) { // -1 due to null termination
        return *this;
      }
      buf_[act_pos_] = str[i];
      act_pos_++;
    }
    return *this;
  }

  ConstStringBuilder &AddChar(char ch) {
    if (act_pos_ >= (buf_len_ - 1U)) { // -1 due to null termination
      return *this;
    }
    buf_[act_pos_] = ch;
    act_pos_++;
    return *this;
  }
  ConstStringBuilder &AddInt(i32 no) {
    if (act_pos_ >= (buf_len_ - 1U)) { // -1 due to null termination
      return *this;
    }
    std::size_t left_space = buf_len_ - static_cast<size_t>(act_pos_) -
                             1U; // -1 to have enough space for null termination

    act_pos_ += IntToString(&buf_[act_pos_], left_space - 1U, no);
    return *this;
  }

  ConstStringBuilder &AddUInt(u32 no) {
    if (act_pos_ >= (buf_len_ - 1U)) { // -1 due to null termination
      return *this;
    }
    std::size_t left_space = buf_len_ - static_cast<size_t>(act_pos_) -
                             1U; // -1 to have enough space for null termination

    act_pos_ += UIntToString(&buf_[act_pos_], left_space - 1U, no);
    return *this;
  }

  ConstStringBuilder &Terminate() {
    buf_[act_pos_] = '\0';
    return *this;
  }

private:
  /// @brief Convert a integer number into a const char string ans insert it
  /// into a given buffer
  u32 IntToString(char *buf, std::size_t len, i32 number) {
    std::size_t index = 0U;
    i32 is_negative = 0;

    // Überprüfen, ob die Zahl negativ ist
    if (number < 0) {
      is_negative = 1;
      number = -number;
    }

    do {
      if (index >= len - 1U) { // -1 for minus sign
        break;
      }
      buf[index++] = (char)((number % 10) + '0');
      number /= 10;
    } while (number > 0);

    // Add negative sign if needed
    if (is_negative) {
      if (index < len) {
        buf[index++] = '-';
      }
    }
    ReverseString(buf, index);
    return index;
  }

  /// @brief Convert a integer number into a const char string ans insert it
  /// into a given buffer
  u32 UIntToString(char *buf, std::size_t len, u32 number) {
    std::size_t index = 0U;

    do {
      if (index >= len) {
        break;
      }
      buf[index++] = (char)((number % 10) + '0');
      number /= 10;
    } while (number > 0);
    ReverseString(buf, index);
    return index;
  }

  void ReverseString(char *str, size_t len) {
    std::size_t start = 0;
    std::size_t end = len - 1;
    while (start < end) {
      char temp = str[start];
      str[start] = str[end];
      str[end] = temp;
      start++;
      end--;
    }
  }

  u32 act_pos_{0U};
  char *buf_;
  std::size_t buf_len_;
};

} // namespace internal
} // namespace microemu