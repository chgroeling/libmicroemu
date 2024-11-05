
#include "libmicroemu/logger.h"
using namespace microemu;

void (*StaticLogger::callback_)(microemu::LogLevel level, const char *, ...) noexcept;
