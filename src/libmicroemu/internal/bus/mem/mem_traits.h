#pragma once

// Helper type for SFINAE
// Empty struct to be used as a placeholder for the second template parameter
template <typename...> using void_t = void;

template <typename T, typename = void> struct has_kRegisters : std::false_type {};

// Specialization if T::kRegisters is valid
template <typename T> struct has_kRegisters<T, void_t<decltype(T::kRegisters)>> : std::true_type {};

// Helper variable for simpler usage
template <typename T> constexpr bool has_kRegisters_v = has_kRegisters<T>::value;
