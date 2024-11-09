#pragma once
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>

namespace libmicroemu {

enum class LogLevel { kTrace, kDebug, kInfo, kWarn, kError, kCritical };

class StaticLogger {

public:
  template <typename... Args> static void Info(const char *format, Args &&...args) noexcept {
    if (callback_) {
      callback_(LogLevel::kInfo, format, std::forward<Args>(args)...);
    }
  }
  template <typename... Args> static void Debug(const char *format, Args &&...args) noexcept {
    if (callback_) {
      callback_(LogLevel::kDebug, format, std::forward<Args>(args)...);
    }
  }
  template <typename... Args> static void Trace(const char *format, Args &&...args) noexcept {
    if (callback_) {
      callback_(LogLevel::kTrace, format, std::forward<Args>(args)...);
    }
  }
  template <typename... Args> static void Warn(const char *format, Args &&...args) noexcept {
    if (callback_) {
      callback_(LogLevel::kWarn, format, std::forward<Args>(args)...);
    }
  }
  template <typename... Args> static void Error(const char *format, Args &&...args) noexcept {
    if (callback_) {
      callback_(LogLevel::kError, format, std::forward<Args>(args)...);
    }
  }
  template <typename... Args> static void Critical(const char *format, Args &&...args) noexcept {
    if (callback_) {
      callback_(LogLevel::kCritical, format, std::forward<Args>(args)...);
    }
  }

  static void RegisterLoggerCallback(void (*callback)(libmicroemu::LogLevel level, const char *,
                                                      ...) noexcept) noexcept {
    callback_ = callback;
  };

private:
  static void (*callback_)(libmicroemu::LogLevel level, const char *, ...) noexcept;
};
class NullLogger {
public:
  template <typename... Args> static void Info(const char *format, Args &&...args) noexcept {
    static_cast<void>(format);
    (void)std::initializer_list<int>{(static_cast<void>(args), 0)...}; // "Verbraucht" alle Args
  }
  template <typename... Args> static void Debug(const char *format, Args &&...args) noexcept {
    static_cast<void>(format);
    (void)std::initializer_list<int>{(static_cast<void>(args), 0)...};
  }
  template <typename... Args> static void Trace(const char *format, Args &&...args) noexcept {
    static_cast<void>(format);
    (void)std::initializer_list<int>{(static_cast<void>(args), 0)...};
  }
  template <typename... Args> static void Warn(const char *format, Args &&...args) noexcept {
    static_cast<void>(format);
    (void)std::initializer_list<int>{(static_cast<void>(args), 0)...};
  }
  template <typename... Args> static void Error(const char *format, Args &&...args) noexcept {
    static_cast<void>(format);
    (void)std::initializer_list<int>{(static_cast<void>(args), 0)...};
  }
  template <typename... Args> static void Critical(const char *format, Args &&...args) noexcept {
    static_cast<void>(format);
    (void)std::initializer_list<int>{(static_cast<void>(args), 0)...};
  }
};

}; // namespace libmicroemu

// ---------------------
// LOGGER MACROS
// ---------------------
#if defined(LOGLEVEL_TRACE)
#undef LOGLEVEL_TRACE
#define LOGLEVEL_TRACE 1
#define LOGLEVEL_DEBUG 1
#define LOGLEVEL_INFO 1
#define LOGLEVEL_WARN 1
#define LOGLEVEL_ERROR 1
#define LOGLEVEL_CRITICAL 1

#elif defined(LOGLEVEL_DEBUG)
#undef LOGLEVEL_DEBUG
#define LOGLEVEL_TRACE 0
#define LOGLEVEL_DEBUG 1
#define LOGLEVEL_INFO 1
#define LOGLEVEL_WARN 1
#define LOGLEVEL_ERROR 1
#define LOGLEVEL_CRITICAL 1

#elif defined(LOGLEVEL_INFO)
#undef LOGLEVEL_INFO
#define LOGLEVEL_TRACE 0
#define LOGLEVEL_DEBUG 0
#define LOGLEVEL_INFO 1
#define LOGLEVEL_WARN 1
#define LOGLEVEL_ERROR 1
#define LOGLEVEL_CRITICAL 1
#elif defined(LOGLEVEL_WARN)
#undef LOGLEVEL_WARN
#define LOGLEVEL_TRACE 0
#define LOGLEVEL_DEBUG 0
#define LOGLEVEL_INFO 0
#define LOGLEVEL_WARN 1
#define LOGLEVEL_ERROR 1
#define LOGLEVEL_CRITICAL 1
#elif defined(LOGLEVEL_ERROR)
#undef LOGLEVEL_ERROR
#define LOGLEVEL_TRACE 0
#define LOGLEVEL_DEBUG 0
#define LOGLEVEL_INFO 0
#define LOGLEVEL_WARN 0
#define LOGLEVEL_ERROR 1
#define LOGLEVEL_CRITICAL 1
#else
#define LOGLEVEL_TRACE 0
#define LOGLEVEL_DEBUG 0
#define LOGLEVEL_INFO 0
#define LOGLEVEL_WARN 0
#define LOGLEVEL_ERROR 0
#define LOGLEVEL_CRITICAL 0
#endif

#if LOGLEVEL_TRACE == 1
#define LOG_TRACE(CTX, ...) CTX::Trace(__VA_ARGS__)
#define IS_LOGLEVEL_TRACE_ENABLED true
#else
#define LOG_TRACE(CTX, ...)
#define IS_LOGLEVEL_TRACE_ENABLED false
#endif

#if LOGLEVEL_DEBUG == 1
#define LOG_DEBUG(CTX, ...) CTX::Debug(__VA_ARGS__)
#define IS_LOGLEVEL_DEBUG_ENABLED true
#else
#define LOG_DEBUG(CTX, ...)
#define IS_LOGLEVEL_DEBUG_ENABLED false
#endif

#if LOGLEVEL_INFO == 1
#define LOG_INFO(CTX, ...) CTX::Info(__VA_ARGS__)
#define IS_LOGLEVEL_INFO_ENABLED true
#else
#define LOG_INFO(CTX, ...)
#define IS_LOGLEVEL_INFO_ENABLED false
#endif

#if LOGLEVEL_WARN == 1
#define LOG_WARN(CTX, ...) CTX::Warn(__VA_ARGS__)
#define IS_LOGLEVEL_WARN_ENABLED true
#else
#define LOG_WARN(CTX, ...)
#define IS_LOGLEVEL_WARN_ENABLED false
#endif

#if LOGLEVEL_ERROR == 1
#define LOG_ERROR(CTX, ...) CTX::Error(__VA_ARGS__)
#define IS_LOGLEVEL_ERROR_ENABLED true
#else
#define LOG_ERROR(CTX, ...)
#define IS_LOGLEVEL_ERROR_ENABLED false
#endif

#if LOGLEVEL_CRITICAL == 1
#define LOG_CRITICAL(CTX, ...) CTX::Critical(__VA_ARGS__)
#define IS_LOGLEVEL_CRITICAL_ENABLED true
#else
#define LOG_CRITICAL(CTX, ...)
#define IS_LOGLEVEL_CRITICAL_ENABLED false
#endif
